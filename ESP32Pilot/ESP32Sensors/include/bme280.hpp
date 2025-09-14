#ifndef _BME280_H
#define _BME280_H

#include <cstdint>
namespace kl
{
    class BME280
    {
    public:
        BME280();
        void init();
        void makeMeasurement();
        uint32_t getPress() { return press; };
        int32_t getTemp() { return temp; };
        uint32_t getHum() { return hum; };

    private:
        // Kalibracje
        uint8_t dig_H1, dig_H3, utempdig;
        int8_t dig_H6;
        uint16_t dig_T1, dig_P1;
        int16_t dig_H2, dig_H4, dig_H5;
        int16_t dig_T2, dig_T3;
        int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

        // Zmienne pomocnicze
        int32_t t_fine, adc_T, adc_P, adc_H, temp;
        uint32_t press, hum;
        // metody do pomiarow
        void readCalibrations();
        void readPTH();
        int32_t compensateTemperature();
        uint32_t compensatePressure();
        uint32_t compensateHumidity();

        // Stałe adresy rejestrów
        static constexpr uint8_t PRESS_MSB = 0xf7;
        static constexpr uint8_t PRESS_LSB = 0xf8;
        static constexpr uint8_t PRESS_XLSB = 0xf9;
        static constexpr uint8_t TEMP_MSB = 0xfa;
        static constexpr uint8_t TEMP_LSB = 0xfb;
        static constexpr uint8_t TEMP_XLSB = 0xfc;
        static constexpr uint8_t HUM_MSB = 0xfd;
        static constexpr uint8_t HUM_LSB = 0xfe;

        static constexpr uint8_t ADDR_DIG_T1 = 0x88;
        static constexpr uint8_t ADDR_DIG_T2 = 0x8a;
        static constexpr uint8_t ADDR_DIG_T3 = 0x8c;

        static constexpr uint8_t ADDR_DIG_P1 = 0x8e;
        static constexpr uint8_t ADDR_DIG_P2 = 0x90;
        static constexpr uint8_t ADDR_DIG_P3 = 0x92;
        static constexpr uint8_t ADDR_DIG_P4 = 0x94;
        static constexpr uint8_t ADDR_DIG_P5 = 0x96;
        static constexpr uint8_t ADDR_DIG_P6 = 0x98;
        static constexpr uint8_t ADDR_DIG_P7 = 0x9a;
        static constexpr uint8_t ADDR_DIG_P8 = 0x9c;
        static constexpr uint8_t ADDR_DIG_P9 = 0x9e;

        static constexpr uint8_t ADDR_DIG_H1 = 0xa1;
        static constexpr uint8_t ADDR_DIG_H2 = 0xe1;
        static constexpr uint8_t ADDR_DIG_H3 = 0xe3;
        static constexpr uint8_t ADDR_DIG_H4 = 0xe4;
        static constexpr uint8_t ADDR_DIG_H5 = 0xe5;
        static constexpr uint8_t ADDR_DIG_H6 = 0xe7;

        static constexpr uint8_t CTRL_HUM = 0xf2;
        static constexpr uint8_t CTRL_MEAS = 0xf4;

        static constexpr uint8_t I2C_ADDR = 0x76;

        // Pomocnicze metody (do odczytu z rejestrów itd.)
        bool readRegister(uint8_t, uint8_t *);
        bool writeRegister(uint8_t, uint8_t);
        bool readRegisters(uint8_t, uint8_t *, uint8_t);
    };
}
#endif // _BME280_H
