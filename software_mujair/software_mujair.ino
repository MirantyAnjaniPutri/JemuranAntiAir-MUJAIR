#include <ArduinoJson.h>
#include <WiFi.h>

const char* host = "api.open-meteo.com";  // Updated host to match your URL structure
const int httpPort = 80;
const char* ssid = "VI4";
const char* password = "vi421241";

// Function to create the URL with custom latitude and longitude
String createURL(double latitude, double longitude) {
    // Base URL template
    const char* baseURL = "/v1/forecast?latitude=%.4f&longitude=%.4f&hourly=temperature_2m,precipitation,rain,showers&timezone=Asia%%2FBangkok";

    // Create a String object to store the URL
    String url = String(baseURL);

    // Format the URL with custom latitude and longitude
    url.replace("%.4f", String(latitude));
    url.replace("%.4f", String(longitude));

    return url;
}

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    
    Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
}

void loop() {
    double customLatitude, customLongitude;
    
    Serial.println("Enter Latitude:");
    while (!Serial.available()) {}
    customLatitude = Serial.parseFloat();
    Serial.println(customLatitude);
    
    Serial.println("Enter Longitude:");
    while (!Serial.available()) {}
    customLongitude = Serial.parseFloat();
    Serial.println(customLongitude);

    String url = createURL(customLatitude, customLongitude);

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

    while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.println(line);
    }

    Serial.println("Closing Connection");
    client.stop();
}
