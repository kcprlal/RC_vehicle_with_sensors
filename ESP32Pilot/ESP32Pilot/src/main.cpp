#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "wificredentials.hpp"
#include <esp_now.h>

WebServer server(80);
#define PWMA 23
#define AIN2 22
#define AIN1 21
#define STBY 19
#define BIN1 5
#define BIN2 18
#define PWMB 4

struct SensorData
{
  float pressure;
  float temperature;
  float humidity;
  int16_t ax, ay, az;
  uint16_t eco2;
  uint16_t tvoc;
};

SensorData lastData;

void onReceiveData(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(SensorData))
  {
    memcpy(&lastData, incomingData, sizeof(SensorData));
  }
  else
  {
    lastData.pressure = -1;
    lastData.temperature = -1;
    lastData.humidity = -1;
  }
}

void moveCommands();

void setup()
{
  Serial.begin(115200);

  IPAddress local_IP(192, 168, 1, 123);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Błąd inicjalizacji ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onReceiveData);

  // sterowanie silnikiem
  digitalWrite(STBY, HIGH);
  moveCommands();

  server.begin();
}

void loop()
{
  server.handleClient();
}

void moveCommands()
{
  server.on("/forward", []()
            {
              int speedA = constrain(server.arg("speedA").toInt(), 0, 255);
              int speedB = constrain(server.arg("speedB").toInt(), 0, 255);
              
              digitalWrite(STBY, HIGH);

              digitalWrite(AIN1, HIGH);
              digitalWrite(AIN2, LOW);
              digitalWrite(BIN1, HIGH);
              digitalWrite(BIN2, LOW);

              analogWrite(PWMA, speedA);
              analogWrite(PWMB, speedB);
            });

  server.on("/backward", []()
            {
              int speedA = constrain(server.arg("speedA").toInt(), 0, 255);
              int speedB = constrain(server.arg("speedB").toInt(), 0, 255);
              
              digitalWrite(STBY, HIGH);

              digitalWrite(AIN1, LOW);
              digitalWrite(AIN2, HIGH);
              digitalWrite(BIN1, LOW);
              digitalWrite(BIN2, HIGH);

              analogWrite(PWMA, speedA);
              analogWrite(PWMB, speedB);
            });

  server.on("/turnleft", []()
            {
              int speedA = constrain(server.arg("speedA").toInt(), 0, 255);
              int speedB = constrain(server.arg("speedB").toInt(), 0, 255);
              
              digitalWrite(STBY, HIGH);

              digitalWrite(AIN1, LOW); // silnik A wstecz
              digitalWrite(AIN2, HIGH);
              digitalWrite(BIN1, HIGH); // silnik B do przodu
              digitalWrite(BIN2, LOW);

              analogWrite(PWMA, speedA);
              analogWrite(PWMB, speedB);
            });

  server.on("/turnright", []()
            {
              int speedA = constrain(server.arg("speedA").toInt(), 0, 255);
              int speedB = constrain(server.arg("speedB").toInt(), 0, 255);
              
              digitalWrite(STBY, HIGH);

              digitalWrite(AIN1, HIGH); // silnik A do przodu
              digitalWrite(AIN2, LOW);
              digitalWrite(BIN1, LOW); // silnik B wstecz
              digitalWrite(BIN2, HIGH);

              analogWrite(PWMA, speedA);
              analogWrite(PWMB, speedB);
            });

  server.on("/stop", []()
            { digitalWrite(STBY, LOW);
            analogWrite(PWMA, 0);
            analogWrite(PWMB, 0); });

  // server.on("/start", []()
  //           { 
  //           analogWrite(PWMA, 0);
  //           analogWrite(PWMB, 0); });

  server.on("/sensors", []()
            {
  String json = "{";
  json += "\"pressure\":" + String(lastData.pressure) + ",";
  json += "\"temperature\":" + String(lastData.temperature) + ",";
  json += "\"humidity\":" + String(lastData.humidity) + ",";
  json += "\"x\":" + String(lastData.ax) + ",";
  json += "\"y\":" + String(lastData.ay) + ",";
  json += "\"z\":" + String(lastData.az) + ",";
  json += "\"eco2\":" + String(lastData.eco2) + ",";
  json += "\"tvoc\":" + String(lastData.tvoc);
  json += "}";
  server.send(200, "application/json", json); });
}