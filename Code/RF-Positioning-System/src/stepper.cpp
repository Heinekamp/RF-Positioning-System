/**
 * @file stepper.cpp
 * @author Max Heinekamp
 * @brief This file contains the function definitions for the STEPPER class, which handles the stepper motor control.
 * @details The STEPPER class uses the TMCStepper library to interface with the stepper motor driver. It provides methods to control the motor's movement, direction, and configuration settings.
 * @version 0.1
 * @date 29-04-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "stepper.h"
#include "flash.h"

extern FLASH flash_memory;

STEPPER::STEPPER(int address, int step_pin, int enable_pin) : 
    address(address),
    step_pin(step_pin),
    enable_pin(enable_pin),
    stepper(&MOTOR_SERIAL_PORT, R_SENSE, address), 
    direction(false) {
}

void STEPPER::setup() {
    pinMode(step_pin, OUTPUT);
    pinMode(enable_pin, OUTPUT);

    // Enable the stepper driver
    digitalWrite(enable_pin, LOW);

    // Retrieve stepper settings from flash memory
    int microsteps = flash_memory.preferences.getInt("MICROSTEPS", MICROSTEPS);
    int hold_current = flash_memory.preferences.getInt("HOLD-CURRENT", HOLD_CURRENT);
    int rms_current = flash_memory.preferences.getInt("RMS-CURRENT", RMS_CURRENT);

    stepper.begin();
    //stepper.en_spreadCycle(true);           // Enable spreadCycle mode
    stepper.toff(5);                        // Enables driver in software
    stepper.rms_current(rms_current);       // Set motor RMS current
    stepper.ihold(hold_current);            // Set hold current to 25% of run current
    stepper.microsteps(microsteps);         // Set microsteps to 1/16th
    stepper.pwm_autoscale(true);            // Needed for stealthChop
    stepper.shaft(direction);               // Set motor direction
}

void STEPPER::step() {
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(STEP_DELAY);
    digitalWrite(step_pin, LOW);
}

void STEPPER::SetDirection(bool dir) {
    stepper.shaft(dir);
    direction = dir;
}

bool STEPPER::GetDirection() {
    return direction;
}

void STEPPER::updateRMSCurrent() {
    int rms_current = flash_memory.preferences.getInt("RMS-CURRENT", RMS_CURRENT);
    stepper.rms_current(rms_current);
    updateHoldCurrent();   // rms_current() overwrites IHOLD internally; reapply the stored hold current
}

void STEPPER::updateHoldCurrent() {
    int hold_current = flash_memory.preferences.getInt("HOLD-CURRENT", HOLD_CURRENT);
    stepper.ihold(hold_current);
}

uint16_t STEPPER::getRMSCurrent() {
    return stepper.rms_current();
}

uint8_t STEPPER::getHoldCurrent() {
    return stepper.ihold();
}

void STEPPER::updateMicrosteps() {
    int microsteps = flash_memory.preferences.getInt("MICROSTEPS", MICROSTEPS);
    stepper.microsteps(microsteps);
}

void STEPPER::move(float targetSpeed, float acceleration, float angle, int steps_revolution) {
    float currentSpeed = 0;
    float stepInterval = 0;
    unsigned long lastStepTime = 0;
    unsigned long currentTime;
    int stepsTaken = 0;

    // Convert angle to steps
    float steps_degree = (float)steps_revolution / 360.0;
    float steps = angle * steps_degree;
    steps += steps_remainder;
    int steps_rounded = (int)floorf(steps);
    steps_remainder = steps - steps_rounded;

    if (steps_rounded <= 0) return; // No movement needed

    // Compute max reachable speed for triangular profile
    float maxReachableSpeed = sqrt(acceleration * steps_rounded);

    // Limit target speed if we don't have enough steps to reach it
    if (targetSpeed > maxReachableSpeed) {
        targetSpeed = maxReachableSpeed;
    }

    // Now compute acceleration distance for (possibly reduced) targetSpeed
    float accelTime = targetSpeed / acceleration;
    float accelDist = 0.5 * acceleration * accelTime * accelTime;

    // Determine if there's a cruise phase
    float cruiseDist = steps_rounded - 2 * accelDist;
    bool hasCruise = cruiseDist > 0;

    while (stepsTaken < steps_rounded) {
        currentTime = micros();

        // Determine current speed based on phase
        float phaseTime;
        if (stepsTaken < accelDist) {
            // Acceleration phase
            phaseTime = sqrt(2.0 * stepsTaken / acceleration);
            currentSpeed = acceleration * phaseTime;
        } else if (hasCruise && stepsTaken < (steps_rounded - accelDist)) {
            // Cruising at target speed
            currentSpeed = targetSpeed;
        } else {
            // Deceleration phase
            int stepsRemaining = steps_rounded - stepsTaken;
            phaseTime = sqrt(2.0 * stepsRemaining / acceleration);
            currentSpeed = acceleration * phaseTime;
        }

        // Avoid division by zero or too slow speeds
        if (currentSpeed < 1) currentSpeed = 1;

        stepInterval = 1000000.0 / currentSpeed;

        if ((micros() - lastStepTime) >= stepInterval) {
            // Perform step pulse
            lastStepTime = micros();
            digitalWrite(step_pin, HIGH);
            delayMicroseconds(4);
            digitalWrite(step_pin, LOW);
            stepsTaken++;
        }
    }

    // Update the current position depending on the direction
    if(direction) {
      current_position -= angle; // Update the current position in degrees
    } else {
      current_position += angle; // Update the current position in degrees
    }

    // Ensure the current position rolls over within the range [0, 360)
    while (current_position < 0) {
      current_position += 360;
    }
    while (current_position >= 360) {
      current_position -= 360;
    }
  }

  void STEPPER::move(float targetSpeed, float acceleration, float angle, int steps_revolution, STEPPER* stepper2, int stepper2_ratio) {
    float currentSpeed = 0;
    float stepInterval = 0;
    unsigned long lastStepTime = 0;
    unsigned long currentTime;
    int stepsTaken = 0;
    bool updatedStepper2Direction = false;

    //Check if stpper2 is set to rotate in the same direction
    if (stepper2->GetDirection() == direction) {
      stepper2->SetDirection(!direction);
      updatedStepper2Direction = true;
    }
  
    // Convert angle to steps
    float steps_degree = (float)steps_revolution / 360.0;
    float steps = angle * steps_degree;
    steps += steps_remainder;
    int steps_rounded = (int)floorf(steps);
    steps_remainder = steps - steps_rounded;

    if (steps_rounded <= 0) return; // No movement needed

    // Compute max reachable speed for triangular profile
    float maxReachableSpeed = sqrt(acceleration * steps_rounded);

    // Limit target speed if we don't have enough steps to reach it
    if (targetSpeed > maxReachableSpeed) {
        targetSpeed = maxReachableSpeed;
    }

    // Now compute acceleration distance for (possibly reduced) targetSpeed
    float accelTime = targetSpeed / acceleration;
    float accelDist = 0.5 * acceleration * accelTime * accelTime;

    // Determine if there's a cruise phase
    float cruiseDist = steps_rounded - 2 * accelDist;
    bool hasCruise = cruiseDist > 0;
  
    while (stepsTaken < steps_rounded) {
      currentTime = micros();
  
      // Determine current speed based on phase
      float phaseTime;
      if (stepsTaken < accelDist) {
        // Acceleration phase
        phaseTime = sqrt(2.0 * stepsTaken / acceleration);
        currentSpeed = acceleration * phaseTime;
      } else if (hasCruise && stepsTaken < (steps_rounded - accelDist)) {
        // Cruising at target speed
        currentSpeed = targetSpeed;
      } else {
        // Deceleration phase
        int stepsRemaining = steps_rounded - stepsTaken;
        phaseTime = sqrt(2.0 * stepsRemaining / acceleration);
        currentSpeed = acceleration * phaseTime;
      }
  
      // Avoid division by zero or too small a speed
      if (currentSpeed < 1) currentSpeed = 1;
  
      stepInterval = 1000000.0 / currentSpeed;
  
      if ((micros() - lastStepTime) >= stepInterval) {
        lastStepTime = micros();
        step();

        if(stepsTaken % stepper2_ratio == 0) {
          stepper2->step();
        }

        stepsTaken++;
      }
    }

    //Update stepper 2 remainder
    int residual = stepsTaken % stepper2_ratio;
    stepper2->steps_remainder = (float)residual / (float)stepper2_ratio;

    // Restore the original direction of stepper2 if it was changed
    if (updatedStepper2Direction) {
      stepper2->SetDirection(!stepper2->GetDirection());
    }

    // Update the current position depending on the direction
    if(direction) {
      current_position -= angle; // Update the current position in degrees
    } else {
      current_position += angle; // Update the current position in degrees
    }

    // Ensure the current position rolls over within the range [0, 360)
    while (current_position < 0) {
      current_position += 360;
    }
    while (current_position >= 360) {
      current_position -= 360;
    }
  }

  void STEPPER::absoluteMove(float targetSpeed, float acceleration, float angle, int steps_revolution) {
    // Calculate required movement angle based on current position
    float requiredAngle = angle - current_position;

    // Ensure the angle is within the range [0, 360)
    while (requiredAngle < 0 || requiredAngle >= 360) {
      if (requiredAngle < 0) {
        requiredAngle += 360; // Wrap around to positive angle
      } else if (requiredAngle >= 360) {
        requiredAngle -= 360; // Wrap around to negative angle
      }
    }

    // Determine the direction based on the required angle
    if (requiredAngle > 180) {
      SetDirection(true); // CCW
      requiredAngle = 360 - requiredAngle; // Adjust angle for CCW direction
    } else {
      SetDirection(false); // CW
    }

    // Move the motor to the target position
    move(targetSpeed, acceleration, requiredAngle, steps_revolution);

    // Set current position to new location
    current_position = angle;
  }

  void STEPPER::absoluteMove(float targetSpeed, float acceleration, float angle, int steps_revolution, STEPPER* stepper2, int stepper2_ratio) {
    // Calculate required movement angle based on current position
    float requiredAngle = angle - current_position;

    // Ensure the angle is within the range [0, 360)
    while (requiredAngle < 0 || requiredAngle >= 360) {
      if (requiredAngle < 0) {
        requiredAngle += 360; // Wrap around to positive angle
      } else if (requiredAngle >= 360) {
        requiredAngle -= 360; // Wrap around to negative angle
      }
    }

    // Determine the direction based on the required angle
    if (requiredAngle > 180) {
      SetDirection(true); // CCW
      requiredAngle = 360 - requiredAngle; // Adjust angle for CCW direction
    } else {
      SetDirection(false); // CW
    }

    // Move the motor to the target position
    move(targetSpeed, acceleration, requiredAngle, steps_revolution, stepper2, stepper2_ratio);

    // Set current position to new location
    current_position = angle;
  }

    
  
  void STEPPER::runStepperSCurve(float targetSpeed, float maxJerk, int distance) {
    int stepsTaken = 0;
    float pos = 0;
    float vel = 0;
    float acc = 0;
    float jerk = maxJerk;
  
    unsigned long lastStepTime = 0;
    unsigned long lastUpdateTime = micros();
  
    const float dt = 0.001; // 1ms update time in seconds
  
    while (stepsTaken < distance) {
      unsigned long now = micros();
      if ((now - lastUpdateTime) < dt * 1e6) continue; // maintain dt
      lastUpdateTime = now;
  
      // Compute S-curve profile
      // Phase 1: Ramp up acceleration (jerk > 0)
      // Phase 2: Hold acceleration (jerk = 0)
      // Phase 3: Ramp down acceleration (jerk < 0)
      // Phase 4: Reverse for deceleration
  
      // Compute stopping distance to avoid overshooting
      float stoppingDist = (vel * vel) / (2 * acc + 1e-6); // avoid divide-by-zero
  
      // Jerk logic: decide whether to accelerate, cruise, or decelerate
      if (vel < targetSpeed && pos < (distance - stoppingDist)) {
        acc += jerk * dt;
      } else if (vel > 0) {
        acc -= jerk * dt;
      }
  
      // Clamp acceleration and velocity
      float maxAcc = sqrt(targetSpeed * maxJerk);
      if (acc > maxAcc) acc = maxAcc;
      if (acc < -maxAcc) acc = -maxAcc;
  
      vel += acc * dt;
      if (vel < 0) vel = 0;
  
      pos += vel * dt;
  
      // Time between steps
      float stepInterval = 1e6 / (vel + 1e-3); // in microseconds
  
      if ((micros() - lastStepTime) >= stepInterval && stepsTaken < distance) {
        // Send one step pulse (insert actual step control here)
        // e.g., digitalWrite(stepPin, HIGH); delayMicroseconds(2); digitalWrite(stepPin, LOW);
        lastStepTime = micros();
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(4);
        digitalWrite(step_pin, LOW);
        stepsTaken++;
      }
    }
  }

  int STEPPER::Debug() {
    delay(10);  // Give the driver time to respond
    uint32_t status = stepper.DRV_STATUS();
    delay(10);  // Give time before next read
    return (int)status;
}