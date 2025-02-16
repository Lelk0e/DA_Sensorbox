#include "namedMesh.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include "ulog_sqlite.h"

#define BME280_ADDRESS 0x77
BME280 bme280Sensor;

#define cs_pin 5
#define sda_pin 21
#define scl_pin 22

#define BUF_SIZE 2048
byte buf[BUF_SIZE];
char filename[] = "/BME280.db";
FILE *myFile;

volatile bool lowPowerMode = false;
volatile bool LPMsig = false;

using namespace std;

Scheduler userSched;
namedMesh mesh;

String nodeName = "BME280";
int hour, minute, second = 0;

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

void initMesh() {
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Sensorbox", "12345678", &userSched, 5555);
  mesh.setContainsRoot(true);
  Serial.println(mesh.getAPIP());
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
}

void sendRoot() {
  mesh.sendBroadcast(nodeName);
}

void receivedCallback(String &from, String &msg) {
  if (from.equals("mainESP")) {
    if (messageType(msg) == "Time") {
      timeSplit(msg, ':');
    }
  }
  if (from.equals("mainESP")){
    if (messageType(msg) == "LPMsh"){
      LPMsig = true;
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
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

void timeSplit(String s, char del) {
  int prefixEnd = s.indexOf(':');
  if (prefixEnd != -1) {
    String timePortion = s.substring(prefixEnd + 1);
    int firstDel = timePortion.indexOf(del);
    int secondDel = timePortion.indexOf(del, firstDel + 1);
    hour = timePortion.substring(0, firstDel).toInt();
    minute = timePortion.substring(firstDel + 1, secondDel).toInt();
    second = timePortion.substring(secondDel + 1).toInt();
  } else {
    hour = 0;
    minute = 0;
    second = 0;
  }
}

String messageType(String msg) {
  if (msg.indexOf(':') != -1) {
    int firstDel = msg.indexOf(':');
    return msg.substring(0, firstDel);
  }
  return "";
}

float readBme() {
  float pressure = bme280Sensor.readFloatPressure();
  if (isnan(pressure))
    return -1;
  return pressure;
}

void logSensorData() {
  File myFile = SD.open(filename, FILE_APPEND);
  if (!myFile) {
    Serial.println("Open Error");
    return;
  }

  char ts[24];
  sprintf(ts, "%02d:%02d:%02d", hour, minute, second);
  float pressure = readBme();

  if (pressure != -1) {
    myFile.print("Time: ");
    myFile.print(ts);
    myFile.print(", Pressure: ");
    myFile.println(pressure, 2);
    Serial.println("Data logged to file");
  } else {
    Serial.println("Sensor read error");
  }

  myFile.close();
}



int32_t read_fn_rctx(struct dblog_read_context *ctx, void *buf, uint32_t pos, size_t len) {
  if (fseek(myFile, pos, SEEK_SET))
    return DBLOG_RES_SEEK_ERR;
  size_t ret = fread(buf, 1, len, myFile);
  if (ret != len)
    return DBLOG_RES_READ_ERR;
  return ret;
}

void sendDB() {
  File myFile = SD.open(filename, FILE_READ);
  if (!myFile) {
    Serial.println("Error opening DB for reading");
    return;
  }

  struct dblog_read_context rctx;
  rctx.page_size_exp = 12;
  rctx.read_fn = read_fn_rctx;
  rctx.buf = buf;
  int res = dblog_read_init(&rctx);
  if (res) {
    print_error(res);
    myFile.close();
    return;
  }

  while (true) {
    uint32_t colType0, colType1;
    uint8_t *colVal0 = (uint8_t *)dblog_read_col_val(&rctx, 0, &colType0);
    uint8_t *colVal1 = (uint8_t *)dblog_read_col_val(&rctx, 1, &colType1);
    if (!colVal0 || !colVal1)
      break;

    char ts[32];
    strncpy(ts, (const char *)colVal0, sizeof(ts) - 1);
    ts[sizeof(ts) - 1] = '\0';

    float pressure;
    memcpy(&pressure, colVal1, sizeof(float));

    String msg = "Data:Time:" + String(ts) + ":BME280:" + String(pressure, 2);
    String rootNode = "mainESP";
    mesh.sendSingle(rootNode, msg);
    delay(50);

    if (dblog_read_next_row(&rctx) != 0)
      break;
  }

  myFile.close();
}

void LPM(unsigned long durationMillis) {
  if (!LPMsig) return;
  LPMsig = false;

  lowPowerMode = true;
  mesh.stop();
  Serial.println("Entering LPM");
  Task *exitTask = new Task(durationMillis, 1, []() { exitLPM(); });
  userSched.addTask(*exitTask);
  exitTask->enable();
}

void exitLPM() {
  lowPowerMode = false;
  Serial.println("Exiting LPM");
  initMesh();
  while(!mesh.isConnected("mainESP"));
  sendDB();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(sda_pin, scl_pin);
  Wire.setClock(400000);
  initMesh();
  initSDCard();
  if (!bme280Sensor.beginI2C(Wire)) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (true)
      ;
  }

}

void loop() {
  userSched.execute();
  if (LPMsig){
    LPM(30000);
  }
  if (!lowPowerMode) {
    mesh.update();
  }
  
  logSensorData();
  
  delay(1000);
}
