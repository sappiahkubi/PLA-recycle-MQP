#include Adafruit_MAX31856.h
#include Adafruit_MAX31856.h
#define DRDY_PIN 5
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);
void setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(10);
    Serial.println(MAX31856 thermocouple test);
    pinMode(DRDY_PIN, INPUT);
    if (!maxthermo.begin())
    {
        Serial.println(Could not initialize thermocouple.);
        while (1)
            delay(10);
    }
    maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
    Serial.print(Thermocouple type J);
    switch (maxthermo.getThermocoupleType())
    {
case MAX31856_TCTYPE_K Serial.println(J Type); break;
case MAX31856_VMODE_G8 Serial.println(Voltage x8 Gain mode); break;
case MAX31856_VMODE_G32 Serial.println(Voltage x8 Gain mode); break;
default Serial.println(Unknown); break;}
maxthermo.setConversionMode(MAX31856_CONTINUOUS);}

void loop() {
 The DRDY output goes low when a new conversion result is available
int count = 0;
while (digitalRead(DRDY_PIN)) {
if (count++  200) {
count = 0;
Serial.print(.);} }
Serial.println(maxthermo.readThermocoupleTemperature());}