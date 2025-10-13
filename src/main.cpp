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
 #include <avr/io.h>
#include <util/delay.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_PA6, OUTPUT);
  pinMode(PIN_PA, OUTPUT);
}

void loop() {
    analogWrite(PIN_PA6, 200);
    digitalWrite(PIN_PA7, LOW);
    delay(2000);
    analogWrite(PIN_PA6, 50);
    digitalWrite(PIN_PA7, LOW);
    delay(2000);
    analogWrite(PIN_PA6, 0);
    digitalWrite(PIN_PA7, LOW);
    delay(2000);
}


// int main(void)
// {
//     // --- System clock to full speed (disable prescaler) ---
//     _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

//     // --- Make PA2 output ---
//     PORTA.DIRSET = PIN2_bm;

//     // --- Ensure TCA0 is off and in known state ---
//     TCA0.SINGLE.CTRLA = 0;             // stop timer
//     TCA0.SINGLE.CTRLD = 0;
//     TCA0.SINGLE.CTRLECLR = TCA_SINGLE_SPLITM_bm; // clear split if set
//     TCA0.SINGLE.CTRLESET = TCA_SINGLE_SPLITM_bm; // enable split mode

//     // --- Configure split-mode PWM on WO2 (PA2) ---
//     TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP2EN_bm;  // enable WO2
//     TCA0.SPLIT.LPER  = 255;                   // period = 255
//     TCA0.SPLIT.LCMP2 = 128;                   // 50 % duty

//     // --- Start timer with /64 prescale ---
//     TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV64_gc | TCA_SPLIT_ENABLE_bm;

//     // --- Main loop ---
//     while (1) {
//         // toggle duty between 25 % and 75 %
//         TCA0.SPLIT.LCMP2 = 64;
//         _delay_ms(500);
//         TCA0.SPLIT.LCMP2 = 192;
//         _delay_ms(500);
//     }
// }

// #include <Arduino.h>
// #include <avr/sleep.h>
// #include <avr/interrupt.h>
// #include <avr/io.h>

// // ========== HARDWARE CONFIGURATION ==========

// // Pin assignments
// const uint8_t PIN_LED_RED   = PIN_PA3;  // Red LED indicator
// const uint8_t PIN_LED_GREEN = PIN_PA2;  // Green LED indicator
// const uint8_t PIN_BTN       = PIN_PA1;  // Mode button (with pull-up)
// const uint8_t PIN_DRIVE_INA = PIN_PA6;  // Motor driver input A (PWM)
// const uint8_t PIN_DRIVE_INB = PIN_PA7;  // Motor driver input B


// #include <util/delay.h>


// void setup_pwm_on_pa6(void)
// {
//     // force TCA0 into split mode
//     TCA0.SINGLE.CTRLESET = TCA_SINGLE_SPLITM_bm;

//     // configure PA2 for direct PWM test
//     PORTA.DIRSET = PIN2_bm;

//     // set up split mode PWM
//     TCA0.SPLIT.CTRLD = 0;
//     TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP2EN_bm;
//     TCA0.SPLIT.LPER  = 255;
//     TCA0.SPLIT.LCMP2 = 128; // mid duty
//     TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV64_gc | TCA_SPLIT_ENABLE_bm;
// }

// static uint8_t duty = 10;
// static int8_t step  = -1;

// void setup(void)
// {
//     setup_pwm_on_pa6();
// }

// void loop(void)
// {
//     // simple fade in/out loop
//     TCA0.SPLIT.LCMP2 = duty;
//     _delay_ms(5);

//     duty += step;
//     if (duty == 0 || duty == 255)
//         step = -step; // reverse direction at ends
// }

// // ========== TIMING CONSTANTS ==========

// const uint16_t DEBOUNCE_MS = 40;    // Button debounce time
// const uint16_t LONG_MS     = 1500;  // Long press threshold

// // ========== MOTOR CONFIGURATION ==========

// const uint8_t MAX_DUTY_PERCENT = 70;  // Global duty cycle limiter (safety)
// const uint8_t MAX_DUTY = (255 * MAX_DUTY_PERCENT) / 100;

// // ========== MODE DEFINITIONS ==========

// enum Mode {
//   MODE_OFF = 0,     // Sleep mode (motor off, LEDs off)
//   MODE_CHILL = 1,   // Gentle, slow movements
//   MODE_WARMUP = 2,  // Moderate speed with bursts
//   MODE_CRAZY = 3    // Fast, erratic movements
// };

// // ========== GLOBAL STATE ==========

// volatile bool buttonPressed = false;  // Set by interrupt on wake
// uint8_t mode = MODE_CHILL;            // Current play mode
// bool sleeping = false;                // Sleep request flag

