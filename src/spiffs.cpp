#include "spiffs.h"


void Spiffs::begin()
{
  SPIFFS.begin();
  delay(500);

  Serial.println("\nSearching files...");
   Dir existDir = SPIFFS.openDir("/");
   if (existDir.next())
   {
     Serial.println("Spiffs already formatted");
   }else
   {
     Serial.println("Please wait 30 secs for SPIFFS to be formatted");
     SPIFFS.format();
     Serial.println("Spiffs formatted");
     File f = SPIFFS.open("/formatComplete.txt", "w");
   }
  Dir dir = SPIFFS.openDir("/");
while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.print(" ");
    Serial.println(f.size());
}
}
