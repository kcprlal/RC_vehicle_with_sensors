#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <camera_pins.hpp>
#include <Arduino.h>

const char *ssid = "ssid";
const char *password = "password";

esp_err_t err = 0;

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

void caminit()
{
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  // // problem gdzies tu
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;

  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Jeśli nie masz PSRAM, ustaw odpowiednie wartości
  config.frame_size = FRAMESIZE_VGA; // 160x120
  config.fb_count = 2;
  config.jpeg_quality = 10; // Liczba buforów (1, jeśli nie masz PSRAM)
  // Inicjalizacja kamery
  err = esp_camera_init(&config);

  Serial.println((int)err);
  camera_fb_t *fb = esp_camera_fb_get(); // wstępny "warm-up shot"
  if (fb)
    esp_camera_fb_return(fb);
}

void setup()
{
  Serial.begin(115200);
  delay(10000);
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
  caminit();

  server.on("/", handle_root);
  server.on("/jpg", HTTP_GET, handle_jpg);

  server.begin();
}

void loop()
{
  server.handleClient();
}
