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
        bool softReset();
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
        static constexpr uint16_t GET_IAQ_BASELINE = 0x2015;
        static constexpr uint16_t SET_IAQ_BASELINE = 0x201e;
        static constexpr uint16_t SET_ABSOLUTE_HUMIDITY = 0x2061;
        static constexpr uint16_t MEASURE_TEST = 0x2032;
        static constexpr uint16_t GET_FEATURE_SET = 0x202f;
        static constexpr uint16_t MEASURE_RAW = 0x2050;
        static constexpr uint16_t GET_TVOC_INCEPTIVE_BASELINE = 0x20b3;
        static constexpr uint16_t SET_TVOC_BASELINE = 0x2077;
        static constexpr uint16_t SOFT_RESET = 0x0006;
    };
}

#endif