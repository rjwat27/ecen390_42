#include "trigger.h"
#include "autoReloadTimer.h"
#include "buttons.h"
#include "sound.h"
#include "transmitter.h"
#include "utils.h"
#include <stdint.h>

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10 // JF1 (pg. 25 of ZYBO reference manual).
#define TRIGGER_DEBOUNCE_TICKS 5000 // Based on a system tick-rate of 100 kHz.
#define GUN_TRIGGER_PRESSED 1
#define BOUNCE_DELAY 5
#define STARTING_SHOTS 10

#define VOLUME SOUND_VOLUME_2

typedef enum {
  IDLE,
  DEBOUNCE_PRESSED,
  PRESSED,
  DEBOUNCE_RELEASED
} transmitterState_t;

volatile static bool ignoreGunInput;
volatile static bool enabled;
volatile transmitterState_t currentState;
volatile static uint16_t debounceTicks;
volatile static uint32_t reloadHoldCount;
volatile static trigger_shotsRemaining_t shotsLeft;

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
  sound_setVolume(VOLUME);
  mio_init(false);
  // mio_setPinAsOutput(TRIGGER_GUN_TRIGGER_MIO_PIN);
  ignoreGunInput = false;
  debounceTicks = 0;
  enabled = true;
  shotsLeft = STARTING_SHOTS;
  currentState = IDLE;
  reloadHoldCount = 0;
}

// Trigger can be activated by either btn0 or the external gun that is attached
// to TRIGGER_GUN_TRIGGER_MIO_PIN Gun input is ignored if the gun-input is high
// when the init() function is invoked.
bool triggerPressed() {
  if ((!ignoreGunInput &&
       (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) ||
      (buttons_read() && BUTTONS_BTN0_MASK)) {
    return true;
  }
  return false;
}

// Standard tick function.
void trigger_tick() {

  // State machine transitions
  switch (currentState) {
  case IDLE:
    if (enabled && triggerPressed()) {
      currentState = DEBOUNCE_PRESSED;
      debounceTicks = TRIGGER_DEBOUNCE_TICKS;
    } else {
      currentState = IDLE;
    }
    break;
  case DEBOUNCE_PRESSED:
    // Go back to idle if the trigger goes low, otherwise keep debouncince
    if (triggerPressed()) {
      if (debounceTicks == 0) {
        if (shotsLeft != 0) {
          // Go to pressed, run transmitter and sound
          sound_playSound(sound_gunFire_e);
          reloadHoldCount = 0;
          currentState = PRESSED;
          transmitter_run();
          shotsLeft--;
        } else {
          // Play out of ammo sound
          reloadHoldCount = 0;
          sound_playSound(sound_gunClick_e);
          currentState = PRESSED;
        }
      } else {
        currentState = DEBOUNCE_PRESSED;
      }
    } else {
      currentState = IDLE;
    }
    break;
  case PRESSED:
    // Leave when trigger signal goes low
    if (!triggerPressed()) {
      currentState = DEBOUNCE_RELEASED;
      debounceTicks = TRIGGER_DEBOUNCE_TICKS;
    } else {
      currentState = PRESSED;
    }
    break;
  case DEBOUNCE_RELEASED:
    // Return to PRESSED if the low signal doesn't last long enough
    if (!triggerPressed()) {
      if (debounceTicks == 0) {
        currentState = IDLE;
      } else {
        currentState = DEBOUNCE_RELEASED;
      }
    } else {
      currentState = PRESSED;
    }
    break;
  default:
    break;
  }

  // State machine actions
  switch (currentState) {
  case IDLE:
    if (!autoReloadTimer_running() && shotsLeft == 0) {
      autoReloadTimer_start();
    }
    break;
  case DEBOUNCE_PRESSED:
    debounceTicks--;
    break;
  case PRESSED:
    // Reload after 3 second hold, add sound here
    if (reloadHoldCount >= AUTO_RELOAD_EXPIRE_VALUE) {
      sound_playSound(sound_gunReload_e);
      shotsLeft = AUTO_RELOAD_SHOT_VALUE;
    }
    reloadHoldCount++;
    break;
  case DEBOUNCE_RELEASED:
    debounceTicks--;
    break;
  default:
    break;
  }
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() { enabled = true; }

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() { enabled = false; }

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() { return shotsLeft; }

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
  shotsLeft = count;
}

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest() {
  printf("starting trigger_runTest()\n");
  mio_init(false);
  buttons_init();  // Using buttons
  switches_init(); // and switches.

  while (!(buttons_read() &
           BUTTONS_BTN3_MASK)) { // Run continuously until BTN3 is pressed.
  }
  do {
    utils_msDelay(BOUNCE_DELAY);
  } while (buttons_read());
  printf("exiting trigger_runTest()\n");
}
