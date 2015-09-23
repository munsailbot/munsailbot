/*
 * Include files
 */

#include <Arduino.h>
#include "rtos.h"
#include "rtos_assert.h"

#include "Common.h"
#include "Motor.h"


/*
 * Defines
 */
 
/** Pin 13 has an LED connected on most Arduino boards. */
#define LED 13
 
/** Stack size of all the tasks. */
#define STACK_SIZE_TASK00_C0   256
#define STACK_SIZE_TASK01_C0   256
#define STACK_SIZE_TASK02_C0   256
#define STACK_SIZE_TASK03_C0   256
#define STACK_SIZE_TASK04_C0   256
 

/*
 * Local type definitions
 */
 
 
/*
 * Local prototypes
 */
 
static void task00_class00(uint16_t postedEventVec);
static void task01_class00(uint16_t postedEventVec);
static void task02_class00(uint16_t postedEventVec);
static void task03_class00(uint16_t postedEventVec);
static void task04_class00(uint16_t postedEventVec);
 
 
/*
 * Data definitions
 */
 
static uint8_t _taskStack00_C0[STACK_SIZE_TASK00_C0]
             , _taskStack01_C0[STACK_SIZE_TASK01_C0]
             , _taskStack02_C0[STACK_SIZE_TASK02_C0]
             , _taskStack03_C0[STACK_SIZE_TASK03_C0]
             , _taskStack04_C0[STACK_SIZE_TASK04_C0];

/*static volatile uint16_t _noLoopsTask00_C0 = 0;
static volatile uint16_t _noLoopsTask01_C0 = 0;
static volatile uint16_t _noLoopsTask00_C1 = 0;
 
static volatile uint16_t _task00_C0_cntWaitTimeout = 0;
static volatile uint8_t _touchedBySubRoutine;*/

Motor* mainMotor = new Motor(MAIN_START, MAIN_MOTOR_CONVERSION_A, MAIN_MOTOR_CONVERSION_B);
//Motor* jibMotor = new Motor(JIB_START, JIB_MOTOR_CONVERSION_A, JIB_MOTOR_CONVERSION_B);
Motor* rudderMotor = new Motor(RUDDER_START, RUDDER_MOTOR_CONVERSION_A, RUDDER_MOTOR_CONVERSION_B);
//Motor* portRudderMotor = new Motor(RUDDER_START, RUDDER_MOTOR_CONVERSION_A, RUDDER_MOTOR_CONVERSION_B);
//Motor* stbdRudderMotor = new Motor(RUDDER_START, RUDDER_MOTOR_CONVERSION_A, RUDDER_MOTOR_CONVERSION_B);

volatile int enableAutonomy = 0; //should be off by default

 
/*
 * Function implementation
 */
 
 /**
 * Trivial routine that flashes the LED a number of times to give simple feedback. The
 * routine is blocking.
 *   @param noFlashes
 * The number of times the LED is lit.
 */
 
static void blink(uint8_t noFlashes)
{
#define TI_FLASH 150

    while(noFlashes-- > 0)
    {
        digitalWrite(LED, HIGH);  /* Turn the LED on. (HIGH is the voltage level.) */
        delay(TI_FLASH);          /* The flash time. */
        digitalWrite(LED, LOW);   /* Turn the LED off by making the voltage LOW. */
        delay(TI_FLASH);          /* Time between flashes. */
    }                              
    delay(1000-TI_FLASH);         /* Wait for a second after the last flash - this command
                                     could easily be invoked immediately again and the
                                     bursts need to be separated. */
#undef TI_FLASH
}

/**
 * Test of redefining the central interrupt of RTuinOS. The default implementation of the
 * interrupt configuration function is overridden by redefining the same function.\n
 */ 

