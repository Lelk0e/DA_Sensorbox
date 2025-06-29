#include <SPI.h>
#include "namedMesh.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <SD.h>
#include "ulog_sqlite.h"
#include "DNSServer.h"
#include "RTClib.h"
#include <WiFi.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include "Adafruit_HTU21DF.h"
#include "Adafruit_MAX31855.h"

#define cs_pin 5
#define sda_pin 21
#define scl_pin 22
#define ozon_pin 32
#define external_analogIn 33

// Sensor objects
BME280 bme280Sensor;
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_MAX31855 thermocouple(14, 15, 12); // MAXCLK, MAXCS, MAXDO

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

Scheduler userSched;
namedMesh mesh;

String nodeName = "mainESP";

// Time variables
int year;
int month;
int day;
int hour;
int minute;
int second;
int dyear;
int dmonth;
int dday;
int dhour;
int dminute;
int dsecond;

String sensorTable;
float BMEDataValue;
float HTUDataValue;
float TypKDataValue;
float OzonDataValue;

String webData;

volatile bool lowPowerMode = false;
volatile bool meshEnabled = true;
volatile int apClientsConnected = 0;
volatile bool webSocketConnected = false;
volatile bool websiteReady = false; // Flag to indicate website is ready to receive data

FILE *dbFile;
FILE *readDbFile = NULL;

#define BUF_SIZE 4096
byte buf[BUF_SIZE];

AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

DNSServer dnsServer;
const byte DNS_PORT = 53;

bool toggleOnOff = false;

// Task for reading local sensor data
Task *localSensorTask;

void receivedCallback(String &from, String &msg);
void newConnectionCallback(uint32_t nodeId);
String messageType(String msg);
void dataSplit(String s, char del);
void logNodeData();
void readLocalSensors();
void toggleMesh(bool enable);
int32_t read_fn_wctx(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len);
int flush_fn(struct dblog_write_context *ctx);
int32_t write_fn(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len);
int32_t read_fn_rctx(struct dblog_read_context *ctx, void *buffer, uint32_t pos, size_t len);
void print_error(int res);
void exitLPM();
String wrDBtoWs(const char *filename);
void verifyMeshStatus();
void initMesh();
void checkAPConnections();
void meshReceivedCallback(uint32_t from, String &msg);
void sendRoot();

// Sensor reading functions
float readHTU()
{
  float humidity = htu.readHumidity();
  if (isnan(humidity))
    return -1;
  return humidity;
}

float readBme()
{
  float pressure = bme280Sensor.readFloatPressure();
  if (isnan(pressure))
    return -1;
  return pressure;
}

float readTypK()
{
  float temperatureK = thermocouple.readCelsius();
  return temperatureK;
}

uint16_t readOzon()
{
  uint16_t ppmValue = analogRead(ozon_pin);
  return ppmValue / 0.805;
}

