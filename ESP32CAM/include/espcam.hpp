#ifndef _ESPCAM_H
#define _ESPCAM_H

#include "camera_pins.hpp"
#include "esp_camera.h"

namespace kl
{
    class Esp32cam
    {
    public:
        Esp32cam(int, pixformat_t, framesize_t, int, int);
        esp_err_t getError() { return err; }
        bool begin();

    private:
        camera_config_t config;
        esp_err_t err = 0;
    };
}
#endif