void rtos_enableIRQTimerTic(void)
{
    Serial.println("Overloaded interrupt initialization rtos_enableIRQTimerTic in " __FILE__);
    
#ifdef __AVR_ATmega2560__
    /* Initialization of the system timer: Arduino (wiring.c, init()) has initialized
       timer2 to count up and down (phase correct PWM mode) with prescaler 64 and no TOP
       value (i.e. it counts from 0 till MAX=255). This leads to a call frequency of
       16e6Hz/64/510 = 490.1961 Hz, thus about 2 ms period time.
         Here, we found on this setting (in order to not disturb any PWM related libraries)
       and just enable the overflow interrupt. */
    TIMSK2 |= _BV(TOIE2);
#else
# error Modifcation of code for other AVR CPU required
#endif
    
} /* End of rtos_enableIRQTimerTic */


/* XBEE */
static void task00_class00(uint16_t initCondition)
{
    byte buffer[17];
    size_t checksum = 0;    
    
    for(;;){
        Serial.println("xbee");
        
        if(Serial1.available() > 0){
            //Serial.println("Serial1 Available");
            byte commandByte = Serial1.read();
            Serial.println("woah");

            if(commandByte == MOTOR_COMMAND){
                checksum = 0;
                for(int i=0; i<8; i++){
                    buffer[i] = static_cast<byte>(Serial1.read());
                    checksum += buffer[i];
                }
                buffer[8] = Serial1.read();
                buffer[9] = Serial1.read();

                size_t checksumTransmitted = static_cast<size_t>(buffer[8]) + (static_cast<size_t>(buffer[9]) << 8);
                if(checksum == checksumTransmitted){
                    digitalWrite(12, HIGH);

                    uint16_t mainCommanded = static_cast<uint16_t>(buffer[0]) + (static_cast<uint16_t>(buffer[1]) << 8);
                    uint16_t jibCommanded = static_cast<uint16_t>(buffer[2]) + (static_cast<uint16_t>(buffer[3]) << 8);
                    uint16_t portCommanded = static_cast<uint16_t>(buffer[4]) + (static_cast<uint16_t>(buffer[5]) << 8);
                    uint16_t stbdCommanded = static_cast<uint16_t>(buffer[6]) + (static_cast<uint16_t>(buffer[7]) << 8);

                    mainMotor->setCommandedPosition(mainCommanded);
                    rudderMotor->setCommandedPosition(portCommanded);
                    //jibMotor->setCommandedPosition(jibCommanded);
                    //portRudderMotor->setCommandedPosition(portCommanded);
                    //stbdRudderMotor->setCommandedPosition(stbdCommanded);

                    digitalWrite(12, LOW);
                }
            }
            else if(commandByte == STATUS){

            }
            else if(commandByte == SET_AUTONOMOUS_MODE){
                uint8_t controlByte = Serial1.read();

                if(controlByte == 1 &&enableAutonomy == 0)
                    enableAutonomy = 1;
                else if(controlByte == 0 && enableAutonomy == 1)
                    enableAutonomy = 0;

                Beagle::sendAutonomySentence(enableAutonomy);
            }
            else{
                Serial1.flush();
            }
        }        
    }
    //while(rtos_suspendTaskTillTime(28));
} /* End of task00_class00 */

/* SENSORS */
static void task01_class00(uint16_t initCondition)
{
    for(;;){
        Serial.println("sensors");
    }
    //while(rtos_suspendTaskTillTime(28));
} /* End of task00_class00 */