void readLocalSensors()
{
  if (!meshEnabled) {
    Serial.println("=== Reading Local Sensors ===");
    
    int BMEValue = static_cast<int>(readBme());
    int HTUValue = static_cast<int>(readHTU());
    int TypKValue = static_cast<int>(readTypK());
    int OzonValue = static_cast<int>(readOzon());
    
    Serial.print("BME: "); Serial.print(BMEValue); Serial.println(" Pa");
    Serial.print("HTU: "); Serial.print(HTUValue); Serial.println(" %");
    Serial.print("TypK: "); Serial.print(TypKValue); Serial.println(" °C");
    Serial.print("Ozon: "); Serial.print(OzonValue); Serial.println(" mV");
    
    DateTime now = rtc.now();
    char timestamp[24];
    snprintf(timestamp, sizeof(timestamp), "%04d:%02d:%02d:%02d:%02d:%02d",
             now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    
    Serial.print("Timestamp: "); Serial.println(timestamp);
    Serial.print("WebSocket clients: "); Serial.println(ws.count());
    Serial.print("Website ready: "); Serial.println(websiteReady ? "Yes" : "No");
    
    // Only send data via WebSocket if website is ready AND WebSocket clients are connected
    if (websiteReady && ws.count() > 0) {
      String sensorData = "{\"timestamp\":\"" + String(timestamp) + "\",\"node\":\"mainESP\",\"bme\":" + String(BMEValue) + ",\"htu\":" + String(HTUValue) + ",\"typk\":" + String(TypKValue) + ",\"ozon\":" + String(OzonValue) + "}";
      ws.textAll(sensorData);
      // Buffer main ESP data as well
      webData += sensorData + "\n";
      Serial.println("Local sensor data sent via WebSocket");
    } else {
      if (!websiteReady) {
        Serial.println("Website not ready (OnOff not activated) - data not sent");
      } else if (ws.count() == 0) {
        Serial.println("No WebSocket clients connected - data paused (auto-pause)");
      }
    }
    
    Serial.println("================================");
  } else {
    Serial.println("Mesh enabled - skipping local sensor reading");
  }
}

void toggleMesh(bool enable)
{
  if (enable && !meshEnabled) {
    Serial.println("=== ENABLING MESH NETWORK ===");
    meshEnabled = true;
    initMesh();
    delay(2000); // Give mesh time to initialize
    sendRoot();  // Always send Root message after mesh is enabled
    delay(100);  // Give time for the message to propagate
    // Send broadcast to tell clients to send their data
    Serial.println("Sending 'SendData' broadcast to all clients");
    mesh.sendBroadcast("SendData");
    Serial.println("Mesh network enabled and broadcast sent");
    verifyMeshStatus();
    Serial.println("================================");
  } else if (!enable && meshEnabled) {
    Serial.println("=== DISABLING MESH NETWORK ===");
    meshEnabled = false;
    // More thorough mesh stopping
    Serial.println("Stopping mesh network...");
    mesh.stop();
    delay(1000);
    // Force WiFi mode to AP only for web server
    WiFi.mode(WIFI_AP);
    delay(500);
    Serial.println("Mesh network stopped and WiFi mode set to AP only");
    verifyMeshStatus();
    Serial.println("================================");
  } else {
    Serial.print("Mesh toggle ignored - current state: ");
    Serial.println(meshEnabled ? "enabled" : "disabled");
  }
}

void initMesh()
{
  Serial.println("=== INITIALIZING MESH NETWORK ===");
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Mesh", "12345678", &userSched, 5555);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);
  Serial.println(mesh.getAPIP());
  mesh.setName(nodeName);
  Serial.println("Registering receivedCallback...");
  // mesh.onReceive(&receivedCallback);  // Commented out - namedMesh is blocking messages
  // Note: If messages are not reaching receivedCallback, it might be due to
  // namedMesh internal processing. Consider using painlessMesh::onReceive directly
  // or checking if namedMesh is properly forwarding non-nameBroadCast messages.
  
  // Alternative: Try using the base painlessMesh callback if namedMesh fails
  // mesh.onReceive(&meshReceivedCallback);  // This still goes through namedMesh
  
  // Try to bypass namedMesh by accessing the underlying painlessMesh
  static_cast<painlessMesh*>(&mesh)->onReceive(&meshReceivedCallback);
  
  Serial.println("Registering newConnectionCallback...");
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]()
                            { Serial.printf("Changed connection\n"); });
  Serial.println("Mesh network initialized successfully");
  Serial.println("================================");
}

void sendRoot()
{
  mesh.sendBroadcast("Root:" + String(mesh.getNodeId()));
}

// Direct callback for painlessMesh messages
void meshReceivedCallback(uint32_t from, String &msg) {
  Serial.println("=== MESH RECEIVED CALLBACK ===");
  Serial.printf("From ID: %u\n", from);
  Serial.printf("Message: %s\n", msg.c_str());
  Serial.printf("Message length: %d\n", msg.length());
  Serial.printf("Message starts with JSON: %s\n", msg.startsWith("{\"timestamp\":" ) ? "YES" : "NO");
  
  // If the message is 'On', send all stored data line by line to WebSocket clients
  if (msg == "On") {
    Serial.println("Received 'On' from client - sending all stored data to website clients");
    int startPos = 0;
    int newlinePos = webData.indexOf('\n', startPos);
    while (newlinePos != -1) {
      String record = webData.substring(startPos, newlinePos);
      if (record.length() > 0) {
        ws.textAll(record);
        delay(10); // Small delay to avoid flooding
      }
      startPos = newlinePos + 1;
      newlinePos = webData.indexOf('\n', startPos);
    }
    // Send any remaining data (in case there's no final newline)
    if (startPos < webData.length()) {
      String lastRecord = webData.substring(startPos);
      if (lastRecord.length() > 0) {
        ws.textAll(lastRecord);
      }
    }
    Serial.println("All stored data sent to website clients");
    // Optionally clear webData here if you want to avoid duplicates
    // webData = "";
  }
  // Check if message is JSON format (from clients) or old format
  else if (msg.startsWith("{\"timestamp\":")) {
    // JSON format from clients - add to webData for later sending
    Serial.println("Received JSON data from client - adding to webData");
    webData += msg + "\n";
    Serial.println("Data added to webData for WebSocket transmission");
    Serial.printf("webData length now: %d\n", webData.length());
  } else if (messageType(msg) == "Data") {
    // Old format - keep for backward compatibility
    Serial.println("Processing old format data message...");
    dataSplit(msg, ':');
    logNodeData();
    webData += msg;
    Serial.println("Data logged and added to web data");
  } else {
    Serial.println("Message format not recognized");
  }
  Serial.println("=============================");
}

