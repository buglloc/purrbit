/*
 * Configuration Constants and Pin Definitions
 * Central place for all hardware and timing configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========== HARDWARE PIN ASSIGNMENTS ==========

const uint8_t PIN_LED_RED   = PIN_PA3;  // Red LED indicator
const uint8_t PIN_LED_GREEN = PIN_PA2;  // Green LED indicator
const uint8_t PIN_BTN       = PIN_PA1;  // Mode button (with pull-up)
const uint8_t PIN_MOTOR_INA = PIN_PA6;  // Motor driver input A (TCD0 WOA - PWM)
const uint8_t PIN_MOTOR_INB = PIN_PA7;  // Motor driver input B (GPIO)

// ========== TIMING CONSTANTS ==========

const uint16_t DEBOUNCE_MS        = 40;      // Button debounce time
const uint16_t LONG_PRESS_MS      = 1500;    // Long press threshold
const uint32_t AUTO_SLEEP_MS      = 600000;  // 10 minutes auto-sleep
const uint32_t MODE_PAUSE_MS      = 20000;   // 20 seconds pause after mode change

// ========== MOTOR CONFIGURATION ==========

const uint8_t MAX_DUTY_PERCENT = 70;  // Global duty cycle limiter (safety)
const uint8_t MAX_DUTY = (255 * MAX_DUTY_PERCENT) / 100;

// ========== PLAY MODE DEFINITIONS ==========

enum PlayMode {
  MODE_OFF    = 0,  // Sleep mode (motor off, LEDs off)
  MODE_CHILL  = 1,  // Gentle, slow movements
  MODE_WARMUP = 2,  // Moderate speed with bursts
  MODE_CRAZY  = 3   // Fast, erratic movements
};

#endif // CONFIG_H