// // -----------------------------------------------------------------------------
// // Low-level PWM setup for ATtiny412 on PA6
// // -----------------------------------------------------------------------------

// void initMotorPWM() {
//   // === GPIO setup ===
//   pinMode(PIN_DRIVE_INA, OUTPUT);
//   digitalWrite(PIN_DRIVE_INA, LOW);
//   pinMode(PIN_DRIVE_INB, OUTPUT);
//   digitalWrite(PIN_DRIVE_INB, LOW);

//   // === Configure TCA0 in split mode ===
//   // (Arduino core usually enables split mode by default, but we ensure it)
//   TCA0.SPLIT.CTRLD = 0;  // normal operation, no high/low sync
//   TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP2EN_bm;  // Enable WO2 (low compare 2)
//   TCA0.SPLIT.LPER = 255;                    // 8-bit full range
//   TCA0.SPLIT.LCMP2 = 0;                     // Start with 0% duty
//   TCA0.SPLIT.CTRLA = TCA_SPLIT_ENABLE_bm | TCA_SPLIT_CLKSEL_DIV64_gc;

//   // === Configure CCL LUT0 to route WO2 → PA6 ===
//   // Input select: use TCA0 WO2 (CCL input channel 2)
//   CCL.LUT0CTRLB = 0;        // Not using first 2 inputs
//   CCL.LUT0CTRLC = 0x08;     // Connect TCA0 WO2 to LUT input 2
//   CCL.TRUTH0 = (1 << 4);    // Output = TRUE when input2 is TRUE
//   CCL.LUT0CTRLA = CCL_OUTEN_bm | CCL_ENABLE_bm; // Enable LUT0 and output on PA6
//   CCL.CTRLA = CCL_ENABLE_bm; // Enable CCL globally
// }

// void stopMotor() {
//   // Disable PWM output by forcing LUT0 output low
//   CCL.LUT0CTRLA &= ~CCL_ENABLE_bm;
//   digitalWrite(PIN_DRIVE_INA, LOW);
//   digitalWrite(PIN_DRIVE_INB, HIGH);  // Coast mode (optional)
//   delay(1);                           // short delay for stability
//   // Re-enable LUT for next start
//   CCL.LUT0CTRLA |= CCL_ENABLE_bm;
// }

// void setMotor(uint8_t duty) {
//   if (duty > MAX_DUTY) duty = MAX_DUTY;
//   if (duty == 0) {
//     stopMotor();
//     return;
//   }

//   // CST6118 forward: INA = PWM (PA6), INB = LOW
//   digitalWrite(PIN_DRIVE_INB, LOW);

//   // Update duty cycle (0–255)
//   TCA0.SPLIT.LCMP2 = duty;
// }

// // Motor behavior for CHILL mode: gentle, slow movements with long pauses
// void motorBehaviorChill(uint32_t currentTime, uint32_t& lastUpdate, uint8_t& state) {
//   const uint32_t CYCLE_TIME = 6000;  // 6 seconds total cycle
//   const uint32_t ACTIVE_TIME = 2000; // 2 seconds active, 4 seconds rest
//   const uint8_t SPEED = MAX_DUTY * 30 / 100;  // 30% speed

//   // Initialize timer on first run
//   if (lastUpdate == 0) {
//     lastUpdate = currentTime;
//   }

//   uint32_t elapsed = currentTime - lastUpdate;

//   if (elapsed > CYCLE_TIME) {
//     lastUpdate = currentTime;
//     state = 0;
//     elapsed = 0;
//   }

//   if (elapsed < ACTIVE_TIME) {
//     setMotor(SPEED);
//   } else {
//     stopMotor();
//   }
// }

// // Motor behavior for WARMUP mode: moderate speed with random bursts
// void motorBehaviorWarmup(uint32_t currentTime, uint32_t& lastUpdate, uint8_t& state) {
//   const uint32_t CYCLE_TIME = 3000;  // 3 seconds cycle
//   const uint32_t MODERATE_TIME = 1500;
//   const uint32_t BURST_TIME = 2000;
//   const uint8_t MODERATE_SPEED = MAX_DUTY * 50 / 100;  // 50% speed
//   const uint8_t BURST_SPEED = MAX_DUTY * 80 / 100;     // 80% speed

//   // Initialize timer on first run
//   if (lastUpdate == 0) {
//     lastUpdate = currentTime;
//     state = random(0, 3);
//   }

//   uint32_t elapsed = currentTime - lastUpdate;

//   if (elapsed > CYCLE_TIME) {
//     lastUpdate = currentTime;
//     state = random(0, 3);  // Randomize pattern
//     elapsed = 0;
//   }

