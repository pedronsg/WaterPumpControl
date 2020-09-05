#include "spiffs.h"
#include "FS.h"

void Spiffs::begin()
{
  SPIFFS.begin();
  delay(500);
#ifdef DEBUG_ENABLED 
  Serial1.println("\nSearching files...");
#endif
   Dir existDir = SPIFFS.openDir("/");
   if (existDir.next())
   {
     #ifdef DEBUG_ENABLED 
     Serial1.println("Spiffs already formatted");
     #endif
   }else
   {
     #ifdef DEBUG_ENABLED 
     Serial1.println("Please wait 30 secs for SPIFFS to be formatted");
     #endif
     SPIFFS.format();
     #ifdef DEBUG_ENABLED 
     Serial1.println("Spiffs formatted");
     #endif
     File f = SPIFFS.open("/formatComplete.txt", "w");
   }
  Dir dir = SPIFFS.openDir("/");
while (dir.next()) {
  #ifdef DEBUG_ENABLED 
    Serial1.print(dir.fileName());
  #endif
    File f = dir.openFile("r");
    #ifdef DEBUG_ENABLED 
    Serial1.print(" ");
    Serial1.println(f.size());
    #endif
}
}
