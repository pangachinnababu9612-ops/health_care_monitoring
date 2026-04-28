#define BLYNK_TEMPLATE_ID "TMPL3VBqbX3jx"
#define BLYNK_TEMPLATE_NAME "smart health care"
#define BLYNK_AUTH_TOKEN "vRnF-xQnKkoLKGV8uB5RL9H1-2cYuduT"
/*
pin for the max is 6,7  (SDA,SCL)
pin for the ds18b20 is 2
pin for the dht22 is 4
*/

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

#define BUFFER_SIZE 100
// Blynk Credentials

char ssid[] = "sanath";
char pass[] = "";

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// MAX30102
MAX30105 particleSensor;


uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
 
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

BlynkTimer timer;

// 🔹 Function to read sensors
void sendData() {

  // DHT22
  float roomTemp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // DS18B20
  sensors.requestTemperatures();
  float bodyTemp = sensors.getTempCByIndex(0);

  // MAX30102 (simple read placeholder)
  
    for (byte i = 0; i < BUFFER_SIZE; i++)
  {
    while (particleSensor.available() == false)
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
      irBuffer,
      BUFFER_SIZE,
      redBuffer,
      &spo2,
      &validSPO2,
      &heartRate,
      &validHeartRate);

  if (validHeartRate && validSPO2)   //CHECK AND SHOW VALUE
  {
    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.println(" BPM");

    Serial.print("SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");
  }
  else
  {
    Serial.println("Reading not valid. Keep finger steady.");
  }


  // Send to Blynk
  Blynk.virtualWrite(V3, heartRate);
  Blynk.virtualWrite(V4, spo2);
  Blynk.virtualWrite(V2, bodyTemp);
  Blynk.virtualWrite(V0, roomTemp);
  Blynk.virtualWrite(V1, humidity);
   Serial.print(heartRate);
  // 🔴 Alert Conditions
  String alert = "";

  if (heartRate < 60 || heartRate > 100)
    alert += "BPM Abnormal ";

  if (spo2 < 95)
    alert += "SpO2 Low ";

  if (bodyTemp > 37.5)
    alert += "Fever ";

  if (alert == "")
    alert = "Normal";

  Blynk.virtualWrite(V5, alert);
}


void max30102(){


 Serial.println("Place finger...");

  for (byte i = 0; i < BUFFER_SIZE; i++)
  {
    while (particleSensor.available() == false)
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
      irBuffer,
      BUFFER_SIZE,
      redBuffer,
      &spo2,
      &validSPO2,
      &heartRate,
      &validHeartRate);

  if (validHeartRate && validSPO2)
  {
    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.println(" BPM");

    Serial.print("SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");
  }
  else
  {
    Serial.println("Reading not valid. Keep finger steady.");
  }

  delay(3000);


}

void setup() {
  Serial.begin(115200);

  dht.begin();
  sensors.begin();

  Wire.begin(6, 7);  //sda,scl

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("Sensor not found");
   // while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x2F);
  particleSensor.setPulseAmplitudeIR(0x2F);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // ⏱ Every 1 second
  timer.setInterval(1000L, sendData);
}

void loop() {
  Blynk.run();
  timer.run();
}
