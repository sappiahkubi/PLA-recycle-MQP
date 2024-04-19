// Wait 35 secs for system to start at first //

//#include "DHT.h"
//#define DHTPIN 37
//#define DHTTYPE DHT22
#include "DFRobot_MLX90614.h"
#include <Wire.h>

#define MLX90614_I2C_ADDR1 0x5A
#define MLX90614_I2C_ADDR2 0x5B
DFRobot_MLX90614_I2C sensor1(MLX90614_I2C_ADDR1, &Wire);  // instantiate an object to drive the sensor
DFRobot_MLX90614_I2C sensor2(MLX90614_I2C_ADDR2, &Wire);  // instantiate an object to drive the sensor

//DHT dht(DHTPIN, DHTTYPE);

//STEPPER MOTOR DEFINITIONS
#define DCPIN 9
#define DIRECTIONPIN 8
#define ENABLEPIN 7
#define WINDEPIN 3

const int fan1 = 12;  // fan1 pin
const int fan2 = 11;  // fan2 pin
int fanSpeed = 255;
const int fanBuffer = 25;
int buffer = 25;
//const int tempSensor = 37;
//int val = 0;
int previousMillis = 0;
float lastTemp1 = 0;
float lastTemp2 = 0;
const float desiredTemp = 65;  // These may need to be adjusted
const float minTemp = 60;
const float maxTemp = 70;

const float L = 0.00175;  // Current thickness of PLA in meters
const float k = 0.111;    // May need to be adjusted

int x;

float currentHeaterTemp = 20;
const float desiredHeaterTemp = 137;

void moveStepper() {
  // CONTROL STEPPER MOTOR
  for (int x = 0; x < 1000; x++) {  //this loop dictates the distance it travels (or maybe the time that it travels??)
    digitalWrite(DCPIN, LOW);  //pulse (move) the motor
    delayMicroseconds(250);    //wait (pause) for 100 microseconds (the shorter the pause the )

    digitalWrite(DCPIN, HIGH);  //stop the motor
    delayMicroseconds(250);     //wait (pause) for 100 microseconds
  }
  delayMicroseconds(100);
}

char mystr[10]; //Initialized variable to store recieved data

void setup() {

  // recive data from arduino writer
  Serial1.begin(115200);
  Serial.begin(9600);

  // initialize the sensor
  while (NO_ERR != sensor1.begin()) {
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
  }
  while (NO_ERR != sensor2.begin()) {
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
  }
  Serial.println("Begin ok!");
  sensor1.setEmissivityCorrectionCoefficient(1.0);
  sensor2.setEmissivityCorrectionCoefficient(1.0);
  sensor1.setMeasuredParameters(sensor1.eIIR100, sensor1.eFIR1024);
  sensor2.setMeasuredParameters(sensor2.eIIR100, sensor2.eFIR1024);
  sensor1.enterSleepMode();  // Needs to be done so that the temperature sensors work correctly
  delay(200);
  sensor1.enterSleepMode(false);
  delay(200);
  sensor2.enterSleepMode();
  delay(200);
  sensor2.enterSleepMode(false);
  delay(200);
  //dht.begin();
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  pinMode(WINDEPIN, OUTPUT);
  analogWrite(fan1, fanSpeed);
  analogWrite(fan2, fanSpeed);
  pinMode(ENABLEPIN, OUTPUT);        //enable, ENA
  pinMode(DCPIN, OUTPUT);            //pulse, PUL
  pinMode(DIRECTIONPIN, OUTPUT);     //direction, DIR, used to be pin 8
  digitalWrite(ENABLEPIN, HIGH);     //set to ENABLE
  digitalWrite(DIRECTIONPIN, HIGH);  //Set direction to be the opposite way
  delayMicroseconds(2000);
}

