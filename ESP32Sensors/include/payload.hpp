#ifndef _PAYLOAD_H
#define _PAYLOAD_H
#include<cstdint>

typedef struct SensorData
{
    float pressure;
    float temperature;
    float humidity;
    int16_t ax, ay, az;
    uint16_t eco2;
    uint16_t tvoc;
} SensorData;

#endif