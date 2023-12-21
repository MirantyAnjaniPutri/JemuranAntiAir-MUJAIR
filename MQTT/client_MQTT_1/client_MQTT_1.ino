#include <WiFi.h>
#include <PubSubClient.h>
#include <AccelStepper.h>
#include <cstring>

// Update these with values suitable for your network.
const char* ssid = "v";
const char* password = "vivita21";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char serverData[2];

const int DIR = 12;
const int STEP = 14;
#define MOTOR_ENA 13
#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

#define RAIN_PIN 4
bool rain_flag = false;

void enableOutput() {
  digitalWrite(MOTOR_ENA, LOW); // Enable the motor output
}

void disableOutput() {
  digitalWrite(MOTOR_ENA, HIGH); // Disable the motor output
}

void setup_wifi() {
 delay(10);
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);

 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);

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
    serverData[i] = (char)payload[i];
  }
  Serial.println();

  // Null-terminate the serverData array
  serverData[length] = '\0';

  // Send Rain Sensor Value
  if(rain_flag == true || strcmp(serverData, "1") == 0){
    // Print a message to the serial monitor
    Serial.println("Rain detected or received '1'!");
    
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
  }
}


void reconnect() {
 while (!client.connected()) {
   Serial.print("Attempting MQTT connection...");
   String clientId = "ESP32Client-";
   clientId += String(random(0xffff), HEX);
   if (client.connect(clientId.c_str())) {
     Serial.println("connected");
     client.subscribe("/esp32-mqtt/weather");
   } else {
     Serial.print("failed, rc=");
     Serial.print(client.state());
     Serial.println(" try again in 5 seconds");
     delay(5000);
   }
 }
}

void setup() {
  pinMode(RAIN_PIN, INPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  disableOutput();

  motor.setMaxSpeed(1000); // Set maximum speed value for the stepper
  motor.setAcceleration(500); // Set acceleration value for the stepper
  motor.setCurrentPosition(0); // Set the current position to 0 steps
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
 if (!client.connected()) {
   reconnect();
 }
 client.loop();

 int rain_state = digitalRead(RAIN_PIN);
  if (rain_state == HIGH)
    rain_flag = false;
  else
    rain_flag = true;
    
 long now = millis();
 if (now - lastMsg > 5000) {
   lastMsg = now;

   // Convert the boolean data to a char array
   char dataString[2];
   sprintf(dataString, "%d", rain_flag);
   Serial.print("Data: ");
   Serial.println(dataString);
   client.publish("/esp32-2/data", dataString);
 }
}