void receivedCallback(String &from, String &msg)
{
  Serial.println("=== MESH MESSAGE RECEIVED ===");
  Serial.printf("From: %s\n", from.c_str());
  Serial.printf("Message: %s\n", msg.c_str());
  Serial.printf("Message length: %d\n", msg.length());
  Serial.printf("Message starts with JSON: %s\n", msg.startsWith("{\"timestamp\":") ? "YES" : "NO");
  
  // Check if message is JSON format (from clients) or old format
  if (msg.startsWith("{\"timestamp\":")) {
    // JSON format from clients - add to webData for WebSocket
    Serial.println("Received JSON data from client - adding to webData");
    webData += msg + "\n";
    Serial.println("Data added to webData for WebSocket transmission");
    Serial.printf("webData length now: %d\n", webData.length());
  } else if (messageType(msg) == "Data") {
    // Old format - keep for backward compatibility
    Serial.println("Processing old format data message...");
    dataSplit(msg, ':');
    logNodeData();
    webData += msg;
    Serial.println("Data logged and added to web data");
  } else {
    Serial.println("Message format not recognized");
  }
  Serial.println("=============================");
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.println("=== NEW MESH NODE CONNECTED ===");
  Serial.printf("Node ID: %u\n", nodeId);
  
  sendRoot();
  Serial.println("Root message sent to new node");

  DateTime now = rtc.now();
  String timeMessage = "Time:" +
                       String(now.year()) + ":" +
                       String(now.month()) + ":" +
                       String(now.day()) + ":" +
                       String(now.hour()) + ":" +
                       String(now.minute()) + ":" +
                       String(now.second());
  mesh.sendBroadcast(timeMessage);
  Serial.println("Time sync message sent to all nodes");
  Serial.println("================================");
}

void dataSplit(String s, char del)
{
  int idx1 = s.indexOf(del);
  if (idx1 == -1)
    return;
  int idx2 = s.indexOf(del, idx1 + 1);
  if (idx2 == -1)
    return;
  int idx3 = s.indexOf(del, idx2 + 1);
  if (idx3 == -1)
    return;
  int idx4 = s.indexOf(del, idx3 + 1);
  if (idx4 == -1)
    return;
  int idx5 = s.indexOf(del, idx4 + 1);
  if (idx5 == -1)
    return;
  int idx6 = s.indexOf(del, idx5 + 1);
  if (idx6 == -1)
    return;
  int idx7 = s.indexOf(del, idx6 + 1);
  if (idx7 == -1)
    return;
  int idx8 = s.indexOf(del, idx7 + 1);
  if (idx8 == -1)
    return;
  int idx9 = s.indexOf(del, idx8 + 1);
  if (idx9 == -1)
    return;
  int idx10 = s.indexOf(del, idx9 + 1);
  if (idx10 == -1)
    return;
  int idx11 = s.indexOf(del, idx10 + 1);
  if (idx11 == -1)
    return;
  int idx12 = s.indexOf(del, idx11 + 1);
  if (idx12 == -1)
    return;
  String yearStr = s.substring(idx2 + 1, idx3);
  String monthStr = s.substring(idx3 + 1, idx4);
  String dayStr = s.substring(idx4 + 1, idx5);
  String hourStr = s.substring(idx5 + 1, idx6);
  String minuteStr = s.substring(idx6 + 1, idx7);
  String secondStr = s.substring(idx7 + 1, idx8);
  dyear = yearStr.toInt();
  dmonth = monthStr.toInt();
  dday = dayStr.toInt();
  dhour = hourStr.toInt();
  dminute = minuteStr.toInt();
  dsecond = secondStr.toInt();
  sensorTable = s.substring(idx8 + 1, idx9);
  BMEDataValue = (s.substring(idx9 + 1)).toFloat();
  HTUDataValue = (s.substring(idx10 + 1)).toFloat();
  TypKDataValue = (s.substring(idx11 + 1)).toFloat();
  OzonDataValue = (s.substring(idx12 + 1)).toFloat();
}

