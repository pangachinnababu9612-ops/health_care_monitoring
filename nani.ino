#define BLYNK_TEMPLATE_ID "TMPLIAjddf20T5"
#define BLYNK_TEMPLATE_NAME "Temperature and Humidity Monitor"
#define BLYNK_AUTH_TOKEN "122RywymdfdddgGfMd1jkZ0STNhRQecR12ayq"

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Your_WiFi";
char pass[] = "Your_Password";


MAX30105 particleSensor;

#define ONE_WIRE_BUS 12
#define SDA_PIN 8
#define SCL_PIN 9
#define DHTPIN 4        // GPIO4
#define DHTTYPE DHT22   // Sensor type

DHT dht(DHTPIN, DHTTYPE);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define BUFFER_SIZE 100

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

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
void temperature(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  
}
void ds18b20(){
  sensors.requestTemperatures();

  float temperature = sensors.getTempCByIndex(0);
  Serial.println(temperature);
}
void setup()
{
  Serial.begin(115200);
  dht.begin();
  sensors.begin();
  Wire.begin(SDA_PIN, SCL_PIN);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("MAX30102 BPM + SpO2");

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("Sensor not found");
    while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x2F);
  particleSensor.setPulseAmplitudeIR(0x2F);
}

void loop()
{
 Blynk.run();
}
