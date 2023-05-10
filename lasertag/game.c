/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

/*
The code in runningModes.c can be an example for implementing the game here.
*/

#include <stdio.h>
#include <stdint.h>


#include "sound.h" 
#include "hitLedTimer.h"
#include "interrupts.h"
#include "bluetooth.h"
#include "runningModes.h"
#include "detector.h"
#include "lockoutTimer.h"
#include "intervalTimer.h"
#include "switches.h"
#include "transmitter.h"
#include "trigger.h"
#include "invincibilityTimer.h"
#include "histogram.h"


#define MAX_LIVES 50
#define HITS_PER_LIFE 2
#define SHOTS 10
#define VOLUME SOUND_VOLUME_3
#define RESPAWN_TIME 5

#define DETECTOR_HIT_ARRAY_SIZE  10         // The array contains one location per user frequency.     

#define ISR_CUMULATIVE_TIMER INTERVAL_TIMER_TIMER_0 // Used by the ISR.
#define TOTAL_RUNTIME_TIMER                                                    \
  INTERVAL_TIMER_TIMER_1 // Used to compute total run-time.
#define MAIN_CUMULATIVE_TIMER                                                  \
  INTERVAL_TIMER_TIMER_2 // Used to compute cumulative run-time in main.

#define FILTER_FREQUENCY_COUNT 10 
#define SW_0 0x1 
#define TEAM_A 6
#define TEAM_B 9
  

// This game supports two teams, Team-A and Team-B.
// Each team operates on its own configurable frequency.
// Each player has a fixed set of lives and once they
// have expended all lives, operation ceases and they are told
// to return to base to await the ultimate end of the game.
// The gun is clip-based and each clip contains a fixed number of shots
// that takes a short time to reload a new clip.
// The clips are automatically loaded.
// Runs until BTN3 is pressed.


uint32_t lives; 
uint32_t hearts;  
uint32_t rounds; 
static uint8_t killCount[FILTER_FREQUENCY_COUNT] = {0};

#define dataLength 100 
uint8_t incomingData[1];
uint8_t outgoingData[dataLength];
uint8_t outgoingNum[2];
uint8_t playerNum;

#define INTERRUPTS_CURRENTLY_ENABLED true
void updateBluetooth();