void logNodeData()
{
  if (sensorTable.length() == 0)
  {
    Serial.println("No table id");
    return;
  }

  char dbFilename[32];
  sprintf(dbFilename, "/sd/%s.db", sensorTable.c_str());

  dbFile = fopen(dbFilename, "a+b");
  if (!dbFile)
  {
    Serial.println("Failed to open SQLite DB file!");
    return;
  }

  struct dblog_write_context ctx;
  ctx.buf = buf;
  ctx.col_count = 3;
  ctx.page_resv_bytes = 0;
  ctx.page_size_exp = 11;
  ctx.max_pages_exp = 0;
  ctx.read_fn = read_fn_wctx;
  ctx.write_fn = write_fn;
  ctx.flush_fn = flush_fn;

  int res = dblog_write_init(&ctx);
  if (!res)
  {
    char ts[32];
    sprintf(ts, "%04d:%02d:%02d:%02d:%02d:%02d", dyear, dmonth, dday, dhour, dminute, dsecond);
    int BMEValue = (int)BMEDataValue;
    int HTUValue = (int)HTUDataValue;
    res = dblog_set_col_val(&ctx, 0, DBLOG_TYPE_TEXT, ts, strlen(ts));
    res = dblog_set_col_val(&ctx, 1, DBLOG_TYPE_INT, &BMEValue, sizeof(BMEValue));
    res = dblog_set_col_val(&ctx, 2, DBLOG_TYPE_INT, &HTUValue, sizeof(HTUValue));
    res = dblog_append_empty_row(&ctx);
  }
  res = dblog_finalize(&ctx);
  fclose(dbFile);
  if (res)
    print_error(res);
  else
    Serial.println("Data logged");
}

