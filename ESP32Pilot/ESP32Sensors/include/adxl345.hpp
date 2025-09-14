#ifndef _ADXL345_H
#define _ADXL345_H

#include <cstdint>

namespace kl
{
    class ADXL345
    {
    public:
        ADXL345();
        bool begin();
        bool measureAcceleration();
        bool setRange(uint8_t);
        int16_t getX() { return x; };
        int16_t getY() { return y; };
        int16_t getZ() { return z; };

    private:
        int16_t x, y, z;

        bool readRegister(uint8_t, uint8_t *);
        bool writeRegister(uint8_t, uint8_t);
        bool readRegisters(uint8_t, uint8_t *, uint8_t);

        static constexpr uint8_t ADDRESS = 0x53;
        static constexpr uint8_t POWER_CTL = 0x2d;
        static constexpr uint8_t DATA_FORMAT = 0x31;
        static constexpr uint8_t DATA_X0 = 0x32;
    };
}
#endif