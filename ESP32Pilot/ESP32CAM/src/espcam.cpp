#include "espcam.hpp"
namespace kl
{
    Esp32cam::Esp32cam(int freq, pixformat_t format, framesize_t fsize, int fbcount, int quality)
    {
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;

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

        config.xclk_freq_hz = freq;
        config.pixel_format = format;

        config.frame_size = fsize; 
        config.fb_count = fbcount;
        config.jpeg_quality = quality;
    }

    bool Esp32cam::begin(){
        return ESP_OK == esp_camera_init(&config);
    }
}
