#include "transmitter.h"
#include "buttons.h"
#include "filter.h"
#include "mio.h"
#include <stdio.h>
// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

#define MODE_CHANGE_ERROR                                                      \
  "User tried to change to continuous mode while running\n"
#define LED_ON 1
#define LED_OFF 0
#define TEST_DELAY 400

volatile static bool run_transmission;
typedef enum { IDLE, RUNNING_HIGH, RUNNING_LOW } transmitterState_t;
volatile static transmitterState_t current_state;
volatile static uint16_t current_frequency_num;
volatile static uint16_t next_frequency_num;
volatile static uint16_t pulse_timer;
volatile static uint16_t duty_cycle_timer;
volatile static bool continuous_flag;

// Standard init function.
void transmitter_init() {
  current_state = IDLE;
  run_transmission = false;
  mio_init(false);
  mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);
  pulse_timer = TRANSMITTER_PULSE_WIDTH;
  duty_cycle_timer = 0;
  continuous_flag = false;
  next_frequency_num = 0;
}

// Standard tick function.
void transmitter_tick() {
  // State Machines Transitions
  switch (current_state) {
  case IDLE:
    // Initialize the timer and start running high
    if (run_transmission) {
      run_transmission = false;
      // Set the pulse timer and duty cycle
      pulse_timer = TRANSMITTER_PULSE_WIDTH;
      current_frequency_num = next_frequency_num;
      duty_cycle_timer = filter_frequencyTickTable[current_frequency_num];
      mio_writePin(TRANSMITTER_OUTPUT_PIN, LED_ON);
      current_state = RUNNING_HIGH;
    } else {
      current_state = IDLE;
    }
    break;
  case RUNNING_HIGH:

    // If the pulse is finished, reset if continuous or go idle
    if (pulse_timer == 0) {
      // printf("pulse running high: %d\n", pulse_timer);
      if (continuous_flag) {
        current_frequency_num = next_frequency_num;
        duty_cycle_timer = filter_frequencyTickTable[current_frequency_num];
        pulse_timer = TRANSMITTER_PULSE_WIDTH;

        current_state = RUNNING_HIGH;
      } else {
        current_state = IDLE;
        mio_writePin(TRANSMITTER_OUTPUT_PIN, LED_OFF);
      }
    } else if (duty_cycle_timer ==
               0) { // If the duty cycle on is over, go to low signal state
      mio_writePin(TRANSMITTER_OUTPUT_PIN, LED_OFF);
      current_state = RUNNING_LOW;
      duty_cycle_timer = filter_frequencyTickTable[current_frequency_num];
    } else {
      current_state = RUNNING_HIGH;
    }
    break;
  case RUNNING_LOW:
    // If the pulse is finished, reset if continuous or go idle
    if (pulse_timer == 0) {
      printf("pulse running low: %d\n", pulse_timer);
      if (continuous_flag) {
        current_frequency_num = next_frequency_num;
        duty_cycle_timer = filter_frequencyTickTable[current_frequency_num];
        pulse_timer = TRANSMITTER_PULSE_WIDTH;

        mio_writePin(TRANSMITTER_OUTPUT_PIN, LED_ON);
        current_state = RUNNING_HIGH;
      } else {
        current_state = IDLE;
      }
    } else if (duty_cycle_timer ==
               0) { // If the duty cycle on is over, go to high signal state
      mio_writePin(TRANSMITTER_OUTPUT_PIN, LED_ON);
      current_state = RUNNING_HIGH;
      duty_cycle_timer = filter_frequencyTickTable[current_frequency_num];
    } else {
      current_state = RUNNING_LOW;
    }
    break;
  default:
    break;
  }

  // State Machine Actions
  switch (current_state) {
  case IDLE:
    break;
  case RUNNING_HIGH:
    duty_cycle_timer--;
    pulse_timer--;
    break;
  case RUNNING_LOW:
    duty_cycle_timer--;
    pulse_timer--;
    break;
  default:
    break;
  }
}

// Activate the transmitter.
void transmitter_run() { run_transmission = true; }

