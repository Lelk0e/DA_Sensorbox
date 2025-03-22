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

void receivedCallback(String &from, String &msg);
void newConnectionCallback(uint32_t nodeId);
String messageType(String msg);
void timeSplit(String s, char del);
void exitLPM();
void resetLogging();

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
  mesh.init("Sensorbox", "12345678", &userSched, 5555);
  mesh.setContainsRoot(true);
  Serial.println(mesh.getAPIP());
  esp_base_mac_addr_get(baseMac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
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
  if (from.equals("mainESP"))
  {
    String type = messageType(msg);
    if (type == "Time")
    {
      timeSplit(msg, ':');
    }
    if (type == "LPMsh")
    {
      LPMsig = true;
    }
    if (type == "Finalize")
    {
      finalizeSignal = true;
    }
    if (type == "Reset")
    {
      resetLogging();
    }
    if (type == "OnOff")
    {
      toggleOnOff = !toggleOnOff;
    }
    if (type == "Root")
    {
      int pos = msg.indexOf(':');
      rootName = msg.substring(pos + 1);
    }
  }
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
  int prefixEnd = s.indexOf(':');
  if (prefixEnd != -1)
  {
    String datePortion = s.substring(prefixEnd + 1);

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

    DateTime newDateTime(year, month, day, hour, minute, second);
    rtc.adjust(newDateTime);
  }
}

String messageType(String msg)
{
  if (msg == NULL)
  {
    return "";
  }

  int pos = msg.indexOf(':');
  if (pos != -1)
  {
    return msg.substring(0, pos);
  }
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
    String msg = "Data:Time:" + String(timestamp) + ":" + String(nodeName) + ":" + String(BMEValue) + ":" + String(HTUValue) + ":" + String(TypKValue) + ":" + String(OzonValue);
    if (!msg.isEmpty())
    {
      mesh.sendSingle(rootName, msg);
      msg = "";
    }
  }
}

void sendDB()
{
  readDbFile = fopen(dbFileName, "rb");
  if (!readDbFile)
  {
    Serial.println("Error opening DB for reading");
    return;
  }
  struct dblog_read_context rctx;
  rctx.page_size_exp = 11;
  rctx.read_fn = (int32_t (*)(struct dblog_read_context *, void *, uint32_t, size_t))db_read_fn_rctx;
  rctx.buf = buf;
  int res = dblog_read_init(&rctx);
  if (res)
  {
    print_error(res);
    fclose(readDbFile);
    return;
  }
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
      String msg = "Data:Time:" + String(ts) + ":" + String(nodeName) + ":" + String(BMEValue) + ":" + String(HTUValue) + ":" + String(TypKValue) + ":" + String(OzonValue);
      mesh.sendSingle(rootName, msg);
      delay(10);
      strcpy(lastSentTs, ts);
    }

    if (dblog_read_next_row(&rctx) != 0)
      break;
  }
  fclose(readDbFile);
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
  lowPowerMode = false;
  Serial.println("Exiting LPM");
  initMesh();
  while (!mesh.isConnected("mainESP"))
  {
    delay(1000);
  }
  sendDB();
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
  logTask->enable();
  startTime = millis();
  finalized = false;
  Serial.println("Reset complete. Logging restarted.");
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
  logTask->enable();
  startTime = millis();
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
