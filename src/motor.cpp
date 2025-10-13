/*
 * Motor Control Module Implementation
 * TCD0 PWM control for CST6118 motor driver
 */

#include "motor.h"
#include "config.h"
#include <Arduino.h>
#include <avr/io.h>

// =============================================================================
// LOW-LEVEL TCD0 PWM FUNCTIONS
// =============================================================================

void pwmInit() {
  // Enable-protected reg must have 0 written to enable bit before changing other bits
  TCD0.CTRLA &= ~TCD_ENABLE_bm;
  
  // Wait for it to be ready
  while (!(TCD0.STATUS & TCD_ENRDY_bm));
  
  // Don't need overlapping PWM signals so just do oneramp
  TCD0.CTRLB = TCD_WGMODE_ONERAMP_gc;
  
  // Disable all input control
  TCD0.INPUTCTRLA = TCD_INPUTMODE_NONE_gc;
  TCD0.INPUTCTRLB = TCD_INPUTMODE_NONE_gc;
  
  // Set/clear values to create desired duty cycle
  // Start with 0% duty (motor off)
  TCD0.CMPASET = 0x000;
  TCD0.CMPACLR = 0x000;
  
  // System clock with DIV32 prescaler for lower PWM frequency
  // Lower frequency is better for motor drivers (avoids stalling)
  TCD0.CTRLA = TCD_CLKSEL_SYSCLK_gc | TCD_CNTPRES_DIV32_gc | TCD_SYNCPRES_DIV1_gc;
}

void pwmStart() {
  // Configure PA6 as OUTPUT so TCD can control it
  VPORTA.DIR |= PIN6_bm;
  
  // Turn off output override (we want PWM to run)
  TCD0.CTRLC &= ~TCD_CMPOVR_bm;
  
  // Enable WOA (PA6) only. FAULTCTRL is write protected.
  CPU_CCP = CCP_IOREG_gc;
  TCD0.FAULTCTRL = TCD_CMPAEN_bm;
  
  while (!(TCD0.STATUS & TCD_ENRDY_bm)) {
    ;
  }
  
  TCD0.CTRLA |= TCD_ENABLE_bm;
}

void pwmSync() {
  TCD0.CTRLE = TCD_SYNCEOC_bm;
}

void setPWMDuty(uint8_t duty) {
  // Scale 8-bit (0-255) to 9-bit (0-511)
  uint16_t scaled = ((uint16_t)duty << 1);
  
  TCD0.CMPASET = 0x000;
  TCD0.CMPACLR = scaled;
  pwmSync();
}

// =============================================================================
// HIGH-LEVEL MOTOR CONTROL
// =============================================================================

void motorStop() {
  setPWMDuty(0);
  digitalWrite(PIN_MOTOR_INB, LOW);
}

void motorForward(uint8_t speed) {
  // Apply global duty limiter
  if (speed > MAX_DUTY) {
    speed = MAX_DUTY;
  }
  
  digitalWrite(PIN_MOTOR_INB, LOW);
  setPWMDuty(speed);
}

void motorReverse(uint8_t speed) {
  // Apply global duty limiter
  if (speed > MAX_DUTY) {
    speed = MAX_DUTY;
  }
  
  digitalWrite(PIN_MOTOR_INB, HIGH);
  setPWMDuty(255 - speed);  // Inverted duty for reverse
}