// Returns true if the transmitter is still running.
bool transmitter_running() {
  bool test = current_state == RUNNING_HIGH || current_state == RUNNING_LOW ||
              run_transmission == true;
  return test;
}

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
  next_frequency_num = frequencyNumber;
}

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() { return current_frequency_num; }

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, it
// transmits one burst and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until a 200 ms burst is complete. NOTE: while running continuously,
// the transmitter will only change frequencies in between 200 ms bursts.
void transmitter_setContinuousMode(bool continuousModeFlag) {
  // Don't update the mode to true if the state machine is already running
  if (!continuousModeFlag ||
      (current_state != RUNNING_HIGH && current_state != RUNNING_LOW)) {
    continuous_flag = continuousModeFlag;
  } else {
    printf(MODE_CHANGE_ERROR);
  }
}

/******************************************************************************
***** Test Functions
******************************************************************************/

// Prints out the clock waveform to stdio. Terminates when BTN3 is pressed.
// Prints out one line of 1s and 0s that represent one period of the clock
// signal, in terms of ticks.
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define BOUNCE_DELAY 5
void transmitter_runTest() {
  printf("starting transmitter_runTest()\n");
  mio_init(false);
  buttons_init();     // Using buttons
  switches_init();    // and switches.
  transmitter_init(); // init the transmitter.
  while (!(buttons_read() &
           BUTTONS_BTN3_MASK)) { // Run continuously until BTN3 is pressed.
    uint16_t switchValue =
        switches_read() %
        FILTER_FREQUENCY_COUNT; // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(
        switchValue);  // set the frequency number based upon switch value.
    transmitter_run(); // Start the transmitter.
    while (transmitter_running()) {
      utils_msDelay(TEST_DELAY);
    }
    // printf("completed one test period.\n");
  }
  do {
    utils_msDelay(BOUNCE_DELAY);
  } while (buttons_read());
  printf("exiting transmitter_runTest()\n");
}
// Tests the transmitter in non-continuous mode.
// The test runs until BTN3 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestNoncontinuous() {
  printf("starting transmitter_runTest()\n");
  mio_init(false);
  buttons_init();     // Using buttons
  switches_init();    // and switches.
  transmitter_init(); // init the transmitter.
  mio_writePin(TRANSMITTER_OUTPUT_PIN, LED_ON);
  // utils_msDelay(50);
  while (!(buttons_read() &
           BUTTONS_BTN3_MASK)) { // Run continuously until BTN3 is pressed.
    uint16_t switchValue =
        switches_read() %
        FILTER_FREQUENCY_COUNT; // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(
        switchValue);  // set the frequency number based upon switch value.
    transmitter_run(); // Start the transmitter.
    while (transmitter_running()) {
    }
    // printf("completed one test period.\n");
    utils_msDelay(TEST_DELAY);
  }

  do {
    utils_msDelay(BOUNCE_DELAY);
  } while (buttons_read());
  printf("exiting transmitter_runTest()\n");
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes in the slide switches.
// Test runs until BTN3 is pressed.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestContinuous() {
  printf("starting transmitter continuous test\n");
  mio_init(false);
  buttons_init();     // Using buttons
  switches_init();    // and switches.
  transmitter_init(); // init the transmitter.
  transmitter_setContinuousMode(true);
  transmitter_run();
  uint8_t runs = 10;
  while (runs--) {
    transmitter_tick(); // tick.
    utils_msDelay(
        TRANSMITTER_TEST_TICK_PERIOD_IN_MS); // short delay between ticks.
    if ((buttons_read() &
         BUTTONS_BTN3_MASK)) { // Run continuously until BTN3 is pressed.
      uint16_t switchValue =
          switches_read() %
          FILTER_FREQUENCY_COUNT; // Compute a safe number from the switches.
      transmitter_setFrequencyNumber(
          switchValue); // set the frequency number based upon switch value.
                        // Start the transmitter.
      printf("updating frequency\n");
      do {
        utils_msDelay(BOUNCE_DELAY);
      } while (buttons_read());
    }
  }
}
