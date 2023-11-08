/**
 * @file Diamatron.ino
 * @brief MQP Team Phoenix from WPI
 * Operate Phoenix MQP Plastic Recycling with ease
 * Also has SLR manually-coded in.
 * @version 0.5
 * @date 2022-02-19
 *
 * @copyright Open Source 2022
 *
 **/
// Needs an ADC to run - best to use the Adafruit ADS1015 board.
#include <Adafruit_ADS1X15.h>
#include <wire.h>
Adafruit_ADS1115 ads1115; // Construct an ads1115
// Use this to define your Analog Pin. Default is A0.
#define ANALOGPIN A0
float slr_slope;
float slr_intercept;
float input_voltage;
float temp;
void setup()
{
    Serial.begin(9600); // Turn on Serial Monitor
    // CALIBRATION DATA - add it all here
    float calibration_diameters[] = {1.57, 1.95, 2.35};   // Known Diameters
    float calibration_voltages[] = {27042, 29348, 30845}; // Known Output Volts
    float m = 3;                                          // Number of calibration diameters used
    // Perform Simple Linear Regression.
    slr_slope = slope(calibration_voltages, calibration_diameters, m);
    slr_intercept = intercept(calibration_voltages, calibration_diameters, m);
    pinMode(ANALOGPIN, INPUT); // Define ANALOGPIN as an input.
    ads1115.begin(0x48);       // Initialize ads1115 at address 0x48
    ads1115.setGain(GAIN_TWO);
}
void loop()
{
    int16_t adc0;
    adc0 = ads1115.readADC_SingleEnded(0);
    temp = slr_intercept + slr_slope * adc0;

    Serial.print(adc0);
    Serial.print(" -> ");
    Serial.print(temp, 3);
    Serial.println();
    delay(100);
}

float diameter(float voltage)
{
    // uses the simple linear regression model to return a diameter for a given voltage
    return slr_intercept + slr_slope * voltage;
}

float sum(float samples[], int m)
{
    // Sums all items in the list samples
    float total = 0.0;
    for (int i = 0; i < m; i++)
    {
        total += samples[i];
    }
    return total;
}
float average(float samples[], int m)
{
    // Averages all items in the list samples
    return sum(samples, m) / m;
}
float sd(float samples[], int m)
{
    // finds standard deviation of all items in the list samples
    float temp_mean = average(samples, m);
    float difference_square_sum = 0.0;
    for (int i = 0; i < m; i++)
    {
        difference_square_sum += sq(samples[i] - temp_mean);
    }
    return sqrt(difference_square_sum / (m - 1));
}
float slope(float x[], float y[], int m)
{
    // Calculates the slope of SLR:
    // B1 (slope) = sum((xi-X)(yi-Y))/sum(Xi-X)^2
    float mean_x = average(x, m);
    float mean_y = average(y, m);
    float numerator = 0.0;
    float denominator = 0.0;
    for (int i = 0; i < m; i++)
    {
        // Numerator = (xi-X) * (yi-Y)
        numerator += (x[i] - mean_x) * (y[i] - mean_y);
        // Denominator = (xi - X)^2
        denominator += sq(x[i] - mean_x);
    }
    return numerator / denominator;
}
float intercept(float x[], float y[], int m)
{
    // Calculates the intercept of SLR
    // B0 (intercept) = Y - B1*X
    float mean_x = average(x, m);
    float mean_y = average(y, m);
    float given_slope = slope(x, y, m);
    return