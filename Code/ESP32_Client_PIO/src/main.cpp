#include "namedMesh.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include "ulog_sqlite.h"
#include "RTClib.h"
#include "Adafruit_HTU21DF.h"
#include "Adafruit_MAX31855.h"

#define BME280_ADDRESS 0x76
BME280 bme280Sensor;

#define MAXCLK 14
#define MAXCS 15
#define MAXDO 12
#define cs_pin 5
#define sda_pin 21
#define scl_pin 22
#define ozon_pin 32
#define external_analogIn 33
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const char *dbFileName = "/sd/data.db";
char lastSentTs[32] = "0000:00:00:00:00:00";

#define BUF_SIZE 4096
byte buf[BUF_SIZE];

FILE *dbFile;            // Used for writing/finalizing the DB
FILE *readDbFile = NULL; // Used exclusively for reading in sendDB
struct dblog_write_context sqliteLogger;
int sqliteMeasurementCount = 0;

volatile bool toggleOnOff = false;
volatile bool lowPowerMode = false;
volatile bool LPMsig = false;
bool finalizeSignal = false;
bool finalized = false; // flag: true when finalization has occurred
bool timeSynchronized = false; // flag: true when time has been synchronized
Task *logTask;

bool rtcStat;
bool bmeStat;
bool htuStat;

using namespace std;

Scheduler userSched;
namedMesh mesh;

String nodeName = "";
uint8_t baseMac[6];
String rootName;
int hour = 0, minute = 0, second = 0;
int year = 0, month = 0, day = 0;

unsigned long startTime = 0;

uint32_t rootNodeId = 0;

void receivedCallback(String &from, String &msg);
void newConnectionCallback(uint32_t nodeId);
String messageType(String msg);
void timeSplit(String s, char del);
void exitLPM();
void resetLogging();
void sendDB();

int32_t read_fn(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len)
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

