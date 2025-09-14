#include <cstdint>
#include <Arduino.h>

#ifndef _SGP30_H_
#define _SGP30_H_

namespace kl
{
    class SGP30
    {
    public:
        SGP30();
        void calibrateHumidity(float, float);
        bool begin();
        bool measureAQ();
        bool setHumidity();
        uint16_t getECO2() { return eCO2; }
        uint16_t getTVOC() { return TVOC; }

    private:
        uint16_t absoluteHumidity, eCO2, TVOC;
        bool writeCommand(uint16_t);
        bool readData(uint8_t *, uint8_t, uint8_t *, uint8_t);
        uint8_t genCRC(const uint8_t *, uint8_t);
        bool checkCRC(const uint8_t *, uint8_t);

        // stałe adresy rejestrów

        static constexpr uint8_t ADDRESS = 0x58;
        static constexpr uint16_t IAQ_INIT = 0x2003;
        static constexpr uint16_t MEASURE_IAQ = 0x2008;
        static constexpr uint16_t SET_ABSOLUTE_HUMIDITY = 0x2061;
    };
}

#endif