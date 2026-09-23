/**
 * @file stepper.h
 * @author Max Heinekanmp
 * @brief 
 * @version 0.1
 * @date 22-04-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "TMCStepper.h"
#include "config.h"

/**
 * @brief The STEPPER class is used to manage stepper motor instances. It provides methods to control the motor's movement, direction, and configuration settings.
 * @details The STEPPER class uses teemutalut stepper library to interface with the stepper motor driver. It is currently setup to use TMC2209 stepper drivers, but can be adapted to work with other drivers as well.
 * 
 */
class STEPPER{
    public:

    /**
     * @brief Construct a new STEPPER object
     * 
     * @param address SPI address of the stepper driver
     * @param step_pin Pin that is connected to the step pin of the driver
     * @param enable_pin Pin that is connected to the enable pin of the driver
     */
    STEPPER(int address, int step_pin, int enable_pin);

    /**
     * @brief Set up the stepper driver and configure it.
     * 
     */
    void setup();

    /**
     * @brief Step the motor one step.
     * 
     */
    void step();

    /**
     * @brief Set the direction of the stepper motor.
     * 
     * @param dir Direction to set (true for ccw, false for cw)
     */
    void SetDirection(bool dir);

    /**
     * @brief Get the current direction of the stepper motor.
     * 
     * @return true if the motor is set to ccw, false if cw
     */
    bool GetDirection();

    /**
     * @brief Update the RMS current of the stepper driver from the current flash value.
     * 
     */
    void updateRMSCurrent();

    /**
     * @brief Update the hold current of the stepper driver from the current flash value.
     * 
     */
    void updateHoldCurrent();

    /**
     * @brief Get the actual RMS current currently applied by the stepper driver.
     *
     * @return uint16_t Applied RMS current in mA, back-converted from the driver's CS register
     */
    uint16_t getRMSCurrent();

    /**
     * @brief Get the actual hold current currently applied by the stepper driver.
     *
     * @return uint8_t Applied raw IHOLD register value
     */
    uint8_t getHoldCurrent();

    /**
     * @brief Update the microsteps of the stepper driver from the current flash value.
     * 
     */
    void updateMicrosteps();

    /**
     * @brief Move the motor a given angle with a given acceleration and distance.
     * @note The direction is based on the current state of stepper.direction
     * 
     * @param targetSpeed Target speed in steps per second
     * @param acceleration Acceleration in steps per second squared
     * @param angle Distance to move in steps
     * @param steps_revolution Number of steps per revolution
     */
    void move(float targetSpeed, float acceleration, float position, int steps_revolution);

    /**
     * @brief Move the motor a given angle with a given acceleration and distance.
     * @note The direction is based on the current state of stepper.direction
     * 
     * @param targetSpeed Target speed in steps per second
     * @param acceleration Acceleration in steps per second squared
     * @param angle Distance to move in steps
     * @param steps_revolution Number of steps per revolution
     * @param stepper2 Second stepper motor to move (if motors are mechanically coupled)
     * @param steps_revolution2 Number of steps per revolution for the second stepper motor
     */
    void move(float targetSpeed, float acceleration, float position, int steps_revolution, STEPPER* stepper2, int stepper2_ratio);

    /**
     * @brief Move the motor to a target location based on an absolute angle with a given speed and acceleration.
     * @note the optimal direction will be selected automatically to minimize the travel distance.
     * 
     * @param targetSpeed Target speed in steps per second
     * @param acceleration Acceleration in steps per second squared
     * @param position Absolute target position in degrees
     * @param steps_revolution Number of steps per revolution
     */
    void absoluteMove(float targetSpeed, float acceleration, float angle, int steps_revolution);

    /**
     * @brief Move the motor to a target location based on an absolute angle with a given speed and acceleration.
     * @note the optimal direction will be selected automatically to minimize the travel distance.
     * 
     * @param targetSpeed Target speed in steps per second
     * @param acceleration Acceleration in steps per second squared
     * @param position Absolute target position in degrees
     * @param steps_revolution Number of steps per revolution
     * @param stepper2 Second stepper motor to move (if motors are mechanically coupled)
     * @param steps_revolution2 Number of steps per revolution for the second stepper motor
     */
    void absoluteMove(float targetSpeed, float acceleration, float angle, int steps_revolution, STEPPER* stepper2, int stepper2_ratio);

    /**
     * @brief Run the stepper motor using an S-curve profile.
     * 
     * @param targetSpeed Target speed in steps per second
     * @param maxJerk Maximum jerk in steps per second cubed
     * @param distance Distance to move in steps
     */
    void runStepperSCurve(float targetSpeed, float maxJerk, int distance);

    /**
     * @brief Get the current position of the motor in degrees.
     * 
     * @return float Current position of the motor in degrees
     */
    float getCurrentPosition() {
        return current_position;
    }

    int Debug();
    
    private:
    /**
     * @brief Stepper driver object. This is the object that interfaces with the stepper driver library.
     * 
     */
    TMC2209Stepper stepper;

    /**
     * @brief Direction of the motor. This is used to set the direction of the motor
     * @note true for ccw, false for cw
     * @note this is set over the SPI inter, so the dir pin on the driver is not required.
     */
    bool direction;

    /** 
     * @brief SPI address of the stepper driver.
     */
    int address;

    /**
     * @brief Pin that is connected to the step pin of the driver.
     * @note This pin is used to send step pulses to the driver.
     */
    int step_pin;


    /**
     * @brief Pin that is connected to the enable pin of the driver.
     * @note This pin is used to enable/disable the driver.
     */
    int enable_pin;

    /**
     * @brief This stores the current position of the motor in degrees
     * 
     */
    float current_position = 0;

    /**
     * @brief This stores the fractional step that needs to be carried over to the next move.
     * 
     * As steps can only be taken in whole integer amounts, depending on the selected angle 
     * there may be a fractional step that needs to be carried over to the next move. This 
     * value is obtained by rounding down the number of steps to the nearest integer and storing
     * the remainder here. It can then be added to the number of steps to be taken for the next 
     * move to ensure that these small errors do not build up over time, causing a drift in the
     * position of the device.
     */
    float steps_remainder = 0;          // Fractional step that needs to be carried over to the next move.

};

