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
String hour;
String minute;
String second;

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

void timeSplit(string s, char del) {
    size_t prefixEnd = s.find(':');
    if (prefixEnd != string::npos) {
        string timePortion = s.substr(prefixEnd + 1);
        size_t firstDel = timePortion.find(del);
        size_t secondDel = timePortion.find(del, firstDel + 1);
        hour = stoi(timePortion.substr(0, firstDel));
        minute = stoi(timePortion.substr(firstDel + 1, secondDel - firstDel - 1));
        second = stoi(timePortion.substr(secondDel + 1));
    } else {
        hour = minute = second = 0;
    }
}


string messageType(string msg) {
    if (msg.find(':') != string::npos) {
        size_t firstDel = msg.find(':');
        return msg.substr(0, firstDel);
    }
    return "";
}

float readBme(){
  float pressure = bme280Sensor.readFloatPressure();
  return pressure;

}

void setup() {
  Serial.begin(115200);

  Wire.begin(sda_pin, scl_pin);

  Wire.setClock(400000);

  initMesh();

  if (!bme280Sensor.beginI2C(Wire)) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (true);
  }
}

void loop() {
  mesh.update();
}