//   if (elapsed < MODERATE_TIME) {
//     setMotor(MODERATE_SPEED);
//   } else if (elapsed < BURST_TIME && state == 1) {
//     setMotor(BURST_SPEED);  // Random burst
//   } else {
//     stopMotor();
//   }
// }

// // Motor behavior for CRAZY mode: fast, erratic movements
// void motorBehaviorCrazy(uint32_t currentTime, uint32_t& lastUpdate, uint8_t& state) {
//   const uint32_t CYCLE_TIME = 1000;  // 1 second cycle
//   const uint8_t FULL_SPEED = MAX_DUTY;
//   const uint8_t HIGH_SPEED = MAX_DUTY * 90 / 100;
//   const uint8_t MED_SPEED = MAX_DUTY * 60 / 100;

//   // Initialize timer on first run
//   if (lastUpdate == 0) {
//     lastUpdate = currentTime;
//     state = random(0, 4);
//   }

//   uint32_t elapsed = currentTime - lastUpdate;

//   if (elapsed > CYCLE_TIME) {
//     lastUpdate = currentTime;
//     state = random(0, 4);  // 4 random patterns
//     elapsed = 0;
//   }

//   // Execute pattern based on current state
//   switch(state) {
//     case 0:
//       // Pattern 0: Full power burst (continuous)
//       setMotor(FULL_SPEED);
//       break;

//     case 1:
//       // Pattern 1: Quick pulse (500ms on, 500ms off)
//       if (elapsed < 500) {
//         setMotor(HIGH_SPEED);
//       } else {
//         stopMotor();
//       }
//       break;

//     case 2:
//       // Pattern 2: Medium speed run (700ms on, 300ms off)
//       if (elapsed < 700) {
//         setMotor(MED_SPEED);
//       } else {
//         stopMotor();
//       }
//       break;

//     case 3:
//       // Pattern 3: Rapid on/off flickering (100ms intervals)
//       if ((elapsed / 100) % 2 == 0) {
//         setMotor(FULL_SPEED);
//       } else {
//         stopMotor();
//       }
//       break;

//     default:
//       stopMotor();
//       break;
//   }
// }

// // Main motor behavior dispatcher
// void updateMotorBehavior() {
//   static uint32_t lastMotorUpdate = 0;
//   static uint8_t motorState = 0;
//   uint32_t currentTime = millis();

//   switch(mode) {
//     case MODE_CHILL:
//       motorBehaviorChill(currentTime, lastMotorUpdate, motorState);
//       break;

//     case MODE_WARMUP:
//       motorBehaviorWarmup(currentTime, lastMotorUpdate, motorState);
//       break;

//     case MODE_CRAZY:
//       motorBehaviorCrazy(currentTime, lastMotorUpdate, motorState);
//       break;

//     default:
//       stopMotor();
//       break;
//   }
// }

// // =============================================================================
// // LED CONTROL FUNCTIONS
// // =============================================================================

// void setLedMode(uint8_t m) {
//   switch(m) {
//     case MODE_OFF:  // OFF - all LEDs off
//       digitalWrite(PIN_LED_RED, HIGH);
//       digitalWrite(PIN_LED_GREEN, HIGH);
//       break;
//     case MODE_CHILL:  // CHILL - red on
//       digitalWrite(PIN_LED_RED, LOW);
//       digitalWrite(PIN_LED_GREEN, HIGH);
//       break;
//     case MODE_WARMUP:  // WARMUP - green on
//       digitalWrite(PIN_LED_RED, HIGH);
//       digitalWrite(PIN_LED_GREEN, LOW);
//       break;
//     case MODE_CRAZY:  // CRAZY - red and green on
//       digitalWrite(PIN_LED_RED, LOW);
//       digitalWrite(PIN_LED_GREEN, LOW);
//       break;
//     default:  // Fallback to OFF
//       digitalWrite(PIN_LED_RED, HIGH);
//       digitalWrite(PIN_LED_GREEN, HIGH);
//       break;
//   }
// }

// // =============================================================================
// // INTERRUPT HANDLERS
// // =============================================================================

// // Port A interrupt handler - wakes device from sleep on button press
// ISR(PORTA_PORT_vect) {
//   // MUST clear the interrupt flag - this is critical!
//   // Clear ALL flags to handle the interrupt
//   VPORTA.INTFLAGS = 0xFF;

//   // Set wake flag
//   buttonPressed = true;
// }

// // =============================================================================
// // POWER MANAGEMENT
// // =============================================================================

// void enterSleep() {
//   // Stop motor and turn off all LEDs
//   stopMotor();
//   setLedMode(MODE_OFF);

