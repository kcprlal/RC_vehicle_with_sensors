#include "as5600.hpp"
#include "i2c.hpp"

extern kl::I2C i2c;

namespace kl
{
    bool AS5600::begin()
    {
        return i2c.ping(AS5600_ADDR);
    }

    uint16_t AS5600::getRawAngle()
    {
        uint8_t msb = 0, lsb = 0;
        if (!i2c.read(AS5600_ADDR, REG_ANGLE_MSB, &msb))
            return 0;
        if (!i2c.read(AS5600_ADDR, REG_ANGLE_LSB, &lsb))
            return 0;

        uint16_t angle = ((msb << 8) | lsb) & 0x0FFF;
        return angle;
    }

    float AS5600::getAngleDegrees()
    {
        uint16_t raw = getRawAngle();
        return (raw * 360.0f) / 4096.0f;
    }

    uint8_t AS5600::magnetStatus()
    {
        uint8_t status = 0;
        if (!i2c.read(AS5600_ADDR, 0x0B, &status))
            return 0xFF;
        return status;
    }
}