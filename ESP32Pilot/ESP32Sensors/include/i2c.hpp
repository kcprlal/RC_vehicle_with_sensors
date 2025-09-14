#include <cstdint>
#include "driver/i2c.h"

#ifndef _I2C_H
#define _I2C_H
namespace kl
{
    class I2C
    {
    public:
        I2C(i2c_port_t i2c_num = I2C_NUM_0, uint8_t sda_pin = 21, uint8_t scl_pin = 22, uint32_t baud_rate = 100000);
        void begin();
        bool write(uint8_t, uint8_t, uint8_t);
        bool write(uint8_t, const uint8_t *, uint8_t);
        bool read(uint8_t, uint8_t, uint8_t *);
        bool read(uint8_t, const uint8_t *, uint8_t, uint8_t *, uint8_t);
        bool ping(uint8_t);

    private:
        i2c_port_t _i2c_num;
        uint8_t _sda_pin;
        uint8_t _scl_pin;
        uint32_t _baud_rate;
        i2c_config_t _conf;
    };
}
#endif