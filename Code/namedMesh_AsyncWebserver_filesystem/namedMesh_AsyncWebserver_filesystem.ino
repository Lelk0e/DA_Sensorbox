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

AsyncWebServer server(80);

void sendRoot();

void sendRoot() {
  mesh.sendBroadcast(nodeName);
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

void receivedCallback(String &from, String &msg) {
  Serial.printf("Received from=%s msg=%s\n", from.c_str(), msg.c_str());
  if (from.equals("BME280")) {
    msgBME = msg;
  } else if (from.equals("Ozon")) {
    msgOzon = msg;
  } else if (from.equals("PT100")) {
    msgPT = msg;
  } else if (from.equals("Interchangable")) {
    msgInter = msg;
  }
}

void newConnectionCallback(uint32_t nodeId) {
  sendRoot();
}

void setup() {
  Serial.begin(115200);
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
  initSDCard();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webpage/website.html", "text/html");
  });
  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest *request) {
    String hour;
    String minute;
    String second;
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
  server.begin();

}

void loop() {
  mesh.update();
  if (!msgBME.isEmpty()) {
    Serial.println("BME280 Message: " + msgBME);  
    Serial.println("____________________________________");
    delay(1000);
  }
  if (!msgOzon.isEmpty()) {
    Serial.println("Ozon Message: " + msgOzon);
    Serial.println("____________________________________");
    delay(1000);
  }
  if (!msgPT.isEmpty()) {
    Serial.println("PT100 Message: " + msgPT);
    Serial.println("____________________________________");
    delay(1000);
  }
  if (!msgInter.isEmpty()) {
    Serial.println("Interchangable Message: " + msgInter);
    Serial.println("____________________________________");
    delay(1000);
  }
}
