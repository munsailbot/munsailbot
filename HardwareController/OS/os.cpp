/**
 * @file os.c
 *
 * @brief A Real Time Operating System
 *
 * Our implementation of the operating system described by Mantis Cheng in os.h.
 *
 * @author Scott Craig
 * @author Justin Tanner
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "os.h"
#include "kernel.h"
#include "error_code.h"

/* Needed for memset */
/* #include <string.h> */

/** @brief main function provided by user application. The first task to run. */
extern int r_main();

/** PPP and PT defined in user application. */
extern const unsigned char PPP[];

/** PPP and PT defined in user application. */
extern const unsigned int PT;

/** The task descriptor of the currently RUNNING task. */
static task_descriptor_t* cur_task = NULL;

/** Since this is a "full-served" model, the kernel is executing using its own stack. */
static volatile uint16_t kernel_sp;

/** This table contains all task descriptors, regardless of state, plus idler. */
static task_descriptor_t task_desc[MAXPROCESS + 1];

/** The special "idle task" at the end of the descriptors array. */
static task_descriptor_t* idle_task = &task_desc[MAXPROCESS];

/** The current kernel request. */
static volatile kernel_request_t kernel_request;

/** Arguments for Task_Create() request. */
static volatile create_args_t kernel_request_create_args;

/** Return value for Task_Create() request. */
static volatile int kernel_request_retval;

/** Argument and return value for Event class of requests. */
static volatile EVENT* kernel_request_event_ptr;

/** Number of tasks created so far */
static queue_t dead_pool_queue;

/** Number of events created so far */
static uint8_t num_events_created = 0;

/** The ready queue for RR tasks. Their scheduling is round-robin. */
static queue_t rr_queue;

/** The ready queue for SYSTEM tasks. Their scheduling is first come, first served. */
static queue_t system_queue;

/** An array of queues for tasks waiting on events. */
static queue_t event_queue[MAXEVENT];

/** time remaining in current slot */
static volatile uint8_t ticks_remaining = 0;

/** Indicates if periodic task in this slot has already run this time */
static uint8_t slot_task_finished = 0;

/** Index of name of task in current slot in PPP array. An even number from 0 to 2*(PT-1). */
static unsigned int slot_name_index = 0;

/** The task descriptor for index "name of task" */
static task_descriptor_t* name_to_task_ptr[MAXNAME + 1];

/** The names that appear in PPP */
static uint8_t name_in_PPP[MAXNAME + 1];

/** Error message used in OS_Abort() */
static uint8_t volatile error_msg = ERR_RUN_1_USER_CALLED_OS_ABORT;


/* Forward declarations */
/* kernel */
static void kernel_main_loop(void);
static void kernel_dispatch(void);
static void kernel_handle_request(void);
/* context switching */
static void exit_kernel(void) __attribute((noinline, naked));
static void enter_kernel(void) __attribute((noinline, naked));
extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal, naked));

static int kernel_create_task();
static void kernel_terminate_task(void);
/* events */
static void kernel_event_wait(void);
static void kernel_event_signal(uint8_t is_broadcast, uint8_t and_next);
/* queues */

static void enqueue(queue_t* queue_ptr, task_descriptor_t* task_to_add);
static task_descriptor_t* dequeue(queue_t* queue_ptr);

static void kernel_update_ticker(void);
static void check_PPP_names(void);
static void idle (void);
static void _delay_25ms(void);

/*
 * FUNCTIONS
 */
/**
 *  @brief The idle task does nothing but busy loop.
 */
static void idle (void)
{
    for(;;)
    {};
}


/**
 * @fn kernel_main_loop
 *
 * @brief The heart of the RTOS, the main loop where the kernel is entered and exited.
 *
 * The complete function is:
 *
 *  Loop
 *<ol><li>Select and dispatch a process to run</li>
 *<li>Exit the kernel (The loop is left and re-entered here.)</li>
 *<li>Handle the request from the process that was running.</li>
 *<li>End loop, go to 1.</li>
 *</ol>
 */
static void kernel_main_loop(void)
{
    for(;;)
    {
        kernel_dispatch();

        exit_kernel();

        /* if this task makes a system call, or is interrupted,
         * the thread of control will return to here. */

        kernel_handle_request();
    }
}


/**
 * @fn kernel_dispatch
 *
 *@brief The second part of the scheduler.
 *
 * Chooses the next task to run.
 *
 */
