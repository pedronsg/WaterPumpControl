#include "display.h"
#include "config.h"
#include "SSD1306Wire.h"

SSD1306Wire ssdDisplay;
ConfigData *display_config;

void Display::drawProgressBarValue(const int progress) {
  //int progress = (counter / 5) % 100;
  // draw the progress bar
  ssdDisplay.drawProgressBar(0, 32, 120, 10, progress);

  // draw the percentage as String
  ssdDisplay.setTextAlignment(TEXT_ALIGN_CENTER);
  ssdDisplay.drawString(64, 15, String(progress) + "%");
}

Display::Display()
{
  display_config=CConfig::GetInstance();
  ssdDisplay.begin(DISPLAYADDRESS, SDAPIN, SCLPIN);
}

void Display::init()
{
   blink=false;
  blinkMillis=0;
  ssdDisplay.init();
  //ssdDisplay.flipScreenVertically();
  ssdDisplay.setFont(ArialMT_Plain_16);
}

void Display::drawBars(const float min, const float max, const float bars)
{
  //ssdDisplay.clear();
  ssdDisplay.setTextAlignment(TEXT_ALIGN_LEFT);
  ssdDisplay.setFont(ArialMT_Plain_24);
  ssdDisplay.drawString(0, 0, (String)bars);
  ssdDisplay.setFont(ArialMT_Plain_10);
  ssdDisplay.drawString(48, 12, "bar");
  ssdDisplay.drawString(83, 0, "min");
  ssdDisplay.drawString(78, 12, (String)min);
  ssdDisplay.drawString(108, 0, "max");
  ssdDisplay.drawString(108, 12, (String)max);
}

void Display::drawAmps(const float min, const float max, const float amps)
{
  ssdDisplay.setFont(ArialMT_Plain_24);
  ssdDisplay.drawString(0, 24, (String)amps);
  ssdDisplay.setFont(ArialMT_Plain_10);
  ssdDisplay.drawString(48, 36, "amp");
  ssdDisplay.drawString(83, 24, "min");
  ssdDisplay.drawString(78, 36, (String)min);
  ssdDisplay.drawString(108, 24, "max");
  ssdDisplay.drawString(108, 36, (String)max);
}

void Display::drawMsg(const char* msg)
{
  ssdDisplay.setFont(ArialMT_Plain_16);
  ssdDisplay.setTextAlignment(TEXT_ALIGN_CENTER);
  ssdDisplay.drawString(64, 49, msg);
}

void Display::drawMsg(const char* msg, int x, int y)
{
  ssdDisplay.setFont(ArialMT_Plain_16);
  ssdDisplay.setTextAlignment(TEXT_ALIGN_CENTER);
  ssdDisplay.drawString(x, y, msg);
}


void Display::print()
{
  ssdDisplay.display();
}

void Display::clear()
{
  ssdDisplay.clear();
}

void Display::printConnectingWifi()
{
  ssdDisplay.setFont(ArialMT_Plain_16);
  ssdDisplay.setTextAlignment(TEXT_ALIGN_CENTER);
  ssdDisplay.drawString(64,8, "Connecting");
  ssdDisplay.drawString(64,24, "Wifi");
  ssdDisplay.display();
}

void Display::printInitIp(const String ip)
{
  ssdDisplay.clear();
  ssdDisplay.setFont(ArialMT_Plain_16);
  ssdDisplay.drawString(64,8, "IP address ");
  ssdDisplay.drawString(64,24,ip);
  ssdDisplay.display();
  delay ( 2000 );
}

void Display::printFirmwareVersion()
{
  ssdDisplay.clear();
  ssdDisplay.setTextAlignment(TEXT_ALIGN_CENTER);
  ssdDisplay.setFont(ArialMT_Plain_16);
  ssdDisplay.drawString(64,8, "PumpControl");
  ssdDisplay.drawString(64,24,(String("v") + FIRMWAREVERSION ).c_str());
  ssdDisplay.display();
  delay ( 2000 );
}


void Display::printProgressValue(const uint8_t value, const char* msg)
{
  // Initialising the UI will init the display too.
  //ssdDisplay.init();
  //ssdDisplay.flipScreenVertically();
 // ssdDisplay.setFont(ArialMT_Plain_16);

  ssdDisplay.clear();
  drawProgressBarValue(value);
  drawMsg(msg);
  ssdDisplay.display();
}

void Display::printRunning(Pump &pump, Tank &tank)
{
  if(millis()-blinkMillis>LCDBLINKMSGINTERVAL)
  {
    clear();
    drawBars(display_config->minBars, display_config->maxBars, tank.getBars());
    drawAmps(display_config->minAmps, display_config->maxAmps, pump.getAmps());
    if (blink) 
      drawMsg(pump.getTextStatus().c_str());

    blink=!blink;
    print();
    blinkMillis=millis();
  }
}

void Display::printFlashing(uint8_t &percent)
{
  clear();
  drawProgressBarValue(percent);
  drawMsg(String("flashing...").c_str()); 
  print();
}

void Display::printRebooting()
{
  clear();
  drawProgressBarValue(100);
  drawMsg(String("Rebooting...").c_str());
  print();
}

void Display::printReset()
{
    clear();
    drawMsg(String("Resetting...").c_str(),64,29);
    print();
    delay(1000);
    drawMsg(String("Calibrating...").c_str(),64,29);
    print();
}
