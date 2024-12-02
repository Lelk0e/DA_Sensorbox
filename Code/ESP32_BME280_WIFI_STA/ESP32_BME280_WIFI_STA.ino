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
 
    // Parse JSON data
    int humidityIndex = data.indexOf("\"humidity\":") + 11;
    int pressureIndex = data.indexOf("\"pressure\":") + 11;
    int altitudeIndex = data.indexOf("\"altitude\":") + 11;
    int temperatureIndex = data.indexOf("\"temperature\":") + 14;
 
    float humidity = data.substring(humidityIndex, data.indexOf(",", humidityIndex)).toFloat();
    float pressure = data.substring(pressureIndex, data.indexOf(",", pressureIndex)).toFloat();
    float altitude = data.substring(altitudeIndex, data.indexOf(",", altitudeIndex)).toFloat();
    float temperature = data.substring(temperatureIndex, data.indexOf("}", temperatureIndex)).toFloat();
 
    // Print parsed data
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Pressure: ");
    Serial.print(pressure);
    Serial.print(" Pa, Altitude: ");
    Serial.print(altitude);
    Serial.print(" m, Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
 
  delay(500); // Wait before next read
}