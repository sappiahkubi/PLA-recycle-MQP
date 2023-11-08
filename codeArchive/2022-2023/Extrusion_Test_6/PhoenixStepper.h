/**
 * @file PhoenixStepper.h
 * @author Dominick Gravante (DJGravante@WPI.edu)
 * @brief MQP Team Phoenix from WPI
 * Move NEMA-17 Steppers with ease
 * @version 0.2
 * @date 2022-02-19
 *
 * @copyright Open Source 2022
 *
 */
#ifndef PhoenixStepper_h
#define PhoenixStepper_h

#include "Arduino.h"
#include "AccelStepper.h"
class PhoenixStepper
{
public:
    int stepsPerRev;   // # of steps to make one revolution
    int microstepping; // Microstepping:
    bool enabled = true;
    long absPosition = 0; // Absolute Position from now until restart
    /**
     * @brief Construct a new Phoenix Stepper object
     *
     * @param stepPin Arduino Pin for STP
     * @param dirPin Arduino Pin for DIR
     * @param enPin Arduino Pin for EN
     * @param microsteppingInput Microstepping; 1=full, 2=1/2, 4=1/4...
     */
    PhoenixStepper(int stepPin, int dirPin, int enPin, int microsteppingInput)
    {
        // Create AccelStepper Object
        // Define Microstepping, max speed
        // Set rpm, rev, etc
        // Set Pins:
        _stepPin = stepPin;
        _dirPin = dirPin;
        _enPin = enPin;
        // Create Stepper Object
        stepper = AccelStepper(1, _stepPin, _dirPin);
        // Set Microstepping
        microstepping = microsteppingInput;
        stepsPerRev = 200 * microstepping; // 200 steps per full rev
        stepper.setMaxSpeed(8 * stepsPerRev);
        Print();
    };
    /**
     * @brief Disables the motor for an amount of time. Waits for the
     * whole time.
     *
     * @param milliseconds How many ms to disable for
     */
    void disableTime(int milliseconds)
    {
        digitalWrite(_enPin, HIGH);
        delay(milliseconds);
        digitalWrite(_enPin, LOW);
    };

    /**
     * @brief Disables motor indefinitely
     *
     */
    void disable()
    {
        digitalWrite(_enPin, HIGH);
        enabled = false;
    }

    /**
     * @brief Enables motor indefinitely
     *
     */
    void enable()
    {
        digitalWrite(_enPin, LOW);
        enabled = true;
    }

    /**
     * @brief Runs a specified number of steps in a specified direction
     *
     * @param steps Number of steps to move (pos or neg)
     * @param speed Speed to move in steps per second
     * @param dir Direction (false = forward). Optional based on steps.
     * @return long Returns absolute position
     */
    long runSteps(long steps, long speed, bool dir = false)
    {
        // Convention: dir=0 is the default positive speed
        if (dir)
        {
            // if specified negative direction, make steps negative
            steps = -abs(steps);
        }
        // The desired position is the current one, plus the pos or neg
        // steps required to get there.
        long desiredPosition = stepper.currentPosition() + steps;
        // If it needs to go backwards, flip the speed.
        if (steps < 0)
        {
            speed = -speed; // flip speed to go in opposite direction
        }
        // Loop until we reach the desired position
        while (stepper.currentPosition() != desiredPosition)
        {
            stepper.setSpeed(speed);
            stepper.runSpeed();
        }
        // Set the object's absolute position to the current position.
        absPosition = stepper.currentPosition();
        return absPosition;
    };
    /**
     * @brief Runs the stepper motor for a specified number of revolutions
     * at a specified RPM
     *
     * @param rev Number of revolutions to move (can be fractional)
     * @param rpm Speed to move (based on Microstep)
     * @param dir false = CCW. Optional
     * @return long
     */
    long run(long rev, long rpm, bool dir = false)
    {
        runSteps(rev * stepsPerRev, rpm * stepsPerRev, dir);
    }

    /**
     * @brief Runs the motor to a specified angular position
     * Angular position is from 0 to 1 revolution of steps
     *
     * @param pos Desired position (angular)
     * @param speed How quickly to move (steps/sec)
     * @param runShortDir Defaults to running the shortest Direction
     * Set to False to run the long direction.
     * @return long Returns absolute position
     */
    long runPos(int pos, long speed, bool runShortDir = true)
    {
        // pos is the angular spot we want to be at
        // angPosition is the anguler spot we are at
        long angPosition = getAngPosition();
        // difference > 0 = needs to move forwards
        // difference < 0 = needs to move backwards
        long difference = pos - angPosition;
        long opp_difference = (difference > 0) ? difference - stepsPerRev : difference + stepsPerRev;
        // So now we have two directions, one that's larger than the other.
        // by default, let's go the smaller speed:
        if (runShortDir)
        {
            if (abs(difference) < abs(opp_difference))
            {
                runSteps(difference, speed);
            }
            else
            {
                runSteps(opp_difference, speed);
            }
        }
        else
        {
            // Run in long direction
            if (abs(difference) < abs(opp_difference))
            {
                runSteps(opp_difference, speed);
            }
            else
            {
                runSteps(difference, speed);
            }
        }
        return absPosition;
    };

    /**
     * @brief Sets the current absPosition and angPosition as zero
     * Also sets stepper current position to zero
     *
     */
    void Zero()
    {
        absPosition = 0;
        stepper.setCurrentPosition(0);
    };

    /**
     * @brief Get the Angular Position.
     * Angular Position is defined as from 0 to one full revolution
     * Analogous to (0, 360) degrees, but (0, stepsPerRev)
     *
     * @return long Positive Angular Position (0 to full step)
     */
    long getAngPosition()
    {
        // define _tempPosition as a variable holding the position to be
        // manipulated
        long _tempPosition = absPosition;
        // Continuously subtract stepsPerRev from absPosition
        while (_tempPosition >= stepsPerRev)
        {
            _tempPosition -= stepsPerRev;
        }
        // if absPosition is negative, this will bring it to positive:
        while (_tempPosition < 0)
        {
            _tempPosition += stepsPerRev;
        }
        // Now we have the _tempPosition between 0 and stepsPerRev.
        return _tempPosition;
    }

    void Print()
    {
        // Prints all aspects of the class out to Serial Console
        Serial.println("======\nPrinting Class:");
        Serial.print("\nStep Pin: ");
        Serial.print(_stepPin);
        Serial.print(" Dir Pin: ");
        Serial.print(_dirPin);
        Serial.print(" Enable Pin: ");
        Serial.print(_enPin);
        Serial.print("\nAbsolute Position: ");
        Serial.print(absPosition);
        Serial.print("\nAngular Position: ");
        Serial.print(getAngPosition());
        Serial.println();
    }

private:
    AccelStepper stepper; // AccelStepper Object
    int _stepPin;         // Step Pin on Arduino
    int _dirPin;          // Direction Pin on Arduino
    int _enPin;           // Enable Pin on Arduino
};
#endif
