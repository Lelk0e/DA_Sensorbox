#include "namedMesh.h"
#include <ESPAsyncWebServer.h>
#include "IPAddress.h"

// Global variables
Scheduler userSched;
namedMesh mesh;
AsyncWebServer server(80);

String nodeName = "mainESP";
String msgBME = "";
String msgOzon = "";
String msgPT = "";
String msgInter = "";

void sendRoot() {
  mesh.sendBroadcast(nodeName);
}

IPAddress getlocalIP() {
    return IPAddress(mesh.getAPIP()); 
}

void receivedCallback(String &from, String &msg) {
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

  // Initialize the mesh
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Sensorbox", "12345678", &userSched, 5555);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);
  mesh.setAPIP(IPAddress(192, 168, 4, 1));
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  // Setup web server routes
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
}
