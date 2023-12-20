#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <WiFi.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
//#include <ThingsBoard.h>
#include "wifi_connection.h"

#define TOKEN_TB "KuOo88nwpZC0nmdk4B02"
const char* ssid = "v";
const char* password = "vivita21";
const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";
char thingsboardServer[] = "192.168.1.8";
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

double latitude;
double longitude;

#define RXD2 16
#define TXD2 17

HardwareSerial neogps(1);
TinyGPSPlus gps;

#define RAIN_PIN 4
bool rain_flag = false;

#define DIR 12
#define STEP 14
#define MOTOR_ENA 13

#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void enableOutput() {
  digitalWrite(MOTOR_ENA, LOW); // Enable the motor output
}

void disableOutput() {
  digitalWrite(MOTOR_ENA, HIGH); // Disable the motor output
}

struct WeatherData {
  String weatherMain;
  float temp;
  float tempMin;
  float tempMax;
  float pressure;
  float humidity;
};

void IRAM_ATTR rain_isr() {
  // Read the rain sensor value
  int rain_value = digitalRead(RAIN_PIN);

  // Check if rain is detected
  if (rain_value == LOW) {
    // Set the flag to true
    rain_flag = true;
  }
}

void setup() {
  pinMode(RAIN_PIN, INPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  disableOutput();

  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rain_isr, FALLING);

  motor.setMaxSpeed(1000);
  motor.setAcceleration(500);
  motor.setCurrentPosition(0);
  Serial.begin(9600);
  
  setupWIFI(ssid,password);

  //Begin serial communication Neo6mGPS
  Serial.begin(9600);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(2000);

  InitWiFi();
  lastSend = 0;
}

void loop() {
  // Your code here
  boolean newData = false;

  if(rain_flag == true) {

    // Print a message to the serial monitor
    Serial.println("Rain detected!");

    // Move the motor to position 800
    enableOutput();
    motor.moveTo(800);
    while (motor.distanceToGo() != 0) {
      motor.run();
    }

    // Disable motor
    disableOutput();
    
    // Wait for rain to stop
    while (rain_flag == true) {
      delay(10000);
    }

    // Enable the motor output
    enableOutput();

    // Move the motor to position 0
    motor.moveTo(0);
    while (motor.distanceToGo() != 0) {
      motor.run();
    }

    // Disable the motor output
    disableOutput();

    // Reset the flag to false
    rain_flag = false;
  } else{
    while(gps.location.lat() == 0 && gps.location.lng() == 0){
      for (unsigned long start = millis(); millis() - start < 1000;)
      {
        while (neogps.available())
        {
          if (gps.encode(neogps.read()))
          {
            newData = true;
          }
        }
    }

    //If newData is true
    if(newData == true)
    {
      newData = false;
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }
    else
    {
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

      DynamicJsonDocument jsonDoc(2048); // Adjust the buffer size as needed

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
  }
}