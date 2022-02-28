#include "STMPE811_SPI.h"

#define chipSelectPin 25

uLog theLog;

STMPE811 theTouchController(chipSelectPin);

void setup() {
    theTouchController.initialize();
    Serial.begin(115200);
    Serial.flush();
    Serial.print("STMPE811 - Version ");
    Serial.println(theTouchController.getVersion());
}

void loop() {
    if (theTouchController.isTouched()) {
        Serial.print("Touched : X =  ");
        Serial.print(theTouchController.getX());
        Serial.print(" Y =  ");
        Serial.println(theTouchController.getY());
        delay(1000);
    }
}