void initSDCard()
{
  if (!SD.begin(cs_pin))
  {
    Serial.println("Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Size: %lluMB\n", cardSize);
}

int32_t read_fn_wctx(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len)
{
  if (fseek(dbFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fread(buffer, 1, len, dbFile);
  if (ret != len)
    return DBLOG_RES_READ_ERR;
  return ret;
}

int32_t write_fn(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len)
{
  if (fseek(dbFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fwrite(buffer, 1, len, dbFile);
  if (ret != len)
    return DBLOG_RES_ERR;
  if (fflush(dbFile))
    return DBLOG_RES_FLUSH_ERR;
  fsync(fileno(dbFile));
  return ret;
}

int flush_fn(struct dblog_write_context *ctx)
{
  return DBLOG_RES_OK;
}

int32_t read_fn_rctx(struct dblog_read_context *ctx, void *buffer, uint32_t pos, size_t len)
{
  if (fseek(dbFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fread(buffer, 1, len, dbFile);
  if (ret != len)
    return DBLOG_RES_READ_ERR;
  return ret;
}

void print_error(int res)
{
  Serial.print(F("Err: "));
  Serial.println(res);
}

String messageType(String msg)
{
  if (msg.indexOf(':') != -1)
  {
    int firstDel = msg.indexOf(':');
    return msg.substring(0, firstDel);
  }
  return "";
}

void LPM(unsigned long durationMillis)
{
  mesh.sendBroadcast("LPMsh");
  lowPowerMode = true;
  mesh.stop();
  Serial.println("Entering LPM");
  Task *exitTask = new Task(durationMillis, 1, []()
                            { exitLPM(); });
  userSched.addTask(*exitTask);
  exitTask->enable();
}

void exitLPM()
{
  lowPowerMode = false;
  Serial.println("Exiting LPM");
  initMesh();
  Task *nextLpmTask = new Task(5000, 1, []()
                               { LPM(30000); });
  userSched.addTask(*nextLpmTask);
  nextLpmTask->enable();
}

String wrDBtoWs(const char *filename)
{
  String data = "";
  dbFile = fopen(filename, "rb");
  if (!dbFile)
  {
    Serial.println("Error opening DB for reading");
    return data;
  }
  struct dblog_read_context rctx;
  rctx.page_size_exp = 12;
  rctx.read_fn = read_fn_rctx;
  rctx.buf = buf;
  int res = dblog_read_init(&rctx);
  if (res)
  {
    print_error(res);
    fclose(dbFile);
    return data;
  }
  while (true)
  {
    uint32_t colType0, colType1;
    uint8_t *colVal0 = (uint8_t *)dblog_read_col_val(&rctx, 0, &colType0);
    uint8_t *colVal1 = (uint8_t *)dblog_read_col_val(&rctx, 1, &colType1);
    if (!colVal0 || !colVal1)
      break;
    char ts[32];
    strncpy(ts, (const char *)colVal0, sizeof(ts) - 1);
    ts[sizeof(ts) - 1] = '\0';
    int sensorValue;
    memcpy(&sensorValue, colVal1, sizeof(sensorValue));
    data += "Time:" + String(ts) + ":" + filename + ":" + String(sensorValue) + "\n";
    if (dblog_read_next_row(&rctx) != 0)
      break;
  }
  fclose(dbFile);
  return data;
}

void verifyMeshStatus()
{
  Serial.println("=== MESH STATUS VERIFICATION ===");
  Serial.print("meshEnabled flag: "); Serial.println(meshEnabled ? "true" : "false");
  Serial.print("lowPowerMode: "); Serial.println(lowPowerMode ? "true" : "false");
  Serial.print("AP clients: "); Serial.println(apClientsConnected);
  Serial.print("WiFi mode: ");
  switch(WiFi.getMode()) {
    case WIFI_MODE_NULL: Serial.println("NULL"); break;
    case WIFI_MODE_STA: Serial.println("STA"); break;
    case WIFI_MODE_AP: Serial.println("AP"); break;
    case WIFI_MODE_APSTA: Serial.println("APSTA"); break;
    default: Serial.println("UNKNOWN"); break;
  }
  Serial.println("================================");
}

void checkAPConnections()
{
  static int lastAPClients = 0;
  int currentAPClients = WiFi.softAPgetStationNum();
  
  if (currentAPClients != lastAPClients) {
    Serial.println("=== AP CONNECTION CHANGE ===");
    Serial.print("Previous AP clients: "); Serial.println(lastAPClients);
    Serial.print("Current AP clients: "); Serial.println(currentAPClients);
    
    apClientsConnected = currentAPClients;
    
    if (currentAPClients > lastAPClients) {
      // New client connected
      Serial.println("New AP client connected");
      if (apClientsConnected == 1) {
        Serial.println("First AP client - requesting data before disabling mesh");
        
        // Send SendData broadcast first to collect client data
        if (meshEnabled) {
          Serial.println("Sending SendData broadcast to collect client data");
          mesh.sendBroadcast("SendData");
          
          // Wait for clients to respond (give them time to send data)
          Serial.println("Waiting 3 seconds for clients to send data...");
          delay(3000);
          
          Serial.println("Now disabling mesh network");
          toggleMesh(false);
        } else {
          Serial.println("Mesh already disabled - skipping data collection");
        }
      }
    } else if (currentAPClients < lastAPClients) {
      // Client disconnected
      Serial.println("AP client disconnected");
      if (apClientsConnected == 0) {
        Serial.println("No AP clients remaining - enabling mesh network");
        toggleMesh(true);
      }
    }
    
    Serial.println("=============================");
    lastAPClients = currentAPClients;
  }
  
  // Additional check: if AP clients are connected but mesh is still enabled, force disable
  if (currentAPClients > 0 && meshEnabled) {
    Serial.println("=== FORCE MESH DISABLE ===");
    Serial.print("AP clients: "); Serial.println(currentAPClients);
    Serial.println("Mesh still enabled - forcing disable");
    
    // Send SendData broadcast first to collect client data
    Serial.println("Sending SendData broadcast to collect client data");
    mesh.sendBroadcast("SendData");
    
    // Wait for clients to respond
    Serial.println("Waiting 3 seconds for clients to send data...");
    delay(3000);
    
    Serial.println("Now disabling mesh network");
    toggleMesh(false);
    Serial.println("================================");
  }
  
  // Additional check: if no AP clients but mesh is disabled, force enable
  if (currentAPClients == 0 && !meshEnabled) {
    Serial.println("=== FORCE MESH ENABLE ===");
    Serial.println("No AP clients and mesh disabled - forcing enable");
    toggleMesh(true);
    Serial.println("================================");
  }
}

void setup()
{
  Serial.begin(115200);
  
  // Initialize I2C and sensors
  Wire.begin(sda_pin, scl_pin);
  delay(100);
  
  // Initialize sensors
  bme280Sensor.setI2CAddress(0x76);
  if (!bme280Sensor.beginI2C(Wire)) {
    Serial.println("BME280 not working");
  } else {
    Serial.println("BME280 working");
  }
  
  if (!htu.begin()) {
    Serial.println("HTU21DF not working");
  } else {
    Serial.println("HTU21DF working");
  }
  
  if (!thermocouple.begin()) {
    Serial.println("MAX31855 not working");
  } else {
    Serial.println("MAX31855 working");
  }
  
  // Initialize ozone sensor pin
  pinMode(ozon_pin, INPUT);
  
  initMesh();
  initSDCard();
  //WiFi.mode(WIFI_AP);
  //WiFi.softAP("SensorBoxAP", "12345678");
  //IPAddress myIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "sensorbox.com", mesh.getAPIP());
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }
  
  // Create local sensor reading task
  localSensorTask = new Task(1000, TASK_FOREVER, readLocalSensors);
  userSched.addTask(*localSensorTask);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SD, "/webpage/website.html", "text/html"); });

  server.on("/design.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SD, "/webpage/design.css", "text/css"); });

  server.on("/web_code.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SD, "/webpage/web_code.js", "application/javascript"); });

  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if(request->hasParam("year") && request->hasParam("month") && request->hasParam("day") && request->hasParam("hour") && request->hasParam("minute") && request->hasParam("second")){
    year = request->getParam("year")->value().toInt();
    month = request->getParam("month")->value().toInt();
    day = request->getParam("day")->value().toInt();
    hour = request->getParam("hour")->value().toInt();
    minute = request->getParam("minute")->value().toInt();
    second = request->getParam("second")->value().toInt();
    Serial.print("Received time: ");
    Serial.print(year); Serial.print("-");
    Serial.print(month); Serial.print("-");
    Serial.print(day); Serial.print(" ");
    Serial.print(hour); Serial.print(":");
    Serial.print(minute); Serial.print(":");
    Serial.println(second);
    DateTime newDateTime(year, month, day, hour, minute, second);
    rtc.adjust(newDateTime);
    String timeMessage = "Time:" +
                       String(year) + ":" +
                       String(month) + ":" +
                       String(day) + ":" +
                       String(hour) + ":" +
                       String(minute) + ":" +
                       String(second);
  mesh.sendBroadcast(timeMessage);
    request->send(200, "text/plain", "Time received successfully");
  } else {
    request->send(400, "text/plain", "Missing time parameters");
  } });

  server.on("/OnOff", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if(request->hasParam("On") || request->hasParam("Off")){
      if(request->hasParam("On")){
        toggleOnOff = true;
        websiteReady = true; // Website is ready to receive data
        Serial.println("=== WEBSITE READY ===");
        Serial.println("OnOff toggle activated - system is ready to send data");
        Serial.println("Data will be sent when WebSocket clients are connected");
        Serial.println("=========================");
        mesh.sendBroadcast("On");
        // Send all buffered data to all WebSocket clients
        int startPos = 0;
        int newlinePos = webData.indexOf('\n', startPos);
        while (newlinePos != -1) {
          String record = webData.substring(startPos, newlinePos);
          if (record.length() > 0) {
            ws.textAll(record);
            delay(10);
          }
          startPos = newlinePos + 1;
          newlinePos = webData.indexOf('\n', startPos);
        }
        if (startPos < webData.length()) {
          String lastRecord = webData.substring(startPos);
          if (lastRecord.length() > 0) {
            ws.textAll(lastRecord);
          }
        }
      } else if (request->hasParam("Off")) {
        if (toggleOnOff == true)
        {
          mesh.sendBroadcast("Finalize");
          mesh.sendBroadcast("Off");
        }
        toggleOnOff = false;
        websiteReady = false; // Website is no longer ready
        Serial.println("=== WEBSITE NOT READY ===");
        Serial.println("OnOff toggle deactivated - system stopped");
        Serial.println("===========================");
        // Do NOT clear webData here - keep data for next On cycle
        // webData = "";
      }
    } else {
      request->send(400, "text/plain", "error toggle on off");
    } });

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
             {
      if (type == WS_EVT_CONNECT) {
        Serial.println("=== WEBSOCKET CLIENT CONNECTED ===");
        Serial.print("Client IP: "); Serial.println(client->remoteIP());
        Serial.print("Total WebSocket clients: "); Serial.println(ws.count());
        Serial.print("Website ready: "); Serial.println(websiteReady ? "Yes" : "No");
        
        webSocketConnected = true;
        
        // Auto-resume data sending if website is ready
        if (websiteReady) {
          Serial.println("=== AUTO-RESUME DATA SENDING ===");
          Serial.println("WebSocket client connected - resuming data transmission");
          Serial.println("=================================");
        }
        
        // Send stored web data to new client
        if (webData != "" && !webData.isEmpty()) {
          Serial.println("Sending stored web data to new client");
          Serial.print("webData length: "); Serial.println(webData.length());
          // Split webData by newlines and send each measurement individually
          int startPos = 0;
          int newlinePos = webData.indexOf('\n', startPos);
          int count = 0;
          while (newlinePos != -1) {
            String measurement = webData.substring(startPos, newlinePos);
            if (measurement.length() > 0) {
              Serial.print("Sending measurement: "); Serial.println(measurement);
              client->text(measurement);
              delay(50); // Increased delay between messages
              count++;
              if (count % 10 == 0) delay(200); // Longer pause every 10 messages
            }
            startPos = newlinePos + 1;
            newlinePos = webData.indexOf('\n', startPos);
          }
          // Send any remaining data (in case there's no final newline)
          if (startPos < webData.length()) {
            String lastMeasurement = webData.substring(startPos);
            if (lastMeasurement.length() > 0) {
              Serial.print("Sending final measurement: "); Serial.println(lastMeasurement);
              client->text(lastMeasurement);
            }
          }
          Serial.println("All stored measurements sent individually");
          // Do NOT clear webData here - keep it for other clients
        }
        Serial.println("=====================================");
      } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("=== WEBSOCKET CLIENT DISCONNECTED ===");
        Serial.print("Client IP: "); Serial.println(client->remoteIP());
        Serial.print("Remaining WebSocket clients: "); Serial.println(ws.count());
        Serial.print("Current AP clients: "); Serial.println(WiFi.softAPgetStationNum());
        Serial.print("Mesh enabled: "); Serial.println(meshEnabled ? "Yes" : "No");
        
        if (ws.count() == 0) {
          webSocketConnected = false;
          
          // Auto-pause data sending when no WebSocket clients
          if (websiteReady) {
            Serial.println("=== AUTO-PAUSE DATA SENDING ===");
            Serial.println("No WebSocket clients connected - pausing data transmission");
            Serial.println("Data will resume when clients reconnect");
            Serial.println("=================================");
          }
        }
        
        // Force check AP connections after WebSocket disconnect to ensure correct state
        Serial.println("=== FORCE AP CHECK AFTER WEBSOCKET DISCONNECT ===");
        int currentAPClients = WiFi.softAPgetStationNum();
        Serial.print("AP clients after WebSocket disconnect: "); Serial.println(currentAPClients);
        
        // If AP clients are still connected but mesh is enabled, force disable
        if (currentAPClients > 0 && meshEnabled) {
          Serial.println("AP clients still connected - forcing mesh disable");
          toggleMesh(false);
        }
        // If no AP clients but mesh is disabled, force enable
        else if (currentAPClients == 0 && !meshEnabled) {
          Serial.println("No AP clients - forcing mesh enable");
          toggleMesh(true);
        }
        Serial.println("================================================");
        
        Serial.println("=======================================");
      } else if (type == WS_EVT_ERROR) {
        Serial.println("=== WEBSOCKET ERROR ===");
        Serial.println("WebSocket error occurred");
        Serial.println("=======================");
      } else if (type == WS_EVT_PONG) {
        Serial.println("WebSocket pong received");
      } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
          data[len] = 0;
          String message = String((char*)data);
          Serial.print("WebSocket message received: ");
          Serial.println(message);
        }
      } });

  server.addHandler(&ws);
  
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      if (request->hasParam("file")) {
          String fileName = request->getParam("file")->value();
          String filePath = "/" + fileName;

          if (SD.exists(filePath)) {
              request->send(SD, filePath, String(), true);
          } else {
              request->send(404, "text/plain", "File Not Found");
          }
      } else {
          request->send(400, "text/plain", "Bad Request - No file specified");
      } });
  server.begin();
}