static void kernel_dispatch(void)
{
    /* If the current state is RUNNING, then select it to run again.
     * kernel_handle_request() has already determined it should be selected.
     */

    if(cur_task->state != RUNNING || cur_task == idle_task)
    {
		if(system_queue.head != NULL)
        {
            cur_task = dequeue(&system_queue);
        }
        else if(!slot_task_finished && PT > 0 && name_to_task_ptr[PPP[slot_name_index]] != NULL)
        {
            /* Keep running the current PERIODIC task. */
            cur_task = name_to_task_ptr[PPP[slot_name_index]];
        }
        else if(rr_queue.head != NULL)
        {
            cur_task = dequeue(&rr_queue);
        }
        else
        {
            /* No task available, so idle. */
            cur_task = idle_task;
        }

        cur_task->state = RUNNING;
    }
}


/**
 * @fn kernel_handle_request
 *
 *@brief The first part of the scheduler.
 *
 * Perform some action based on the system call or timer tick.
 * Perhaps place the current process in a ready or waitng queue.
 */
static void kernel_handle_request(void)
{
   switch(kernel_request)
    {
    case NONE:
        /* Should not happen. */
        break;

    case TIMER_EXPIRED:
        kernel_update_ticker();

        /* Round robin tasks get pre-empted on every tick. */
        if(cur_task->level == RR && cur_task->state == RUNNING)
        {
            cur_task->state = READY;
            enqueue(&rr_queue, cur_task);
        }
        break;

    case TASK_CREATE:
        kernel_request_retval = kernel_create_task();

        /* Check if new task has higer priority, and that it wasn't an ISR
         * making the request.
         */
        if(kernel_request_retval)
        {
            /* If new task is SYSTEM and cur is not, then don't run old one */
            if(kernel_request_create_args.level == SYSTEM && cur_task->level != SYSTEM)
            {
                cur_task->state = READY;
            }

            /* If cur is RR, it might be pre-empted by a new PERIODIC. */
            if(cur_task->level == RR &&
               kernel_request_create_args.level == PERIODIC &&
               PPP[slot_name_index] == kernel_request_create_args.name)
            {
                cur_task->state = READY;
            }

            /* enqueue READY RR tasks. */
            if(cur_task->level == RR && cur_task->state == READY)
            {
                enqueue(&rr_queue, cur_task);
            }
        }
        break;

    case TASK_TERMINATE:
		if(cur_task != idle_task)
		{
        	kernel_terminate_task();
		}
        break;

    case TASK_NEXT:
		switch(cur_task->level)
		{
	    case SYSTEM:
	        enqueue(&system_queue, cur_task);
			break;

	    case PERIODIC:
	        slot_task_finished = 1;
	        break;

	    case RR:
	        enqueue(&rr_queue, cur_task);
	        break;

	    default: /* idle_task */
			break;
		}

		cur_task->state = READY;
        break;

    case TASK_GET_ARG:
        /* Should not happen. Handled in task itself. */
        break;

    case EVENT_INIT:
        kernel_request_event_ptr = NULL;
        if(num_events_created < MAXEVENT)
        {
            /* Pass a number back to the task, but pretend it is a pointer.
             * It is the index of the event_queue plus 1.
             * (0 is return value for failure.)
             */
            kernel_request_event_ptr = (EVENT *)(uint16_t)(num_events_created + 1);
            /*
            event_queue[num_events_created].head = NULL;
            event_queue[num_events_created].tail = NULL;
            */
            ++num_events_created;
        }
        else
        {
            kernel_request_event_ptr = (EVENT *)(uint16_t)0;
        }
        break;

    case EVENT_WAIT:
        /* idle_task does not wait. */
		if(cur_task != idle_task)
        {
            kernel_event_wait();
        }

        break;

    case EVENT_SIGNAL:
        kernel_event_signal(0 /* not broadcast */, 0 /* not task_next */);
        break;

    case EVENT_BROADCAST:
        kernel_event_signal(1 /* is broadcast */, 0 /* not task_next */);
        break;

    case EVENT_SIGNAL_AND_NEXT:
        if(cur_task->level == PERIODIC)
        {
            slot_task_finished = 1;
        }

        kernel_event_signal(0 /* not broadcast */, 1 /* is task_next */);

        break;

    case EVENT_BROADCAST_AND_NEXT:
        if(cur_task->level == PERIODIC)
        {
            slot_task_finished = 1;
        }

        kernel_event_signal(1 /* is broadcast */, 1 /* is task_next */);
        break;

    default:
        /* Should never happen */
        error_msg = ERR_RUN_8_RTOS_INTERNAL_ERROR;
        OS_Abort();
        break;
    }

    kernel_request = NONE;
}