/* BEAGLE */
static void task02_class00(uint16_t initCondition)
{
    byte buffer[20];
    size_t checksum = 0;    
    
    for(;;){
        Serial.println("beagle");
        
        if(enableAutonomy){
            if(Serial3.available()){
                char c = Serial3.read();
                if(c == 'm'){
                    Serial.println("m");
                    checksum = 0;
                    for(size_t i=0; i<3; i++){
                        buffer[i] = static_cast<byte>(Serial3.read());
                        checksum += static_cast<size_t>(buffer[i]);
                        //Serial.println(buffer[i]);
                    }
                    buffer[3] = Serial3.read();
                    buffer[4] = Serial3.read();

                    size_t checksumRec = static_cast<size_t>(buffer[3]) + (static_cast<size_t>(buffer[4]) << 8);

                    //char recBuf[5];
                    //itoa(checksumRec, recBuf, 10);
                    //Serial.println(recBuf);

                    //char buf[5];
                   // itoa(buffer[2], buf, 10);
                    //Serial.println(buf);

                    if(checksum == checksumRec){
                        if(isSailPosition(buffer[0]) && isSailPosition(buffer[1]) && isRudderPosition(buffer[2])){
                            digitalWrite(12, HIGH);
                            //Serial.println("passed");

                            int16_t mainPosition = static_cast<int16_t>(buffer[0]);
                            int16_t jibPosition = static_cast<int16_t>(buffer[1]);
                            int16_t rudderPosition = static_cast<int16_t>(buffer[2]);

                            mainMotor->setCommandedPosition(mainPosition);
                            rudderMotor->setCommandedPosition(rudderPosition-21);
                            //jibMotor->setCommandedPosition(jibPosition);
                            //portRudderMotor->setCommandedPosition(rudderPosition-21); //Beagle sends rudder values 0-70
                            //stbdRudderMotor->setCommandedPosition(rudderPosition-21);

                            digitalWrite(12, LOW);
                            Serial.println("__PASSED__");

                            /*char* buf = new char[10];
                            char* buf2 = new char[10];
                            itoa(mainMotor->getCommandedPosition(), buf, 10);
                            itoa(rudderMotor->getCommandedPosition(), buf2, 10);

                            Serial.println(buf);
                            Serial.println(buf2);

                            delete[] buf;
                            delete[] buf2;*/

                            //check = false;
                        }
                        else{
                            Serial.println("invalid");
                        }
                    }
                    else{
                        Serial.println("failed");
                    }
                }
                //else
                //    Serial3.flush();
            }
            //else
            //    check = false;
        } //enableAutonomy        
    }
    //while(rtos_suspendTaskTillTime(28));
} /* End of task00_class00 */

