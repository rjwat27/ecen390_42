#include <stdbool.h>
#include <stdint.h> 
#include "intervalTimer.h"
#include "buttons.h" 
#include "utils.h" 
#include "mio.h" 
#include "hitLedTimer.h" 
 
// The hitLedTimer is active for 1/2 second once it is started.
// While active, it turns on the LED connected to MIO pin 11
// and also LED LD0 on the ZYBO board.


/*led not bright unless i write to it continually
think of solution but functionally good enough for now
*/

#define HIT_LED_TIMER_EXPIRE_VALUE 55000 // Defined in terms of 100 kHz ticks.
#define CYCLE_PERIOD                1e-5 // Clock period in seconds
#define LED_FLASH_DELAY             500  // change led every half second
#define HIT_LED_TIMER_OUTPUT_PIN 11      // JF-3
#define HIT_VALUE 1                      // Value to write to pin when on
#define OFF_VALUE 0                      // Pin value when off

// counts number of times the sm is ticked 
volatile static uint16_t tick_count = 0; 

// When true, the timer starts running.
volatile static bool timerStartFlag = false;

// flag indicating if the hit timer is currently running
volatile static bool timerIsRunning = false; 

// hitLEDTimer states
typedef enum {OFF, HIT_OFF, HIT_ON} ledTimerState_t;

// test states
typedef enum {FLAG_OFF, WAIT, FLAG_ON} testState_t; 

volatile static ledTimerState_t current_state; 
volatile static testState_t test_state; 

//simple helper function 
// Calling this stops the timer. 
void hitLedTimer_stop() {
    tick_count = 0; 
    timerStartFlag = false; 
    timerIsRunning = false; 
    hitLedTimer_turnLedOff();
}

// Need to init things.
void hitLedTimer_init() {
    current_state = OFF;
    test_state = FLAG_OFF; 
    timerStartFlag = false; 
    hitLedTimer_enable();
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
}

// Standard tick function.
void hitLedTimer_tick() {
    //state transitions

    switch(current_state) {

    case OFF:
        if (timerStartFlag) {  
            current_state = HIT_ON; 
            hitLedTimer_turnLedOn();
  
        }
        break; 

    case HIT_ON:
        if ((tick_count >= HIT_LED_TIMER_EXPIRE_VALUE-1) ) {
  
            hitLedTimer_turnLedOff();
            hitLedTimer_disable();
        }
        break; 
    }

    //state actions
    switch(current_state) {
    case OFF: 
        break; 

    case HIT_ON:
        tick_count++;   //keep track of time through number of sm ticks
        break; 
    }

}

//assert the timer flag at hit signal
void raiseTimerFlag() {
    timerStartFlag = true; 
}

// Calling this starts the timer.
void hitLedTimer_start() {
    hitLedTimer_enable(); 
    timerStartFlag = true; 
    current_state = OFF; 
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return ((tick_count < HIT_LED_TIMER_EXPIRE_VALUE)&& timerStartFlag); 
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
    timerStartFlag = false; 
    hitLedTimer_turnLedOff();
}

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    tick_count = 0; 
}

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {


    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
        hitLedTimer_start(); 
        while(hitLedTimer_running()) {}
        utils_msDelay(500); 
        
        
    }

    hitLedTimer_disable();

}











