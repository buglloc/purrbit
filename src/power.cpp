/*
 * Power Management Module Implementation
 * Sleep/wake and power saving functionality
 */

#include "power.h"
#include "config.h"
#include "motor.h"
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// Flag set by interrupt when button is pressed during sleep
volatile bool buttonPressed = false;

// =============================================================================
// INTERRUPT HANDLER
// =============================================================================

// Port A interrupt handler - wakes device from sleep on button press
ISR(PORTA_PORT_vect) {
  // MUST clear the interrupt flag - this is critical!
  // Clear ALL flags to handle the interrupt
  VPORTA.INTFLAGS = 0xFF;

  // Set wake flag
  buttonPressed = true;
}

// =============================================================================
// PUBLIC API
// =============================================================================

void powerInit() {
  // Disable pin interrupt initially (only enable during sleep)
  PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;

  // Enable global interrupts
  sei();
}

void enterSleep() {
  // Stop motor and turn off all LEDs
  motorStop();
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);

  // Make absolutely sure button is released and stable
  while (digitalRead(PIN_BTN) == LOW) {
    delay(10);
  }
  delay(200);  // Wait for button to be completely stable

  // Clear any pending interrupt flags BEFORE enabling interrupt
  VPORTA.INTFLAGS = 0xFF;
  buttonPressed = false;

  // Configure PA1 for interrupt on BOTH EDGES (any change)
  // This is more reliable for waking from sleep
  PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;

  // Short delay to let hardware settle
  delay(10);

  // Clear flags again after configuring
  VPORTA.INTFLAGS = 0xFF;

  // Set sleep mode to POWER DOWN (deepest sleep, lowest power consumption)
  // All clocks stopped, only async pin interrupts can wake
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Sleep sequence
  sleep_enable();
  sei();
  sleep_cpu();

  // ====== Woke up here ======
  sleep_disable();

  // IMMEDIATELY disable pin interrupt to prevent re-triggering
  PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;

  // Clear all flags
  VPORTA.INTFLAGS = 0xFF;
  buttonPressed = false;

  // Wait for button to be released before continuing
  while (digitalRead(PIN_BTN) == LOW) {
    delay(10);
  }
  delay(200);  // Debounce after release
}

bool checkAutoSleep(uint32_t activityStartTime) {
  return (millis() - activityStartTime >= AUTO_SLEEP_MS);
}