//   // Make absolutely sure button is released and stable
//   while (digitalRead(PIN_BTN) == LOW) {
//     delay(10);
//   }
//   delay(200);  // Wait for button to be completely stable

//   // Clear any pending interrupt flags BEFORE enabling interrupt
//   VPORTA.INTFLAGS = 0xFF;
//   buttonPressed = false;

//   // Configure PA1 for interrupt on BOTH EDGES (any change)
//   // This is more reliable for waking from sleep
//   PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;

//   // Short delay to let hardware settle
//   delay(10);

//   // Clear flags again after configuring
//   VPORTA.INTFLAGS = 0xFF;

//   // Set sleep mode to POWER DOWN (deepest sleep, lowest power consumption)
//   // All clocks stopped, only async pin interrupts can wake
//   set_sleep_mode(SLEEP_MODE_PWR_DOWN);

//   // Sleep sequence
//   sleep_enable();
//   sei();
//   sleep_cpu();

//   // ====== Woke up here ======
//   sleep_disable();

//   // IMMEDIATELY disable pin interrupt to prevent re-triggering
//   PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;

//   // Clear all flags
//   VPORTA.INTFLAGS = 0xFF;
//   buttonPressed = false;

//   // Wait for button to be released before continuing
//   while (digitalRead(PIN_BTN) == LOW) {
//     delay(10);
//   }
//   delay(200);  // Debounce after release
// }

// // =============================================================================
// // ARDUINO SETUP & INITIALIZATION
// // =============================================================================

// void setup() {
//   // Configure pins
//   pinMode(PIN_BTN, INPUT_PULLUP);
//   pinMode(PIN_LED_RED, OUTPUT);
//   pinMode(PIN_LED_GREEN, OUTPUT);
//   pinMode(PIN_DRIVE_INA, OUTPUT);
//   pinMode(PIN_DRIVE_INB, OUTPUT);

//   initMotorPWM();  // <<< initialize hardware PWM on PA6

//   // Initialize motor stopped
//   stopMotor();

//   // Disable pin interrupt initially (only enable during sleep)
//   PORTA.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;

//   // Enable global interrupts
//   sei();

//   // Show initial mode
//   setLedMode(mode);

//   // Hardware test: brief motor pulse on startup to verify connection
//   delay(500);  // Wait for power stabilization
//   setMotor(MAX_DUTY / 2);  // Run at 50% of max (35% of full scale)
//   delay(300);  // 300ms test pulse
//   stopMotor();
//   delay(200);  // Pause before starting normal operation
// }

// // =============================================================================
// // INPUT HANDLING
// // =============================================================================

// void handleButton() {
//   static bool lastButtonState = HIGH;
//   static uint32_t lastChangeTime = 0;
//   static uint32_t pressStartTime = 0;

//   bool currentButtonState = digitalRead(PIN_BTN);

//   // Debounce: detect state changes
//   if (currentButtonState != lastButtonState) {
//     lastChangeTime = millis();
//     lastButtonState = currentButtonState;
//   }

//   // Wait for debounce period
//   if (millis() - lastChangeTime < DEBOUNCE_MS) {
//     return;
//   }

//   // Button press detected (transition to LOW)
//   if (currentButtonState == LOW && pressStartTime == 0) {
//     pressStartTime = millis();
//   }

//   // Button release detected (transition to HIGH)
//   if (currentButtonState == HIGH && pressStartTime != 0) {
//     uint32_t pressDuration = millis() - pressStartTime;
//     pressStartTime = 0;

//     if (pressDuration >= LONG_MS) {
//       // Long press: enter sleep mode
//       sleeping = true;
//     } else {
//       // Short press: cycle through modes (CHILL -> WARMUP -> CRAZY -> CHILL)
//       mode = (mode % 3) + 1;
//       setLedMode(mode);
//     }
//   }
// }

// void handleSleep() {
//   static bool lastButtonState = HIGH;
//   static uint32_t lastChangeTime = 0;
//   static uint32_t pressStartTime = 0;

//   if (!sleeping) {
//     return;
//   }

//   // Enter sleep
//   enterSleep();
//   sleeping = false;

//   // Reset button state after wake
//   lastButtonState = HIGH;
//   lastChangeTime = millis();
//   pressStartTime = 0;

//   // Ensure we're in an active mode
//   if (mode == MODE_OFF) {
//     mode = MODE_CHILL;
//   }

//   // Restore LED indication
//   setLedMode(mode);
// }

// // =============================================================================
// // MAIN LOOP
// // =============================================================================

// void loop() {
//   // Handle button input
//   handleButton();

//   // Handle sleep/wake cycle
//   handleSleep();

//   // Update motor behavior for current mode
//   updateMotorBehavior();
// }
