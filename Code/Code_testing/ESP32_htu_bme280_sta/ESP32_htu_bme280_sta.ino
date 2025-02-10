#include <WiFi.h>

// Network credentials (must match the AP settings on the sending ESP32)
const char *ssid = "ESP32_AccessPoint";
const char *password = "12345678";

// Server IP and port (match the IP of the sending ESP32 AP)
const char *serverIP = "192.168.4.1"; // Default IP for ESP32 in AP mode
const uint16_t serverPort = 80;       // Same port as server on the sending ESP32

WiFiClient client;

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Connect to the Wi-Fi network created by the first ESP32
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);

  // Wait until the client is connected to the access point
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi");
}

void loop()
{
  // Check if we're connected to the server
  if (!client.connected())
  {
    Serial.println("Connecting to server...");
    if (client.connect(serverIP, serverPort))
    {
      Serial.println("Connected to server");
    }
    else
    {
      Serial.println("Connection failed!");
      delay(2000); // Retry every 2 seconds
      return;
    }
  }

  // Read incoming data from the server
  while (client.available())
  {
    String data = client.readStringUntil('\n'); // Read until end of line (JSON line)
    Serial.println("Received data: " + data);

    // Parse JSON data for BME280
    int bmeHumidityIndex = data.indexOf("\"bmeHumidity\":") + 14;
    int bmePressureIndex = data.indexOf("\"bmePressure\":") + 14;
    int bmeAltitudeIndex = data.indexOf("\"bmeAltitude\":") + 14;
    int bmeTemperatureIndex = data.indexOf("\"bmeTemperature\":") + 17;

    float bmeHumidity = data.substring(bmeHumidityIndex, data.indexOf(",", bmeHumidityIndex)).toFloat();
    float bmePressure = data.substring(bmePressureIndex, data.indexOf(",", bmePressureIndex)).toFloat();
    float bmeAltitude = data.substring(bmeAltitudeIndex, data.indexOf(",", bmeAltitudeIndex)).toFloat();
    float bmeTemperature = data.substring(bmeTemperatureIndex, data.indexOf(",", bmeTemperatureIndex)).toFloat();

    // Parse JSON data for HTU21DF
    int htuTemperatureIndex = data.indexOf("\"htuTemperature\":") + 17;
    int htuHumidityIndex = data.indexOf("\"htuHumidity\":") + 14;

    float htuTemperature = data.substring(htuTemperatureIndex, data.indexOf(",", htuTemperatureIndex)).toFloat();
    float htuHumidity = data.substring(htuHumidityIndex, data.indexOf("}", htuHumidityIndex)).toFloat();

    // Print parsed data for both sensors
    Serial.println("BME280 Sensor Data:");
    Serial.print("  Humidity: ");
    Serial.print(bmeHumidity);
    Serial.print("%, Pressure: ");
    Serial.print(bmePressure);
    Serial.print(" hPa, Altitude: ");
    Serial.print(bmeAltitude);
    Serial.print(" m, Temperature: ");
    Serial.print(bmeTemperature);
    Serial.println(" °C");

    Serial.println("HTU21DF Sensor Data:");
    Serial.print("  Temperature: ");
    Serial.print(htuTemperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(htuHumidity);
    Serial.println("%");
  }

  delay(500); // Wait before next read
}
