/*
 * Play Mode Behaviors Implementation
 * Motor behavior patterns for CHILL, WARMUP, and CRAZY modes
 */

#include "behaviors.h"
#include "motor.h"
#include "config.h"
#include <Arduino.h>

// Static state for behavior tracking
static uint32_t lastMotorUpdate = 0;
static uint8_t motorState = 0;

// =============================================================================
// INDIVIDUAL MODE BEHAVIORS
// =============================================================================

// CHILL mode: gentle alternating forward/reverse with long pauses
static void behaviorChill(uint32_t currentTime) {
  const uint32_t CYCLE_TIME = 8000;     // 8 seconds total cycle
  const uint32_t FORWARD_TIME = 2000;   // 2 seconds forward
  const uint32_t PAUSE1_TIME = 3000;    // +1 second pause (at 3s)
  const uint32_t REVERSE_TIME = 5000;   // +2 seconds reverse (at 5s)
  const uint8_t SPEED = MAX_DUTY * 60 / 100;  // 60% speed

  // Initialize timer on first run
  if (lastMotorUpdate == 0) {
    lastMotorUpdate = currentTime;
    motorState = 0;
  }

  uint32_t elapsed = currentTime - lastMotorUpdate;

  if (elapsed > CYCLE_TIME) {
    lastMotorUpdate = currentTime;
    motorState = 0;
    elapsed = 0;
  }

  // Pattern: Forward 2s → Pause 1s → Reverse 2s → Pause 3s
  if (elapsed < FORWARD_TIME) {
    motorForward(SPEED);
  } else if (elapsed < PAUSE1_TIME) {
    motorStop();
  } else if (elapsed < REVERSE_TIME) {
    motorReverse(SPEED);
  } else {
    motorStop();
  }
}

// WARMUP mode: random forward/reverse with bursts and direction changes
static void behaviorWarmup(uint32_t currentTime) {
  const uint32_t CYCLE_TIME = 3000;  // 3 seconds cycle
  const uint32_t MODERATE_TIME = 1500;
  const uint32_t BURST_TIME = 2000;
  const uint8_t MODERATE_SPEED = MAX_DUTY * 50 / 100;  // 50% speed
  const uint8_t BURST_SPEED = MAX_DUTY * 80 / 100;     // 80% speed

  // Initialize timer on first run
  if (lastMotorUpdate == 0) {
    lastMotorUpdate = currentTime;
    motorState = random(0, 4);  // 4 patterns now
  }

  uint32_t elapsed = currentTime - lastMotorUpdate;

  if (elapsed > CYCLE_TIME) {
    lastMotorUpdate = currentTime;
    motorState = random(0, 4);  // Randomize pattern including direction
    elapsed = 0;
  }

  // Patterns: 0=forward, 1=forward+burst, 2=reverse, 3=reverse+burst
  bool useReverse = (motorState >= 2);
  bool useBurst = (motorState == 1 || motorState == 3);

  if (elapsed < MODERATE_TIME) {
    if (useReverse) {
      motorReverse(MODERATE_SPEED);
    } else {
      motorForward(MODERATE_SPEED);
    }
  } else if (elapsed < BURST_TIME && useBurst) {
    if (useReverse) {
      motorReverse(BURST_SPEED);  // Random reverse burst
    } else {
      motorForward(BURST_SPEED);  // Random forward burst
    }
  } else {
    motorStop();
  }
}

// CRAZY mode: fast, chaotic movements with rapid direction changes
static void behaviorCrazy(uint32_t currentTime) {
  const uint32_t CYCLE_TIME = 1000;  // 1 second cycle
  const uint8_t FULL_SPEED = MAX_DUTY;
  const uint8_t HIGH_SPEED = MAX_DUTY * 90 / 100;
  const uint8_t MED_SPEED = MAX_DUTY * 60 / 100;

  // Initialize timer on first run
  if (lastMotorUpdate == 0) {
    lastMotorUpdate = currentTime;
    motorState = random(0, 8);  // 8 patterns with direction changes
  }

  uint32_t elapsed = currentTime - lastMotorUpdate;

  if (elapsed > CYCLE_TIME) {
    lastMotorUpdate = currentTime;
    motorState = random(0, 8);  // Random pattern with direction
    elapsed = 0;
  }

  // Execute pattern based on current state
  // Patterns 0-3: Forward, 4-7: Reverse (mirror patterns)
  bool useReverse = (motorState >= 4);
  uint8_t pattern = motorState % 4;

  switch(pattern) {
    case 0:
      // Pattern 0: Full power burst (continuous)
      if (useReverse) {
        motorReverse(FULL_SPEED);
      } else {
        motorForward(FULL_SPEED);
      }
      break;

    case 1:
      // Pattern 1: Quick pulse (500ms on, 500ms off)
      if (elapsed < 500) {
        if (useReverse) {
          motorReverse(HIGH_SPEED);
        } else {
          motorForward(HIGH_SPEED);
        }
      } else {
        motorStop();
      }
      break;

    case 2:
      // Pattern 2: Medium speed run (700ms on, 300ms off)
      if (elapsed < 700) {
        if (useReverse) {
          motorReverse(MED_SPEED);
        } else {
          motorForward(MED_SPEED);
        }
      } else {
        motorStop();
      }
      break;

    case 3:
      // Pattern 3: Rapid direction changes (200ms intervals)
      if ((elapsed / 200) % 2 == 0) {
        if (useReverse) {
          motorReverse(FULL_SPEED);
        } else {
          motorForward(FULL_SPEED);
        }
      } else {
        // Alternate direction every 200ms
        if (useReverse) {
          motorForward(FULL_SPEED);
        } else {
          motorReverse(FULL_SPEED);
        }
      }
      break;

    default:
      motorStop();
      break;
  }
}

// =============================================================================
// PUBLIC API
// =============================================================================

void updateMotorBehavior(PlayMode currentMode) {
  uint32_t currentTime = millis();

  switch(currentMode) {
    case MODE_CHILL:
      behaviorChill(currentTime);
      break;

    case MODE_WARMUP:
      behaviorWarmup(currentTime);
      break;

    case MODE_CRAZY:
      behaviorCrazy(currentTime);
      break;

    default:
      motorStop();
      break;
  }
}

void resetBehaviorState() {
  lastMotorUpdate = 0;
  motorState = 0;
}