/* MOTOR */
static void task03_class00(uint16_t initCondition)
{
    byte readBuffer[22];
    byte sendBuffer[10];
    size_t checksum = 0;    
    
    for(;;){
        Serial.println("motor");
        
        //Read measured positions from rudder motors
        /*Wire.requestFrom(1, 18);
        _delay_ms(2); //not sure if this is necessary, but it was included in the old code

        checksum = 0;
        for(int8_t i=0; i<16; i++){
            readBuffer[i] = static_cast<byte>(Wire.read());
            checksum += readBuffer[i];
        }
        readBuffer[16] = static_cast<byte>(Wire.read());
        readBuffer[17] = static_cast<byte>(Wire.read());

        size_t checksumTransmitted = static_cast<size_t>(readBuffer[16]) + static_cast<size_t>(readBuffer[17] << 8);
        if(checksum == checksumTransmitted){
            //Leaving out the reading of PWM max and current, for now
            uint32_t portRead = static_cast<uint32_t>(readBuffer[0]) + (static_cast<uint32_t>(readBuffer[1]) << 8) + (static_cast<uint32_t>(readBuffer[2]) << 16) + (static_cast<uint32_t>(readBuffer[3]) << 24);
            uint32_t stbdRead = static_cast<uint32_t>(readBuffer[8]) + (static_cast<uint32_t>(readBuffer[9]) << 8) + (static_cast<uint32_t>(readBuffer[10]) << 16) + (static_cast<uint32_t>(readBuffer[11]) << 24);

            portRudderMotor->setMeasuredPosition(portRead);
            stbdRudderMotor->setMeasuredPosition(stbdRead);
        }

        Serial.println("endreadrudder");

        //Read measured positions from sail motors
        Wire.requestFrom(2, 18);
        _delay_ms(2);

        checksum = 0;
        for(int8_t i=0; i<16; i++){
            readBuffer[i] = static_cast<byte>(Wire.read());
            checksum += readBuffer[i];
        }
        readBuffer[16] = static_cast<byte>(Wire.read());
        readBuffer[17] = static_cast<byte>(Wire.read());

        checksumTransmitted = static_cast<size_t>(readBuffer[16]) + static_cast<size_t>(readBuffer[17] << 8);
        if(checksum == checksumTransmitted){
            uint32_t mainRead = static_cast<uint32_t>(readBuffer[0]) + (static_cast<uint32_t>(readBuffer[1]) << 8) + (static_cast<uint32_t>(readBuffer[2]) << 16) + (static_cast<uint32_t>(readBuffer[3]) << 24);
            uint32_t jibRead = static_cast<uint32_t>(readBuffer[8]) + (static_cast<uint32_t>(readBuffer[9]) << 8) + (static_cast<uint32_t>(readBuffer[10]) << 16) + (static_cast<uint32_t>(readBuffer[11]) << 24);

            mainMotor->setMeasuredPosition(mainRead);
            jibMotor->setMeasuredPosition(jibRead);
        }

        Serial.println("endreadsails");*/

        //Send positions to rudders
        Wire.beginTransmission(1);
        checksum = 0;

        sendBuffer[0] = 'g';
        checksum += static_cast<size_t>(sendBuffer[0]);

        //each of these loops only send 2 chars, but our positions are stored as 32-bit integers (as per the old code)
        //need to investigate whether or not 32-bit precision is actually needed or not
        for(uint8_t i=1; i<3; i++){
            sendBuffer[i] = static_cast<byte>(rudderMotor->getCommandedPositionRaw() >> ((i-1)*8));
            checksum += static_cast<size_t>(sendBuffer[i]);
        }
        for(uint8_t i=3; i<5; i++){
            sendBuffer[i] = static_cast<byte>(rudderMotor->getCommandedPositionRaw() >> ((i-3)*8));
            checksum += static_cast<size_t>(sendBuffer[i]);
        }
        for(uint8_t i=5; i<7; i++){
            sendBuffer[i] = static_cast<byte>(checksum >> ((i-5)*8));
        }

        //Serial.println("endrudders");

        Wire.write(sendBuffer, 7);
        Wire.endTransmission();

        //Send positions to winches
        Wire.beginTransmission(2);
        //I2c.begin();
        checksum = 0;

        sendBuffer[0] = 'g';
        checksum += static_cast<size_t>(sendBuffer[0]);

        //ditto above with regards to bit precision
        for(uint8_t i=1; i<3; i++){
            sendBuffer[i] = mainMotor->getCommandedPositionRaw() >> ((i-1)*8);
            checksum += static_cast<size_t>(sendBuffer[i]);
        }
        for(uint8_t i=3; i<5; i++){
            sendBuffer[i] = rudderMotor->getCommandedPositionRaw() >> ((i-3)*8);
            checksum += static_cast<size_t>(sendBuffer[i]);
        }
        for(uint8_t i=5; i<7; i++){
            sendBuffer[i] = checksum >> ((i-5)*8);
        }

        Wire.write(sendBuffer, 7);
        Wire.endTransmission();      
        
            char checkStr[20];
            itoa(mainMotor->getCommandedPosition(), checkStr, 10);   
            Serial.println(checkStr);              
    }
    //while(rtos_suspendTaskTillTime(28));
} /* End of task00_class00 */

/* IDLE/DUMMY */
static void task04_class00(uint16_t initCondition)
{
    for(;;){
        blink(1);
    }
    //while(rtos_suspendTaskTillTime(28));
} /* End of task00_class00 */


/**
 * The initalization of the RTOS tasks and general board initialization.
 */ 

