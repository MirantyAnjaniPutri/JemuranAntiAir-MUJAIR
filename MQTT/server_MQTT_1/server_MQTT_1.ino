#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <WiFi.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <PubSubClient.h>

// Wifi
/*const char* ssid = "v";
const char* password = "vivita21";
const char* apiKey = "ca7c56cd09d32f53c7b5840220207650";*/

//MQTT
//const char* mqtt_server = "broker.hivemq.com";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

//GPS pin, variable etc
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
TinyGPSPlus gps;

//motor pin etc
#define DIR 12
#define STEP 14
#define MOTOR_ENA 13
#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

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

void setup_wifi() {
 delay(10);
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println("v");

 WiFi.mode(WIFI_STA);
 WiFi.begin("v", "vivita21");

 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }

 randomSeed(micros());

 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i = 0; i < length; i++) {
   Serial.print((char)payload[i]);
 }
 Serial.println();
}

void reconnect() {
 while (!client.connected()) {
   Serial.print("Attempting MQTT connection...");
   String clientId = "ESP32Client-";
   clientId += String(random(0xffff), HEX);
   if (client.connect(clientId.c_str())) {
     Serial.println("connected");
     client.subscribe("/esp32-2/data");
   } else {
     Serial.print("failed, rc=");
     Serial.print(client.state());
     Serial.println(" try again in 5 seconds");
     delay(5000);
   }
 }
}

void setup() {
  motor.setMaxSpeed(1000);
  motor.setAcceleration(500);
  motor.setCurrentPosition(0);

  //Begin serial communication Neo6mGPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.begin(115200);
  setup_wifi();
  client.setServer("broker.hivemq.com", 1883);
  client.setCallback(callback);
}

void loop() {
  boolean newData = false;
  double latitude;
  double longitude;
  
  pinMode(MOTOR_ENA, OUTPUT);
  disableOutput();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

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

    // Fetch weather data
    HTTPClient http;
    String apiUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(-4.06) + "&lon=" + String(106.78) + "&appid=" + "ca7c56cd09d32f53c7b5840220207650";
    http.begin(apiUrl);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();

      // Parse JSON
      DynamicJsonDocument jsonDoc(2048); // Adjust the buffer size as needed
      DeserializationError error = deserializeJson(jsonDoc, payload);

      if (!error) {
        WeatherData weather;
        weather.weatherMain = jsonDoc["weather"][0]["main"].as<String>();
        weather.temp = jsonDoc["main"]["temp"].as<float>();
        weather.tempMin = jsonDoc["main"]["temp_min"].as<float>();
        weather.tempMax = jsonDoc["main"]["temp_max"].as<float>();
        weather.pressure = jsonDoc["main"]["pressure"].as<float>();
        weather.humidity = jsonDoc["main"]["humidity"].as<float>();
        jsonDoc.shrinkToFit();

        // Convert the weather data to a char array
        char weatherString[128];
        sprintf(weatherString, "Weather Main: %s, Temperature: %f, Temp Min: %f, Temp Max: %f, Pressure: %f, Humidity: %f", weather.weatherMain.c_str(), weather.temp, weather.tempMin, weather.tempMax, weather.pressure, weather.humidity);
        Serial.print("Weather Data: ");
        Serial.println(weatherString);
        client.publish("/esp32-mqtt/weather", weatherString);
      } else {
        Serial.print("JSON Parsing Error: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("HTTP error code: ");
      Serial.println(httpResponseCode);
    }
    //client.publish("/esp32-mqtt/weather", weatherString); //send data to client
    http.end();
  }
}
