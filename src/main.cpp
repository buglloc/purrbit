/*
 * ATtiny412 + CST6118 Motor Driver
 *
 * Features:
 * - 3 play modes with bidirectional motor control:
 *   * CHILL:  Gentle alternating forward/reverse (60% speed, predictable pattern)
 *   * WARMUP: Random forward/reverse bursts (50-80% speed, medium chaos)
 *   * CRAZY:  Rapid erratic movements with direction changes (60-100% speed, maximum chaos)
 * - Forward/Reverse motor control with TCD0 PWM
 * - Global 70% duty cycle limiter for motor safety
 * - 20-second placement pause when changing modes (motor stopped for safe positioning)
 * - Auto-sleep after 10 minutes of activity (battery saving)
 * - Manual power-down sleep mode for battery saving
 * - Button control: short press = mode change, long press = sleep
 *
 * Hardware:
 * - PA6 -> CST6118 INA (TCD0 WOA - PWM controlled)
 * - PA7 -> CST6118 INB (GPIO for direction control)
 * - PA1 -> Button (with pull-up)
 * - PA2 -> Green LED
 * - PA3 -> Red LED
 *
 * Motor Control Modes:
 * - Forward (Mode A):  PA6=PWM, PA7=LOW (conduction <-> standby)
 * - Reverse (Mode B):  PA6=PWM, PA7=HIGH (conduction <-> brake, duty inverted)
 */

#include <Arduino.h>
#include "config.h"
#include "motor.h"
#include "behaviors.h"
#include "power.h"
#include "ui.h"

// ========== GLOBAL STATE ==========

PlayMode currentMode = MODE_CHILL;       // Current play mode
uint32_t activityStartTime = 0;          // Tracks when activity started (for auto-sleep)
uint32_t modeChangeTime = 0;             // Tracks when mode was changed (for placement pause)
bool paused = false;                     // True during the 20-second placement pause

// =============================================================================
// SETUP - Initialize all hardware and subsystems
// =============================================================================

void setup() {
  // Initialize UI (button and LEDs)
  buttonInit();
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  
  // Set LEDs to known state (HIGH = off for active-low LEDs)
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);

  // Initialize motor control (TCD0 PWM on PA6)
  pwmInit();
  pwmStart();
  
  // Configure motor control pin PA7 as output
  pinMode(PIN_MOTOR_INB, OUTPUT);
  digitalWrite(PIN_MOTOR_INB, LOW);

  // Initialize motor stopped
  motorStop();

  // Initialize power management
  powerInit();

  // Show initial mode LED
  setLedMode(currentMode);

  // Hardware test: brief motor pulse on startup to verify connection
  delay(1000);  // Wait for power stabilization
  
  // Restore mode LED
  setLedMode(currentMode);
  
  // Start the activity timer (10 minute auto-sleep countdown)
  activityStartTime = millis();
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
  // Check for auto-sleep after 10 minutes of activity
  if (checkAutoSleep(activityStartTime)) {
    // Enter sleep mode
    enterSleep();
    
    // Woke up from sleep - reset timers and state
    activityStartTime = millis();
    paused = false;
    resetBehaviorState();
    
    // Ensure we're in an active mode
    if (currentMode == MODE_OFF) {
      currentMode = MODE_CHILL;
    }
    
    // Restore LED indication
    setLedMode(currentMode);
  }

  // Handle button input
  bool sleepRequested = false;
  bool modeChanged = false;
  currentMode = handleButton(currentMode, sleepRequested, modeChanged);
  
  // If mode was changed, start placement pause
  if (modeChanged) {
    paused = true;
    modeChangeTime = millis();
    motorStop();
    resetBehaviorState();
  }
  
  // If sleep was requested via long press
  if (sleepRequested) {
    // Enter sleep mode
    enterSleep();
    
    // Woke up from sleep - reset timers and state
    activityStartTime = millis();
    paused = false;
    resetBehaviorState();
    
    // Restore LED indication
    setLedMode(currentMode);
  }

  // Check if we're in the mode change placement pause
  if (paused) {
    if (millis() - modeChangeTime >= MODE_PAUSE_MS) {
      // Pause is over, resume normal operation
      paused = false;
    } else {
      // Still in pause - keep motor stopped, skip behavior update
      motorStop();
      return;
    }
  }

  // Update motor behavior for current mode (only when not in pause)
  updateMotorBehavior(currentMode);
}
