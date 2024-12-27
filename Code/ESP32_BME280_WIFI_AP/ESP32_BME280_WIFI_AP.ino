#include <WiFi.h>
#include <Wire.h>
#include "SparkFunBME280.h"

BME280 mySensor;

// Set network credentials
const char *ssid = "ESP32_AccessPoint";
const char *password = "12345678";  // At least 8 characters

WiFiServer server(80);  // Create a server on port 80

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Wire.begin(19, 20);
  Wire.setClock(400000);  // Increase to fast I2C speed!

  // Initialize BME280 sensor
  mySensor.beginI2C();
  mySensor.setReferencePressure(101300);

  // Configure ESP32 as an access point
  WiFi.softAP(ssid, password);

  // Start the TCP server
  server.begin();

  // Print the IP address of the AP
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected");


    while (client.connected()) {
      // Read sensor data
      float humidity = mySensor.readFloatHumidity();
      float pressure = mySensor.readFloatPressure();
      float altitude = mySensor.readFloatAltitudeMeters();
      float temperature = mySensor.readTempC();

      // Send data to the client in JSON format
      String data = "{";
      data += "\"humidity\":" + String(humidity, 1) + ",";
      data += "\"pressure\":" + String(pressure, 1) + ",";
      data += "\"altitude\":" + String(altitude, 1) + ",";
      data += "\"temperature\":" + String(temperature, 1);
      data += "}";
      client.println(data);
      Serial.println("Data sent to client: " + data);

      delay(500);
    }

    // Close the connection when the client disconnects
    client.stop();
    Serial.println("Client Disconnected");
  }
}
