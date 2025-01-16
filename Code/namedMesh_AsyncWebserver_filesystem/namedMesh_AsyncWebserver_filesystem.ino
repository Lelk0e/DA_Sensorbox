#include "namedMesh.h"
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <SPI.h>
#include <SD.h>

#define cs_pin 5

Scheduler userSched;
namedMesh mesh;

String nodeName = "mainESP";
String msgBME = "";
String msgOzon = "";
String msgPT = "";
String msgInter = "";
String hour;
String minute;
String second;

AsyncWebServer server(80);

void initMesh(){
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
    msgBME = msg;
    if (msgBME != ""){
      File file = SD.open("/" + from + ".txt", FILE_APPEND);
      if(file.print(msgBME)){
        Serial.println("Data Printed To BME");
      }else{
        Serial.println("Printing Failed")
      }
      file.close();
    }

  } else if (from.equals("Ozon")) {
    msgOzon = msg;
    if (msgOzon != ""){
      File file = SD.open("/" + from + ".txt", FILE_APPEND);
      if(file.print(msgOzon)){
        Serial.println("Data Printed To Ozon");
      }else{
        Serial.println("Printing Failed")
      }
      file.close();
    }

  } else if (from.equals("PT100")) {
    msgPT = msg;
    if (msgPT != ""){
      File file = SD.open("/" + from + ".txt", FILE_APPEND);
      if(file.print(msgPT)){
        Serial.println("Data Printed To PT100");
      }else{
        Serial.println("Printing Failed")
      }
      file.close();
    }

  } else if (from.equals("Interchangable")) {
    msgInter = msg;
    if (msgInter != ""){
      File file = SD.open("/" + from + ".txt", FILE_APPEND);
      if(file.print(msgInter)){
        Serial.println("Data Printed To Interchangable");
      }else{
        Serial.println("Printing Failed")
      }
      file.close();
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
  sendRoot();
  mesh.sendBroadcast(hour + ":" + minute + ":" + second);
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



void setup() {

  Serial.begin(115200);

  initMesh();

  initSDCard();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webpage/website.html", "text/html");
  });

  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    if (request->hasParam("hour") && request->hasParam("minute") && request->hasParam("second")) {
      hour = request->getParam("hour")->value();
      minute = request->getParam("minute")->value();
      second = request->getParam("second")->value();
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

  server.on("/data/bme280", HTTP_GET, [](AsyncWebServerRequest *request) {
    serveFile(request, "/BME280.txt");
  });

  server.on("/data/ozon", HTTP_GET, [](AsyncWebServerRequest *request) {
    serveFile(request, "/Ozon.txt");
  });

  server.on("/data/pt100", HTTP_GET, [](AsyncWebServerRequest *request) {
    serveFile(request, "/PT100.txt");
  });

  server.on("/data/interchangable", HTTP_GET, [](AsyncWebServerRequest *request) {
    serveFile(request, "/Interchangable.txt");
  });

  server.begin();

}

void loop() {
  mesh.update();
}
