#include <ArduinoJson.h>
#include <WiFi.h>
#include <string.h>

const char* host = "api.open-meteo.com";  // Updated host to match your URL structure
const int httpPort = 80;

// Define arrays to store data globally
const int maxDataSize = 170; // Adjust this based on your expected maximum data size

struct WeatherData {
  String time;
  float temperature;
  float precipitation;
  float precipitationProbability;
  float rain;
  float showers;
};

WeatherData weatherData[maxDataSize]; // Store weather data in an array

#define WIFI_SSID "LAPTOP-ELGTLQVF 9258"
#define WIFI_PASSWORD "Miranty27"

String createURL(String latitude, String longitude, String timezone) {
    // Base URL template
    const char* baseURL = "/v1/forecast?latitude=%LAT%&longitude=%LONG%&hourly=temperature_2m,precipitation_probability,precipitation,rain,showers&timezone=%TIMEZONE%";

    // Format the URL with custom latitude, longitude, and timezone
    String url = String(baseURL);
    url.replace("%LAT%", latitude);
    url.replace("%LONG%", longitude);
    url.replace("%TIMEZONE%", timezone);

    return url;
}


void toExtract(const char* hourlyData) {
  char* token = strtok((char*)hourlyData, "[");
  int i = 0;

  while (token != NULL) {
    // Skip the first token (it's not needed)
    if (i == 0) {
      token = strtok(NULL, "[");
      i++;
      continue;
    }

    // Extract time
    weatherData[i - 1].time = strtok(token, ",");
    // Extract temperature
    weatherData[i - 1].temperature = atof(strtok(NULL, ","));
    // Extract precipitation probability
    weatherData[i - 1].precipitationProbability = atoi(strtok(NULL, ","));
    // Extract precipitation
    weatherData[i - 1].precipitation = atoi(strtok(NULL, ",")) == 1;
    // Extract rain
    weatherData[i - 1].rain = atoi(strtok(NULL, ",")) == 1;
    // Extract showers
    weatherData[i - 1].showers = atoi(strtok(NULL, "]")) == 1;

    i++;

    // Move to the next token
    token = strtok(NULL, "[");
  }
}

/*void parseJsonResponse(String jsonResponse) {
  // Find the positions of the keywords in the JSON response
    int timePos = jsonResponse.indexOf("\"time\":[");
    int temperaturePos = jsonResponse.indexOf("\"temperature_2m\":");
    int precipitationPos = jsonResponse.indexOf("\"precipitation\":");
    int rainPos = jsonResponse.indexOf("\"rain\":");
    int showersPos = jsonResponse.indexOf("\"showers\":");

    if (timePos == -1 || temperaturePos == -1 || precipitationPos == -1 || rainPos == -1 || showersPos == -1) {
        Serial.println("One or more keyword positions not found.");
        return;
    }

    // Extract and print the time data
    Serial.print("Time: ");
    int timeEnd = jsonResponse.indexOf("]", timePos) + 1;
    String timeData = jsonResponse.substring(timePos, timeEnd);
    Serial.println(timeData);

    // Extract and print the temperature data
    Serial.print("Temperature: ");
    int temperatureEnd = jsonResponse.indexOf("]", temperaturePos) + 1;
    String temperatureData = jsonResponse.substring(temperaturePos, temperatureEnd);
    Serial.println(temperatureData);

    // Extract and print the precipitation data
    Serial.print("Precipitation: ");
    int precipitationEnd = jsonResponse.indexOf("]", precipitationPos) + 1;
    String precipitationData = jsonResponse.substring(precipitationPos, precipitationEnd);
    Serial.println(precipitationData);

    // Extract and print the rain data
    Serial.print("Rain: ");
    int rainEnd = jsonResponse.indexOf("]", rainPos) + 1;
    String rainData = jsonResponse.substring(rainPos, rainEnd);
    Serial.println(rainData);

    // Extract and print the showers data
    Serial.print("Showers: ");
    int showersEnd = jsonResponse.indexOf("]", showersPos) + 1;
    String showersData = jsonResponse.substring(showersPos, showersEnd);
    Serial.println(showersData);
}*/


void printData() {
  // Print the parsed data
  // for (int j = 0; j < maxDataSize; j++)
  for (int j = 0; j <= 2; j++) { // Assuming you have two data points
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
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi ");
    Serial.print(WIFI_SSID);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println(" Connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    double customLatitude, customLongitude;
    String timezone = "Asia/Bangkok";
    
    Serial.println("Enter Latitude (-180.000000 to 180.000000):");
    while (!Serial.available()) {}  // Wait for user input
    String latitudeInput = Serial.readStringUntil('\n');  // Read input as a string
    customLatitude = latitudeInput.toDouble();
    Serial.println(latitudeInput);
    
    // Validate latitude input
    if (customLatitude < -90.0 || customLatitude > 90.0) {
        Serial.println("Invalid latitude. Please enter a value between -90.000000 and 90.000000.");
        return;  // Exit the loop if the latitude is out of range
    }

    Serial.println("Enter Longitude (-180.000000 to 180.000000):");
    while (!Serial.available()) {}  // Wait for user input
    String longitudeInput = Serial.readStringUntil('\n');  // Read input as a string
    customLongitude = longitudeInput.toDouble();
    Serial.println(longitudeInput);

    // Validate longitude input
    if (customLongitude < -180.0 || customLongitude > 180.0) {
        Serial.println("Invalid longitude. Please enter a value between -180.000000 and 180.000000.");
        return;  // Exit the loop if the longitude is out of range
    }

    String url = createURL(latitudeInput, longitudeInput, timezone);

    WiFiClient client;
    DynamicJsonDocument doc(2048); // Increase the size as needed

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

    // Parse JSON
    DeserializationError error = deserializeJson(doc, client);
    
    if (error) {
        Serial.print("JSON Parsing Error: ");
        Serial.println(error.c_str());

        // Print the raw JSON response for debugging
        Serial.println("Raw JSON response:");
        String jsonResponse = "";

        while (client.available()) {
            char c = client.read();
            jsonResponse += c;
        }

        Serial.println(jsonResponse);
        const char* jsonData = jsonResponse.c_str();
        const char* hourlyData = strstr(jsonData, "\"hourly\":");
        
        toExtract(hourlyData);

        //toExtract(hourlyData);
        // Now, parse the JSON response and store the data in global arrays
        //parseJsonResponse(jsonResponse);
    }

    Serial.println("Closing Connection");
    client.stop();

    printData();
}