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

#include "hitLedTimer.h"
#include "interrupts.h"
#include "runningModes.h"

#define MAX_LIVES 3
#define HITS_PER_LIFE 5
#define SHOTS 10
#define VOLUME 8

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


#define INTERRUPTS_CURRENTLY_ENABLED false

void game_twoTeamTag(void) {
  uint16_t hitCount = 0;
  runningModes_initAll();
  sound_setVolume(sound_volume_t VOLUME); //FIXME 

  // Configuration...
  lives = MAX_LIVES;
  hearts = HITS_PER_LIFE; 
  rounds = SHOTS; 
  // Implement game loop...
  while (lives > 0) {
    detector(INTERRUPTS_CURRENTLY_ENABLED);
    if (detector_hitDetected()) {  // Hit detected
      hearts--;          
      sound_setSound(sound_hit_e);                
      detector_clearHit();                 
      if (hearts <= 0) {
        lives--; 
        hearts = HITS_PER_LIFE; 
        sound_setSound(sound_loseLife_e);
      }
    }
  }

  // End game loop...
  interrupts_disableArmInts(); // Done with game loop, disable the interrupts.
  hitLedTimer_turnLedOff();    // Save power :-)
  runningModes_printRunTimeStatistics(); // Print the run-time statistics.
}
