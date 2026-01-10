#include "sgp30.hpp"
#include "i2c.hpp"
#include "bme280.hpp"

extern kl::I2C i2c;
extern kl::BME280 bme280;
extern HardwareSerial serial;

namespace kl
{
    SGP30::SGP30() : absoluteHumidity(0), TVOC(0), eCO2(0) {}
    // pozniej mozna dodac funkcje zeby aktualizowac wilgotnosc co iles czasu
    void SGP30::calibrateHumidity(float hum, float temp)
    {
        float absH = 216.7 * ((hum / 100) * 6.112 * pow(2.72, (17.62 * temp) / (243.12 + temp)) / (273.15 + temp));
        absoluteHumidity = static_cast<uint16_t>(absH * 256.0f);
        setHumidity();
    }

    void SGP30::begin()
    {
        writeCommand(IAQ_INIT);
        delay(10);
    }

    bool SGP30::measureAQ()
    {
        uint8_t cmd[2] = {MEASURE_IAQ >> 8, MEASURE_IAQ & 0xFF};
        uint8_t data[6];
        if (!readData(cmd, 2, data, 6))
        {
            Serial.println("SGP30: readData failed\n");
            return false;
        }

        if (!checkCRC(data, data[2]) || !checkCRC(&data[3], data[5]))
        {
            Serial.println("SGP30: CRC error\n");
            return false;
        }

        this->eCO2 = (data[0] << 8) | data[1];
        this->TVOC = (data[3] << 8) | data[4];

        return true;
    }

    bool SGP30::setHumidity()
    {
        uint8_t data[2] = {static_cast<uint8_t>(absoluteHumidity >> 8), absoluteHumidity & 0xFF};
        uint8_t crc = genCRC(data, 2);
        uint8_t buff[5] = {
            static_cast<uint8_t>(SET_ABSOLUTE_HUMIDITY >> 8),
            static_cast<uint8_t>(SET_ABSOLUTE_HUMIDITY & 0xFF),
            data[0], data[1], crc};
        return i2c.write(ADDRESS, buff, 3);
    }

    bool SGP30::writeCommand(uint16_t command)
    {
        uint8_t cmd[2] = {static_cast<uint8_t>(command >> 8), static_cast<uint8_t>(command & 0xFF)};
        return i2c.write(ADDRESS, cmd, 2);
    }

    bool SGP30::readData(uint8_t *cmd, uint8_t cmd_len, uint8_t *buf, uint8_t len)
    {
        return i2c.read(ADDRESS, cmd, cmd_len, buf, len);
    }

    uint8_t SGP30::genCRC(const uint8_t *data, uint8_t len)
    {
        uint8_t crc = 0xFF;
        for (uint8_t i = 0; i < len; ++i)
        {
            crc ^= data[i];
            for (int8_t j = 0; j < 8; ++j)
            {
                if (crc & 0x80)
                    crc = (crc << 1) ^ 0x31;
                else
                    crc <<= 1;
            }
        }
        return crc;
    }

    bool SGP30::checkCRC(const uint8_t *data, uint8_t crc)
    {
        return genCRC(data, 2) == crc;
    }
}