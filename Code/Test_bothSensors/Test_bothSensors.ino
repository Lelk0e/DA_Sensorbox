#include <WiFi.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include "Adafruit_HTU21DF.h"

// BME280 initialization with address 0x77
#define BME280_ADDRESS 0x77
BME280 bme280Sensor;

// HTU21D-F initialization with address 0x40
#define HTU21D_ADDRESS 0x40
Adafruit_HTU21DF htu = Adafruit_HTU21DF();  // Create the sensor object for HTU21D-F

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
  
  // Initialize I2C communication
  Wire.begin(19, 20);  // I2C pins: GPIO19 (SDA), GPIO20 (SCL)
  Wire.setClock(400000);  // Set I2C to fast mode

  // Initialize BME280 sensor at address 0x77
  if (!bme280Sensor.beginI2C(Wire, BME280_ADDRESS)) {
    Serial.println("Failed to initialize BME280 sensor at address 0x77!");
    while (true);  // Halt if BME280 fails to initialize
  }
  bme280Sensor.setReferencePressure(101300);  // Set reference pressure for altitude calculation

  // Initialize HTU21D-F sensor at address 0x40
  if (!htu.begin()) {
    Serial.println("Failed to initialize HTU21D-F sensor at address 0x40!");
    while (true);  // Halt if HTU21D-F fails to initialize
  }

  // Configure ESP32 as an access point
  WiFi.softAP(ssid, password);
  server.begin();

  // Print the IP address of the AP
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

// Function to read and print sensor data
void readAndPrintSensorData() {
  // BME280 data
  float bmeHumidity = bme280Sensor.readFloatHumidity();
  float bmePressure = bme280Sensor.readFloatPressure();
  float bmeAltitude = bme280Sensor.readFloatAltitudeMeters();
  float bmeTemperature = bme280Sensor.readTempC();

  // HTU21D-F data
  float htuTemperature = htu.readTemperature();
  float htuHumidity = htu.readHumidity();

  // Print data from both sensors
  Serial.print("BME280 -> Humidity: ");
  Serial.print(bmeHumidity, 1);
  Serial.print("%, Pressure: ");
  Serial.print(bmePressure / 100.0, 1);  // Convert to hPa
  Serial.print(" hPa, Altitude: ");
  Serial.print(bmeAltitude, 1);
  Serial.print(" m, Temperature: ");
  Serial.print(bmeTemperature, 1);
  Serial.println(" °C");

  Serial.print("HTU21D-F -> Temperature: ");
  Serial.print(htuTemperature, 1);
  Serial.print(" °C, Humidity: ");
  Serial.print(htuHumidity, 1);
  Serial.println(" %");
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
        data += "\"bmeHumidity\":" + String(bme280Sensor.readFloatHumidity(), 1) + ","; 
        data += "\"bmePressure\":" + String(bme280Sensor.readFloatPressure() / 100.0, 1) + ","; 
        data += "\"bmeAltitude\":" + String(bme280Sensor.readFloatAltitudeMeters(), 1) + ","; 
        data += "\"bmeTemperature\":" + String(bme280Sensor.readTempC(), 1) + ","; 
        data += "\"htuTemperature\":" + String(htu.readTemperature(), 1) + ","; 
        data += "\"htuHumidity\":" + String(htu.readHumidity(), 1);
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
