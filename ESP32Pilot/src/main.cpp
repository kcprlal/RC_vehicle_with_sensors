#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include "wificredentials.hpp"
#include "pins.hpp"

void configWIFI(int);
void configPins();

WiFiUDP udp;

char incomingPacket[50];

void setup()
{

  Serial.begin(115200);

  configWIFI(5);
  configPins();
}

void loop()
{
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
    if (len > 0)
      incomingPacket[len] = '\0';

    int speedA, speedB, dirA, dirB;
    int parsed = sscanf(incomingPacket, "%d,%d,%d,%d", &speedA, &speedB, &dirA, &dirB);
    Serial.println(incomingPacket);
    if (parsed == 4)
    {
      digitalWrite(AIN1, dirA == 1);
      digitalWrite(AIN2, dirA == 0);
      digitalWrite(BIN1, dirB == 1);
      digitalWrite(BIN2, dirB == 0);

      analogWrite(PWMA, constrain(speedA, 0, 255));
      analogWrite(PWMB, constrain(speedB, 0, 255));
    }
    else
    {
      // error handling
    }
  }
}

void configWIFI(int retries)
{

  WiFi.config(local_IP, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (retries--)
  {
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    throw std::runtime_error("WIFI connection error");
  }
  udp.begin(UDP_PORT);
}

void configPins()
{
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
}
