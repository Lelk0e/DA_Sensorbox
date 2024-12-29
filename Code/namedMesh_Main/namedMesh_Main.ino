#include "namedMesh.h"

Scheduler userSched;
namedMesh mesh;

String nodeName = "mainESP";
String msgBME = "";
String msgOzon = "";

void sendRoot();

void sendRoot() {
  mesh.sendBroadcast(nodeName);
}

void receivedCallback(String &from, String &msg) {
  Serial.printf("Received from %s msg=%s\n", from.c_str(), msg.c_str());
  if (from.equals("BME280")) {
    msgBME = msg;
  } else if (from.equals("Ozon")) {
    msgOzon = msg;
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
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
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
}
