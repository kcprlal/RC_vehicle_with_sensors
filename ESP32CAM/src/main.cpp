#include "esp_camera.h"
#include "espcam.hpp"
#include "wificredentials.hpp"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <camera_pins.hpp>
#include <Arduino.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
kl::Esp32cam cam(20000000, PIXFORMAT_JPEG, FRAMESIZE_QVGA, 2, 15);

WiFiUDP udp;
const char *udpAddress = "192.168.137.1"; 
const int udpPort = 1234;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  cam.begin();
  udp.begin(udpPort);
}

void loop()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  size_t packetSize = 512;
  uint16_t totalPackets = (fb->len + packetSize - 1) / packetSize;

  for (uint16_t i = 0; i < totalPackets; i++)
  {
    size_t chunkSize = (i < totalPackets - 1) ? packetSize : (fb->len - i * packetSize);

    udp.beginPacket(udpAddress, udpPort);

    udp.write((uint8_t *)&i, sizeof(i));
    udp.write((uint8_t *)&totalPackets, sizeof(totalPackets));

    udp.write(fb->buf + i * packetSize, chunkSize);

    udp.endPacket();
    delay(1);
  }

  esp_camera_fb_return(fb);
  delay(100);
}

