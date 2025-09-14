#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include "wificredentials.hpp"

#define PWMA 23
#define AIN2 22
#define AIN1 21
#define STBY 19
#define BIN1 5
#define BIN2 18
#define PWMB 4

WiFiUDP udp;
const int UDP_PORT = 4210;

char incomingPacket[50];

IPAddress local_IP(192, 168, 137, 50);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {

  // Konfiguracja pinów silników
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);  // włącz sterowanie

  WiFi.config(local_IP, gateway, subnet);
  // Połącz z siecią WiFi (laptop jako AP)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Start nasłuchiwania UDP
  udp.begin(UDP_PORT);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
    if (len > 0) incomingPacket[len] = '\0';

    int speedA, speedB, dirA, dirB;
    int parsed = sscanf(incomingPacket, "%d,%d,%d,%d", &speedA, &speedB, &dirA, &dirB);
    if (parsed == 4) {
      // Ustaw kierunki i prędkości
      digitalWrite(AIN1, dirA == 1);
      digitalWrite(AIN2, dirA == 0);
      digitalWrite(BIN1, dirB == 1);
      digitalWrite(BIN2, dirB == 0);

      analogWrite(PWMA, constrain(speedA, 0, 255));
      analogWrite(PWMB, constrain(speedB, 0, 255));
    } else {
      //error handling
    }
  }
}
