#include <Arduino.h>
#include <WiFi.h>
#include "bme280.hpp"
#include "sgp30.hpp"
#include "i2c.hpp"
#include "adxl345.hpp"
#include "wificredentials.hpp"
#include "networkconfig.hpp"
#include "payload.hpp"

#define micPin 32
#define micsPin 33
#define micsEN 19

kl::I2C i2c;
kl::BME280 bme280;
kl::SGP30 sgp30;
kl::ADXL345 adxl345;

WiFiClient client;

unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 1000;

SensorData sensorPayload;

std::string fillPayload(bool, std::string);
void configWIFI(int);
void configSensors();
void sendData(const char*);

void setup()
{
  // Serial.begin(115200);
  // Serial.println("dnaknd");
  // i2c.begin();
  configWIFI(5);
  configSensors();
  // pinMode(micsPin, INPUT);
  // analogSetPinAttenuation(micsPin, ADC_11db);
}

void loop()
{
  while (true)
  {
    static int i = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastMeasurementTime >= measurementInterval)
    {
      lastMeasurementTime = currentMillis;
      if (bme280.get_status())
        bme280.makeMeasurement();
      if (adxl345.get_status())
        adxl345.measureAcceleration();

      if (i >= 15)
      {
        if (sgp30.get_status())
          sgp30.measureAQ();
      }
      i++;

      if (bme280.get_status())
      {
        sensorPayload.pressure = bme280.getPress();
        sensorPayload.temperature = float(bme280.getTemp()) / 100;
        sensorPayload.humidity = 64;
      }
      else
      {
        sensorPayload.pressure = 0;
        sensorPayload.temperature = 0;
        sensorPayload.humidity = 0;
      }

      if (adxl345.get_status())
      {
        sensorPayload.ax = adxl345.getX();
        sensorPayload.ay = adxl345.getY();
        sensorPayload.az = adxl345.getZ();
      }
      else
      {
        sensorPayload.ax = 0;
        sensorPayload.ay = 0;
        sensorPayload.az = 0;
      }

      if (sgp30.get_status())
      {
        sensorPayload.eco2 = sgp30.getECO2();
        sensorPayload.tvoc = sgp30.getTVOC();
      }
      else
      {
        sensorPayload.eco2 = 0;
        sensorPayload.tvoc = 0;
      }

      sensorPayload.soundlevel = analogRead(micPin);
      sensorPayload.co = 2000;

      if (i >= 60)
      {
        sensorPayload.co = analogRead(micsPin);
        // Serial.println(sensorPayload.co);
      }
      if (i % 60 == 0)
      {
        if (bme280.get_status() && sgp30.get_status())
          sgp30.calibrateHumidity(sensorPayload.humidity, sensorPayload.temperature);
        // w przypadku błędu bme280 ustaw wartości deafaultowe
        if (!bme280.get_status() && sgp30.get_status())
          sgp30.calibrateHumidity(60, 25);
      }
      if (client.connected())
      {
        client.print(fillPayload(true, "none").c_str());
        // Serial.println(fillPayload(true, "none").c_str());
      }
      else
      {
        client.connect(host, port);
      }
      // Serial.println(i2c.ping(0x76));
      // Serial.println(i2c.ping(0x77));
      // Serial.println(i2c.ping(0x58));
      // Serial.println(i2c.ping(0x36));
    }
    // lastMeasurementTime = currentMillis;
  }
}

std::string fillPayload(bool ok = true, std::string error = "none")
{
  if (ok)
  {
    std::string payload = "{";
    payload += "\"pressure\":" + std::to_string(sensorPayload.pressure) + ",";
    payload += "\"temperature\":" + std::to_string(sensorPayload.temperature) + ",";
    payload += "\"humidity\":" + std::to_string(sensorPayload.humidity) + ",";
    payload += "\"ax\":" + std::to_string(sensorPayload.ax) + ",";
    payload += "\"ay\":" + std::to_string(sensorPayload.ay) + ",";
    payload += "\"az\":" + std::to_string(sensorPayload.az) + ",";
    payload += "\"eco2\":" + std::to_string(sensorPayload.eco2) + ",";
    payload += "\"tvoc\":" + std::to_string(sensorPayload.tvoc) + ",";
    payload += "\"sound\":" + std::to_string(sensorPayload.soundlevel) + ",";
    payload += "\"co\":" + std::to_string(sensorPayload.co);
    payload += "}\n";

    return payload;
  }
  else
  {
    std::string payload = "{";
    payload += error;
    payload += "}\n";

    return payload;
  }
}

void configWIFI(int reconnects)
{
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (reconnects--)
  {
    delay(500);
  }
  client.connect(host, port);
  if (WiFi.status() != WL_CONNECTED)
  {
    sendData("WIFI could not connect\n");
    throw std::runtime_error("WIFI connection error");
  }
}

void configSensors()
{

  pinMode(micPin, INPUT);
  pinMode(micsPin, INPUT);
  pinMode(micsEN, OUTPUT);
  digitalWrite(micsEN, LOW);

  i2c.begin();
  i2c.ping(0x76) ? bme280.init() : bme280.set_notfunctional();
  if (!bme280.get_status())
  {
    sendData("BME280 initialization error\n");
  }

  i2c.ping(0x53) ? adxl345.begin() : adxl345.set_notfunctional();
  if (!adxl345.get_status())
  {
    sendData("ADXL345 initialization error\n");
  }
  if (!adxl345.setRange(2))
  {
    sendData("ADXL345 set range error\n");
  }

  if (bme280.get_status())
    bme280.makeMeasurement();
  delay(100);

  i2c.ping(0x58) ? sgp30.begin() : sgp30.set_notfunctional();
  if (!sgp30.get_status())
  {
    sendData("SGP30 initialization error\n");
  }

  if (bme280.get_status() && sgp30.get_status())
    sgp30.calibrateHumidity(64, float(bme280.getTemp()) / 100);
  // w przypadku błędu bme280 ustaw wartości deafaultowe
  if (!bme280.get_status() && sgp30.get_status())
    sgp30.calibrateHumidity(60, 25);
}

void sendData(const char* data)
{
  client.print(data);
}