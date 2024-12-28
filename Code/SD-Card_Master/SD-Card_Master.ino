#include <SPI.h>
#include <SD.h>

#define cs_pin 5 //kein CS Pin auf dem ESP32 vorhanden

File data;
String readData = "";
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  if(!SD.begin(cs_pin))
  {
    Serial.println("Kaputt");
    while (1); //Das SD-Karte nicht kaputt gehen kann bei einem Fehler
  }

  data = SD.open("data.txt", FILE_WRITE); //Datei öffnen 

  if (data){ //Ist die Datei offen
    Serial.println("Writing");
    data.println("BME280: 25°C; 70%; 1025hPa");
    data.close(); //File Schliessen 
    Serial.println("Done writing");
  }

  delay(1000);

  data = SD.open("data.txt", FILE_READ);

  if (data){ //Ist die Datei offen
  
    Serial.println("Reading");
    while (data.available()){
      char c = data.read();
      readData += c;
      
    }
    Serial.println(readData);
    data.close();
    Serial.println("Done reading");
  }
}
void loop()
{

}
