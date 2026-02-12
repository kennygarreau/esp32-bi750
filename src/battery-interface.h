#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declare batteryData so other files can access it
typedef struct {
    uint16_t capacity;
    uint8_t percentage;
    uint8_t temperature;
    uint8_t chargeMode;
    float inV;
    float outV;
    float inA;
    float outA;
    bool needsRefresh;
    char deviceName[6];
    char fwVer[4];
    char chargeMinutes[6];
    char dischargeMinutes[6];
} BatteryData;

extern BatteryData batteryData;
//#define DEBUG

void set_0x33_data(uint8_t percentage, uint8_t temperature, uint8_t chargeMode);
void set_0x34_data(float inV, float outV, float outA);

#ifdef __cplusplus
}
#endif
