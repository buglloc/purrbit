/*
 * ATtiny412 + CST6118 Motor Driver with PWM Control
 *
 * PA6 -> CST6118 INA (TCD0 WOA - PWM controlled)
 * PA7 -> CST6118 INB (Digital output for direction control)
 *
 * Modes used:
 * - Forward (Mode A):  PA6=PWM, PA7=LOW (conduction <-> standby)
 * - Reverse (Mode B):  PA6=PWM, PA7=HIGH (conduction <-> brake, duty inverted)
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
  // We only use WOA (PA6) for PWM; PA7 stays GPIO for direction
  
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

// (No PWM on PA7; PA7 is used as a GPIO for direction)

// Motor control using CST6118
// PA6 = PWM (speed control)
// PA7 = Direction/control level

// Stop motor
void motorStop() {
  setPWM_PA6(0);
  digitalWrite(PIN_PA7, LOW);
}

// Run motor forward with speed control (0-255)
// Mode A: PA6=PWM, PA7=LOW toggles between Forward (H,L) and Standby (L,L)
// Higher duty => more conduction time => faster
void motorForward(uint8_t speed) {
  digitalWrite(PIN_PA7, LOW);
  setPWM_PA6(speed);
}

// Run motor in reverse with speed control (0-255)
// Mode B for reverse: PA7=HIGH, PA6=PWM toggles between Reverse (L,H) and Brake (H,H)
// Invert duty: higher speed = lower duty (more reverse conduction time)
void motorReverse(uint8_t speed) {
  digitalWrite(PIN_PA7, HIGH);
  setPWM_PA6(255 - speed);
}

void setup() {
  // Setup PA6/PA7 as digital outputs (TCD will drive them when enabled)
  pinMode(PIN_PA6, OUTPUT);
  pinMode(PIN_PA7, OUTPUT);
  
  // Initialize PWM on PA6
  pwm_init();
  pwm_start();
  
  // Start with motor stopped
  motorStop();
  _delay_ms(100);
}

void loop() {
  // Demo: forward then reverse at a few speeds

  // motorStop();
  // _delay_ms(1000);

  // // Forward
  // motorForward(80);
  // _delay_ms(2000);
  motorForward(80);
  _delay_ms(2000);
  // motorForward(255);
  // _delay_ms(2000);

  motorStop();
  _delay_ms(1000);

  // Reverse
  // motorReverse(80);
  // _delay_ms(2000);
  motorReverse(80);
  _delay_ms(2000);
  // motorReverse(255);
  // _delay_ms(2000);

  // motorStop();
  // _delay_ms(1500);
}
