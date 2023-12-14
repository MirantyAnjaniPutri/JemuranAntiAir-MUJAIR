/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//Library for API, Wifi, and Motor
#include <Arduino.h>
//#include <HTTPClient.h>
//#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <WiFi.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
//#include <ThingsBoard.h>

//Wifi variables
const char* ssid = "v";
const char* password = "vivita21";
const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";
double latitude;
double longitude;

//GPS variables
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
TinyGPSPlus gps;

//Motor variable
#define DIR 12
#define STEP 14
#define MOTOR_ENA 13
#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

// Initialize all pointers
BLEServer* pServer = NULL;                         // Pointer to the server
BLECharacteristic* pCharacteristic_status = NULL;  // Pointer to Characteristic Status
BLEDescriptor* pDescr;                             // Pointer to BLE2902 of Characteristic
BLE2902* pBLE2902_status;                          // Pointer to BLE2902 of Characteristic Status

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t data = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_STATUS "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

//Motor function
void enableOutput() {
  digitalWrite(MOTOR_ENA, LOW);  // Enable the motor output
}

void disableOutput() {
  digitalWrite(MOTOR_ENA, HIGH);  // Disable the motor output
}

struct WeatherData {
  String weatherMain;
  float temp;
  float tempMin;
  float tempMax;
  float pressure;
  float humidity;
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};



void setup() {
  Serial.begin(115200);
  //Motor setup
  pinMode(MOTOR_ENA, OUTPUT);
  disableOutput();
  motor.setMaxSpeed(1000);
  motor.setAcceleration(500);
  motor.setCurrentPosition(0);
  Serial.begin(9600);

  //Wifi setup
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Begin serial communication Neo6mGPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_status = pService->createCharacteristic(
    CHARACTERISTIC_UUID_STATUS,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor

  pBLE2902_status = new BLE2902();
  pBLE2902_status->setNotifications(true);
  pCharacteristic_status->addDescriptor(pBLE2902_status);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set data to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  boolean newData = false;
  while (gps.location.lat() == 0 && gps.location.lng() == 0) {
    for (unsigned long start = millis(); millis() - start < 1000;) {
      while (neogps.available()) {
        if (gps.encode(neogps.read())) {
          newData = true;
        }
      }
    }

    //If newData is true
    if (newData == true) {
      newData = false;
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    } else {
      Serial.println("No Data");
    }
    Serial.println("Satelitte: " + String(gps.satellites.value()));
    Serial.println(gps.location.isValid());
    Serial.println(String(latitude) + " and " + String(longitude));
  }
  
  HTTPClient http;
  String apiUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude) + "&lon=" + String(longitude) + "&appid=" + apiKey;
  http.begin(apiUrl);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();

    // Print the raw JSON response for debugging
    Serial.println("Raw JSON response:");
    Serial.println(payload);

    DynamicJsonDocument jsonDoc(2048);  // Adjust the buffer size as needed

    // Parse JSON
    DeserializationError error = deserializeJson(jsonDoc, payload);

    if (!error) {
      WeatherData weather;
      weather.weatherMain = jsonDoc["weather"][0]["main"].as<String>();
      weather.temp = jsonDoc["main"]["temp"].as<float>();
      weather.tempMin = jsonDoc["main"]["temp_min"].as<float>();
      weather.tempMax = jsonDoc["main"]["temp_max"].as<float>();
      weather.pressure = jsonDoc["main"]["pressure"].as<float>();
      weather.humidity = jsonDoc["main"]["humidity"].as<float>();

      // Now you can use the 'weather' struct to access the data.
      Serial.print("Weather Main: ");
      Serial.println(weather.weatherMain);
      Serial.print("Temperature: ");
      Serial.println(weather.temp);
      Serial.print("Temp Min: ");
      Serial.println(weather.tempMin);
      Serial.print("Temp Max: ");
      Serial.println(weather.tempMax);
      Serial.print("Pressure: ");
      Serial.println(weather.pressure);
      Serial.print("Humidity: ");
      Serial.println(weather.humidity);
    } else {
      Serial.print("JSON Parsing Error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(1000);
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
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}