void game_twoTeamTag(void) {
  runningModes_initAll();
  sound_setVolume(VOLUME); 
  uint16_t hitCount = 0;

  bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++)
      ignoredFrequencies[i] = false;
    uint16_t our_frequency = switches_read() %9; 
    playerNum = our_frequency;
    ignoredFrequencies[playerNum] = true;
    uint8_t teamIgnore = (playerNum < 5)? 0:5;
    for(uint8_t i = 0; i < 5; i++, teamIgnore++){
      ignoredFrequencies[teamIgnore] = true;
    }
    ignoredFrequencies[9] = true;
    detector_setIgnoredFrequencies(ignoredFrequencies);
    transmitter_setFrequencyNumber(
        our_frequency);    // Read the switches and switch
                                                // frequency as required.

    trigger_enable(); // Makes the state machine responsive to the trigger.
    interrupts_enableTimerGlobalInts(); // Allow timer interrupts.
    interrupts_startArmPrivateTimer();  // Start the private ARM timer running.
    intervalTimer_reset(
        ISR_CUMULATIVE_TIMER); // Used to measure ISR execution time.
    intervalTimer_reset(
        TOTAL_RUNTIME_TIMER); // Used to measure total program execution time.
    intervalTimer_reset(
        MAIN_CUMULATIVE_TIMER); // Used to measure main-loop execution time.
    intervalTimer_start(
        TOTAL_RUNTIME_TIMER);   // Start measuring total execution time.
    interrupts_enableArmInts(); // ARM will now see interrupts after this.
    lockoutTimer_start(); // Ignore erroneous hits at startup (when all power
                          // values are essentially 0).
  bool start = false;
  while(!start){
    interrupts_disableArmInts();
    uint16_t bytesRead = bluetooth_receiveQueueRead(incomingData, 1);
    if(incomingData[0] == 's' && bytesRead == 1){
        start = true;

    }
        interrupts_enableArmInts(); // ARM will now see interrupts after this.


  }
  // TODO: Timer Start here
  //welcome to laser tag 3000
  sound_playSound(sound_gameStart_e);
  while (!sound_isSoundComplete()) {} 
  detector_clearHit(); 

  // Configuration...
  lives = MAX_LIVES;
  hearts = HITS_PER_LIFE; 
  rounds = SHOTS; 
  // Implement game loop...
  while (lives > 0) { // add timer stuff here
    updateBluetooth();
    intervalTimer_start(MAIN_CUMULATIVE_TIMER); // Measure run-time when you are
                                                // doing something.
    detector(INTERRUPTS_CURRENTLY_ENABLED);
    if (detector_hitDetected()) {  // Hit detected
      detector_clearHit();
      if(!invincibilityTimer_running()){          
        hearts--;    
        hitCount++;                                    
        sound_playSound(sound_hit_e);     
        detector_hitCount_t
            hitCounts[DETECTOR_HIT_ARRAY_SIZE]; // Store the hit-counts here.
        detector_getHitCounts(hitCounts);       // Get the current hit counts.
        histogram_plotUserHits(hitCounts);      // Plot the hit counts on the TFT.               
        if (hearts <= 0) {
          lives--; 

          hearts = HITS_PER_LIFE; 
          sound_playSound(sound_loseLife_e);
          invincibilityTimer_start(RESPAWN_TIME);
          killCount[detector_getFrequencyNumberOfLastHit()]++;
        }
        intervalTimer_stop(MAIN_CUMULATIVE_TIMER); // All done with actual processing.
      }
    }

  }

  //all lives lost... too bad
  sound_playSound(sound_gameOver_e);
  while (!sound_isSoundComplete()) {} 
  runningModes_printRunTimeStatistics(); // Print the run-time statistics.
  // End game loop...
  while (true) {

    trigger_disable();
    updateBluetooth();
    sound_playSound(sound_returnToBase_e);
    while (!sound_isSoundComplete()) {} 

    sound_playSound(sound_oneSecondSilence_e);
    while (!sound_isSoundComplete()) {} 
  }
}

void intToString(uint8_t number) {
  outgoingNum[1] = number % 10 + '0';
  outgoingNum[0] = (number / 10) + '0';
}

uint8_t getTotalDeaths() {
  uint8_t sum = 0;
  if(playerNum < 5) {
    for(uint8_t i = 5; i < 10; i++) {
      sum += killCount[i];
    }
  } else {
    for(uint8_t i = 0; i < 5; i++) {
      sum += killCount[i];
    }
  }
  return sum;
}

void updateBluetooth() {
  interrupts_disableArmInts();
    uint16_t bytesRead = bluetooth_receiveQueueRead(incomingData, 1);
    if(bytesRead == 1) {
      uint8_t dataPlayerNum = incomingData[0] - '0';
      if (incomingData[0] == 's') {
        intToString(getTotalDeaths());
        if(playerNum < 5) {
          sprintf(outgoingData, "A 0 0 B %c %c \n", outgoingNum[0],outgoingNum[1]);
        } else {
          sprintf(outgoingData, "A %c %c B 0 0 \n", outgoingNum[0],outgoingNum[1]);
        }
        bluetooth_transmitQueueWrite(outgoingData, dataLength);
      } else if(dataPlayerNum <= 9 && dataPlayerNum >= 0) {
        if(dataPlayerNum == playerNum) {
          intToString(getTotalDeaths());
          sprintf(outgoingData, "P %c D : %c %c \n", incomingData[0], outgoingNum[0],outgoingNum[1]);
          bluetooth_transmitQueueWrite(outgoingData, dataLength);
        } else if((dataPlayerNum <= 4 && playerNum >= 5) || (dataPlayerNum >= 5 && playerNum <= 4)) {
          intToString(killCount[dataPlayerNum]);
          sprintf(outgoingData, "P %c K : %c %c \n", incomingData[0], outgoingNum[0],outgoingNum[1]);
          bluetooth_transmitQueueWrite(outgoingData, dataLength);
        }
      }
    }
  interrupts_enableArmInts();
}