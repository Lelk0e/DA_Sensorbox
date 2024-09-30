https://www.upesy.com/blogs/tutorials/how-to-connect-wifi-acces-point-with-esp32
--- 
creation date: 2024-09-30 16:17 
modification date: Monday 30th September 2024 16:17:32 
 << [[2024-09-29]] | [[2024-10-01]] >>
___
```
#include <WiFi.h>

const char* ssid = "yourNetworkName";
const char* password = "yourNetworkPassword";

void setup(){
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

void loop(){}
```


