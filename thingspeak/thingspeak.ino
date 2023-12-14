#include <WiFi.h>
#include <ThingSpeak.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_AP "Miranty"
#define WIFI_PASSWORD "Miranty27"

unsigned long myChannelNumber = 2377351;
const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";
const char* writeKey = "TH0A2OF9MLW1321D";
double latitude = 6.200000;
double longitude = 106.816666;

WiFiClient wifiClient;

struct WeatherData {
  String weatherMain;
  float temp;
  float tempMin;
  float tempMax;
  float pressure;
  float humidity;
};

unsigned long lastSend;

void setup() {
  Serial.begin(115200);
  delay(10);
  InitWiFi();
  lastSend = 0;
  ThingSpeak.begin(wifiClient);
  getAndSendWeatherData();
}

void loop() {
  if (millis() - lastSend > 300000) { // Update and send only after 10 minutes
    getAndSendWeatherData();
    lastSend = millis();
  }
}

void getAndSendWeatherData() {
  Serial.println("Collecting weather data...");

  // Call the function to fetch weather data
  WeatherData weather = getWeatherData();

  // Check if any reads failed and exit early (to try again).
  if (isnan(weather.temp) || isnan(weather.humidity)) {
    Serial.println("Failed to read weather data!");
    return;
  }

  Serial.println("Sending data to ThingSpeak:");
  Serial.print("Weather Main: ");
  Serial.println(weather.weatherMain);
  Serial.print("Temperature: ");
  Serial.print(weather.temp);
  Serial.println(" *C");
  Serial.print("Temp Min: ");
  Serial.print(weather.tempMin);
  Serial.println(" *C");
  Serial.print("Temp Max: ");
  Serial.print(weather.tempMax);
  Serial.println(" *C");
  Serial.print("Pressure: ");
  Serial.print(weather.pressure);
  Serial.println(" hPa");
  Serial.print("Humidity: ");
  Serial.print(weather.humidity);
  Serial.println(" %");

  // Send data to ThingSpeak
  ThingSpeak.setField(1, weather.temp);  // Field 1 for Temperature
  ThingSpeak.setField(2, weather.humidity);  // Field 2 for Humidity
  ThingSpeak.setField(3, weather.pressure);  // Field 3 for Pressure
  ThingSpeak.setField(4, weather.tempMin);  // Field 4 for Temp Min
  ThingSpeak.setField(5, weather.tempMax);  // Field 5 for Temp Max
  ThingSpeak.setField(6, weather.weatherMain.c_str());  // Field 6 for Weather Main

  int writeSuccess = ThingSpeak.writeFields(myChannelNumber, writeKey);

  if (writeSuccess) {
    Serial.println("Write successful!");
  } else {
    Serial.println("Failed to write to ThingSpeak.");
  }
}

WeatherData getWeatherData() {
  HTTPClient http;
  String apiUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude) + "&lon=" + String(longitude) + "&appid=" + apiKey;
  http.begin(apiUrl);

  int httpResponseCode = http.GET();
  WeatherData weather;

  if (httpResponseCode > 0) {
    String payload = http.getString();

    // Print the raw JSON response for debugging
    Serial.println("Raw JSON response:");
    Serial.println(payload);

    DynamicJsonDocument jsonDoc(2048); // Adjust the buffer size as needed

    // Parse JSON
    DeserializationError error = deserializeJson(jsonDoc, payload);

    if (!error) {
      weather.weatherMain = jsonDoc["weather"][0]["main"].as<String>();
      weather.temp = jsonDoc["main"]["temp"].as<float>();
      weather.tempMin = jsonDoc["main"]["temp_min"].as<float>();
      weather.tempMax = jsonDoc["main"]["temp_max"].as<float>();
      weather.pressure = jsonDoc["main"]["pressure"].as<float>();
      weather.humidity = jsonDoc["main"]["humidity"].as<float>();
    } else {
      Serial.print("JSON Parsing Error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return weather;
}

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}