int32_t db_read_fn_rctx(struct dblog_read_context *ctx, void *buffer, uint32_t pos, size_t len)
{
  if (fseek(readDbFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fread(buffer, 1, len, readDbFile);
  if (ret != len)
    return DBLOG_RES_READ_ERR;
  return ret;
}

void print_error(int res)
{
  Serial.print(F("Err: "));
  Serial.println(res);
}

void initMesh()
{
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Mesh", "12345678", &userSched, 5555);
  mesh.setContainsRoot(true);
  Serial.println(mesh.getAPIP());
  esp_base_mac_addr_get(baseMac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x;%02x;%02x;%02x;%02x;%02x",
           baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  String macString = String(macStr);
  mesh.setName(macString);
  nodeName = macStr;
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]()
                            { Serial.printf("Changed connection\n"); });
}

void receivedCallback(String &from, String &msg)
{
  Serial.println("=== CLIENT: MESH MESSAGE RECEIVED ===");
  Serial.printf("From: %s\n", from.c_str());
  Serial.printf("Message: %s\n", msg.c_str());
  Serial.printf("My node name: %s\n", mesh.getName().c_str());
  Serial.printf("Connected nodes: %d\n", mesh.getNodeList().size());
  
  // Always check message type first
  String type = messageType(msg);

  // Always process Root messages to set rootNodeId
  if (type == "Root")
  {
    // Extract the value after the colon
    int colonPos = msg.indexOf(':');
    if (colonPos != -1) {
      String idStr = msg.substring(colonPos + 1);
      rootNodeId = idStr.toInt();
      Serial.print("Root node ID set to: "); Serial.println(rootNodeId);
    } else {
      Serial.println("Root message format error: no colon found");
    }
  }
  
  // Check if the message is from the mainESP node
  if (from.equals("mainESP"))
  {
    Serial.println("Message is from mainESP - processing...");
    // type already determined above
    Serial.print("Message type: "); Serial.println(type);
    Serial.print("Type length: "); Serial.println(type.length());
    Serial.print("Type equals 'Time': "); Serial.println(type.equals("Time") ? "YES" : "NO");
    Serial.print("Type == 'Time': "); Serial.println((type == "Time") ? "YES" : "NO");
    
    if (type == "Time")
    {
      Serial.println("*** TIME MESSAGE DETECTED - PROCESSING ***");
      Serial.println("Processing time sync...");
      timeSplit(msg, ':');
      timeSynchronized = true;
      Serial.println("Time synchronized - logging can now begin");
      if (!logTask->isEnabled()) {
        Serial.println("Enabling logging task after time sync");
        logTask->enable();
      }
    }
    else if (type == "LPMsh")
    {
      Serial.println("Low power mode signal received");
      LPMsig = true;
    }
    else if (type == "Finalize")
    {
      Serial.println("Finalize signal received");
      finalizeSignal = true;
    }
    else if (type == "Reset")
    {
      Serial.println("Reset command received");
      resetLogging();
    }
    else if (type == "OnOff")
    {
      Serial.println("OnOff toggle received");
      toggleOnOff = !toggleOnOff;
    }
    else if (type == "SendData")
    {
      Serial.println("=== SENDDATA COMMAND RECEIVED ===");
      Serial.println("Sending all stored data to mainESP");
      sendDB();
      Serial.println("Data sending complete");
      Serial.println("================================");
    }
    else
    {
      Serial.print("Unknown message type: '"); Serial.print(type); Serial.println("'");
    }
  }
  else
  {
    Serial.print("Message is NOT from mainESP (from: '"); Serial.print(from); Serial.println("') - ignoring");
    Serial.println("Trying alternative approach - processing anyway since we're connected to mesh...");
    // type already determined above
    if (type == "Time")
    {
      Serial.println("*** TIME MESSAGE DETECTED (ALTERNATIVE) - PROCESSING ***");
      Serial.println("Processing time sync...");
      timeSplit(msg, ':');
      timeSynchronized = true;
      Serial.println("Time synchronized - logging can now begin");
      // Start logging task if not already started
      if (!logTask->isEnabled()) {
        Serial.println("Starting logging task");
        logTask->enable();
      }
    }
  }
  Serial.println("=====================================");
}

void newConnectionCallback(uint32_t nodeId)
{
}

void initSDCard()
{
  if (!SD.begin(cs_pin))
  {
    Serial.println("Mount Failed");
    return;
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Size: %lluMB\n", cardSize);
}

void timeSplit(String s, char del)
{
  Serial.println("=== CLIENT: PARSING TIME MESSAGE ===");
  Serial.print("Original message: "); Serial.println(s);
  
  int prefixEnd = s.indexOf(':');
  if (prefixEnd != -1)
  {
    String datePortion = s.substring(prefixEnd + 1);
    Serial.print("Date portion: "); Serial.println(datePortion);

    // Parse year
    int firstDel = datePortion.indexOf(del);
    year = datePortion.substring(0, firstDel).toInt();

    // Parse month
    int secondDel = datePortion.indexOf(del, firstDel + 1);
    month = datePortion.substring(firstDel + 1, secondDel).toInt();

    // Parse day
    int thirdDel = datePortion.indexOf(del, secondDel + 1);
    day = datePortion.substring(secondDel + 1, thirdDel).toInt();

    // Parse hour
    int fourthDel = datePortion.indexOf(del, thirdDel + 1);
    hour = datePortion.substring(thirdDel + 1, fourthDel).toInt();

    // Parse minute
    int fifthDel = datePortion.indexOf(del, fourthDel + 1);
    minute = datePortion.substring(fourthDel + 1, fifthDel).toInt();

    // Parse second
    second = datePortion.substring(fifthDel + 1).toInt();

    Serial.println("Parsed values:");
    Serial.print("Year: "); Serial.println(year);
    Serial.print("Month: "); Serial.println(month);
    Serial.print("Day: "); Serial.println(day);
    Serial.print("Hour: "); Serial.println(hour);
    Serial.print("Minute: "); Serial.println(minute);
    Serial.print("Second: "); Serial.println(second);

    DateTime newDateTime(year, month, day, hour, minute, second);
    Serial.println("Setting RTC to new date/time...");
    rtc.adjust(newDateTime);
    
    // Verify the RTC was updated
    delay(100);
    DateTime now = rtc.now();
    Serial.println("RTC verification:");
    Serial.print("RTC Year: "); Serial.println(now.year());
    Serial.print("RTC Month: "); Serial.println(now.month());
    Serial.print("RTC Day: "); Serial.println(now.day());
    Serial.print("RTC Hour: "); Serial.println(now.hour());
    Serial.print("RTC Minute: "); Serial.println(now.minute());
    Serial.print("RTC Second: "); Serial.println(now.second());
    Serial.println("================================");
  } else {
    Serial.println("ERROR: No colon found in time message");
    Serial.println("================================");
  }
}

String messageType(String msg)
{
  Serial.println("=== CLIENT: MESSAGE TYPE DETECTION ===");
  Serial.print("Input message: "); Serial.println(msg);
  
  if (msg == NULL)
  {
    Serial.println("Message is NULL");
    Serial.println("================================");
    return "";
  }

  int pos = msg.indexOf(':');
  Serial.print("Colon position: "); Serial.println(pos);
  
  if (pos != -1)
  {
    String type = msg.substring(0, pos);
    Serial.print("Extracted type: "); Serial.println(type);
    Serial.print("Type length: "); Serial.println(type.length());
    Serial.print("Type equals 'Time': "); Serial.println(type.equals("Time") ? "YES" : "NO");
    Serial.println("================================");
    return type;
  }
  
  Serial.println("No colon found in message");
  Serial.println("================================");
  return msg;
}
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

void logNodeData()
{
  // Only log data if time has been synchronized
  if (!timeSynchronized) {
    Serial.println("=== CLIENT: SKIPPING LOG - TIME NOT SYNCHRONIZED ===");
    return;
  }
  
  int BMEValue = static_cast<int>(readBme());
  int HTUValue = static_cast<int>(readHTU());
  int TypKValue = static_cast<int>(readTypK());
  int OzonValue = static_cast<int>(readOzon());
  DateTime now;
  try
  {
    now = rtc.now();
  }
  catch (const exception &e)
  {
    Serial.println(e.what());
  }

  char timestamp[24];
  snprintf(timestamp, sizeof(timestamp), "%04d:%02d:%02d:%02d:%02d:%02d",
           now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

  Serial.println("=== CLIENT: LOGGING SENSOR DATA ===");
  Serial.print("RTC Time: "); Serial.println(timestamp);
  Serial.print("RTC Year: "); Serial.println(now.year());
  Serial.print("RTC Month: "); Serial.println(now.month());
  Serial.print("RTC Day: "); Serial.println(now.day());
  Serial.print("RTC Hour: "); Serial.println(now.hour());
  Serial.print("RTC Minute: "); Serial.println(now.minute());
  Serial.print("RTC Second: "); Serial.println(now.second());

  int res = dblog_set_col_val(&sqliteLogger, 0, DBLOG_TYPE_TEXT, timestamp, strlen(timestamp));
  if (res != 0)
  {
    Serial.print("Error setting timestamp: ");
    Serial.println(res);
  }

  res = dblog_set_col_val(&sqliteLogger, 1, DBLOG_TYPE_INT, &BMEValue, sizeof(BMEValue));
  if (res != 0)
  {
    Serial.print("Error setting sensor value: ");
    Serial.println(res);
  }
  res = dblog_set_col_val(&sqliteLogger, 2, DBLOG_TYPE_INT, &HTUValue, sizeof(HTUValue));
  if (res != 0)
  {
    Serial.print("Error setting sensor value: ");
    Serial.println(res);
  }
  res = dblog_set_col_val(&sqliteLogger, 3, DBLOG_TYPE_INT, &TypKValue, sizeof(TypKValue));
  if (res != 0)
  {
    Serial.print("Error setting sensor value: ");
    Serial.println(res);
  }
  res = dblog_set_col_val(&sqliteLogger, 4, DBLOG_TYPE_INT, &OzonValue, sizeof(OzonValue));
  if (res != 0)
  {
    Serial.print("Error setting sensor value: ");
    Serial.println(res);
  }
  res = dblog_append_empty_row(&sqliteLogger);
  if (res != 0)
  {
    Serial.print("Error appending row: ");
    Serial.println(res);
  }
  else
  {
    Serial.println("Time: " + String(timestamp));
    Serial.println("BME: " + String(BMEValue) + "Pa");
    Serial.println("HTU: " + String(HTUValue) + "%");
    Serial.println("TypK: " + String(TypKValue) + "°C");
    Serial.println("Ozon: " + String(OzonValue) + "mV");
    
    // Send data in JSON format to match main ESP32
    String jsonData = String("{\"timestamp\":\"") + String(timestamp) + "\",\"node\":\"" + String(nodeName) + "\",\"bme\":" + String(BMEValue) + ",\"htu\":" + String(HTUValue) + ",\"typk\":" + String(TypKValue) + ",\"ozon\":" + String(OzonValue) + "}";
    
    Serial.print("logNodeData: rootNodeId: "); Serial.println(rootNodeId);
    Serial.print("logNodeData: mesh.isConnected(rootNodeId): "); Serial.println(((painlessMesh&)mesh).isConnected(rootNodeId) ? "YES" : "NO");
    if (rootNodeId != 0 && ((painlessMesh&)mesh).isConnected(rootNodeId)) {
      Serial.println("Sending sensor data to root node via mesh.sendSingle (by ID)");
      mesh.sendSingle(rootNodeId, jsonData);
    } else {
      Serial.println("WARNING: Not sending data - rootNodeId not set or not connected");
    }
  }
  Serial.println("==================================");
}

void sendDB()
{
  Serial.println("=== CLIENT: SENDING DATABASE ===");
  readDbFile = fopen(dbFileName, "rb");
  if (!readDbFile)
  {
    Serial.println("Error opening DB for reading");
    return;
  }
  Serial.println("Database file opened successfully");
  
  struct dblog_read_context rctx;
  rctx.page_size_exp = 11;
  rctx.read_fn = (int32_t (*)(struct dblog_read_context *, void *, uint32_t, size_t))db_read_fn_rctx;
  rctx.buf = buf;
  int res = dblog_read_init(&rctx);
  if (res)
  {
    Serial.print("Error initializing DB read: ");
    print_error(res);
    fclose(readDbFile);
    return;
  }
  Serial.println("Database read context initialized");
  
  int recordsSent = 0;
  while (true)
  {
    uint32_t colType0, colType1, colType2, colType3, colType4;
    uint8_t *colVal0 = (uint8_t *)dblog_read_col_val(&rctx, 0, &colType0);
    uint8_t *colVal1 = (uint8_t *)dblog_read_col_val(&rctx, 1, &colType1);
    uint8_t *colVal2 = (uint8_t *)dblog_read_col_val(&rctx, 2, &colType2);
    uint8_t *colVal3 = (uint8_t *)dblog_read_col_val(&rctx, 3, &colType3);
    uint8_t *colVal4 = (uint8_t *)dblog_read_col_val(&rctx, 4, &colType4);
    if (!colVal0 || !colVal1 || !colVal2 || !colVal3 || !colVal4)
      break;

    char ts[32];
    strncpy(ts, (const char *)colVal0, sizeof(ts) - 1);
    ts[sizeof(ts) - 1] = '\0';

    if (strcmp(ts, lastSentTs) <= 0)
    {
      // Skip already sent records
    }
    else
    {
      int BMEValue;
      int HTUValue;
      int TypKValue;
      int OzonValue;
      memcpy(&BMEValue, colVal1, sizeof(BMEValue));
      memcpy(&HTUValue, colVal2, sizeof(HTUValue));
      memcpy(&TypKValue, colVal3, sizeof(TypKValue));
      memcpy(&OzonValue, colVal4, sizeof(OzonValue));
      
      // Send data in JSON format to match main ESP32
      String jsonData = "{\"timestamp\":\"" + String(ts) + "\",\"node\":\"" + String(nodeName) + "\",\"bme\":" + String(BMEValue) + ",\"htu\":" + String(HTUValue) + ",\"typk\":" + String(TypKValue) + ",\"ozon\":" + String(OzonValue) + "}";
      
      Serial.print("Sending record "); Serial.print(recordsSent + 1); Serial.print(": "); Serial.println(ts);
      mesh.sendSingle(rootNodeId, jsonData);
      delay(10);
      strcpy(lastSentTs, ts);
      recordsSent++;
    }

    if (dblog_read_next_row(&rctx) != 0)
      break;
  }
  fclose(readDbFile);
  Serial.print("Database sending complete. Records sent: "); Serial.println(recordsSent);
  Serial.println("=====================================");
}

void LPM(unsigned long durationMillis)
{
  if (!LPMsig)
    return;
  LPMsig = false;
  lowPowerMode = true;
  mesh.stop();
  Serial.println("Entering LPM");
  Task *exitTask = new Task(durationMillis, TASK_ONCE, []()
                            { exitLPM(); });
  userSched.addTask(*exitTask);
  exitTask->enable();
}

void exitLPM()
{
  Serial.println("=== CLIENT: EXITING LOW POWER MODE ===");
  lowPowerMode = false;
  Serial.println("Exiting LPM");
  initMesh();
  Serial.println("Mesh reinitialized, waiting for connection...");
  
  int connectionAttempts = 0;
  while (!mesh.isConnected("mainESP") && connectionAttempts < 30)
  {
    Serial.print("Connection attempt "); Serial.print(connectionAttempts + 1); Serial.println("/30");
    delay(1000);
    connectionAttempts++;
  }
  
  if (mesh.isConnected("mainESP")) {
    Serial.println("Connected to mainESP - sending stored data");
    sendDB();
  } else {
    Serial.println("Failed to connect to mainESP after 30 attempts");
  }
  Serial.println("==========================================");
}

void resetLogging()
{
  Serial.println("Reset command received. Resetting logging");
  SD.remove(dbFileName);
  dbFile = fopen(dbFileName, "w+b");
  if (!dbFile)
  {
    Serial.println("Failed to open new SQLite DB file");
    return;
  }
  sqliteLogger.buf = buf;
  sqliteLogger.col_count = 5;
  sqliteLogger.page_resv_bytes = 0;
  sqliteLogger.page_size_exp = 11;
  sqliteLogger.max_pages_exp = 0;
  sqliteLogger.read_fn = read_fn;
  sqliteLogger.write_fn = write_fn;
  sqliteLogger.flush_fn = flush_fn;
  int res = dblog_write_init(&sqliteLogger);
  if (res != 0)
  {
    Serial.print("Error reinitializing logger: ");
    Serial.println(res);
    return;
  }
  if (!logTask)
  {
    logTask = new Task(1000, TASK_FOREVER, logNodeData);
    userSched.addTask(*logTask);
  }
  
  // Only enable logging if time has been synchronized
  if (timeSynchronized) {
    logTask->enable();
    Serial.println("Reset complete. Logging restarted (time synchronized).");
  } else {
    Serial.println("Reset complete. Logging task created but not enabled - waiting for time sync.");
  }
  
  startTime = millis();
  finalized = false;
}

void setup()
{
  pinMode(ozon_pin, INPUT);
  Serial.begin(115200);
  Wire.begin(sda_pin, scl_pin);
  delay(100);
  initSDCard();
  if (!rtc.begin(&Wire))
  {
    Serial.println("RTC not working");
    rtcStat = false;
  }
  else
  {
    Serial.println("RTC working");
  }
  bme280Sensor.setI2CAddress(0x76);
  if (!bme280Sensor.beginI2C(Wire))
  {
    Serial.println("BME not working");
    while (1)
      ;
  }
  else
  {
    Serial.println("BME working");
  }
  if (!htu.begin())
  {
    Serial.println("Couldn't find sensor!");
    while (1)
      ;
  }
  Serial.print("Initializing sensor...");
  if (!thermocouple.begin())
  {
    Serial.println("ERROR.");
    while (1)
      delay(10);
  }
  delay(500);
  initMesh();
  rootName = "mainESP";
  Serial.println("Static rootName set to: mainESP");

  if (SD.exists(dbFileName))
  {
    dbFile = fopen(dbFileName, "r+b");
    Serial.println("Opening existing database file");
  }
  else
  {
    // Create a new database file
    dbFile = fopen(dbFileName, "w+b");
    Serial.println("Creating new database file");
  }

  if (!dbFile)
  {
    Serial.println("Failed to open SQLite DB file");
    // Handle error appropriately
    return;
  }
  sqliteLogger.buf = buf;
  sqliteLogger.col_count = 5;
  sqliteLogger.page_resv_bytes = 0;
  sqliteLogger.page_size_exp = 11;
  sqliteLogger.max_pages_exp = 0;
  sqliteLogger.read_fn = read_fn;
  sqliteLogger.write_fn = write_fn;
  sqliteLogger.flush_fn = flush_fn;
  dblog_write_init(&sqliteLogger);
  logTask = new Task(1000, TASK_FOREVER, logNodeData);
  userSched.addTask(*logTask);
  // Don't enable the task yet - wait for time sync
  Serial.println("Logging task created but not enabled - waiting for time sync");
  startTime = millis();
  finalized = false;
}

void loop()
{
  if (LPMsig)
    LPM(30000);
  if (!lowPowerMode)
  {
    try
    {
      mesh.update();
    }
    catch (std::exception &e)
    {
      Serial.print("Mesh update error: ");
      Serial.println(e.what());
    }
  }

  if (finalizeSignal && !finalized)
  {
    logTask->disable();
    dblog_finalize(&sqliteLogger);
    fclose(dbFile);
    Serial.println("Database file finalized successfully");
    finalized = true;
    finalizeSignal = false;
  }
  if (toggleOnOff)
  {
    try
    {
      userSched.execute();
    }
    catch (const std::exception &e)
    {
      Serial.println(e.what());
    }
  }
  if (toggleOnOff && finalized && mesh.isConnected("MainESP"))
  {
    try
    {
      sendDB();
    }
    catch (const std::exception &e)
    {
      Serial.println(e.what());
    }
  }
}
