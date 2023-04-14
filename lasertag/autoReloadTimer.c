#include "sound.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "autoReloadTimer.h"
#include "trigger.h"

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensures that only one hit is detected per 1/2-second interval.

typedef enum { OFF, RUNNING } autoReloadTimer_state_t;

volatile static uint32_t tick_count;
volatile static autoReloadTimer_state_t current_state;
volatile static bool timer_run;

// Perform any necessary inits for the lockout timer.
void autoReloadTimer_init() {
  tick_count = 0;
  current_state = OFF;
  timer_run = false;
}

// Standard tick function.
void autoReloadTimer_tick() {
  // State machine transitions
  switch (current_state) {
  case OFF:
    if (timer_run) {
      timer_run = false;
      current_state = RUNNING;
      tick_count = 0;
    }
    break;
  case RUNNING: // Switch off if timer runs out
    if (tick_count >= AUTO_RELOAD_EXPIRE_VALUE) {
      current_state = OFF;
      sound_playSound(sound_gunReload_e);
      trigger_setRemainingShotCount(AUTO_RELOAD_SHOT_VALUE);
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
void autoReloadTimer_start() { timer_run = true; }
// Returns true if the timer is currently running.
bool autoReloadTimer_running() { return current_state == RUNNING; }

// Disables the autoReloadTimer and re-initializes it.
void autoReloadTimer_cancel() { autoReloadTimer_init(); }