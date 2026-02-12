#include "esp32-bi750.h"
#include "battery-interface.h"
#include "NimBLEDevice.h"
#include "init.h"
#include "lvgl.h"
#include "ui/ui.h"

static NimBLEUUID serviceUUID("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
static NimBLEUUID charNotifyUUID("49535343-1E4D-4BD9-BA61-23C647249616");
static NimBLEUUID charWriteUUID("49535343-8841-43F4-A8D4-ECBE34729BB3");

NimBLEClient* pClient = nullptr;
NimBLEAdvertisedDevice* advDevice;
NimBLERemoteCharacteristic* pWriteChar;
NimBLERemoteCharacteristic* pNotifyChar;

bool connected = false;
bool doConnect = false;

uint8_t currentWord = 0x32; // Start by asking for version info
int chargeMode = -1; // -1: Unknown, 1: Charging/Driving, 0: Discharging/Parking
uint8_t batteryPercent = 0;

//BatteryData batteryData = {0, 0, 0, 0, 0, 0, 0, false};
BatteryData batteryData;

RTC_DATA_ATTR uint8_t hoursParked = 0;

void pollBattery(uint8_t word) {
    if (!connected || !pWriteChar) return;
    uint8_t cmd[] = {0x49, 0x54, 0x53, 0x43, 0x00, 0x00, word, 0x00};
    pWriteChar->writeValue(cmd, sizeof(cmd), true);
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

            if (p != nullptr && p->dvName != nullptr) {
                strncpy(batteryData.deviceName, p->dvName, sizeof(batteryData.deviceName) - 1);
                batteryData.deviceName[sizeof(batteryData.deviceName) - 1] = '\0';
            } else {
                batteryData.deviceName[0] = '\0'; // Empty string if invalid
            }
            sprintf(batteryData.fwVer, "%d.%d", p->fwVer / 10, p->fwVer % 10);
            break;
        }
        case 0x3A: {
            Packet3A* p = (Packet3A*)pData;
            Serial.printf("Production ID: %.2s%.2s-%.4s\n", 
                          p->prodCode, p->prodWeek, p->serialId);
            break;
        }
        case 0x33: {
            Packet33* p = (Packet33*)pData;
            batteryPercent = p->percentage;
            batteryData.percentage = p->percentage;
            batteryData.temperature = p->temperature;
            batteryData.chargeMode = p->chargeMode;
            batteryData.needsRefresh = true;
            break;
        }
        case 0x34: {
            Packet34* p = (Packet34*)pData;
            // Byte swap needed because ESP32 is Little Endian but battery is Big Endian
            batteryData.inV = __builtin_bswap16(p->input_mv) / 1000.0;
            batteryData.outV = __builtin_bswap16(p->output_mv) / 1000.0;
            batteryData.inA = __builtin_bswap16(p->input_ma) / 1000.0;
            batteryData.outA = __builtin_bswap16(p->output_ma) / 1000.0;
            batteryData.needsRefresh = true; // Mark that new data arrived
            break;
        }
        case 0x35: {
            Packet35* p = (Packet35*)pData;
            uint16_t capacity = __builtin_bswap16(p->capacity_mah);
            uint16_t dTime = __builtin_bswap16(p->discharge_mins);
            uint16_t cTime = __builtin_bswap16(p->charge_mins);

            batteryData.capacity = capacity;
            sprintf(batteryData.dischargeMinutes, "%02d:%02d", dTime / 60, dTime % 60);
            sprintf(batteryData.chargeMinutes, "%02d:%02d", cTime / 60, cTime % 60);
            #ifdef DEBUG
            Serial.printf("TIME: Remainder: %d Hours\n", dTime);
            Serial.printf("Capacity: %d mAh\n", cap);
            #endif
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

void enterParkingSleep() {
    Serial.println("Vehicle is OFF. Entering hourly Deep Sleep...");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    parseCellinkData(pData, length);
}

class MyScanCallbacks: public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    if (advertisedDevice->getName().find("Bi750") != std::string::npos) {
      Serial.println("Scan found BI750");
      NimBLEDevice::getScan()->stop();
      Serial.printf("Address Type: %d | RSSI: %d\n", 
            (int)advertisedDevice->getAddress().getType(), 
            advertisedDevice->getRSSI());
    Serial.printf("Connectable: %s\n", advertisedDevice->isConnectable() ? "YES" : "NO");
      advDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
      doConnect = true;
    }
  }
};

class MyClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) { connected = true; }
    void onDisconnect(NimBLEClient* pClient) { connected = false; }
};

