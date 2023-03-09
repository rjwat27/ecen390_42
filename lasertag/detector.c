

#include <stdbool.h>
#include <stdint.h>

#include "filter.h"
#include "buffer.h"
#include "lockoutTimer.h"

typedef uint16_t detector_hitCount_t;
#define NUM_FREQ 10
#define NORMALIZING_FACTOR 2047.5


volatile static bool freqIgnore[NUM_FREQ]; 
volatile static uint32_t detector_hitArray[NUM_FREQ]; 
volatile static uint32_t filter_input_count; //how many times filter_addNewInput invoked

volatile static bool detector_hitDetectedFlag; 
volatile static bool god_mode; 

// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void){
    freqIgnore = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
    detector_hitArray = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    filter_input_count = 0; 
    detector_hitDetectedFlag = false; 
}

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]){
    for (int i=0; i<NUM_FREQ; i++) {
        freqignore[i] = freqArray[i]; 
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
void detector(bool interruptsCurrentlyEnabled){
    uint32_t elementCount = buffer_elements(); 
    for (int i=0; i<elementCount; i++) {
        buffer_data_t rawAdcValue;
        if (INTERRUPT ENABLED) {
            interrupts_disableArmInts(); 
            rawAdcValue = buffer_pop();
            interrupts_enableArmInts();
        }
        else {
            rawAdcValue = buffer_pop();
        }
        //scale the raw adc output
        buffer_data_t scaledAdcValue = ((rawAdcValue / NORMALIZING_FACTOR) - 1);    //test

        //add scaled adc value to the input of the filter chain
        filter_addNewInput(scaledAdcValue);
        filter_input_count++; 

        //run filters after every 10 entries to xQueue
        if (filter_input_count>=10) {
            filter_input_count = 0; 
            filter_firFilter();
            //all iir filters
            uint32_t max_power = 0; 
            uint16_t max_channel = 0;
            for (uint16_t i=0; i<NUM_FREQ; i++) {
                filter_iirFilter(i);
                filter_computePower(i, false,   //ryan is unsure about this
                           false);
                // uint32_t temp = filter_getCurrentPowerValue(i);
                // if (temp>max) {
                //     max_power=temp;
                //     max_channel = i; 
                // }
                //hit detection algorithm

            }
            if (!lockoutTimer_running() && !(freqIgnore[max_channel])) {

                lockoutTimer_start();
                hitLedTimer_start();
                (detector_hitArray[max_channel])++; //maybe
                 detector_hitDetectedFlag  = true; 

            }
        }
        
    }

}

// Returns true if a hit was detected.
bool detector_hitDetected(void);

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void);

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void){
    detector_hitDetectedFlag = false; 
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {
    god_mode = flagValue; 
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
    for (int i=0; i < NUM_FREQ; i++) {
        hitArray[i] = detector_hitArray[i]; 
    }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factor) {
    
}

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void);

/******************************************************
******************** Test Routines ********************
******************************************************/

// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void);