/*
 * Context switching
 */
/**
 * It is important to keep the order of context saving and restoring exactly
 * in reverse. Also, when a new task is created, it is important to
 * initialize its "initial" context in the same order as a saved context.
 *
 * Save r31 and SREG on stack, disable interrupts, then save
 * the rest of the registers on the stack. In the locations this macro
 * is used, the interrupts need to be disabled, or they already are disabled.
 */
#define    SAVE_CTX_TOP()       asm volatile (\
    "push   r31             \n\t"\
    "in     r31,__SREG__    \n\t"\
    "cli                    \n\t"::); /* Disable interrupt */

#define STACK_SREG_SET_I_BIT()    asm volatile (\
    "ori    r31, 0x80        \n\t"::);

#define    SAVE_CTX_BOTTOM()       asm volatile (\
    "push   r31             \n\t"\
    "push   r30             \n\t"\
    "push   r29             \n\t"\
    "push   r28             \n\t"\
    "push   r27             \n\t"\
    "push   r26             \n\t"\
    "push   r25             \n\t"\
    "push   r24             \n\t"\
    "push   r23             \n\t"\
    "push   r22             \n\t"\
    "push   r21             \n\t"\
    "push   r20             \n\t"\
    "push   r19             \n\t"\
    "push   r18             \n\t"\
    "push   r17             \n\t"\
    "push   r16             \n\t"\
    "push   r15             \n\t"\
    "push   r14             \n\t"\
    "push   r13             \n\t"\
    "push   r12             \n\t"\
    "push   r11             \n\t"\
    "push   r10             \n\t"\
    "push   r9              \n\t"\
    "push   r8              \n\t"\
    "push   r7              \n\t"\
    "push   r6              \n\t"\
    "push   r5              \n\t"\
    "push   r4              \n\t"\
    "push   r3              \n\t"\
    "push   r2              \n\t"\
    "push   r1              \n\t"\
    "push   r0              \n\t"::);

/**
 * @brief Push all the registers and SREG onto the stack.
 */
#define    SAVE_CTX()    SAVE_CTX_TOP();SAVE_CTX_BOTTOM();

/**
 * @brief Pop all registers and the status register.
 */
#define    RESTORE_CTX()    asm volatile (\
    "pop    r0                \n\t"\
    "pop    r1                \n\t"\
    "pop    r2                \n\t"\
    "pop    r3                \n\t"\
    "pop    r4                \n\t"\
    "pop    r5                \n\t"\
    "pop    r6                \n\t"\
    "pop    r7                \n\t"\
    "pop    r8                \n\t"\
    "pop    r9                \n\t"\
    "pop    r10             \n\t"\
    "pop    r11             \n\t"\
    "pop    r12             \n\t"\
    "pop    r13             \n\t"\
    "pop    r14             \n\t"\
    "pop    r15             \n\t"\
    "pop    r16             \n\t"\
    "pop    r17             \n\t"\
    "pop    r18             \n\t"\
    "pop    r19             \n\t"\
    "pop    r20             \n\t"\
    "pop    r21             \n\t"\
    "pop    r22             \n\t"\
    "pop    r23             \n\t"\
    "pop    r24             \n\t"\
    "pop    r25             \n\t"\
    "pop    r26             \n\t"\
    "pop    r27             \n\t"\
    "pop    r28             \n\t"\
    "pop    r29             \n\t"\
    "pop    r30             \n\t"\
    "pop    r31             \n\t"\
	"out    __SREG__, r31    \n\t"\
    "pop    r31             \n\t"::);


/**
 * @fn exit_kernel
 *
 * @brief The actual context switching code begins here.
 *
 * This function is called by the kernel. Upon entry, we are using
 * the kernel stack, on top of which is the address of the instruction
 * after the call to exit_kernel().
 *
 * Assumption: Our kernel is executed with interrupts already disabled.
 *
 * The "naked" attribute prevents the compiler from adding instructions
 * to save and restore register values. It also prevents an
 * automatic return instruction.
 */
