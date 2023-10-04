#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";
const float latitude = -6.200000;
const float longitude = 106.816666;

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
}

void loop() {
  // Your code here
}