#include <Arduino.h>
#include "bme280.hpp"
#include "sgp30.hpp"
#include "i2c.hpp"
#include "adxl345.hpp"
#include <WiFi.h>
#include "wificredentials.hpp"

kl::I2C i2c;
kl::BME280 bme280;
kl::SGP30 sgp30;
kl::ADXL345 adxl345;

WiFiClient client;

unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 1000; // 1 sekunda

IPAddress local_IP(192, 168, 137, 51);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);
const int port = 50001;
const char *host = "192.168.137.1";

typedef struct SensorData
{
  float pressure;
  float temperature;
  float humidity;
  int16_t ax, ay, az;
  uint16_t eco2;
  uint16_t tvoc;
} SensorData;

SensorData sensorPayload;

std::string fillPayload(bool, std::string);

void setup()
{
  Serial.begin(115200);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  client.connect(host, port);

  i2c.begin();
  bme280.init();

  if (!adxl345.begin())
  {
  }
  if (!adxl345.setRange(2))
  {
  }

  bme280.makeMeasurement();
  delay(100);
  if (!sgp30.begin())
  {
    fillPayload(false, "sgp30 error");
  }

  sgp30.calibrateHumidity(float(bme280.getHum()) / 1024, float(bme280.getTemp()) / 100);
}

void loop()
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
