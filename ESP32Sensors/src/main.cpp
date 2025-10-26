#include <Arduino.h>
#include <WiFi.h>
#include "bme280.hpp"
#include "sgp30.hpp"
#include "i2c.hpp"
#include "adxl345.hpp"
#include "wificredentials.hpp"
#include "networkconfig.hpp"
#include "as5600.hpp"
#include "payload.hpp"

#define micPin 25
#define micsPin 26

kl::I2C i2c;
kl::BME280 bme280;
kl::SGP30 sgp30;
kl::ADXL345 adxl345;
kl::AS5600 as5600;

WiFiClient client;

unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 1000;

SensorData sensorPayload;

std::string fillPayload(bool, std::string);
void configWIFI(int);
void configSensors();

void setup()
{
  Serial.begin(115200);
  configWIFI(5);
  configSensors();
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
      bme280.makeMeasurement();
      adxl345.measureAcceleration();
      // dodac do sgp poprawke na wilgotnosc tak co minute
      if (i >= 15)
      {
        sgp30.measureAQ();
      }
      i++;
      sensorPayload.pressure = bme280.getPress();
      sensorPayload.temperature = float(bme280.getTemp()) / 100;
      sensorPayload.humidity = float(bme280.getHum()) / 1024;
      sensorPayload.ax = adxl345.getX();
      sensorPayload.ay = adxl345.getY();
      sensorPayload.az = adxl345.getZ();
      sensorPayload.eco2 = sgp30.getECO2();
      sensorPayload.tvoc = sgp30.getTVOC();

      if (client.connected())
      {
        client.print(fillPayload(true, "none").c_str());
      }
      else
      {
        client.connect(host, port);
      }
    }
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
    payload += "\"tvoc\":" + std::to_string(sensorPayload.tvoc);
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
  if(WiFi.status() != WL_CONNECTED){
    fillPayload(false, "WIFI could not connect");
    throw std::runtime_error("WIFI connection error");
  }
}

void configSensors()
{

  pinMode(micPin, INPUT);
  pinMode(micsPin, INPUT);

  i2c.begin();
  bme280.init();

  if (!adxl345.begin())
  {
    fillPayload(false, "adxl begin error");
  }
  if (!adxl345.setRange(2))
  {
    fillPayload(false, "adxl set range error");
  }

  if (!as5600.begin())
  {
    fillPayload(false, "as5600 error");
  }

  bme280.makeMeasurement();
  delay(100);
  if (!sgp30.begin())
  {
    fillPayload(false, "sgp30 error");
  }

  sgp30.calibrateHumidity(float(bme280.getHum()) / 1024, float(bme280.getTemp()) / 100);
}