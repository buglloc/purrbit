/*
 * Cat Toy Motor Controller
 * ATtiny412 + CST6118 Motor Driver
 *
 * Features:
 * - 3 play modes: CHILL, WARMUP, CRAZY
 * - Global 70% duty cycle limiter for motor safety
 * - Power-down sleep mode for battery saving
 * - Button control: short press = mode change, long press = sleep
 *
 * Troubleshooting Motor Issues:
 * 1. On startup, motor should pulse for 300ms (hardware test)
 * 2. Check PA6 and PA7 connections to CST6118 INA/INB
 * 3. Verify power supply to CST6118 and motor
 * 4. Try alternative motor control modes (see setMotor() comments)
 * 5. Check LED indicators work (confirms MCU is running)
 */

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// ========== HARDWARE CONFIGURATION ==========

// Pin assignments
const uint8_t PIN_LED_RED   = PIN_PA3;  // Red LED indicator
const uint8_t PIN_LED_GREEN = PIN_PA2;  // Green LED indicator
const uint8_t PIN_BTN       = PIN_PA1;  // Mode button (with pull-up)
const uint8_t PIN_DRIVE_INA = PIN_PA6;  // Motor driver input A (PWM)
const uint8_t PIN_DRIVE_INB = PIN_PA7;  // Motor driver input B

// ========== TIMING CONSTANTS ==========

const uint16_t DEBOUNCE_MS = 40;    // Button debounce time
const uint16_t LONG_MS     = 1500;  // Long press threshold

// ========== MOTOR CONFIGURATION ==========

const uint8_t MAX_DUTY_PERCENT = 70;  // Global duty cycle limiter (safety)
const uint8_t MAX_DUTY = (255 * MAX_DUTY_PERCENT) / 100;

// ========== MODE DEFINITIONS ==========

enum Mode {
  MODE_OFF = 0,     // Sleep mode (motor off, LEDs off)
  MODE_CHILL = 1,   // Gentle, slow movements
  MODE_WARMUP = 2,  // Moderate speed with bursts
  MODE_CRAZY = 3    // Fast, erratic movements
};

// ========== GLOBAL STATE ==========

volatile bool buttonPressed = false;  // Set by interrupt on wake
uint8_t mode = MODE_CHILL;            // Current play mode
bool sleeping = false;                // Sleep request flag

// =============================================================================
// MOTOR CONTROL FUNCTIONS
// =============================================================================




// CST6118 Motor Driver Control Modes:
// Mode 1 (Forward): INA=PWM, INB=LOW
// Mode 2 (Brake):   INA=LOW, INB=LOW
// Mode 3 (Coast):   INA=LOW, INB=HIGH (or both HIGH)
//
// If motor doesn't work, try uncommenting the alternative setMotor() below

void setMotor(uint8_t duty) {
  // Apply global duty limiter
  if (duty > MAX_DUTY) {
    duty = MAX_DUTY;
  }

  // Ensure duty is not zero (use stopMotor() instead)
  // if (duty == 0) {
  //   stopMotor();
  //   return;
  // }

  // CST6118: INA=PWM, INB=LOW for forward direction
  analogWrite(PIN_DRIVE_INA, duty);
  digitalWrite(PIN_DRIVE_INB, LOW);

  // Alternative (if above doesn't work, uncomment below and comment above):
  digitalWrite(PIN_DRIVE_INA, LOW);
  analogWrite(PIN_DRIVE_INB, duty);
}

void stopMotor() {
  setMotor(0);
  // Both LOW = brake/stop (fastest stop)
  // digitalWrite(PIN_DRIVE_INA, LOW);
  // digitalWrite(PIN_DRIVE_INB, LOW);

  // Alternative coast mode (if brake causes issues):
  // analogWrite(PIN_DRIVE_INA, 0);
  // digitalWrite(PIN_DRIVE_INB, HIGH);
}


// Motor behavior for CHILL mode: gentle, slow movements with long pauses
void motorBehaviorChill(uint32_t currentTime, uint32_t& lastUpdate, uint8_t& state) {
  const uint32_t CYCLE_TIME = 6000;  // 6 seconds total cycle
  const uint32_t ACTIVE_TIME = 2000; // 2 seconds active, 4 seconds rest
  const uint8_t SPEED = MAX_DUTY * 30 / 100;  // 30% speed

  // Initialize timer on first run
  if (lastUpdate == 0) {
    lastUpdate = currentTime;
  }

  uint32_t elapsed = currentTime - lastUpdate;

  if (elapsed > CYCLE_TIME) {
    lastUpdate = currentTime;
    state = 0;
    elapsed = 0;
  }

  if (elapsed < ACTIVE_TIME) {
    setMotor(SPEED);
  } else {
    stopMotor();
  }
}

// Motor behavior for WARMUP mode: moderate speed with random bursts
void motorBehaviorWarmup(uint32_t currentTime, uint32_t& lastUpdate, uint8_t& state) {
  const uint32_t CYCLE_TIME = 3000;  // 3 seconds cycle
  const uint32_t MODERATE_TIME = 1500;
  const uint32_t BURST_TIME = 2000;
  const uint8_t MODERATE_SPEED = MAX_DUTY * 50 / 100;  // 50% speed
  const uint8_t BURST_SPEED = MAX_DUTY * 80 / 100;     // 80% speed

  // Initialize timer on first run
  if (lastUpdate == 0) {
    lastUpdate = currentTime;
    state = random(0, 3);
  }

  uint32_t elapsed = currentTime - lastUpdate;

  if (elapsed > CYCLE_TIME) {
    lastUpdate = currentTime;
    state = random(0, 3);  // Randomize pattern
    elapsed = 0;
  }

  if (elapsed < MODERATE_TIME) {
    setMotor(MODERATE_SPEED);
  } else if (elapsed < BURST_TIME && state == 1) {
    setMotor(BURST_SPEED);  // Random burst
  } else {
    stopMotor();
  }
}

