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
#include <ESPAsyncWebServer.h>

//depends on the configurations which were set by the esp
#define STATION_SSID     "Name" 
#define STATION_PASSWORD "Password"

//string data --> contains all important data from the sensors
// data1|data2|data3|data4 --> it will seperated like that
// data1 --> e.g. "10.3;99.3;100.2;22;40;40" etc.
char string_data[]; //this line of code's content will everytime change (realtime)

//Declaring 4 types of strings for string data
char Temperature_str[];
char  AirMoisure_str[];
char AirPressure_str[];
char  GasPortion_str[];

//protocole for the async. 
//Port 80, which is http
//Http, cause right now the security factor is not so important
AsyncWebServer server(80); 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.printIn("beginning setup!")

  rip_string_data(); //here we will get our data

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

}
