Licenses for the code which uses the asynwebserver library, which contains all important factors, for creating a asyn. webserver
More details about the AsyncWebserver library can be found on "https://github.com/me-no-dev/ESPAsyncWebServer"


//******************************************************************************************************************************************
//this code is for the asynchroned hosting
//its main function is to deliver data to webserver
//and show it as a graph
//those data infos will from the sensor we implented 
//--> which will contain temperature, airmoisure, airpressure, gas portions e.g. C02 data
//******************************************************************************************************************************************
//this code will use the asynwebserver library, which contains all important factors, for creating a asyn. webserver
//More details about the AsyncWebserver library can be seen on https://github.com/me-no-dev/ESPAsyncWebServer
//******************************************************************************************************************************************

//those libraries are meant for all ESP's except ESP8266
#include <AsyncTCP.h>
#include "namedMesh.h"
#include <ESPAsyncWebServer.h>
#include "IPAddress.h"

//depends on the configurations which were set by the esp
#define STATION_SSID     "Name" 
#define STATION_PASSWORD "Password"
// Global variables
Scheduler userSched;
namedMesh mesh;
AsyncWebServer server(80);

//string data --> contains all important data from the sensors
// data1|data2|data3|data4 --> it will seperated like that
// data1 --> e.g. "10.3;99.3;100.2;22;40;40" etc.
char string_data[]; //this line of code's content will everytime change (realtime)
String nodeName = "mainESP";
String msgBME = "";
String msgOzon = "";
String msgPT = "";
String msgInter = "";

//Declaring 4 types of strings for string data
char Temperature_str[];
char  AirMoisure_str[];
char AirPressure_str[];
char  GasPortion_str[];
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

//protocole for the async. 
//Port 80, which is http
//Http, cause right now the security factor is not so important
AsyncWebServer server(80); 
void newConnectionCallback(uint32_t nodeId) {
  sendRoot();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.printIn("beginning setup!")

  rip_string_data(); //here we will get our data
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

  //initializing Async. Webserver --> this part will be adjusted later
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)){ 
  request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
  }
  server.begin();
}

void rip_string_data(){

}
void loop() {
  // put your main code here, to run repeatedly:

  mesh.update();
}

