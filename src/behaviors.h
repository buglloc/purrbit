/*
 * Play Mode Behaviors Module
 * Defines motor behavior patterns for each play mode
 */

#ifndef BEHAVIORS_H
#define BEHAVIORS_H

#include <stdint.h>
#include "config.h"

// Update motor behavior based on current mode
// Should be called continuously from main loop
void updateMotorBehavior(PlayMode currentMode);

// Reset behavior state (call when mode changes or waking from sleep)
void resetBehaviorState();

#endif // BEHAVIORS_H

