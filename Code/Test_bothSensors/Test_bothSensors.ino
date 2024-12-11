#include <WiFi.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include "Adafruit_HTU21DF.h"

#define BME280_ADDRESS 0x77
BME280 bme280Sensor;

#define HTU21D_ADDRESS 0x40
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

const char *ssid = "ESP32_AccessPoint";
const char *password = "12345678";

WiFiServer server(80);

const uint32_t ACTIVE_TIME = 60000;
const uint32_t SLEEP_TIME = 300000;

void setup() {
  Serial.begin(115200);
  Wire.begin(19, 20);
  Wire.setClock(400000);
  if (!bme280Sensor.beginI2C(Wire)) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (true);
  }
  bme280Sensor.setI2CAddress(BME280_ADDRESS);
  bme280Sensor.setReferencePressure(101300);
  if (!htu.begin()) {
    Serial.println("Failed to initialize HTU21D-F sensor!");
    while (true);
  }
  WiFi.softAP(ssid, password);
  server.begin();
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void readAndPrintSensorData() {
  float bmeHumidity = bme280Sensor.readFloatHumidity();
  float bmePressure = bme280Sensor.readFloatPressure();
  float bmeAltitude = bme280Sensor.readFloatAltitudeMeters();
  float bmeTemperature = bme280Sensor.readTempC();
  float htuTemperature = htu.readTemperature();
  float htuHumidity = htu.readHumidity();

  Serial.print("BME280 -> Humidity: ");
  Serial.print(bmeHumidity, 1);
  Serial.print("%, Pressure: ");
  Serial.print(bmePressure / 100.0, 1);
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
        readAndPrintSensorData();
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
        delay(1000);
      }
      client.stop();
      Serial.println("Client Disconnected");
    }
    delay(1000);
  }
}

void lightSleepMode() {
  Serial.println("Entering Light Sleep Mode: Reading Sensor Data...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  uint32_t startTime = millis();
  while (millis() - startTime < SLEEP_TIME) {
    readAndPrintSensorData();
    delay(1000);
  }
  Serial.println("Exiting Light Sleep Mode...");
  WiFi.softAP(ssid, password);
  server.begin();
  Serial.println("Wi-Fi and Server Restarted");
}

void loop() {
  activeMode();
  lightSleepMode();
}
