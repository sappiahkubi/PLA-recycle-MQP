/**
 * @file Extrusion_Test_6.ino
 * @brief MQP Team Phoenix from WPI
 * Operate Phoenix MQP Plastic Recycling with ease
 * @version 0.2
 * @date 2022-02-23
 *
 * @copyright Open Source 2022
 *
 **/
#include <Adafruit_MAX31856.h>
#include "PhoenixStepper.h"
// Place PhoenixStepper.h in the same folder as this .ino file.
// Define the Thermocouple
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(3, 4, 5, 6);
float tempMid;
// Set up Steppers with pinout StepPin, DirectionPin, EnablePin, Microstepping
PhoenixStepper augerStepper(11, 10, 12, 16);
PhoenixStepper tempStepper(8, 7, 9, 16);
// positive spin (false) moves in +percent

// CONFIGURATION
int desired_temp = 220; // The temperature it tries to get
int temp_above = desired_temp + 5;
int temp_below = desired_temp - 5;
int temp_off = desired_temp + 15;
float current_percent = 0.5;
float increment = 0.05;
bool augerEnable = false; // Enable Auger Screw.

void setup()
{
  Serial.begin(9600);
  // Ensure Thermocouple is set up.
  if (!maxthermo.begin())
  {
    Serial.println("Could not initialize thermocouple.");
    while (1)
      delay(10);
  }
  // Set up Thermocouple settings:
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
  maxthermo.setConversionMode(MAX31856_CONTINUOUS);
}

void loop()
{
  // Take temperature; assign to tempMid float
  tempMid = maxthermo.readThermocoupleTemperature();
  Serial.print(tempMid);
  // if Temperature is between +- 1 degree of desired:
  if (desired_temp - 1 < tempMid && tempMid < desired_temp + 1)
  {
    // Do not do anything. Temperature is in a happy range.
  }
  // if Temperature is above this range:
  else if (tempMid > desired_temp && current_percent >= 0)
  {
    // if temp is greater than 10 degrees away, move 20% down.
    if (tempMid > desired_temp + 10)
    {
      increment = 0.2;
    }
    else
    {
      increment = 0.05;
    } // Otherwise move 5% down.
    current_percent += -increment;
    // Move Temperature Stepper
    tempStepper.enable();
    tempStepper.runSteps(increment * tempStepper.stepsPerRev, tempStepper.stepsPerRev, false);
    tempStepper.disable();
    Serial.print(" Turning heat to ");
    Serial.print(current_percent, 2);
  }
  else if (tempMid < desired_temp && current_percent <= 0.875)
  {
    // if temp is greater than 10 degrees away, move 20% up.
    if (tempMid < desired_temp - 10)
    {
      increment = 0.2;
    }
    else
    {
      increment = 0.05;
    } // Otherwise move 5% down.
    current_percent += increment;
    // Move Temperature Stepper
    tempStepper.enable();
    tempStepper.runSteps(increment * tempStepper.stepsPerRev, tempStepper.stepsPerRev, true);
    tempStepper.disable();
    Serial.print(" Turning heat to ");
    Serial.print(current_percent, 2);
  }

  else if (tempMid < 1.01)
  {
    // If temperature is below 1 C, something is wrong
    if (tempStepper.getAngPosition() != 0)
    {
      tempStepper.enable();
      tempStepper.runPos(0, tempStepper.stepsPerRev);
      tempStepper.disable();
      Serial.print(" Temperature is too low, turning off stepper");
    }
  }
  Serial.println(); // Print an empty line
  // Auger
  if (tempMid > desired_temp - 15 && augerEnable)
  {
    for (int i = 0; i < 15; i++)
    { // Run Auger for 30 seconds, 15 revolutions at half a rev per sec
          augerStepper.enable();
      augerStepper.runSteps(augerStepper.stepsPerRev, augerStepper.stepsPerRev * 1, true);
    }
  }
  else
  {
    delay(30000);
    augerStepper.disable();
  }
}