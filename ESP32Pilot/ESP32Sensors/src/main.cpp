#include <Arduino.h>
#include "wificredentials.hpp"
#include "bme280.hpp"
#include "sgp30.hpp"
#include "i2c.hpp"
#include "adxl345.hpp"
#include <WiFi.h>
#include <WebServer.h>

kl::I2C i2c;
kl::BME280 bme280;
kl::SGP30 sgp30;
kl::ADXL345 adxl345;

WebServer server(80);

unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 1000; // 1 sekunda

void handleRoot()
{
  String response = "{";
  response += "\"cisnienie\":" + String(bme280.getPress()) + ",";
  response += "\"temperatura\":" + String(float(bme280.getTemp()) / 100) + ",";
  response += "\"wilgotnosc\":" + String(float(bme280.getHum()) / 1024) + ",";
  response += "\"x\":" + String(adxl345.getX()) + ",";
  response += "\"y\":" + String(adxl345.getY()) + ",";
  response += "\"z\":" + String(adxl345.getZ()) + ",";
  response += "\"eco2\":" + String(float(sgp30.getECO2())) + ",";
  response += "\"tvoc\":" + String(float(sgp30.getTVOC()));
  response += "}";

  server.send(200, "application/json", response);
}

void setup()
{
  i2c.begin();
  Serial.begin(115200);
  bme280.init();

  if (!adxl345.begin())
  {
    Serial.println("adxl begin failed");
  }
  if (!adxl345.setRange(2))
  {
    Serial.println("adxl range failed");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Łączenie z Wi-Fi...");
  }

  Serial.println("Połączono z Wi-Fi");
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  bme280.makeMeasurement();
  delay(100);
  if (!sgp30.begin())
    Serial.println("SGP30: IAQ init failed!");
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
  }

  server.handleClient();
}
