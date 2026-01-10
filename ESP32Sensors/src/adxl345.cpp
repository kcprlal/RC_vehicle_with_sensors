#include "adxl345.hpp"
#include "i2c.hpp"
#include <Arduino.h>

extern kl::I2C i2c;

namespace kl
{
    ADXL345::ADXL345() {}
    void ADXL345::begin()
    {
        writeRegister(POWER_CTL, 0x08);
    }

    bool ADXL345::setRange(uint8_t range)
    {
        uint8_t mode = 0;
        if (!readRegister(DATA_FORMAT, &mode))
        {
            return false;
        }

        mode &= ~0x0f;
        mode |= 0x08;

        switch (range)
        {
        case 2:
            break;
        case 4:
            mode |= 0x01;
            break;
        case 8:
            mode |= 0x02;
            break;
        case 16:
            mode |= 0x03;
            break;
        default:
            return false;
        }
        return writeRegister(DATA_FORMAT, mode);
    }

    bool ADXL345::measureAcceleration()
    {
        uint8_t buf[6];
        if (!readRegisters(DATA_X0, buf, 6))
        {
            return false;
        }
        x = (int16_t)((buf[1] << 8) | buf[0]);
        y = (int16_t)((buf[3] << 8) | buf[2]);
        z = (int16_t)((buf[5] << 8) | buf[4]);
        return true;
    }

    bool ADXL345::readRegister(uint8_t reg, uint8_t *data)
    {
        return i2c.read(ADDRESS, reg, data);
    }

    bool ADXL345::writeRegister(uint8_t reg, uint8_t value)
    {
        return i2c.write(ADDRESS, reg, value);
    }

    bool ADXL345::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length)
    {
        for (uint8_t i = 0; i < length; ++i)
        {
            if (!i2c.read(ADDRESS, reg + i, &buffer[i]))
            {
                return false;
            }
        }
        return true;
    }
}