#ifndef BLECOM_H
#define BLECOM_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class MyServerCallbacks: public BLEServerCallbacks {
public:
   void onConnect(BLEServer* pServer);
   void onDisconnect(BLEServer* pServer);
};

class BLECom {
public:
   BLECom();
   void setup();
   void loop();
   // Add other functions and variables as needed
};

#endif // BLECOM_H