static void exit_kernel(void)
{
    /*
     * The PC was pushed on the stack with the call to this function.
     * Now push on the I/O registers and the SREG as well.
     */
     SAVE_CTX();

    /*
     * The last piece of the context is the SP. Save it to a variable.
     */
    kernel_sp = SP;

    /*
     * Now restore the task's context, SP first.
     */
    SP = (uint16_t)(cur_task->sp);

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * return explicitly required as we are "naked".
     * Interrupts are enabled or disabled according to SREG
     * recovered from stack, so we don't want to explicitly
     * enable them here.
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/**
 * @fn enter_kernel
 *
 * @brief All system calls eventually enter here.
 *
 * Assumption: We are still executing on cur_task's stack.
 * The return address of the caller of enter_kernel() is on the
 * top of the stack.
 */
static void enter_kernel(void)
{
    /*
     * The PC was pushed on the stack with the call to this function.
     * Now push on the I/O registers and the SREG as well.
     */
    SAVE_CTX();

    /*
     * The last piece of the context is the SP. Save it to a variable.
     */
    cur_task->sp = (uint8_t*)SP;

    /*
     * Now restore the kernel's context, SP first.
     */
    SP = kernel_sp;

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * return explicitly required as we are "naked".
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/**
 * @fn TIMER1_COMPA_vect
 *
 * @brief The interrupt handler for output compare interrupts on Timer 1
 *
 * Used to enter the kernel when a tick expires.
 *
 * Assumption: We are still executing on the cur_task stack.
 * The return address inside the current task code is on the top of the stack.
 *
 * The "naked" attribute prevents the compiler from adding instructions
 * to save and restore register values. It also prevents an
 * automatic return instruction.
 */
void TIMER1_COMPA_vect(void)
{
	//PORTB ^= _BV(PB7);		// Arduino LED
    /*
     * Save the interrupted task's context on its stack,
     * and save the stack pointer.
     *
     * On the cur_task's stack, the registers and SREG are
     * saved in the right order, but we have to modify the stored value
     * of SREG. We know it should have interrupts enabled because this
     * ISR was able to execute, but it has interrupts disabled because
     * it was stored while this ISR was executing. So we set the bit (I = bit 7)
     * in the stored value.
     */
    SAVE_CTX_TOP();

    STACK_SREG_SET_I_BIT();

    SAVE_CTX_BOTTOM();

    cur_task->sp = (uint8_t*)SP;

    /*
     * Now that we already saved a copy of the stack pointer
     * for every context including the kernel, we can move to
     * the kernel stack and use it. We will restore it again later.
     */
    SP = kernel_sp;

    /*
     * Inform the kernel that this task was interrupted.
     */
    kernel_request = TIMER_EXPIRED;

    /*
     * Prepare for next tick interrupt.
     */
    OCR1A += TICK_CYCLES;

    /*
     * Restore the kernel context. (The stack pointer is restored again.)
     */
    SP = kernel_sp;

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * We use "ret" here, not "reti", because we do not want to
     * enable interrupts inside the kernel.
     * Explilictly required as we are "naked".
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/*
 * Tasks Functions
 */
/**
 *  @brief Kernel function to create a new task.
 *
 * When creating a new task, it is important to initialize its stack just like
 * it has called "enter_kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * @sa enter_kernel
 */
static int kernel_create_task()
{
    /* The new task. */
    task_descriptor_t *p;
    uint8_t* stack_bottom;


    if (dead_pool_queue.head == NULL)
    {
        /* Too many tasks! */
        return 0;
    }

    if(kernel_request_create_args.level == PERIODIC &&
        (kernel_request_create_args.name == IDLE ||
         kernel_request_create_args.name > MAXNAME))
    {
        /* PERIODIC name is out of range [1 .. MAXNAME] */
        error_msg = ERR_2_CREATE_NAME_OUT_OF_RANGE;
        OS_Abort();
    }

    if(kernel_request_create_args.level == PERIODIC &&
        name_in_PPP[kernel_request_create_args.name] == 0)
    {
        error_msg = ERR_5_NAME_NOT_IN_PPP;
        OS_Abort();
    }

    if(kernel_request_create_args.level == PERIODIC &&
    name_to_task_ptr[kernel_request_create_args.name] != NULL)
    {
        /* PERIODIC name already used */
        error_msg = ERR_4_PERIODIC_NAME_IN_USE;
        OS_Abort();
    }

	/* idling "task" goes in last descriptor. */
	if(kernel_request_create_args.level == NULL)
	{
		p = &task_desc[MAXPROCESS];
	}
	/* Find an unused descriptor. */
	else
	{
	    p = dequeue(&dead_pool_queue);
	}

    stack_bottom = &(p->stack[WORKSPACE-1]);

    /* The stack grows down in memory, so the stack pointer is going to end up
     * pointing to the location 32 + 1 + 2 + 2 = 37 bytes above the bottom, to make
     * room for (from bottom to top):
     *   the address of Task_Terminate() to destroy the task if it ever returns,
     *   the address of the start of the task to "return" to the first time it runs,
     *   register 31,
     *   the stored SREG, and
     *   registers 30 to 0.
     */
    uint8_t* stack_top = stack_bottom - (32 + 1 + 2 + 2);

    /* Not necessary to clear the task descriptor. */
    /* memset(p,0,sizeof(task_descriptor_t)); */

    /* stack_top[0] is the byte above the stack.
     * stack_top[1] is r0. */
    stack_top[2] = (uint8_t) 0; /* r1 is the "zero" register. */
    /* stack_top[31] is r30. */
    stack_top[32] = (uint8_t) _BV(SREG_I); /* set SREG_I bit in stored SREG. */
    /* stack_top[33] is r31. */

    /* We are placing the address (16-bit) of the functions
     * onto the stack in reverse byte order (least significant first, followed
     * by most significant).  This is because the "return" assembly instructions
     * (ret and reti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
     * second), even though the AT90 is LITTLE ENDIAN machine.
     */
    stack_top[34] = (uint8_t)((uint16_t)(kernel_request_create_args.f) >> 8);
    stack_top[35] = (uint8_t)(uint16_t)(kernel_request_create_args.f);
    stack_top[36] = (uint8_t)((uint16_t)Task_Terminate >> 8);
    stack_top[37] = (uint8_t)(uint16_t)Task_Terminate;

    /*
     * Make stack pointer point to cell above stack (the top).
     * Make room for 32 registers, SREG and two return addresses.
     */
    p->sp = stack_top;

    p->state = READY;
    p->arg = kernel_request_create_args.arg;
    p->level = kernel_request_create_args.level;
    p->name = kernel_request_create_args.name;

	switch(kernel_request_create_args.level)
	{
	case PERIODIC:
		/* Put this newly created PPP task into the PPP lookup array */
        name_to_task_ptr[kernel_request_create_args.name] = p;
		break;

    case SYSTEM:
    	/* Put SYSTEM and Round Robin tasks on a queue. */
        enqueue(&system_queue, p);
		break;

    case RR:
		/* Put SYSTEM and Round Robin tasks on a queue. */
        enqueue(&rr_queue, p);
		break;

	default:
		/* idle task does not go in a queue */
		break;
	}


    return 1;
}


/**
 * @brief Kernel function to destroy the current task.
 */
static void kernel_terminate_task(void)
{
    /* deallocate all resources used by this task */
    cur_task->state = DEAD;
    if(cur_task->level == PERIODIC)
    {
        name_to_task_ptr[cur_task->name] = NULL;
    }
    enqueue(&dead_pool_queue, cur_task);
}


/**
 * @brief Kernel function to place current task in a waiting queue.
 */
static void kernel_event_wait(void)
{
    /* Check the handle of the event to ensure that it is initialized. */
    uint8_t handle = (uint8_t)((uint16_t)(kernel_request_event_ptr) - 1);

    if(handle >= num_events_created)
    {
        /* Error code. */
        error_msg = ERR_RUN_5_WAIT_ON_BAD_EVENT;
        OS_Abort();
    }
    else if(cur_task->level == PERIODIC)
	{
		error_msg = ERR_RUN_7_PERIODIC_CALLED_WAIT;
		OS_Abort();
	}
	else
    {
        /* Place this task in a queue. */
        cur_task->state = WAITING;
        enqueue(&event_queue[handle], cur_task);
    }
}


/**
 * @brief Kernel function to signal waiting processes.
 *
 * Handles signals and broadcasts, with or without yielding.
 * May cause current task to be suspended.
 */
static void kernel_event_signal(uint8_t is_broadcast, uint8_t and_next)
{
    /* Check the handle of the event to ensure that it is initialized. */
    uint8_t handle = (uint8_t)((uint16_t)(kernel_request_event_ptr) - 1);

    if(handle >= num_events_created)
    {
        /* Error code. */
        error_msg = ERR_RUN_4_SIGNAL_ON_BAD_EVENT;
        OS_Abort();
    }
    else
    {
        uint8_t make_ready = 0;

        /* Signal appropriately, and perhaps place this task in a queue. */
        if(and_next)
        {
            make_ready = 1;
        }

        while(event_queue[handle].head != NULL)
        {
            /* The signalled task */
			task_descriptor_t* task_ptr = dequeue(&event_queue[handle]);
            task_ptr->state = READY;

            switch(task_ptr->level)
            {
            case SYSTEM:
                enqueue(&system_queue, task_ptr);
                break;
            case PERIODIC:
                break;
            case RR:
                enqueue(&rr_queue, task_ptr);
                break;
            default:
                break;
            }

            /* Check to see if current task needs to be pre-empted */
			if(cur_task != idle_task && !make_ready)
            {
                if(cur_task->level != SYSTEM && task_ptr->level == SYSTEM)
                {
                    make_ready = 1;
                }
                else if(cur_task->level == RR &&
                    PT > 0 &&
                    slot_task_finished == 0 &&
                    task_ptr == name_to_task_ptr[PPP[slot_name_index]])
                {
                    make_ready = 1;
                }
            }

            if(!is_broadcast)
            {
                break;
            }
        }

        if(make_ready && cur_task != idle_task)
        {
            cur_task->state = READY;
            if(cur_task->level == RR)
            {
                enqueue(&rr_queue, cur_task);
            }
        }
    }
}


/*
 * Queue manipulation.
 */

/**
 * @brief Add a task the head of the queue
 *
 * @param queue_ptr the queue to insert in
 * @param task_to_add the task descriptor to add
 */
static void enqueue(queue_t* queue_ptr, task_descriptor_t* task_to_add)
{
    task_to_add->next = NULL;

    if(queue_ptr->head == NULL)
    {
        /* empty queue */
        queue_ptr->head = task_to_add;
        queue_ptr->tail = task_to_add;
    }
    else
    {
        /* put task at the back of the queue */
        queue_ptr->tail->next = task_to_add;
        queue_ptr->tail = task_to_add;
    }
}


/**
 * @brief Pops head of queue and returns it.
 *
 * @param queue_ptr the queue to pop
 * @return the popped task descriptor
 */
static task_descriptor_t* dequeue(queue_t* queue_ptr)
{
    task_descriptor_t* task_ptr = queue_ptr->head;

    if(queue_ptr->head != NULL)
    {
        queue_ptr->head = queue_ptr->head->next;
        task_ptr->next = NULL;
    }

    return task_ptr;
}


/**
 * @brief Update the current time.
 *
 * Perhaps move to the next time slot of the PPP.
 */
static void kernel_update_ticker(void)
{
    /* PORTD ^= LED_D5_RED; */

    if(PT > 0)
    {
        --ticks_remaining;

        if(ticks_remaining == 0)
        {
            /* If Periodic task still running then error */
            if(cur_task != NULL && cur_task->level == PERIODIC && slot_task_finished == 0)
            {
                /* error handling */
                error_msg = ERR_RUN_3_PERIODIC_TOOK_TOO_LONG;
                OS_Abort();
            }

            slot_name_index += 2;
            if(slot_name_index >= 2 * PT)
            {
                slot_name_index = 0;
            }

            ticks_remaining = PPP[slot_name_index + 1];

            if(PPP[slot_name_index] == IDLE || name_to_task_ptr[PPP[slot_name_index]] == NULL)
            {
                slot_task_finished = 1;
            }
            else
            {
                slot_task_finished = 0;
            }
        }
    }
}


/**
 * @brief Validate the PPP array.
 */
static void check_PPP_names(void)
{
    uint8_t i;
    uint8_t name;

    for(i = 0; i < 2 * PT; i += 2)
    {
        name = PPP[i];

        /* name == IDLE or 0 < name <= MAXNAME */
        if(name <= MAXNAME)
        {
            name_in_PPP[name] = 1;
        }
        else
        {
            error_msg = ERR_1_PPP_NAME_OUT_OF_RANGE;
            OS_Abort();
        }
    }
}

#undef SLOW_CLOCK

#ifdef SLOW_CLOCK
/**
 * @brief For DEBUGGING to make the clock run slower
 *
 * Divide CLKI/O by 64 on timer 1 to run at 125 kHz  CS3[210] = 011
 * 1 MHz CS3[210] = 010
 */
static void kernel_slow_clock(void)
{
    TCCR1B &= ~(_BV(CS12) | _BV(CS10));
    TCCR1B |= (_BV(CS11));
}
#endif

/**
 * @brief Setup the RTOS and create main() as the first SYSTEM level task.
 *
 * Point of entry from the C runtime crt0.S.
 */
void OS_Init()
{
    int i;

    /* Set up the clocks */

    TCCR1B |= (_BV(CS11));

#ifdef SLOW_CLOCK
    kernel_slow_clock();
#endif

    check_PPP_names();

    /*
     * Initialize dead pool to contain all but last task descriptor.
     *
     * DEAD == 0, already set in .init4
     */
    for (i = 0; i < MAXPROCESS - 1; i++)
    {
        task_desc[i].state = DEAD;
        name_to_task_ptr[i] = NULL;
        task_desc[i].next = &task_desc[i + 1];
    }
    task_desc[MAXPROCESS - 1].next = NULL;
    dead_pool_queue.head = &task_desc[0];
    dead_pool_queue.tail = &task_desc[MAXPROCESS - 1];

	/* Create idle "task" */
    kernel_request_create_args.f = (voidfuncvoid_ptr)idle;
    kernel_request_create_args.level = NULL;
    kernel_create_task();

    /* Create "main" task as SYSTEM level. */
    kernel_request_create_args.f = (voidfuncvoid_ptr)r_main;
    kernel_request_create_args.level = SYSTEM;
    kernel_create_task();

    /* First time through. Select "main" task to run first. */
    cur_task = task_desc;
    cur_task->state = RUNNING;
    dequeue(&system_queue);

    /* Initilize time slot */
    if(PT > 0)
    {
        ticks_remaining = PPP[1];
    }

    /* Set up Timer 1 Output Compare interrupt,the TICK clock. */
    TIMSK1 |= _BV(OCIE1A);
    OCR1A = TCNT1 + TICK_CYCLES;
    /* Clear flag. */
    TIFR1 = _BV(OCF1A);

    /*
     * The main loop of the RTOS kernel.
     */
    kernel_main_loop();
}




/**
 *  @brief Delay function adapted from <util/delay.h>
 */
static void _delay_25ms(void)
{
    //uint16_t i;

    /* 4 * 50000 CPU cycles = 25 ms */
    //asm volatile ("1: sbiw %0,1" "\n\tbrne 1b" : "=w" (i) : "0" (50000));
    _delay_ms(25);
}


/** @brief Abort the execution of this RTOS due to an unrecoverable erorr.
 */
void OS_Abort(void)
{
    uint8_t i, j;
    uint8_t flashes, mask;

    Disable_Interrupt();

    /* Initialize port for output */
    DDRD = LED_RED_MASK | LED_GREEN_MASK;

    if(error_msg < ERR_RUN_1_USER_CALLED_OS_ABORT)
    {
        flashes = error_msg + 1;
        mask = LED_GREEN_MASK;
    }
    else
    {
        flashes = error_msg + 1 - ERR_RUN_1_USER_CALLED_OS_ABORT;
        mask = LED_RED_MASK;
    }


    for(;;)
    {
        PORTD = (uint8_t)(LED_RED_MASK | LED_GREEN_MASK);

        for(i = 0; i < 100; ++i)
        {
               _delay_25ms();
        }

        PORTD = (uint8_t) 0;

        for(i = 0; i < 40; ++i)
        {
               _delay_25ms();
        }


        for(j = 0; j < flashes; ++j)
        {
            PORTD = mask;

            for(i = 0; i < 10; ++i)
            {
                _delay_25ms();
            }

            PORTD = (uint8_t) 0;

            for(i = 0; i < 10; ++i)
            {
                _delay_25ms();
            }
        }

        for(i = 0; i < 20; ++i)
        {
            _delay_25ms();
        }
    }
}


/**
 * @param f  a parameterless function to be created as a process instance
 * @param arg an integer argument to be assigned to this process instanace
 * @param level assigned scheduling level: SYSTEM, PERIODIC or RR
 * @param name assigned PERIODIC process name
 * @return 0 if not successful; otherwise non-zero.
 * @sa Task_GetArg(), PPP[].
 *
 *  A new process  is created to execute the parameterless
 *  function @a f with an initial parameter @a arg, which is retrieved
 *  by a call to Task_GetArg().  If a new process cannot be
 *  created, 0 is returned; otherwise, it returns non-zero.
 *  The created process will belong to its scheduling @a level.
 *  If the process is PERIODIC, then its @a name is a user-specified name
 *  to be used in the PPP[] array. Otherwise, @a name is ignored.
 * @sa @ref policy
 */
int Task_Create(void (*f)(void), int arg, unsigned int level, unsigned int name)
{
    int retval;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request_create_args.f = (voidfuncvoid_ptr)f;
    kernel_request_create_args.arg = arg;
    kernel_request_create_args.level = (uint8_t)level;
    kernel_request_create_args.name = (uint8_t)name;

    kernel_request = TASK_CREATE;
    enter_kernel();

    retval = kernel_request_retval;
    SREG = sreg;

    return retval;
}


/**
  * @brief The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
    uint8_t volatile sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_NEXT;
    enter_kernel();

    SREG = sreg;
}


/**
  * @brief The calling task terminates itself.
  */
void Task_Terminate()
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_TERMINATE;
    enter_kernel();

    SREG = sreg;
}


/** @brief Retrieve the assigned parameter.
 */
int Task_GetArg(void)
{
    int arg;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    arg = cur_task->arg;

    SREG = sreg;

    return arg;
}

/**
 * @brief Initialize a new, non-NULL Event descriptor.
 *
 * @return a non-NULL Event descriptor if successful; NULL otherwise.
 */
EVENT *Event_Init(void)
{
    EVENT* event_ptr;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = EVENT_INIT;
    enter_kernel();

    event_ptr = (EVENT *)kernel_request_event_ptr;

    SREG = sreg;

    return event_ptr;
}


/**
  * @brief Wait for the next occurrence of a signal on \a e. The calling process always blocks.
  *
  * @param e  an Event descriptor
  */
void Event_Wait(EVENT *e)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = EVENT_WAIT;
    kernel_request_event_ptr = e;
    enter_kernel();

    SREG = sreg;
}


/**
  * \param e an Event descriptor
  *
  * @brief Resume a \b single waiting task on \a e. It is a \a no-op if there is no waiting process.
  * \sa Event_Wait()
  */
void Event_Signal(EVENT *e)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = EVENT_SIGNAL;
    kernel_request_event_ptr = e;
    enter_kernel();

    SREG = sreg;
}


