#include <Arduino.h>
#include "html_handlers.h"
#include "index_html.h"



void handleData() {
  server.send(200, "text/plane", String(amps)+"#"+String(bars)); //Send ADC value only to client ajax request
}

void handleBar() {
  server.send(200, "text/plane", String(bars)); //Send ADC value only to client ajax request
}


void handleD5() {
  String D5Value;
  updateGPIO(0,server.arg("D5"));
}

void handleD6() {
  String D6Value;
  updateGPIO(1,server.arg("D6"));
}

void handleD7() {
  String D7Value;
  updateGPIO(2,server.arg("D7"));
}

void handleD8() {
  String D8Value;
  updateGPIO(3,server.arg("D8"));
}

void updateGPIO(int gpio, String DxValue) {
  Serial.println("");
  Serial.println("Update GPIO "); Serial.print(GPIOPIN[gpio]); Serial.print(" -> "); Serial.println(DxValue);

  if ( DxValue == "1" ) {
    digitalWrite(GPIOPIN[gpio], HIGH);
    etatGpio[gpio] = "On";
    server.send ( 200, "text/html", index_html() );
  } else if ( DxValue == "0" ) {
    digitalWrite(GPIOPIN[gpio], LOW);
    etatGpio[gpio] = "Off";
    server.send ( 200, "text/html", index_html() );
  } else {
    Serial.println("Err Led Value");
  }
}

void handleRoot(){
  if ( server.hasArg("D5") ) {
    handleD5();
  } else if ( server.hasArg("D6") ) {
    handleD6();
  } else if ( server.hasArg("D7") ) {
    handleD7();
  } else if ( server.hasArg("D8") ) {
    handleD8();
  } else {
    server.send ( 200, "text/html", index_html() );
  }

}
