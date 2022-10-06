#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"
#include "Arial_Black_16_ISO_8859_1.h"

#include "glsl.h"

#define ROW 1
#define COLUMN 1
#define FONT Arial_Black_16_ISO_8859_1
#define LED_STEP 10

#define WIDTH (ROW * 16)
#define HEIGHT (COLUMN * 32)

DMD led_module(ROW, COLUMN);

void scan_module() {
  led_module.scanDisplayBySPI();
}

void setup() {
  Timer1.initialize(500);
  Timer1.attachInterrupt(scan_module);
  led_module.clearScreen(true);

  led_module.selectFont(FONT);

  analogWrite(9, 255);
  
  Serial.begin(9600);
}

bool preamble = false;
void loop() {
  uint32_t t = millis();
  
  if (Serial.available() > 0) {
    if(!preamble && Serial.read() == '#') {
      preamble = true;
    } else {
      preamble = false;
      
      uint8_t cmd = Serial.read();
      switch(cmd) {
        case '.':
          Serial.println("keepalive");
          led_module.clearScreen(true);
          led_module.drawString(0, 0, ".", 1, GRAPHICS_NORMAL);
          delay(100);
          led_module.clearScreen(true);
        break;
        case '!':
          Serial.println("bang");
          led_module.clearScreen(true);
          led_module.drawString(0, 0, "!!!", 3, GRAPHICS_NORMAL);
          delay(100);
          led_module.clearScreen(true);
        break;
        default: break;
      }
    }
  }
}
