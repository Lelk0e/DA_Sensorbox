#include "namedMesh.h"
//#define ASYNC_TCP_SSL_ENABLED 1
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPI.h>
#include <SD.h>
#include "ulog_sqlite.h"
#define cs_pin 5

Scheduler userSched;
namedMesh mesh;

String nodeName = "mainESP";
String msgBME = "";
String msgOzon = "";
String msgPT = "";
String msgInter = "";
int hour;
int minute;
int second;
int dhour;
int dminute;
int dsecond;

String sensorTable;
float sensorDataValue;

volatile bool lowPowerMode = false;

FILE *myFile;

#define BUF_SIZE 2048
byte buf[BUF_SIZE];

AsyncWebServer server(443);

void initMesh() {
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Sensorbox", "12345678", &userSched, 5555);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);
  Serial.println(mesh.getAPIP());
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
}

void sendRoot();

void sendRoot() {
  mesh.sendBroadcast(nodeName);
}

void receivedCallback(String &from, String &msg) {
  Serial.printf("Received from=%s msg=%s\n", from.c_str(), msg.c_str());
  if (from.equals("BME280")) {
    if (messageType(msg) == "Data") {
      dataSplit(msg, ':');
      logSensorData();
    }
  }
}


void newConnectionCallback(uint32_t nodeId) {
  sendRoot();
  mesh.sendBroadcast(String(hour) + ":" + String(minute) + ":" + String(second));
}


void dataSplit(String s, char del) {

  int idx1 = s.indexOf(del);  // after "data"
  if (idx1 == -1) return;
  int idx2 = s.indexOf(del, idx1 + 1);  // after "Time"
  if (idx2 == -1) return;
  int idx3 = s.indexOf(del, idx2 + 1);  // after hh
  if (idx3 == -1) return;
  int idx4 = s.indexOf(del, idx3 + 1);  // after mm
  if (idx4 == -1) return;
  int idx5 = s.indexOf(del, idx4 + 1);  // after ss
  if (idx5 == -1) return;
  int idx6 = s.indexOf(del, idx5 + 1);  // after sensor table name
  if (idx6 == -1) return;

  // Extract time parts
  String hh = s.substring(idx2 + 1, idx3);
  String mm = s.substring(idx3 + 1, idx4);
  String ss = s.substring(idx4 + 1, idx5);
  dhour = hh.toInt();
  dminute = mm.toInt();
  dsecond = ss.toInt();

  sensorTable = s.substring(idx5 + 1, idx6);
  String sensorValStr = s.substring(idx6 + 1);
  sensorDataValue = sensorValStr.toFloat();
}


void logSensorData() {
  char dbFilename[32];
  sprintf(dbFilename, "/%s.db", sensorTable.c_str());


  myFile = fopen(dbFilename, "a+b");
  if (!myFile) {
    Serial.print("Open Error for ");
    Serial.println(dbFilename);
    return;
  }

  struct dblog_write_context ctx;
  ctx.buf = buf;
  ctx.col_count = 2;  // Two columns: Timestamp and Data
  ctx.page_resv_bytes = 0;
  ctx.page_size_exp = 12;
  ctx.max_pages_exp = 0;
  ctx.read_fn = read_fn_wctx;
  ctx.flush_fn = flush_fn;
  ctx.write_fn = write_fn;

  int res = dblog_write_init(&ctx);
  if (!res) {
    char ts[24];
    sprintf(ts, "%02d:%02d:%02d", dhour, dminute, dsecond);
    res = dblog_set_col_val(&ctx, 0, DBLOG_TYPE_TEXT, ts, strlen(ts));
    res = dblog_set_col_val(&ctx, 1, DBLOG_TYPE_REAL, &sensorDataValue, sizeof(sensorDataValue));
    res = dblog_append_empty_row(&ctx);
  }
  res = dblog_finalize(&ctx);
  fclose(myFile);
  if (res)
    print_error(res);
  else
    Serial.println("Data logged");
}


void initSDCard() {
  if (!SD.begin(cs_pin)) {
    Serial.println("Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Size: %lluMB\n", cardSize);
}

void serveFile(AsyncWebServerRequest *request, const char *filename) {
  File file = SD.open(filename, FILE_READ);

  if (!file || file.isDirectory()) {
    request->send(404, "text/plain", "File not found or is a directory");
    return;
  }

  String fileContent;
  while (file.available()) {
    fileContent += char(file.read());
  }
  file.close();

  request->send(200, "text/plain", fileContent);
}


int32_t read_fn_wctx(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len) {
  if (fseek(myFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fread(buf, 1, len, myFile);
  if (ret != len)
    return DBLOG_RES_READ_ERR;
  return ret;
}

int flush_fn(struct dblog_write_context *ctx) {
  return DBLOG_RES_OK;
}

int32_t write_fn(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len) {
  if (fseek(myFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fwrite(buf, 1, len, myFile);
  if (ret != len)
    return DBLOG_RES_ERR;
  fflush(myFile);
  fsync(fileno(myFile));
  return ret;
}

void print_error(int res) {
  Serial.print(F("Err: "));
  Serial.println(res);
}

String messageType(String msg) {
  if (msg.indexOf(':') != -1) {
    int firstDel = msg.indexOf(':');
    return msg.substring(0, firstDel);
  }
  return "";
}

int32_t read_fn_rctx(struct dblog_read_context *ctx, void *buf, uint32_t pos, size_t len) {
  if (fseek(myFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fread(buf, 1, len, myFile);
  if (ret != len)
    return DBLOG_RES_READ_ERR;
  return ret;
}




void setup() {

  Serial.begin(115200);

  initMesh();

  initSDCard();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/webpage/website.html", "text/html");
  });

  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("hour") && request->hasParam("minute") && request->hasParam("second")) {
      hour = request->getParam("hour")->value().toInt();
      minute = request->getParam("minute")->value().toInt();
      second = request->getParam("second")->value().toInt();
      Serial.print("Received time: ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.println(second);
      request->send(200, "text/plain", "Time received successfully");
    } else {
      request->send(400, "text/plain", "Missing time parameters");
    }
  });

  //   server.onSslFileRequest([](void * arg, const char *filename, uint8_t **buf) -> int {
  //    Serial.printf("SSL File: %s\n", filename);
  //    File file = SPIFFS.open(filename, "r");
  //    if(file){
  //      size_t size = file.size();
  //      uint8_t * nbuf = (uint8_t*)malloc(size);
  //      if(nbuf){
  //        size = file.read(nbuf, size);
  //        file.close();
  //        *buf = nbuf;
  //        return size;
  //      }
  //      file.close();
  //    }
  //    *buf = 0;
  //    return 0;
  //  }, NULL);
  //  server.beginSecure("/server.cer", "/server.key", NULL);
  server.begin();
}

void loop() {
  mesh.update();
}
