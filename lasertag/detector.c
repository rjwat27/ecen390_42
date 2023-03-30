

#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "lockoutTimer.h"
#include "stdio.h"

typedef uint16_t detector_hitCount_t;
#define NUM_FREQ 10
#define DECIMATION_FACTOR 10
#define MEDIAN_INDEX 4
#define NORMAL_FUDGING 100
#define NORMALIZING_FACTOR 2047.5
#define POWER_TEST_1 .1, .2, .3, .25, .225, .2125, .206125, 130, 1, 3
#define POWER_TEST_2 .11, .25, .33, .25, .225, .215, .4, 2, 1, 34

static bool freqIgnore[NUM_FREQ];
static uint32_t detector_hitArray[NUM_FREQ];
static uint32_t filter_input_count;

static bool detector_hitDetectedFlag;
static bool god_mode;
static uint32_t detector_invocationCount;
static uint32_t fudgeFactor;
static uint32_t lastDetectedHitID;

// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {
  for (int i = 0; i < NUM_FREQ; i++) {
    freqIgnore[i] = 0;
    detector_hitArray[i] = 0;
  }
  filter_input_count = 0;
  detector_hitDetectedFlag = false;
  god_mode = false;
  detector_invocationCount = 0;
  fudgeFactor = NORMAL_FUDGING;
  lastDetectedHitID = 0;
}

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {
  for (int i = 0; i < NUM_FREQ; i++) {
    freqIgnore[i] = freqArray[i];
  }
}

// Runs the entire detector: decimating FIR-filter, IIR-filters,
// power-computation, hit-detection. If interruptsCurrentlyEnabled = true,
// interrupts are running. If interruptsCurrentlyEnabled = false you can pop
// values from the ADC buffer without disabling interrupts. If
// interruptsCurrentlyEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits on frequencies specified with detector_setIgnoredFrequencies().
// Assumption: draining the ADC buffer occurs faster than it can fill.
void detector(bool interruptsCurrentlyEnabled) {
  detector_invocationCount++;
  uint32_t elementCount = buffer_elements();
  for (int i = 0; i < elementCount; i++) {
    buffer_data_t rawAdcValue;
    // Disable interrupts for pop
    if (interruptsCurrentlyEnabled) {
      interrupts_disableArmInts();
      rawAdcValue = buffer_pop();
      interrupts_enableArmInts();
    } else {
      rawAdcValue = buffer_pop();
    }
    // scale the raw adc output
    double scaledAdcValue = ((rawAdcValue / NORMALIZING_FACTOR) - 1);

    // add scaled adc value to the input of the filter chain
    filter_addNewInput(scaledAdcValue);
    filter_input_count++;

    // run filters after every 10 entries to xQueue
    if (filter_input_count >= DECIMATION_FACTOR) {
      filter_input_count = 0;
      filter_firFilter();
      // all iir filters
      uint32_t max_power = 0;
      uint16_t max_channel = 0;
      for (uint16_t i = 0; i < NUM_FREQ; i++) {
        filter_iirFilter(i);
        filter_computePower(i, false, false);
      }
      if (!lockoutTimer_running()) {
        double filterPowers[NUM_FREQ];
        filter_getCurrentPowerValues(filterPowers);
        hit_detect(filterPowers);
      }
    }
  }
}

void hit_detect(double *filterPowers) {
  // Don't sort the powers, just the indexes
  uint8_t sortedPowers[NUM_FREQ];
  for (int j = 0; j < NUM_FREQ; j++) {
    sortedPowers[j] = j;
  }

  // Sort the array of indexes
  for (int k = 1; k < NUM_FREQ; k++) {
    uint8_t temp = sortedPowers[k];
    int j = k;
    for (; j > 0 && filterPowers[sortedPowers[j - 1]] > filterPowers[temp];
         j--) {
      sortedPowers[j] = sortedPowers[j - 1];
    }
    sortedPowers[j] = temp;
  }
  double threshold = filterPowers[sortedPowers[MEDIAN_INDEX]] * fudgeFactor;
  uint8_t maxPowerIndex = NUM_FREQ;

  // Don't count players that are being ignored
  while (freqIgnore[sortedPowers[--maxPowerIndex]] && maxPowerIndex != 0) {
  }

  // Detect the hit
  if (!god_mode && filterPowers[sortedPowers[maxPowerIndex]] > threshold) {
    (detector_hitArray[sortedPowers[maxPowerIndex]])++;
    lastDetectedHitID = sortedPowers[maxPowerIndex];
    detector_hitDetectedFlag = true;
    lockoutTimer_start();
    hitLedTimer_start();
  }
}

// Returns true if a hit was detected.
bool detector_hitDetected(void) { return detector_hitDetectedFlag; }

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {
  return lastDetectedHitID;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) { detector_hitDetectedFlag = false; }

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) { god_mode = flagValue; }

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
  for (int i = 0; i < NUM_FREQ; i++) {
    hitArray[i] = detector_hitArray[i];
  }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factor) { fudgeFactor = factor; }

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) { return detector_invocationCount; }

/******************************************************
******************** Test Routines ********************
******************************************************/

// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void) {
  detector_init();
  // Test 1
  double powerData[NUM_FREQ] = {POWER_TEST_1};
  hit_detect(powerData);
  if (detector_hitDetected()) {
    printf("Test 1: Player %d hit you!\n",
           detector_getFrequencyNumberOfLastHit());
  } else {
    printf("No hit detected on Test 1\n");
  }
  detector_clearHit();
  // Test 2
  double powerData2[NUM_FREQ] = {POWER_TEST_2};
  hit_detect(powerData2);
  if (detector_hitDetected()) {
    printf("Test 2: Player %d hit you!\n",
           detector_getFrequencyNumberOfLastHit());
  } else {
    printf("No hit detected on Test 2\n");
  }
}