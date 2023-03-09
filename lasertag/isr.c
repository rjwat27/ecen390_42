#include "hitLedTimer.h"
#include "intervalTimer.h"
#include "lockoutTimer.h"
#include "transmitter.h"
#include "trigger.h"

#define ISR_PERIOD 1e-5

// Perform initialization for interrupt and timing related modules.
void isr_init() {
    // interrupts_register(INTERVAL_TIMER_TIMER_0, isr_function);
    // interrupts_irq_enable(INTERVAL_TIMER_TIMER_0);
    // intervalTimer_initCountDown(INTERVAL_TIMER_TIMER_0, ISR_PERIOD);
    // intervalTimer_enableInterrupt(INTERVAL_TIMER_TIMER_0);
    // intervalTimer_start(INTERVAL_TIMER_TIMER_0);
  hitLedTimer_init();
  lockoutTimer_init();
  transmitter_init();
  trigger_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
  // Tick all of our state machines
  hitLedTimer_tick();
  lockoutTimer_tick();
  transmitter_tick();
  trigger_tick();
}
