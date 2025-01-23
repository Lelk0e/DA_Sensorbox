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
    delimiterSplit(msg, ':');
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

void delimiterSplit(String s, char del){
  int firstDel = s.indexOf(del);
  int secondDel = s.indexOf(del, firstDel + 1);
  
  hour = s.substring(0, firstDel).toInt();
  minute = s.substring(firstDel + 1, secondDel).toInt();
  second = s.substring(secondDel + 1).toInt();
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
