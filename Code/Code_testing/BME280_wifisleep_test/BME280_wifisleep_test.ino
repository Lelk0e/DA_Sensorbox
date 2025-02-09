#include <WiFi.h>
#include <Wire.h>
#include "SparkFunBME280.h"

BME280 mySensor;

// Set network credentials
const char *ssid = "ESP32_AccessPoint";
const char *password = "12345678";  // At least 8 characters

WiFiServer server(80);  // Create a server on port 80

// Sleep and activity durations (in milliseconds)
const uint32_t ACTIVE_TIME = 60000;  // 1 minute
const uint32_t SLEEP_TIME = 300000;  // 5 minutes

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Wire.begin(19, 20);
  Wire.setClock(400000);  // Increase to fast I2C speed!

  // Initialize BME280 sensor
  if (!mySensor.beginI2C()) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (true);
  }
  mySensor.setReferencePressure(101300);

  // Configure ESP32 as an access point
  WiFi.softAP(ssid, password);
  server.begin();

  // Print the IP address of the AP
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void readAndPrintSensorData() {
  // Read sensor data
  float humidity = mySensor.readFloatHumidity();
  float pressure = mySensor.readFloatPressure();
  float altitude = mySensor.readFloatAltitudeMeters();
  float temperature = mySensor.readTempC();

  // Print the data to the serial monitor
  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.print("%, Pressure: ");
  Serial.print(pressure / 100.0, 1);  // Convert to hPa
  Serial.print(" hPa, Altitude: ");
  Serial.print(altitude, 1);
  Serial.print(" m, Temperature: ");
  Serial.print(temperature, 1);
  Serial.println(" °C");
}

void activeMode() {
  uint32_t startTime = millis();
  Serial.println("Active Mode: Sending Data...");
  
  while (millis() - startTime < ACTIVE_TIME) {
    WiFiClient client = server.available();
    if (client) {
      Serial.println("New Client Connected");

      while (client.connected() && millis() - startTime < ACTIVE_TIME) {
        // Read and send sensor data
        readAndPrintSensorData();

        // Send data to the client in JSON format
        String data = "{";
        data += "\"humidity\":" + String(mySensor.readFloatHumidity(), 1) + ",";
        data += "\"pressure\":" + String(mySensor.readFloatPressure(), 1) + ",";
        data += "\"altitude\":" + String(mySensor.readFloatAltitudeMeters(), 1) + ",";
        data += "\"temperature\":" + String(mySensor.readTempC(), 1);
        data += "}";
        client.println(data);
        Serial.println("Data sent to client: " + data);

        delay(500);  // Delay between client updates
      }

      // Close the connection when the client disconnects
      client.stop();
      Serial.println("Client Disconnected");
    }
    delay(1000);  // Delay between active reads
  }
}

void lightSleepMode() {
  Serial.println("Entering Light Sleep Mode: Reading Sensor Data...");
  WiFi.disconnect(true);  // Turn off Wi-Fi
  WiFi.mode(WIFI_OFF);

  uint32_t startTime = millis();
  while (millis() - startTime < SLEEP_TIME) {
    readAndPrintSensorData();
    delay(1000);  // Read sensor data every second
  }

  // Re-enable Wi-Fi and restart the server
  Serial.println("Exiting Light Sleep Mode...");
  WiFi.softAP(ssid, password);
  server.begin();
  Serial.println("Wi-Fi and Server Restarted");
}

void loop() {
  // Alternate between active and light sleep modes
  activeMode();
  lightSleepMode();
}
