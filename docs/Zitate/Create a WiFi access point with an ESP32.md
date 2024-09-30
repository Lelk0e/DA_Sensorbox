https://www.upesy.com/blogs/tutorials/how-create-a-wifi-acces-point-with-esp32
--- 
creation date: 2024-09-30 16:23 
modification date: Monday 30th September 2024 16:24:02 
 << [[2024-09-29]] | [[2024-10-01]] >>
 ___
```
 #include <WiFi.h>

const char* ssid     = "uPesy_AP";
const char* password = "super_strong_password";

void setup()
{
    Serial.begin(115200);
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
}

void loop(){}
```

```
#include <WiFi.h>

const char* wifi_network_ssid     = "Lounge";
const char* wifi_network_password =  "cupcakes";

const char *soft_ap_ssid          = "uPesy_AP";
const char *soft_ap_password      = NULL;

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);

    Serial.println("\n[*] Creating ESP32 AP");
    WiFi.softAP(soft_ap_ssid, soft_ap_password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());

    WiFi.begin(wifi_network_ssid, wifi_network_password);
    Serial.println("\n[*] Connecting to WiFi Network");

    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.print("\n[+] Connected to the WiFi network with local IP : ");
    Serial.println(WiFi.localIP());
}

void loop() {}
```