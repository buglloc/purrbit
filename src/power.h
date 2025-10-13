/*
 * Power Management Module
 * Handles sleep/wake functionality
 */

#ifndef POWER_H
#define POWER_H

#include <stdint.h>

// Initialize power management (configure interrupts)
void powerInit();

// Enter deep sleep mode (returns when woken by button press)
void enterSleep();

// Check if it's time for auto-sleep based on activity timer
// Returns true if auto-sleep should be triggered
bool checkAutoSleep(uint32_t activityStartTime);

#endif // POWER_H