void loop() {
  Serial1.flush();
  // Serial.println(millis());
  if(currentHeaterTemp < desiredHeaterTemp){
    // Check if there is any incoming data only until desired temperature is reached
    if (Serial1.available() > 0) {
      // Read the incoming data and print it to the serial monitor 
      String incomingData = Serial1.readString();
      Serial.println(incomingData);
      Serial.println("incomingData");
      int position = incomingData.indexOf("Average_temp(C):");
      position = position + 16;
      String tempString = incomingData.substring(position);
      currentHeaterTemp = tempString.toFloat();
      Serial.print("Heat Temperature ");
      Serial.print(currentHeaterTemp);
    }
  }
  if (millis() - previousMillis >= 10000) {  // Only adjusts fan speed every 10 seconds unless the PLA is too cold or hot
    previousMillis = millis();

    analogWrite(WINDEPIN, 255);

    if (fanSpeed < 0) {
      fanSpeed = 0;
    } else if (fanSpeed > 255) {
      fanSpeed = 255;
    }
    float objectTemp1 = sensor1.getObjectTempCelsius();
    //objectTemp1 = 100; //testing
    float objectTemp2 = sensor2.getObjectTempCelsius();
    //objectTemp2 = 62; //testing
    float ambientTemp1 = sensor1.getAmbientTempCelsius();
    float ambientTemp2 = sensor2.getAmbientTempCelsius();
    //float temp = dht.readTemperature();
    Serial.print("plaTemp1(c):");
    Serial.print(objectTemp1);
    Serial.print(" , ");
    Serial.print("plaTemp2(c):");
    Serial.print(objectTemp2);
    Serial.print(" , ");
    // Serial.print("ambientTemp1:");
    // Serial.print(ambientTemp1);
    // Serial.print(" , ");
    // Serial.print("ambientTemp2:");
    // Serial.print(ambientTemp2);
    // Serial.print(" , ");
    //Serial.println(temp);
    if (objectTemp2 <= minTemp + 1) {
      buffer = 25;
    } else if (objectTemp2 >= maxTemp - 1) {
      buffer = 75;
    } else {
      buffer = fanBuffer;
    }
    float check1 = ((objectTemp1 + lastTemp1) / 2.0) * 100.0;
    float check2 = ((objectTemp2 + lastTemp2) / 2.0) * 100.0;
    // Serial.println(check1);
    // Serial.println(check2);
    if (check1 >= 5) {                                        // Only adjusts the fan speed if the entering temperature difference is larger than 5 percent
      float ambientTemp = (ambientTemp1 + ambientTemp2) / 2;  // + temp) / 3;
      Serial.print("Ambient_Temp(c):");
      Serial.print(ambientTemp);
      Serial.print(" , ");
      float A = L * 0.04;
      float Rt = L / (k * A);
      float maxCFM = (4.27 / 0.025) * L;
      float P = (objectTemp1 - ambientTemp) / Rt;
      // Serial.println(P);
      float Q = P / (desiredTemp - ambientTemp);
      // Serial.println(Q);
      float val = (Q / maxCFM) * 255;
      int newFanSpeed = int(round(val));
      Serial.print("FanSpeed(ft^3/min):");
      if(newFanSpeed < 1){
        Serial.println(0);
      }else{
        Serial.println(val);
      }
      float check3 = ((fanSpeed + newFanSpeed) / 2.0) * 100;
      if (check3 >= 5) {  // Only adjusts the fan speed if the fan speed difference is larger than 5 percent
        if (newFanSpeed + buffer <= 255) {
          analogWrite(fan1, newFanSpeed + buffer);
          analogWrite(fan2, newFanSpeed + buffer);
        } else {
          analogWrite(fan1, 255);
          analogWrite(fan2, 255);
        }
        if (newFanSpeed > 255) {
          fanSpeed = 255;
        } else {
          fanSpeed = newFanSpeed;
        }
      }
      lastTemp1 = objectTemp1;
      lastTemp2 = objectTemp2;
    } else if (check2 >= 5 && previousMillis >= 60000) {  // Still adjusts the fan speed if the exiting temperature difference is larger than 5 percent
      if (objectTemp2 - desiredTemp < 0) {
        int tempSpeed = fanSpeed - 10;
        if (tempSpeed < 25) {
          analogWrite(fan1, 25);
          analogWrite(fan2, 25);
          fanSpeed = 25;
        } else {
          analogWrite(fan1, tempSpeed + buffer);
          analogWrite(fan2, tempSpeed + buffer);
          fanSpeed = tempSpeed;
        }
      } else if (objectTemp2 - desiredTemp > 0) {
        int tempSpeed = fanSpeed + 10;
        if (tempSpeed > 255) {
          analogWrite(fan1, 255);
          analogWrite(fan2, 255);
          fanSpeed = 255;
        } else {
          analogWrite(fan1, tempSpeed + buffer);
          analogWrite(fan2, tempSpeed + buffer);
          fanSpeed = tempSpeed;
        }
        lastTemp2 = objectTemp2;
      }
    }
  } else {  // Adjusts the speed of the fan if the exiting temperature gets too cold or hot when not in the periodic 10 second checker
    float objectTemp2 = sensor2.getObjectTempCelsius();

    analogWrite(WINDEPIN, 0);

    //objectTemp2 = 62; //testing
    if (objectTemp2 <= minTemp) {
      fanSpeed = 25;
      analogWrite(fan1, fanSpeed);
      analogWrite(fan2, fanSpeed);
    } else if (objectTemp2 >= maxTemp) {
      fanSpeed = 75;
      analogWrite(fan1, fanSpeed);
      analogWrite(fan2, fanSpeed);
    }
  }

  

  // CONTROL AUGUR MOTOR
  if (currentHeaterTemp >= desiredHeaterTemp){
    moveStepper();
  }
  //delay(1000);
}