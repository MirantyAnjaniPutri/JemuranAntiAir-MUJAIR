#include <ThingsBoard.h>
#include <WiFi.h>
#include <AccelStepper.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Include the ArduinoJson library

#define WIFI_AP "Miranty"
#define WIFI_PASSWORD "Miranty27"
#define TOKEN "iDxBefgkQWQwyD4RSi1A"

const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";
double latitude = 6.200000;
double longitude = 106.816666;

#define RAIN_PIN 4
bool rain_flag = false;
bool statusJemuran = false;

#define DIR 12
#define STEP 14
#define MOTOR_ENA 13

#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

char thingsboardServer[] = "thingsboard.cloud";

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

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

  Serial.begin(115200);
  InitWiFi();
  lastSend = 0;
}

void loop() {
  if (!tb.connected()) {
    reconnect();
  }

  if (millis() - lastSend > 30000) { // Update and send only after 30 seconds
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
  Serial.println(weather.weatherMain);
  
  float kelvin = weather.temp;  // Replace with your Kelvin temperature
  float celsius = kelvin - 273.15;
  weather.temp = celsius;
  Serial.print(weather.temp);

  Serial.print(weather.tempMin);
  Serial.print(weather.tempMax);
  Serial.print(weather.pressure);
  Serial.print(weather.humidity);

  const char* rainSensorStatus = (rain_flag == true) ? "True" : "False";
  const char* stringStatusJemuran = (statusJemuran == true) ? "True" : "False";

  if (weather.weatherMain != "Clear") {
    openTerpal();
    // Consistently check for 5 minutes
    for (int i = 0; i < 5; i++) {
      delay(60000);  // 1 minute
      WeatherData newWeather = getWeatherData();
      if (rain_flag == false) {
        closeTerpal();
        break;
      }
    }
  }

  tb.sendTelemetryString("weather", weather.weatherMain.c_str());
  tb.sendTelemetryString("latitude", String(latitude).c_str());
  tb.sendTelemetryString("longitude", String(longitude).c_str());
  tb.sendTelemetryFloat("temperature", weather.temp);
  tb.sendTelemetryFloat("humidity", weather.humidity);
  tb.sendTelemetryFloat("pressure", weather.pressure);
  tb.sendTelemetryString("Rain Sensor", rainSensorStatus);
  tb.sendTelemetryString("Kondisi Terpal", stringStatusJemuran);
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

void openTerpal() {
  // Move the motor to position 800
    enableOutput();
    motor.moveTo(800);
    while (motor.distanceToGo() != 0) {
      motor.run();
    }

    // Disable motor
    disableOutput();

    // Status Jemuran
    statusJemuran = true;
}

void closeTerpal() {
  // Enable the motor output
  enableOutput();

  // Move the motor to position 0
  motor.moveTo(0);
  while (motor.distanceToGo() != 0) {
    motor.run();
  }

  // Disable the motor output
  disableOutput();

  // Status Jemuran
  statusJemuran = false;
}

void enableOutput() {
  digitalWrite(MOTOR_ENA, LOW); // Enable the motor output
}

void disableOutput() {
  digitalWrite(MOTOR_ENA, HIGH); // Disable the motor output
}