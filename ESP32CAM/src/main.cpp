#include "esp_camera.h"
#include "espcam.hpp"
#include "wificredentials.hpp"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <camera_pins.hpp>
#include <Arduino.h>

kl::Esp32cam cam(20000000, PIXFORMAT_JPEG, FRAMESIZE_QVGA, 2, 15);

WiFiUDP udp;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  cam.begin();
  udp.begin(udpPort);
}

void loop()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
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

