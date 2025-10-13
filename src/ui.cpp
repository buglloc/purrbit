/*
 * User Interface Module Implementation
 * Button and LED handling
 */

#include "ui.h"
#include "config.h"
#include <Arduino.h>

// =============================================================================
// LED CONTROL
// =============================================================================

void setLedMode(PlayMode mode) {
  switch(mode) {
    case MODE_OFF:  // OFF - all LEDs off
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, HIGH);
      break;
      
    case MODE_CHILL:  // CHILL - red on
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);
      break;
      
    case MODE_WARMUP:  // WARMUP - green on
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);
      break;
      
    case MODE_CRAZY:  // CRAZY - red and green on
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);
      break;
      
    default:  // Fallback to OFF
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, HIGH);
      break;
  }
}

// =============================================================================
// BUTTON HANDLING
// =============================================================================

void buttonInit() {
  pinMode(PIN_BTN, INPUT_PULLUP);
}

PlayMode handleButton(PlayMode currentMode, bool& sleepRequested, bool& modeChanged) {
  static bool lastButtonState = HIGH;
  static uint32_t lastChangeTime = 0;
  static uint32_t pressStartTime = 0;

  bool currentButtonState = digitalRead(PIN_BTN);

  // Initialize outputs
  sleepRequested = false;
  modeChanged = false;

  // Debounce: detect state changes
  if (currentButtonState != lastButtonState) {
    lastChangeTime = millis();
    lastButtonState = currentButtonState;
  }

  // Wait for debounce period
  if (millis() - lastChangeTime < DEBOUNCE_MS) {
    return currentMode;
  }

  // Button press detected (transition to LOW)
  if (currentButtonState == LOW && pressStartTime == 0) {
    pressStartTime = millis();
  }

  // Button release detected (transition to HIGH)
  if (currentButtonState == HIGH && pressStartTime != 0) {
    uint32_t pressDuration = millis() - pressStartTime;
    pressStartTime = 0;

    if (pressDuration >= LONG_PRESS_MS) {
      // Long press: request sleep mode
      sleepRequested = true;
      return currentMode;
    } else {
      // Short press: cycle through modes (CHILL -> WARMUP -> CRAZY -> CHILL)
      PlayMode newMode = (PlayMode)((currentMode % 3) + 1);
      setLedMode(newMode);
      modeChanged = true;
      return newMode;
    }
  }

  return currentMode;
}

