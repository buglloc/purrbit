/*
 * Motor Control Module
 * Handles TCD0 PWM initialization and motor control functions
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// ========== LOW-LEVEL TCD0 PWM FUNCTIONS ==========

// Initialize TCD0 timer for PWM on PA6
void pwmInit();

// Start TCD0 PWM output
void pwmStart();

// Synchronize PWM changes
void pwmSync();

// Set PWM duty cycle for PA6 (0-255)
void setPWMDuty(uint8_t duty);

// ========== HIGH-LEVEL MOTOR CONTROL ==========

// Stop motor (both pins LOW)
void motorStop();

// Run motor forward with speed control (0-255)
// Speed is automatically limited by MAX_DUTY safety limiter
void motorForward(uint8_t speed);

// Run motor in reverse with speed control (0-255)
// Speed is automatically limited by MAX_DUTY safety limiter
void motorReverse(uint8_t speed);

#endif // MOTOR_H

