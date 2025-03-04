#include <SPI.h>
#include "namedMesh.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <SD.h>
#include "ulog_sqlite.h"
#include "DNSServer.h"
#include "RTClib.h"
#define cs_pin 5

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

volatile bool lowPowerMode = false;

FILE *dbFile;
FILE *readDbFile = NULL;

#define BUF_SIZE 2048
byte buf[BUF_SIZE];

AsyncWebServer server(80);

DNSServer dnsServer;
const byte DNS_PORT = 53;

bool toggleOnOff = false;

void receivedCallback(String &from, String &msg);
void newConnectionCallback(uint32_t nodeId);
String messageType(String msg);
void dataSplit(String s, char del);
void logSensorData();
int32_t read_fn_wctx(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len);
int flush_fn(struct dblog_write_context *ctx);
int32_t write_fn(struct dblog_write_context *ctx, void *buffer, uint32_t pos, size_t len);
int32_t read_fn_rctx(struct dblog_read_context *ctx, void *buffer, uint32_t pos, size_t len);
void print_error(int res);
void exitLPM();
String wrDBtoWs(const char *filename);

void initMesh()
{
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Sensorbox", "12345678", &userSched, 5555);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);
  Serial.println(mesh.getAPIP());
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]()
                            { Serial.printf("Changed connection\n"); });
}

void sendRoot()
{
  mesh.sendBroadcast("Root:" + nodeName);
}

void receivedCallback(String &from, String &msg)
{
  Serial.printf("Received from=%s msg=%s\n", from.c_str(), msg.c_str());
  if (messageType(msg) == "Data")
  {
    if (from.equals("BME280"))
    {
      dataSplit(msg, ':');
      logSensorData();
    }
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  sendRoot();

  DateTime now = rtc.now();
  String timeMessage = "Time:" +
                       String(now.year()) + ":" +
                       String(now.month()) + ":" +
                       String(now.day()) + ":" +
                       String(now.hour()) + ":" +
                       String(now.minute()) + ":" +
                       String(now.second());
  mesh.sendBroadcast(timeMessage);
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
  String BMEValStr = s.substring(idx9 + 1);
  String HTUValStr = s.substring(idx10 + 1);
  BMEDataValue = BMEValStr.toFloat();
}

void logSensorData()
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
  ctx.col_count = 2;
  ctx.page_resv_bytes = 0;
  ctx.page_size_exp = 12;
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

void setup()
{
  Serial.begin(115200);
  initMesh();
  initSDCard();
  dnsServer.start(DNS_PORT, "sensorbox.com", mesh.getAPIP());
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SD, "/webpage/website.html", "text/html"); });
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
    request->send(200, "text/plain", "Time received successfully");
  } else {
    request->send(400, "text/plain", "Missing time parameters");
  } });

  server.on("/OnOff", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if(request->hasParam("On") || request->hasParam("Off")){
      if(request->hasParam("On")){
        toggleOnOff = true;
        mesh.sendBroadcast("");
      } else if (request->hasParam("Off")) {
        toggleOnOff = false;
        mesh.sendBroadcast("Finalize");
      }
    } else {
      request->send(400, "text/plain", "error toggle on off");
    } });
  server.on("/bme280", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String data = wrDBtoWs("/sd/BME280");
    request->send(200, "text/plain", data); });
  server.begin();
}

void loop()
{
  dnsServer.processNextRequest();
  if (toggleOnOff == true)
  {
    userSched.execute();
    
  }
  if (!lowPowerMode)
  {
    mesh.update();
  }
}
