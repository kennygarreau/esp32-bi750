#include "esp32-bi750.h"
#include "NimBLEDevice.h"

static NimBLEUUID serviceUUID("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
static NimBLEUUID charWriteUUID("49535343-1E4D-4BD9-BA61-23C647249616");
static NimBLEUUID charNotifyUUID("49535343-8841-43F4-A8D4-ECBE34729BB3");

static BLERemoteCharacteristic* pWriteChar;
static bool connected = false;
uint8_t currentWord = 0x32; // Start by asking for version info

int chargeMode = -1; // -1: Unknown, 1: Charging/Driving, 0: Discharging/Parking
uint16_t parkingTimeHours = 0;

void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    parseCellinkData(pData, length);
}

void pollBattery(uint8_t word) {
    if (!connected || !pWriteChar) return;
    uint8_t cmd[] = {0x49, 0x54, 0x53, 0x43, 0x00, 0x00, word, 0x00};
    pWriteChar->writeValue(cmd, sizeof(cmd), false);
}

void parseCellinkData(uint8_t* pData, size_t length) {

    // Check for "ITSC" header
    if (length < 15 || memcmp(pData, "ITSC", 4) != 0) return;

    uint8_t wordIndex = pData[6];

    switch (wordIndex) {
        case 0x32: {
            Packet32* p = (Packet32*)pData;
            Serial.printf("Device: %s | FW: %d | Build: 20%02d-%02d-%02d\n", 
                          p->dvName, p->fwVer, p->year, p->month, p->day);
            break;
        }
        case 0x3A: {
            Packet3A* p = (Packet3A*)pData;
            // Production code can be mapped (e.g., 'B' often means February)
            Serial.printf("Production ID: %.2s%.2s-%.4s\n", 
                          p->prodCode, p->prodWeek, p->serialId);
            break;
        }
        case 0x33: {
            Packet33* p = (Packet33*)pData;
            Serial.printf("STAT: %d%% | Temp: %dC | Mode: %s\n",
                p->percentage, p->temperature, (p->chargeMode == 1) ? "Driving" : "Parking");
            break;
        }
        case 0x34: {
            Packet34* p = (Packet34*)pData;
            // Byte swap needed because ESP32 is Little Endian but battery is Big Endian
            float inV = __builtin_bswap16(p->input_mv) / 1000.0;
            float outV = __builtin_bswap16(p->output_mv) / 1000.0;
            float outA = __builtin_bswap16(p->output_ma) / 1000.0;
            Serial.printf("ELEC: In: %.2fV | Out: %.2fV | Load: %.2fA\n", inV, outV, outA);
            break;
        }
        case 0x35: {
            Packet35* p = (Packet35*)pData;
            uint16_t cap = __builtin_bswap16(p->capacity_mah);
            uint16_t dTime = __builtin_bswap16(p->discharge_mins);
            Serial.printf("TIME: Remainder: %d Hours\n", dTime);
            Serial.printf("Capacity: %d mAh\n", cap);
            break;
        }
        case 0x36: {
            Packet36* p = (Packet36*)pData;
            Serial.printf("Settings -> Beep: %d, LED: %d\n", p->beep, p->led);
            break;
        }
        default:
            Serial.printf("Unknown Word Index: 0x%02X\n", wordIndex);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    BLEDevice::init("ESP32_Bi750_Proxy");
    // Start scan and connect logic
}

void loop() {
    static uint32_t lastPoll = 0;
    if (connected && millis() - lastPoll > 1000) {
        pollBattery(currentWord);

        // Get firmware and hardware name first
        if (currentWord == 0x32) currentWord = 0x3A; // obtain Production details   
        else if (currentWord == 0x3A) currentWord = 0x33; // Then jump to telemetry
        else if (currentWord == 0x33) currentWord = 0x34;
        else if (currentWord == 0x34) currentWord = 0x35;
        else currentWord = 0x33; // Repeat telemetry loop
        
        lastPoll = millis();
    }
}