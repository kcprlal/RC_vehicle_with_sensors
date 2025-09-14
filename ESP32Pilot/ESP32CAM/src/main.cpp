#include "esp_camera.h"
#include "espcam.hpp"
#include "wificredentials.hpp"
#include <WiFi.h>
#include <WebServer.h>
#include <camera_pins.hpp>
#include <Arduino.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
kl::Esp32cam cam(20000000, PIXFORMAT_JPEG, FRAMESIZE_VGA, 2, 10);

WebServer server(80);

void handle_jpg()
{
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void handle_root()
{
  server.send(200, "text/html", "<html><body><h1>fotaaaa</h1><img src='/jpg' style='width:100%;'/></body></html>");
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Przed inicjalizacją kamery!");

  // Połączenie z WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("Łączenie z Wi-Fi...");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  delay(1000);

  server.on("/", handle_root);
  server.on("/jpg", HTTP_GET, handle_jpg);

  server.begin();

  if (!cam.begin())
  {
    Serial.println("cam error");
  }
}

void loop()
{
  server.handleClient();
}