/**
  * \param e  an Event descriptor
  *
  * @brief Resume \b ALL waiting tasks on \a e. It is a \a no-op if there is no waiting process.
  * \sa Event_Wait()
  */
void Event_Broadcast(EVENT *e)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = EVENT_BROADCAST;
    kernel_request_event_ptr = e;
    enter_kernel();

    SREG = sreg;
}


/**
  * \param e  an Event descriptor
  *
  * @brief Resume a waiting task on \a e and at the same time relinquish the processor.
  *
  * This is equivalent to "Event_Signal( e ); Task_Next()" in concept. The
  * fundamental difference is that these two operations are performed as
  * an indivisible unit. So conceptually, the calling task resumes another
  * waiting task and gives up its share of the processor simultaneously.
  * \sa Event_Signal(), Task_Next()
  */
void  Signal_And_Next(EVENT *e)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = EVENT_SIGNAL_AND_NEXT;
    kernel_request_event_ptr = e;
    enter_kernel();

    SREG = sreg;
}


/**
  * \param e  an Event descriptor
  *
  * @brief Resume \b ALL waiting tasks on \a e and at the same time relinquish the processor.
  *
  * This is equivalent to "Event_Broadcast( e ); Task_Next()" in concept.
  * \sa Event_Broadcast(), Task_Next()
  */
void  Broadcast_And_Next(EVENT *e)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = EVENT_BROADCAST_AND_NEXT;
    kernel_request_event_ptr = e;
    enter_kernel();

    SREG = sreg;
}

/**
 * Runtime entry point into the program; just start the RTOS.  The application layer must define r_main() for its entry point.
 */
int main()
{
	OS_Init();
	return 0;
}
