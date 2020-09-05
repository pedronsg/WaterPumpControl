#include "littlefs.h"
#include <FS.h>
#include <LittleFS.h>


void LITTLE_FS::begin()
{
//   LittleFS.begin();
//   delay(500);

//   Serial1.println("\nSearching files...");
//    Dir existDir = LittleFS.openDir("/");
//    if (existDir.next())
//    {
//      Serial1.println("LittleFS already formatted");
//    }else
//    {
//      Serial1.println("Please wait 30 secs for LittleFS to be formatted");
//      LittleFS.format();
//      Serial1.println("Spiffs formatted");
//      File f = LittleFS.open("/formatComplete.txt", "w");
//    }
//   Dir dir = LittleFS.openDir("/");
// while (dir.next()) {
//     Serial1.print(dir.fileName());
//     File f = dir.openFile("r");
//     Serial1.print(" ");
//     Serial1.println(f.size());
// }
}
