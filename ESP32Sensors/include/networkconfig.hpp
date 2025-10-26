#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H
#include <Arduino.h>

const IPAddress local_IP(192, 168, 137, 51);
const IPAddress gateway(192, 168, 137, 1);
const IPAddress subnet(255, 255, 255, 0);
const int port = 50001;
const char *host = "192.168.137.1";

#endif