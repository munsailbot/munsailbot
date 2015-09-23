#include "Timer.h"

Timer::Timer(){
    clock_gettime(CLOCK_MONOTONIC, &_init);
}

//Returns the time since startup in milliseconds
uint64_t Timer::millis(){
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    timespec* diff = timeSpecDiff(&now, &_init);
    return (diff->tv_sec*1000) + round(diff->tv_nsec / 1.0e6);
}

//Method borrowed from http://aufather.wordpress.com/2010/09/08/high-performance-time-measuremen-in-linux/
struct timespec* Timer::timeSpecDiff(struct timespec *ts1, struct timespec *ts2){
    static struct timespec ts;
    ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
    ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
    if (ts.tv_nsec < 0) {
        ts.tv_sec--;
        ts.tv_nsec += 1000000000;
    }
    return &ts;
}
