# ESP32 Library for BI-750 Dashcam Battery

I wanted to check the health of my BlackboxMyCar BI-750 battery, and to my dismay there was not an app for iOS that would work. I dug out my Android, loaded up an archived APK thanks to expert customer service from BlackboxMyCar, and reverse-engineered the BLE packets.
---
This will be integrated into other projects of mine, but if you find some use out of it, that's great. I'll write as much helper code as I can, but reading through the header files and the pollBattery function should give you a pretty good idea of how the poll/notify mechanism works.