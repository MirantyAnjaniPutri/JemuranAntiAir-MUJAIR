#include <ArduinoJson.h>
#include <WiFi.h>

const char* host = "api.open-meteo.com";
const int httpPort = 80;

struct WeatherData {
  String time;
  float temperature;
  int precipitationProbability;
  bool precipitation;
  bool rain;
  bool showers;
};

const int maxDataSize = 170;
WeatherData weatherData[maxDataSize];

String createURL(String latitude, String longitude, String timezone) {
  const char* baseURL = "/v1/forecast?latitude=%LAT%&longitude=%LONG%&hourly=temperature_2m,precipitation_probability,precipitation,rain,showers&timezone=%TIMEZONE%";
  String url = String(baseURL);
  url.replace("%LAT%", latitude);
  url.replace("%LONG%", longitude);
  url.replace("%TIMEZONE%", timezone);
  return url;
}

bool parseJsonResponse(String jsonResponse) {
  DynamicJsonDocument doc(3395);
  DeserializationError error = deserializeJson(doc, jsonResponse);

  if (error) {
    Serial.print("JSON Parsing Error: ");
    Serial.println(error.c_str());
    return false;
  }

  JsonArray hourlyData = doc["hourly"].as<JsonArray>();
  int dataSize = min(maxDataSize, static_cast<int>(hourlyData.size()));

  for (int i = 0; i < dataSize; i++) {
    JsonObject data = hourlyData[i];
    weatherData[i].time = data["time"].as<String>();
    weatherData[i].temperature = data["temperature_2m"].as<float>();
    weatherData[i].precipitationProbability = data["precipitation_probability"].as<int>();
    weatherData[i].precipitation = data["precipitation"].as<int>() == 1;
    weatherData[i].rain = data["rain"].as<int>() == 1;
    weatherData[i].showers = data["showers"].as<int>() == 1;
  }
  return true;
}

void printData(int dataSize) {
  for (int j = 0; j < dataSize; j++) {
    Serial.print("Time: ");
    Serial.println(weatherData[j].time);
    Serial.print("Temperature: ");
    Serial.println(weatherData[j].temperature);
    Serial.print("Precipitation Probability: ");
    Serial.println(weatherData[j].precipitationProbability);
    Serial.print("Precipitation: ");
    Serial.println(weatherData[j].precipitation);
    Serial.print("Rain: ");
    Serial.println(weatherData[j].rain);
    Serial.print("Showers: ");
    Serial.println(weatherData[j].showers);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  double customLatitude, customLongitude;
  String timezone = "Asia/Bangkok";

  /*Serial.println("Enter Latitude (-180.000000 to 180.000000):");
  while (!Serial.available()) {
    delay(5000); // Wait for input
  }*/
  String latitudeInput = "-6.200000";
  customLatitude = latitudeInput.toDouble();

  if (customLatitude < -90.0 || customLatitude > 90.0) {
    Serial.println("Invalid latitude. Please enter a value between -90.000000 and 90.000000.");
    return;
  }

  /*Serial.println("Enter Longitude (-180.000000 to 180.000000):");
  while (!Serial.available()) {
    delay(5000); // Wait for input
  }*/
  //String longitudeInput = Serial.readStringUntil('\n');
  String longitudeInput = "106.816666";
  customLongitude = longitudeInput.toDouble();

  if (customLongitude < -180.0 || customLongitude > 180.0) {
    Serial.println("Invalid longitude. Please enter a value between -180.000000 and 180.000000.");
    return;
  }

  String url = createURL(latitudeInput, longitudeInput, timezone);
  WiFiClient client;

  Serial.println("Connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpPort)) {
    Serial.println("Connection Failed");
    return;
  } else {
    Serial.println("Connection Established Successfully");
  }

  Serial.println("Connecting to URL:");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Connection Timeout");
      client.stop();
      return;
    }
  }

  String jsonResponse = "";
  while (client.available()) {
    char c = client.read();
    jsonResponse += c;
  }

  Serial.println("Closing Connection");
  client.stop();

  Serial.println("Received JSON Response:");
  Serial.println(jsonResponse);

  if (parseJsonResponse(jsonResponse)) {
    // Print the parsed data
    int dataSize = min(maxDataSize, static_cast<int>(sizeof(weatherData) / sizeof(weatherData[0])));
    printData(dataSize);
  }
}
