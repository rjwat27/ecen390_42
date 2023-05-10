#include "sound.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "autoReloadTimer.h"
#include "trigger.h"
#include "detector.h" 

#define TICK_RATE 100000
// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensures that only one hit is detected per 1/2-second interval.

typedef enum { OFF, RUNNING } invincibilityTimer_state_t;

volatile static uint32_t tick_count;
volatile static uint32_t ticks_to_wait;
volatile static invincibilityTimer_state_t current_state;
volatile static bool timer_run;

// Perform any necessary inits for the lockout timer.
void invincibilityTimer_init() {
  tick_count = 0;
  current_state = OFF;
  timer_run = false;
}

// Standard tick function.
void invincibilityTimer_tick() {
  // State machine transitions
  switch (current_state) {
  case OFF:
    if (timer_run) {
      timer_run = false;
      current_state = RUNNING;
      trigger_disable();
      tick_count = 0;
    }
    break;
  case RUNNING: // Switch off if timer runs out
    if (tick_count >= AUTO_RELOAD_EXPIRE_VALUE) {
      //printf("Auto reload\n");
      current_state = OFF;
      sound_playSound(sound_gameStart_e);
      //while (!sound_isSoundComplete()) {} 
      detector_clearHit(); 

      trigger_enable();
    }
    break;
  }

  // State machine actions
  switch (current_state) {
  case OFF:
    break;
  case RUNNING:
    tick_count++;
    break;
  }
}

// Calling this starts the timer.
void invincibilityTimer_start(uint32_t seconds) { 
    timer_run = true; 
    ticks_to_wait = seconds * TICK_RATE;}
// Returns true if the timer is currently running.
bool invincibilityTimer_running() { return current_state == RUNNING; }