bool connectToBI750() {
    if (advDevice == nullptr) return false;

    Serial.println(">>> Executing Clean-State Connection...");
    
    if (NimBLEDevice::getScan()->isScanning()) {
        NimBLEDevice::getScan()->stop();
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    if (pClient != nullptr) {
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
    }
    
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallbacks(), false);
    pClient->setConnectionParams(12, 12, 0, 51); // min, max, latency, timeout
    pClient->setConnectTimeout(10000); // 10 second timeout
    
    Serial.printf("Connecting to %s (%s)...\n", 
                  advDevice->getName().c_str(), 
                  advDevice->getAddress().toString().c_str());
    
    pClient->connect(advDevice, true);
    vTaskDelay(pdMS_TO_TICKS(500));

    if (!pClient->isConnected()) {
        Serial.println(">>> Connection failed");
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
        return false;
    }

    Serial.println("Connected! MTU negotiated.");
    Serial.printf(">>> MTU: %d\n", pClient->getMTU());
    
    vTaskDelay(pdMS_TO_TICKS(1000));

    NimBLERemoteService* pSvc = pClient->getService(serviceUUID);
    if (!pSvc) {
        Serial.println(">>> Failed to find service");
        pClient->disconnect();
        return false;
    }

    // DEBUG: List all characteristics
    Serial.println(">>> Available characteristics:");
    std::vector<NimBLERemoteCharacteristic*> pChars = pSvc->getCharacteristics(true);
    for (auto &pChar : pChars) {  // Changed to reference
    Serial.printf("  UUID: %s\n", pChar->getUUID().toString().c_str());
    Serial.printf("    canRead: %d, canWrite: %d, canWriteNoResponse: %d\n",
                  pChar->canRead(), pChar->canWrite(), pChar->canWriteNoResponse());
    Serial.printf("    canNotify: %d, canIndicate: %d\n",
                  pChar->canNotify(), pChar->canIndicate());
    }

    pWriteChar = pSvc->getCharacteristic(charWriteUUID);
    pNotifyChar = pSvc->getCharacteristic(charNotifyUUID);

    if (!pWriteChar) {
        Serial.println(">>> Failed to find write characteristic");
        pClient->disconnect();
        return false;
    }

    if (!pNotifyChar) {
        Serial.println(">>> Failed to find notify characteristic");
        pClient->disconnect();
        return false;
    }

    Serial.println(">>> Both characteristics found");

    if (!pNotifyChar->subscribe(true, notifyCallback)) {
        Serial.println(">>> Failed to subscribe to notifications");
        pClient->disconnect();
        return false;
    }

    Serial.println("Subscribed to bidirectional characteristic");
        
    connected = true;
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("Preparing to init board..");
    init();
    Serial.println("Initializing lvgl UI");
    ui_init();
    loadScreen(SCREEN_ID_MAIN);

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("-------------------------------------");
        Serial.println(">>> HOURLY LOG START");
    } else {
        Serial.println(">>> SYSTEM COLD BOOT / RESET");
    }

    Serial.println("initializing BLE");
    BLEDevice::init("T-Display-S3");
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new MyScanCallbacks());
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(99);
    pScan->start(SCAN_DURATION);
}

void loop() {
    lv_timer_handler(); // Drive the engine

    if (doConnect) {
        doConnect = false;
        connectToBI750();
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    static uint32_t lastPoll = 0;
    if (connected && millis() - lastPoll > 500) {
        pollBattery(currentWord);

        // Get firmware and hardware name first
        if (currentWord == 0x32) currentWord = 0x3A; // obtain Production details   
        else if (currentWord == 0x3A) currentWord = 0x33; // Then jump to telemetry
        else if (currentWord == 0x33) currentWord = 0x34;
        else if (currentWord == 0x34) currentWord = 0x35;
        else currentWord = 0x33; // Repeat telemetry loop
        
        // enter deep sleep if in parking mode
        if (chargeMode == 0) {
            hoursParked++;
            Serial.printf("LOG: Hour %d | %d%% remaining.\n", hoursParked, batteryPercent);
            enterParkingSleep();
        } else { hoursParked = 0; }
        
        lastPoll = millis();
    }

    if (batteryData.needsRefresh) {
        #ifdef DEBUG
        Serial.printf("batteryData.outV: %.2f | batteryData.outA: %.2f\n", batteryData.outV, batteryData.outA);
        #endif
        tick_screen_main();
        batteryData.needsRefresh = false;
    }
    vTaskDelay(pdMS_TO_TICKS(16));

}