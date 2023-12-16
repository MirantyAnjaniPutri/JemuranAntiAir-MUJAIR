#include "BLECom.h"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_STATUS "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

BLECom::BLECom() {
  // Constructor code
  pServer = NULL;
  pCharacteristic_status = NULL;
  pDescr = NULL;
  pBLE2902_status = NULL;

  deviceConnected = false;
  oldDeviceConnected = false;
  data = 0;
}

void MyServerCallbacks::onConnect(BLEServer* pServer) {
  deviceConnected = true;
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer) {
  deviceConnected = false;
}

void MyBLELibrary::setup() {
   // Setup code
   Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_status = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_STATUS,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |                      
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor

  pBLE2902_status = new BLE2902();
  pBLE2902_status->setNotifications(true);  
  pCharacteristic_status->addDescriptor(pBLE2902_status);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set data to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void MyBLELibrary::loop() {
   // Loop code
   // notify changed data
    if (deviceConnected) {
        // pCharacteristic_2 is a std::string (NOT a String). In the code below we read the current value
        // write this to the Serial interface and send a different value back to the Client
        // Here the current value is read using getValue()
        std::string rxValue = pCharacteristic_status->getValue();
        Serial.print("Characteristic Status (getValue): ");
        Serial.println(rxValue.c_str());

        // Here the value is written to the Client using setValue();
        String txValue = "String with random value from Server: " + String(random(1000));
        pCharacteristic_status->setValue(txValue.c_str());
        Serial.println("Characteristic Status (setValue): " + txValue);

        // In this example "delay" is used to delay with one second. This is of course a very basic 
        // implementation to keep things simple. I recommend to use millis() for any production code
        delay(1000);
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

// Add other function definitions as needed
