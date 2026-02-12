#ifndef ESP32_BI750_H
#define ESP32_BI750_H

#include <Arduino.h>

// Base header shared by all ITSC packets
struct ITSCHeader {
    char header[4];      // "ITSC"
    uint16_t deviceId;   // 0x0608 or 0x060B
    uint8_t wordIndex;   // The Multiplexer (0x33, 0x34, 0x35, 0x36)
} __attribute__((packed));

// Word 0x32: Firmware & Date (YY MM DD)
struct Packet32 {
    ITSCHeader head;
    uint8_t fwVer;       // Firmware version
    uint8_t year;        // Build Year (offset from 2000)
    uint8_t month;       // Build Month
    uint8_t day;         // Build Day
    char dvName[5];      // Device Name (e.g., "Bi750")
    uint8_t checksum;
} __attribute__((packed));

// Word 0x3A: Production Serial
struct Packet3A {
    ITSCHeader head;
    char prodCode[2];    // e.g., "B5"
    char prodWeek[2];    // e.g., "20"
    char serialId[4];    // e.g., "1741"
    uint8_t checksum;
} __attribute__((packed));

// Word 0x33: Vitals
struct Packet33 {
    ITSCHeader head;
    uint8_t percentage;
    uint8_t temperature; // °C
    uint8_t chargeMode; // 1 = Charging, 0 = Discharging
    uint8_t settingsMask;
    uint8_t reserved[4];
    uint8_t checksum;
} __attribute__((packed));

// Word 0x34: Electrical
struct Packet34 {
    ITSCHeader head;
    uint16_t input_mv;
    uint16_t input_ma;
    uint16_t output_mv;
    uint16_t output_ma;
    uint8_t checksum;
} __attribute__((packed));

// Word 0x35: Capacity
struct Packet35 {
    ITSCHeader head;
    uint16_t charge_mins;
    uint16_t discharge_mins;
    uint16_t capacity_mah;
    uint16_t reserved;
    uint8_t checksum;
} __attribute__((packed));

// Word 0x36: Config
struct Packet36 {
    ITSCHeader head;
    uint8_t beep;
    uint8_t led;
    uint8_t dvRun;
    uint8_t lvp;
    uint8_t ccs;
    uint8_t reserved[5];
    uint8_t checksum;
} __attribute__((packed));

#endif