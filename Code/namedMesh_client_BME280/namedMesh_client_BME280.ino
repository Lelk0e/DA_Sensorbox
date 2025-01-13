#include "namedMesh.h"
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include <bits/stdc++.h>

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
    delimiterSplit(msg, ":");
  }
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

void delimiterSplit(string s, char del){
  std::stringstream ss (s);
  string part;

  std::getline(ss, part, del);
  hour = stoi(part); //stoi --> Convert string to int
  std::getline(ss, part, del);
  minute = stoi(part); //stoi --> Convert string to int
  std::getline(ss, part, del);
  second = stoi(part); //stoi --> Convert string to int
}

void readBme(){

}

void setup() {
  Serial.begin(115200);

  Wire.begin(sda_pin, scl_pin);

  Wire.SetClock(400000);

  initMesh();

  if (!bme280Sensor.beginI2C(Wire)) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (true);
  }
}

void loop() {
  mesh.update();
}