void loop()
{
  dnsServer.processNextRequest();
  
  // Check AP connections for client detection
  checkAPConnections();
  
  // Periodic force check of AP connections (every 5 seconds)
  static unsigned long lastForceCheck = 0;
  if (millis() - lastForceCheck > 5000) {
    int currentAPClients = WiFi.softAPgetStationNum();
    if (currentAPClients > 0 && meshEnabled) {
      Serial.println("=== PERIODIC FORCE CHECK - AP CLIENTS CONNECTED BUT MESH ENABLED ===");
      Serial.print("AP clients: "); Serial.println(currentAPClients);
      Serial.println("Forcing mesh disable");
      toggleMesh(false);
      Serial.println("================================================================");
    } else if (currentAPClients == 0 && !meshEnabled) {
      Serial.println("=== PERIODIC FORCE CHECK - NO AP CLIENTS BUT MESH DISABLED ===");
      Serial.println("Forcing mesh enable");
      toggleMesh(true);
      Serial.println("=============================================================");
    }
    lastForceCheck = millis();
  }
  
  // Periodic mesh status check
  static unsigned long lastStatusCheck = 0;
  if (millis() - lastStatusCheck > 15000) { // Check every 15 seconds
    if (!meshEnabled && apClientsConnected > 0) {
      Serial.println("=== PERIODIC STATUS CHECK ===");
      Serial.println("Mesh should be DISABLED - checking status...");
      verifyMeshStatus();
      Serial.println("=============================");
    }
    lastStatusCheck = millis();
  }
  
  // Handle local sensor reading when mesh is disabled
  static bool taskWasEnabled = false;
  if (!meshEnabled && apClientsConnected > 0) {
    // Enable task only once when conditions are met
    if (!taskWasEnabled) {
      Serial.println("Enabling local sensor task");
      localSensorTask->enable();
      taskWasEnabled = true;
    }
    
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 5000) { // Debug every 5 seconds
      Serial.println("=== SYSTEM STATUS ===");
      Serial.print("Mesh enabled: "); Serial.println(meshEnabled ? "Yes" : "No");
      Serial.print("AP clients: "); Serial.println(apClientsConnected);
      Serial.print("WebSocket clients: "); Serial.println(ws.count());
      Serial.print("Website ready: "); Serial.println(websiteReady ? "Yes" : "No");
      Serial.print("Data transmission: ");
      if (!websiteReady) {
        Serial.println("STOPPED (OnOff not activated)");
      } else if (ws.count() == 0) {
        Serial.println("PAUSED (no WebSocket clients)");
      } else {
        Serial.println("ACTIVE (sending to WebSocket clients)");
      }
      Serial.print("Local sensor task: "); Serial.println(localSensorTask->isEnabled() ? "Running" : "Stopped");
      Serial.println("===================");
      lastDebug = millis();
    }
    
    // Execute scheduler to run the task
    userSched.execute();
  } else {
    // Disable task when conditions are not met
    if (taskWasEnabled) {
      Serial.println("Disabling local sensor task");
      localSensorTask->disable();
      taskWasEnabled = false;
    }
  }
  
  if (toggleOnOff == true)
  {
    userSched.execute();
  }
  
  // Only update mesh if it's enabled
  if (!lowPowerMode && meshEnabled)
  {
    mesh.update();
  } else if (!lowPowerMode && !meshEnabled) {
    static unsigned long lastMeshSkip = 0;
    if (millis() - lastMeshSkip > 10000) { // Debug every 10 seconds
      Serial.println("Mesh updates SKIPPED - mesh is disabled");
      lastMeshSkip = millis();
    }
  }
}
