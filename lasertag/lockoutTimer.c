#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> 

#include "mio.h"
#include "utils.h" 
#include "lockoutTimer.h"
#include "intervalTimer.h"

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensures that only one hit is detected per 1/2-second interval.

#define LOCKOUT_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.

#define TEST_TIMER INTERVAL_TIMER_TIMER_2

typedef enum {OFF, RUNNING} lockoutTimer_state_t; 

volatile static uint16_t tick_count; 
volatile static lockoutTimer_state_t current_state; 

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init(){
    tick_count = 0; 
    current_state = OFF; 
}

// Standard tick function.
void lockoutTimer_tick(){
    switch(current_state) {
        case OFF:
            break; 
        case RUNNING:
            tick_count++;
            if (tick_count >= LOCKOUT_TIMER_EXPIRE_VALUE) {
                current_state = OFF; 
            }
            break; 
    }
}

// Calling this starts the timer.
void lockoutTimer_start(){
    tick_count = 0; 
    current_state = RUNNING; 
}

// Returns true if the timer is running.
bool lockoutTimer_running() {
    return (current_state==RUNNING); 
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
    //test if timer is set properly
    intervalTimer_status_t t_stat = intervalTimer_test(TEST_TIMER);
    printf("timer status: %f\n", t_stat); 
    intervalTimer_init(TEST_TIMER);
    lockoutTimer_start();
    intervalTimer_start(TEST_TIMER);
    while (lockoutTimer_running()) {
        //lockoutTimer_tick(); 
    }
    intervalTimer_stop(TEST_TIMER);
    double time = intervalTimer_getTotalDurationInSeconds(TEST_TIMER);
    printf("total: %f\n", time); 
}