// Motor behavior for CRAZY mode: fast, erratic movements
void motorBehaviorCrazy(uint32_t currentTime, uint32_t& lastUpdate, uint8_t& state) {
  const uint32_t CYCLE_TIME = 1000;  // 1 second cycle
  const uint8_t FULL_SPEED = MAX_DUTY;
  const uint8_t HIGH_SPEED = MAX_DUTY * 90 / 100;
  const uint8_t MED_SPEED = MAX_DUTY * 60 / 100;

  // Initialize timer on first run
  if (lastUpdate == 0) {
    lastUpdate = currentTime;
    state = random(0, 4);
  }

  uint32_t elapsed = currentTime - lastUpdate;

  if (elapsed > CYCLE_TIME) {
    lastUpdate = currentTime;
    state = random(0, 4);  // 4 random patterns
    elapsed = 0;
  }

  // Execute pattern based on current state
  switch(state) {
    case 0:
      // Pattern 0: Full power burst (continuous)
      setMotor(FULL_SPEED);
      break;

    case 1:
      // Pattern 1: Quick pulse (500ms on, 500ms off)
      if (elapsed < 500) {
        setMotor(HIGH_SPEED);
      } else {
        stopMotor();
      }
      break;

    case 2:
      // Pattern 2: Medium speed run (700ms on, 300ms off)
      if (elapsed < 700) {
        setMotor(MED_SPEED);
      } else {
        stopMotor();
      }
      break;

    case 3:
      // Pattern 3: Rapid on/off flickering (100ms intervals)
      if ((elapsed / 100) % 2 == 0) {
        setMotor(FULL_SPEED);
      } else {
        stopMotor();
      }
      break;

    default:
      stopMotor();
      break;
  }
}

// Main motor behavior dispatcher
void updateMotorBehavior() {
  static uint32_t lastMotorUpdate = 0;
  static uint8_t motorState = 0;
  uint32_t currentTime = millis();

  switch(mode) {
    case MODE_CHILL:
      motorBehaviorChill(currentTime, lastMotorUpdate, motorState);
      break;

    case MODE_WARMUP:
      motorBehaviorWarmup(currentTime, lastMotorUpdate, motorState);
      break;

    case MODE_CRAZY:
      motorBehaviorCrazy(currentTime, lastMotorUpdate, motorState);
      break;

    default:
      stopMotor();
      break;
  }
}

// =============================================================================
// LED CONTROL FUNCTIONS
// =============================================================================

void setLedMode(uint8_t m) {
  switch(m) {
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
// INTERRUPT HANDLERS
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
// POWER MANAGEMENT
// =============================================================================

void enterSleep() {
  // Stop motor and turn off all LEDs
  stopMotor();
  setLedMode(MODE_OFF);

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

// =============================================================================
// ARDUINO SETUP & INITIALIZATION
// =============================================================================

void setup() {
  // Configure pins
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_DRIVE_INA, OUTPUT);
  pinMode(PIN_DRIVE_INB, OUTPUT);

  // Initialize motor stopped
  stopMotor();

  // Disable pin interrupt initially (only enable during sleep)
  PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;

  // Enable global interrupts
  sei();

  // Show initial mode
  setLedMode(mode);

  // Hardware test: brief motor pulse on startup to verify connection
  delay(500);  // Wait for power stabilization
  setMotor(MAX_DUTY / 2);  // Run at 50% of max (35% of full scale)
  delay(300);  // 300ms test pulse
  stopMotor();
  delay(200);  // Pause before starting normal operation
}

// =============================================================================
// INPUT HANDLING
// =============================================================================

void handleButton() {
  static bool lastButtonState = HIGH;
  static uint32_t lastChangeTime = 0;
  static uint32_t pressStartTime = 0;

  bool currentButtonState = digitalRead(PIN_BTN);

  // Debounce: detect state changes
  if (currentButtonState != lastButtonState) {
    lastChangeTime = millis();
    lastButtonState = currentButtonState;
  }

  // Wait for debounce period
  if (millis() - lastChangeTime < DEBOUNCE_MS) {
    return;
  }

  // Button press detected (transition to LOW)
  if (currentButtonState == LOW && pressStartTime == 0) {
    pressStartTime = millis();
  }

  // Button release detected (transition to HIGH)
  if (currentButtonState == HIGH && pressStartTime != 0) {
    uint32_t pressDuration = millis() - pressStartTime;
    pressStartTime = 0;

    if (pressDuration >= LONG_MS) {
      // Long press: enter sleep mode
      sleeping = true;
    } else {
      // Short press: cycle through modes (CHILL -> WARMUP -> CRAZY -> CHILL)
      mode = (mode % 3) + 1;
      setLedMode(mode);
    }
  }
}

void handleSleep() {
  static bool lastButtonState = HIGH;
  static uint32_t lastChangeTime = 0;
  static uint32_t pressStartTime = 0;

  if (!sleeping) {
    return;
  }

  // Enter sleep
  enterSleep();
  sleeping = false;

  // Reset button state after wake
  lastButtonState = HIGH;
  lastChangeTime = millis();
  pressStartTime = 0;

  // Ensure we're in an active mode
  if (mode == MODE_OFF) {
    mode = MODE_CHILL;
  }

  // Restore LED indication
  setLedMode(mode);
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
  // Handle button input
  handleButton();

  // Handle sleep/wake cycle
  handleSleep();

  // Update motor behavior for current mode
  updateMotorBehavior();
}