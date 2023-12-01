#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const char* ssid = "v";
const char* password = "vivita21";
const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";

double latitude = 0;
double longitude = 0;

#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
TinyGPSPlus gps;

struct WeatherData {
  String weatherMain;
  float temp;
  float tempMin;
  float tempMax;
  float pressure;
  float humidity;
};

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  xTaskCreatePinnedToCore(
    gpsTask,        // Function to run the GPS task
    "GPSTask",      // Name of the task
    10000,          // Stack size (adjust as needed)
    NULL,           // Task parameters
    1,              // Task priority
    NULL,           // Task handle
    0               // Run on core 0
  );
}

void gpsTask(void* parameter) {
  (void)parameter;
  while (1) {
    boolean newData = false;
    while (gps.location.lat() == 0 && gps.location.lng() == 0) {
      for (unsigned long start = millis(); millis() - start < 1000;) {
        while (neogps.available()) {
          if (gps.encode(neogps.read())) {
            newData = true;
          }
        }
      }

      if (newData) {
        newData = false;
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      } else {
        Serial.println("No Data");
      }
      vTaskDelay(pdMS_TO_TICKS(1000));  // Delay for 1 second
    }
  }
}

void loop() {
  if (latitude == 0 && longitude == 0) {
    Serial.println("Waiting for GPS data...");
    delay(1000);  // Wait for GPS data to be available
  } else {
    // Your weather API code here
    HTTPClient http;
    String apiUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude) + "&lon=" + String(longitude) + "&appid=" + apiKey;
    http.begin(apiUrl);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      // Your JSON parsing and data retrieval code here
    } else {
      Serial.print("HTTP error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
    delay(3600000);  // Delay for 1 hour before requesting weather data again
  }
}
