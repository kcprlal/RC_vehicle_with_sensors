#pragma once
#include <stdint.h>

namespace kl
{
    class AS5600
    {
    public:
        bool begin();
        uint16_t getRawAngle();
        float getAngleDegrees();
        uint8_t magnetStatus();

    private:
        static constexpr uint8_t AS5600_ADDR = 0x36;
        static constexpr uint8_t REG_ANGLE_MSB = 0x0E;
        static constexpr uint8_t REG_ANGLE_LSB = 0x0F;
    };
}
