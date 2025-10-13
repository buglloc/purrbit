/*
 * ATtiny412 + CST6118 Motor Driver with PWM Control
 *
 * PA6 -> CST6118 INA (TCD0 WOA - PWM controlled)
 * PA7 -> CST6118 INB (Digital output for direction control)
 *
 * CST6118 PWM Mode B: One pin PWM, other pin HIGH
 * - PA6=PWM, PA7=HIGH for reverse direction
 * - PA6=HIGH, PA7=PWM for forward direction (future enhancement)
 */
#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

// Initialize TCD0 for PWM on PA6 only
void pwm_init(void) {
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
  
  // System clock with DIV4 prescaler but no synchronization prescaler
  TCD0.CTRLA = TCD_CLKSEL_SYSCLK_gc | TCD_CNTPRES_DIV4_gc | TCD_SYNCPRES_DIV1_gc;
}

void pwm_start(void) {
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

// Sync PWM after updating compare values
void pwm_sync(void) {
  TCD0.CTRLE = TCD_SYNCEOC_bm;
}

// Set PWM duty cycle for PA6 (0-255)
void setPWM_PA6(uint8_t duty) {
  // Scale 8-bit (0-255) to 9-bit (0-511)
  uint16_t scaled = ((uint16_t)duty << 1);
  
  TCD0.CMPASET = 0x000;
  TCD0.CMPACLR = scaled;
  pwm_sync();
}

// Motor control using CST6118 PWM Mode B
// PA6 = PWM (speed control)
// PA7 = Digital HIGH (constant)

// Stop motor
void motorStop() {
  setPWM_PA6(0);
  digitalWrite(PIN_PA7, LOW);
}

// Run motor in reverse with speed control (0-255)
// Mode B: PA6=PWM, PA7=HIGH alternates between Reverse (L,H) and Brake (H,H)
// Lower PA6 duty = more time in Reverse = faster
// So we invert: higher speed value = lower PWM duty
void motorReverse(uint8_t speed) {
  digitalWrite(PIN_PA7, HIGH);     // Keep PA7 HIGH for Mode B
  setPWM_PA6(255 - speed);         // Invert: high speed = low duty = more reverse time
}

void setup() {
  // Setup PA7 as digital output
  pinMode(PIN_PA7, OUTPUT);
  
  // Initialize PWM on PA6
  pwm_init();
  pwm_start();
  
  // Start with motor stopped
  motorStop();
  _delay_ms(100);
}

void loop() {
  // Test different speeds
  
  // Motor OFF
  motorStop();
  _delay_ms(2000);
  
  // Very slow - 20% speed
  motorReverse(50);
  _delay_ms(3000);
  
  // Medium slow - 40% speed
  motorReverse(100);
  _delay_ms(3000);
  
  // Half speed - 50%
  motorReverse(128);
  _delay_ms(3000);
  
  // Fast - 80% speed
  motorReverse(200);
  _delay_ms(3000);
  
  // Full speed - 100%
  motorReverse(255);
  _delay_ms(3000);
  
  // Stop
  motorStop();
  _delay_ms(2000);
}
