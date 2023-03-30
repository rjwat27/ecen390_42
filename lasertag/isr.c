#include "autoReloadTimer.h"
#include "buffer.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "intervalTimer.h"
#include "lockoutTimer.h"
#include "transmitter.h"
#include "trigger.h"

#define ISR_PERIOD 1e-5

// Perform initialization for interrupt and timing related modules.
void isr_init() {
  hitLedTimer_init();
  lockoutTimer_init();
  transmitter_init();
  trigger_init();
  buffer_init();
  autoReloadTimer_init();
  sound_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
  // Tick all of our state machines
  hitLedTimer_tick();
  lockoutTimer_tick();
  transmitter_tick();
  trigger_tick();
  autoReloadTimer_tick();
  buffer_pushover(interrupts_getAdcData());
  sound_tick();
}