void setup(void)
{
    /* Start serial port at 9600 bps. */
    Serial.begin(9600);
    Serial.println("\n" RTOS_RTUINOS_STARTUP_MSG);
    
    Serial1.begin(9600);
    Serial3.begin(4800);
    
    Wire.begin();

    /* Initialize the digital pin as an output. The LED is used for most basic feedback about
       operability of code. */
    pinMode(13, OUTPUT);
    
    rtos_initializeTask( /* idxTask */          0
                       , /* taskFunction */     task00_class00
                       , /* prioClass */        0
#if RTOS_ROUND_ROBIN_MODE_SUPPORTED == RTOS_FEATURE_ON
                       , /* timeRoundRobin */   28
#endif
                       , /* pStackArea */       &_taskStack00_C0[0]
                       , /* stackSize */        sizeof(_taskStack00_C0)
                       , /* startEventMask */   RTOS_EVT_DELAY_TIMER
                       , /* startByAllEvents */ false
                       , /* startTimeout */     0
                       );
                       
    rtos_initializeTask( /* idxTask */          1
                       , /* taskFunction */     task01_class00
                       , /* prioClass */        0
#if RTOS_ROUND_ROBIN_MODE_SUPPORTED == RTOS_FEATURE_ON
                       , /* timeRoundRobin */   5
#endif
                       , /* pStackArea */       &_taskStack01_C0[0]
                       , /* stackSize */        sizeof(_taskStack01_C0)
                       , /* startEventMask */   RTOS_EVT_DELAY_TIMER
                       , /* startByAllEvents */ false
                       , /* startTimeout */     0
                       );
                       
    rtos_initializeTask( /* idxTask */          2
                       , /* taskFunction */     task02_class00
                       , /* prioClass */        0
#if RTOS_ROUND_ROBIN_MODE_SUPPORTED == RTOS_FEATURE_ON
                       , /* timeRoundRobin */   30
#endif
                       , /* pStackArea */       &_taskStack02_C0[0]
                       , /* stackSize */        sizeof(_taskStack02_C0)
                       , /* startEventMask */   RTOS_EVT_DELAY_TIMER
                       , /* startByAllEvents */ false
                       , /* startTimeout */     0
                       );
                       
    rtos_initializeTask( /* idxTask */          3
                       , /* taskFunction */     task03_class00
                       , /* prioClass */        0
#if RTOS_ROUND_ROBIN_MODE_SUPPORTED == RTOS_FEATURE_ON
                       , /* timeRoundRobin */   30
#endif
                       , /* pStackArea */       &_taskStack03_C0[0]
                       , /* stackSize */        sizeof(_taskStack03_C0)
                       , /* startEventMask */   RTOS_EVT_DELAY_TIMER
                       , /* startByAllEvents */ false
                       , /* startTimeout */     0
                       );  
                       
                       
    rtos_initializeTask( /* idxTask */          4
                       , /* taskFunction */     task04_class00
                       , /* prioClass */        0
#if RTOS_ROUND_ROBIN_MODE_SUPPORTED == RTOS_FEATURE_ON
                       , /* timeRoundRobin */   15
#endif
                       , /* pStackArea */       &_taskStack04_C0[0]
                       , /* stackSize */        sizeof(_taskStack04_C0)
                       , /* startEventMask */   RTOS_EVT_DELAY_TIMER
                       , /* startByAllEvents */ false
                       , /* startTimeout */     0
                       );                                                                                                

} /* End of setup */


/**
 * The application owned part of the idle task. This routine is repeatedly called whenever
 * there's some execution time left. It's interrupted by any other task when it becomes
 * due.
 *   @remark
 * Different to all other tasks, the idle task routine may and should return. (The task as
 * such doesn't terminate). This has been designed in accordance with the meaning of the
 * original Arduino loop function.
 */ 

void loop(void)
{
    blink(2);
    
} /* End of loop */
