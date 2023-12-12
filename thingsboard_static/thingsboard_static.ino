#include <ThingsBoard.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Include the ArduinoJson library

#define WIFI_AP "Sujarwo"
#define WIFI_PASSWORD "Anjani91"
#define TOKEN "KuOo88nwpZC0nmdk4B02"

const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";
double latitude = 6.200000;
double longitude = 106.816666;

char thingsboardServer[] = "192.168.1.8";

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

struct WeatherData {
  String weatherMain;
  float temp;
  float tempMin;
  float tempMax;
  float pressure;
  float humidity;
};

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup() {
  Serial.begin(115200);
  delay(10);
  InitWiFi();
  lastSend = 0;
}

void loop() {
  if (!tb.connected()) {
    reconnect();
  }

  if (millis() - lastSend > 600000) { // Update and send only after 1 second
    getAndSendWeatherData();
    lastSend = millis();
  }

  tb.loop();
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

  Serial.println("Sending data to ThingsBoard:");
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

  tb.sendTelemetryString("main", weather.weatherMain.c_str());
  tb.sendTelemetryFloat("temperature", weather.temp);
  tb.sendTelemetryFloat("humidity", weather.humidity);
  tb.sendTelemetryFloat("pressure", weather.pressure);
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

void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if (status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if (tb.connect(thingsboardServer, TOKEN)) {
      Serial.println("[DONE]");
    } else {
      Serial.print("[FAILED]");
      Serial.println(" : retrying in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
