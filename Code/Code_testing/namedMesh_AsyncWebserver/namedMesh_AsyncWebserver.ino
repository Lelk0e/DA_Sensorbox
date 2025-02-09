#include "namedMesh.h"
#include <ESPAsyncWebServer.h>


Scheduler userSched;
namedMesh mesh;

String nodeName = "mainESP";
String msgBME = "";
String msgOzon = "";
String msgPT = "";
String msgInter = "";

void sendRoot();

void sendRoot() {
  mesh.sendBroadcast(nodeName);
}

IPAddress getlocalIP() {
    return IPAddress(mesh.getAPIP()); 
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
  mesh.setAPIP(IPAddress(192, 168, 4, 1));
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
  server.on("/bme", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", msgBME);
  });
  server.on("/ozon", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", msgOzon);
  });
  server.on("/PT100", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", msgPT);
  });
  server.on("/Inter", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", msgInter);
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
