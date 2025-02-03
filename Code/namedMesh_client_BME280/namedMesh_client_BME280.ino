#include "namedMesh.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "SparkFunBME280.h"

#define BME280_ADDRESS 0x77
BME280 bme280Sensor;

#define cs_pin 5
#define sda_pin 19
#define scl_pin 20

using namespace std;

Scheduler userSched;
namedMesh mesh;

String nodeName = "BME280";
int hour, minute, second;

void initMesh(){
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
  if(from.equals("mainESP")){
    timeSplit(msg, ':');
  }
}
void newConnectionCallback(uint32_t nodeId) {
 
}

void initSDCard(){
  if(!SD.begin(cs_pin)){
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
    if (isnan(pressure)) {
        return -1;
    }
    return pressure;
}


void setup() {
  Serial.begin(115200);

  Wire.begin(sda_pin, scl_pin);

  Wire.setClock(400000);

  initMesh();

  initSDCard();

  if (!bme280Sensor.beginI2C(Wire)) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (true);
  }
}

void loop() {
  mesh.update();
}
