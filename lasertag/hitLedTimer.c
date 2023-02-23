#include <stdbool.h>
#include <stdint.h> 
#include "intervalTimer.h"
#include "buttons.h" 
 
// The hitLedTimer is active for 1/2 second once it is started.
// While active, it turns on the LED connected to MIO pin 11
// and also LED LD0 on the ZYBO board.

#define HIT_LED_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.
#define CYCLE_PERIOD                1e-5 // Clock period in seconds
#define HIT_LED_TIMER_OUTPUT_PIN 11      // JF-3
#define HIT_VALUE 1                      // Value to write to pin when on
#define OFF_VALUE 0                      // Pin value when off

// counts number of times the sm is ticked 
volatile static uint16_t tick_count = 0; 

// When true, the timer starts running.
volatile static bool timerStartFlag = false;

// hitLEDTimer states
typedef enum {OFF, HIT} ledTimerState_t;

// test states
typdef enum {FLAG_OFF, WAIT, FLAG_ON} testState_t; 

volatile static ledTimerState_t current_state; 
volatile static testState_t test_state; 

// Need to init things.
void hitLedTimer_init() {
    current_state = OFF;
    test_state = FLAG_OFF; 
    timerStartFlag = false; 
    buttons_init();
    mio_init(bool printFailedStatusFlag);
    hitLedTimer_enable();
}

// Standard tick function.
void hitLedTimer_tick() {
    //increment tick count
    //state transitions
    switch(current_state) {

    case OFF:
        if (timerStartFlag) {  
            current_state = HIT; 
            hitLedTimer_turnLedOn();
            hitLedTimer_start();
        }
    break; 

    case HIT:
        if (!hitLedTimer_running()) {
            current_state = OFF; 
            hitLedTimer_turnLedOff();
            timerStartFlag  = false; 
            hitLedTimer_stop();
        }
    break; 
    }

    //state actions
    //handled in transitions

}

//assert the timer flag at hit signal
void raiseTimerFlag() {
    timerStartFlag = true; 
}

// Calling this starts the timer.
void hitLedTimer_start() {
    intervalTimer_reset(INTERVAL_TIMER_TIMER_0);
    intervalTimer_start(INTERVAL_TIMER_TIMER_0);
}

// Calling this stops the timer. 
void hitLedTimer_stop() {
    intervalTimer_stop(INTERVAL_TIMER_TIMER_0);
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return (intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_0) < (CYCLE_PERIOD * HIT_LED_TIMER_EXPIRE_VALUE)); 
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, HIT_VALUE);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, OFF_VALUE);
}

// Disables the hitLedTimer.
void hitLedTimer_disable() {
    intervalTimer_stop(INTERVAL_TIMER_TIMER_0);
}

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    intervalTimer_init(INTERVAL_TIMER_TIMER_0);
    // the timer for the test function
    intervalTimer_init(INTERVAL_TIMER_TIMER_1);
}

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {
    //if HIT state, do nothing. 
    //wait in OFF state before re-raising the timerFlag
    //exit loop with button press

    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
        if (current_state==OFF) {
            utils_msDelay(500);
            timerStartFlag = true; 
        }
    }

}











