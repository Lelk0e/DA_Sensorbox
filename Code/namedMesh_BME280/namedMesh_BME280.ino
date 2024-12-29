#include "namedMesh.h"

Scheduler userSched;
namedMesh mesh;

String nodeName = "BME280";
String data = "24°C; 80%; 1013hPa";
String rootNode = "mainESP";

void sendData();

Task SendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendData);

void sendData() {
  mesh.sendSingle(rootNode, data);
  SendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void receivedCallback(String &from, String &msg) {
  Serial.printf("Received from %s msg=%s\n", from.c_str(), msg.c_str());
  if (!from.isEmpty()) {
    rootNode = from;
  } else {
    rootNode = "mainESP";
  }
}

void setup() {
  Serial.begin(115200);
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);
  mesh.init("Sensorbox", "12345678", &userSched, 5555);
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
  userSched.addTask(SendMessage);
  SendMessage.enable();
}

void loop() {
  mesh.update();
}
