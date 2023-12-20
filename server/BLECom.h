#ifndef BLECom_h
#define BLECom_h

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

// Initialize all pointers
extern BLEServer* pServer;                      // Pointer to the server
extern BLECharacteristic* pCharacteristic_status;     // Pointer to Characteristic Status
extern BLEDescriptor *pDescr;                           // Pointer to BLE2902 of Characteristic 
extern BLE2902 *pBLE2902_status;                            // Pointer to BLE2902 of Characteristic Status

extern bool deviceConnected;
extern bool oldDeviceConnected;
extern uint32_t data;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID       "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_STATUS "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

class MyServerCallbacks: public BLEServerCallbacks {
public:
 void onConnect(BLEServer* pServer);
 void onDisconnect(BLEServer* pServer);
};

void setupBLE();
void loopBLE();

#endif
