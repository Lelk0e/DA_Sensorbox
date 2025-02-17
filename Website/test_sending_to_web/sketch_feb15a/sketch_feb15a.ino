#include <ESPAsyncWebServer.h>
#include <WiFi.h>

AsyncWebServer server(80);

// Network credentials (must match the AP settings on the sending ESP32)
const char *ssid = "ESP32_test_sending";
const char *password = "deeznuts";
 
// Server IP and port (match the IP of the sending ESP32 AP)
const char *serverIP = "192.168.4.1"; // Default IP for ESP32 in AP mode
const uint16_t serverPort = 80;       // Same port as server on the sending ESP32

String msgBME = "Time:11:30:08:BME:-6.16\nTime:11:30:38:BME:31.97\nTime:11:31:08:BME:9.05\nTime:11:31:38:BME:1.56\nTime:11:32:08:BME:13.92\nTime:11:32:38:BME:37.68\nTime:11:33:08:BME:33.69\nTime:11:33:38:BME:24.49\nTime:11:34:08:BME:31.39\nTime:11:34:38:BME:25.84\nTime:11:35:08:BME:15.9\nTime:11:35:38:BME:-8.05\nTime:11:36:08:BME:0.58\nTime:11:36:38:BME:29.17\nTime:11:37:08:BME:39.38\nTime:11:37:38:BME:13.2\nTime:11:38:08:BME:22.78\nTime:11:38:38:BME:-0.13\nTime:11:39:08:BME:36.73\nTime:11:39:38:BME:-0.84\nTime:11:40:08:BME:11.67\nTime:11:40:38:BME:38.87\nTime:11:41:08:BME:28.73\nTime:11:41:38:BME:38.58\nTime:11:42:08:BME:-8.95\nTime:11:42:38:BME:10.34\nTime:11:43:08:BME:29.47\nTime:11:43:38:BME:-2.81\nTime:11:44:08:BME:22.15\nTime:11:44:38:BME:25.21\nTime:11:45:08:BME:7.74\nTime:11:45:38:BME:16.08\nTime:11:46:08:BME:-5.23\nTime:11:46:38:BME:17.41\nTime:11:47:08:BME:-3.66\nTime:11:47:38:BME:34.61\nTime:11:48:08:BME:18.48\nTime:11:48:38:BME:-6.05\nTime:11:49:08:BME:35.43\nTime:11:49:38:BME:38.44\nTime:11:50:08:BME:-1.57\nTime:11:50:38:BME:19.93\nTime:11:51:08:BME:20.14\nTime:11:51:38:BME:-6.56\nTime:11:52:08:BME:8.47\nTime:11:52:38:BME:36.72\nTime:11:53:08:BME:20.53\nTime:11:53:38:BME:4.05\nTime:11:54:08:BME:20.84\nTime:11:54:38:BME:29.05\nTime:11:55:08:BME:24.32\nTime:11:55:38:BME:28.73\nTime:11:56:08:BME:24.08\nTime:11:56:38:BME:21.16\nTime:11:57:08:BME:36.69\nTime:11:57:38:BME:25.4\nTime:11:58:08:BME:25.52\nTime:11:58:38:BME:6.03\nTime:11:59:08:BME:29.75\nTime:11:59:38:BME:-3.06\nTime:12:00:08:BME:19.16\nTime:12:00:38:BME:16.07\nTime:12:01:08:BME:14.53\nTime:12:01:38:BME:24.58\nTime:12:02:08:BME:-0.37\nTime:12:02:38:BME:6.06\nTime:12:03:08:BME:-3.31\nTime:12:03:38:BME:4.62\nTime:12:04:08:BME:-9.67\nTime:12:04:38:BME:6.13\nTime:12:05:08:BME:21.29\nTime:12:05:38:BME:16.16\nTime:12:06:08:BME:8.46\nTime:12:06:38:BME:-1.27\nTime:12:07:08:BME:10.0\nTime:12:07:38:BME:-5.95\nTime:12:08:08:BME:18.73\nTime:12:08:38:BME:17.48\nTime:12:09:08:BME:5.92\nTime:12:09:38:BME:30.12\nTime:12:10:08:BME:16.74\nTime:12:10:38:BME:39.02\nTime:12:11:08:BME:11.24\nTime:12:11:38:BME:-3.48\nTime:12:12:08:BME:19.44\nTime:12:12:38:BME:35.31\nTime:12:13:08:BME:13.81\nTime:12:13:38:BME:10.18\nTime:12:14:08:BME:28.26\nTime:12:14:38:BME:17.36\nTime:12:15:08:BME:2.21\nTime:12:15:38:BME:12.95\nTime:12:16:08:BME:30.81\nTime:12:16:38:BME:-1.07\nTime:12:17:08:BME:39.37\nTime:12:17:38:BME:25.26\nTime:12:18:08:BME:-9.11\nTime:12:18:38:BME:-8.35\nTime:12:19:08:BME:6.93\nTime:12:19:38:BME:25.79";
 
WiFiClient client;

void setup() {
  // put your setup code here, to run once:
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

  server.on("/bme", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", msgBME);
  });

  server.begin();
}

void loop() {
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
    int data_index = data.indexOf("BME:");
    int time__index = data.indexOf("Time:");
    int time_end_index = data.indexof(":BME");

    String data_slip = data.substring(data_index); //get information out of string
    String time_slip = data.substring(time_index, time_end_index); //same here
 
    // Print parsed data
    Serial.print("data: ");
    Serial.print(data_slip);
    Serial.print("[h:min:s] time: ");
    Serial.print(time_slip);

    //Serial.println(" °C");
  }
 
  delay(500); // Wait before next read
}

void receivedCallback(String &from, String &msg) {
  if (from.equals("BME280")) {
    msgBME = msg;
  } 
}