/*
 * User Interface Module
 * Handles button input and LED status indicators
 */

#ifndef UI_H
#define UI_H

#include "config.h"

// ========== LED CONTROL ==========

// Set LED indicators based on current mode
void setLedMode(PlayMode mode);

// ========== BUTTON HANDLING ==========

// Initialize button pin
void buttonInit();

// Handle button input (debouncing and press detection)
// Returns the new mode if changed, or current mode if no change
// Sets sleepRequested to true if long press detected
PlayMode handleButton(PlayMode currentMode, bool& sleepRequested, bool& modeChanged);

#endif // UI_H

