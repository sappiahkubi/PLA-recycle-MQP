//////////////////////////////////////////////////////////////////
//
//https://learn.adafruit.com/thermistor/using-a-thermistor
//////////////////////////////////////////////////////////////////

// Heater Switch Pin
#define HEATPIN 3
// which analog pin to connect
#define THERMISTORPIN1 A0
#define THERMISTORPIN2 A2
// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples1 to take and average1, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 20
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 100000

int heatVoltCap = 0;
int largeNum = 36000000000000;
float temp1 = 0;
float temp2 = 0;

// return average of 5 analog values
float getAverage(int THERMISTORPIN) {
  float average;
  int samples[NUMSAMPLES];

  // take N samples in a row, with a slight delay
  for (uint8_t i = 0; i < NUMSAMPLES; i++) {
    samples[i] = analogRead(THERMISTORPIN);
    delay(10);
  }

  // average all the samples out
  average = 0;
  for (uint8_t i = 0; i < NUMSAMPLES; i++) {
    average += samples[i];
  }

  return average /= NUMSAMPLES;
}

// convert the analog value to resistance
float val2Res(float value) {
  value = 1023 / value;
  value = SERIESRESISTOR / value;

  float steinhart;
  steinhart = value / THERMISTORNOMINAL;             // (R/Ro)
  steinhart = log(steinhart);                        // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                         // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);  // + (1/To)
  steinhart = 1.0 / steinhart;                       // Invert
  steinhart -= 273.15;                               // convert absolute temp to C

  return steinhart;
}

void setup() {
  Serial.begin(115200);
  pinMode(HEATPIN, OUTPUT);
}

void loop() {

  Serial.flush();

  temp1 = val2Res(getAverage(THERMISTORPIN1));
  temp2 = val2Res(getAverage(THERMISTORPIN2));
  float avgTemp = (temp1 + temp2) / 2;

  if (avgTemp >= 139.0){

    delay(15);

    heatVoltCap = 0;
    // Serial.println("HEAT OFF, LOW");
    analogWrite(HEATPIN, LOW);  // turn off

    // if(Serial.availableForWrite()){
      Serial.print("Peak Temp Reached: ");
      Serial.print("Temp1:");
      Serial.print(temp1);
      Serial.print(" , ");
      Serial.print("Temp2:");
      Serial.print(temp2);
      Serial.print(" , ");
      Serial.print("Average_temp(C):");
      Serial.println((temp1+temp2)/2);
      // Serial.println(analogRead(HEATPIN));

    // }

  }else{
    heatVoltCap = 0;
    int logVal = HIGH; // HIGH = ON , LOW == OFF
    digitalWrite(HEATPIN, logVal); // turn on cartidge heater

    // if(Serial.availableForWrite()){
      // Serial.print("HEAT ON,"); 
      // if(logVal == HIGH){
      //   Serial.println("HIGH");
      // }else if (logVal == LOW) {
      //   Serial.println("LOW");
      // }
      
      Serial.print("Temp1:");
      Serial.print(temp1);
      Serial.print(" , ");
      Serial.print("Temp2:");
      Serial.print(temp2);
      Serial.print(" , ");
      Serial.print("Average_temp(C):");
      Serial.println((temp1+temp2)/2);
      // Serial.println(digitalRead(HEATPIN));
      // Serial.println(analogRead(HEATPIN));
    // }  
  }

  delay(1000);
  ///
}
