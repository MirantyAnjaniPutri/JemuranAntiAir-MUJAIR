#include <Wire.h>
#include <TinyGPSPlus.h>

#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
TinyGPSPlus gps;

void setup() {
  // put your setup code here, to run once:
  //Begin serial communication Arduino IDE (Serial Monitor)
  Serial.begin(9600);
 
  //Begin serial communication Neo6mGPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:
  boolean newData = false;
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
    Serial.print("Satellite value: ");
    Serial.println(gps.satellites.value());
    Serial.println(gps.location.isValid());
    read_gps_data();
  }
  else
  {
    Serial.println("No Data");
  }
}

void read_gps_data() {      
  if (gps.location.isValid() == 1)
  {
   //String gps_speed = String(gps.speed.kmph());
    // Read latitude, longitude,and wind speed from GPS
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    double wind_speed = gps.speed.kmph();

    // Show data in serial
    Serial.print("Lat: ");
    Serial.println(gps.location.lat(),6);

    Serial.print("Lng: ");
    Serial.println(gps.location.lng(),6);

    Serial.print("Speed: ");
    Serial.println(gps.speed.kmph());
  }
  else
  {
    Serial.println("Data Unavailable");
  }  

}
