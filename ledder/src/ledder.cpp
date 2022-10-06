#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"

#include <FixedPoints.h>
#include <FixedPointsCommon.h>

#include "glsl.h"

#include "math/fp.hpp"
#include "math/rot.hpp"
#include "math/sqrt.hpp"
#include "scene.hpp"
#include "encoder.hpp"

#define ROW 1
#define COLUMN 1
#define FONT1 Arial_Black_16_ISO_8859_1
#define LED_STEP 10

#define WIDTH (ROW * 16)
#define HEIGHT (COLUMN * 32)

DMD led_module(ROW, COLUMN);

void scan_module()
{
  led_module.scanDisplayBySPI();
}

void setup()
{
  Timer1.initialize(500);
  Timer1.attachInterrupt(scan_module);
  led_module.clearScreen(true);

  analogWrite(9, 255);

  Serial.begin(9600);
}

constexpr size_t TOTAL_PIXELS = WIDTH * HEIGHT;

EncoderState ENCODER = {};
float ENCODER_PREV_ANGLE = 0;

bool preamble = false;

void loop() {
  if (Serial.available() > 0) {
    if(!preamble && Serial.read() == '#') {
      preamble = true;
    } else {
      preamble = false;

      int rawByte = Serial.read();
      if (rawByte == -1)
      {
        return;
      }

      uint8_t cmd = static_cast<uint8_t>(rawByte);

      if (cmd < ENCODER_STEPS)
      {
        ENCODER.Update(millis(), cmd);
      }
    }
  }

  uint32_t const t = millis();

  auto const angle = ENCODER.GetAngle(t);
  if (angle == ENCODER_PREV_ANGLE)
  {
    return;
  }

  ENCODER_PREV_ANGLE = angle;
  led_module.clearScreen(true);

  ForEachDisplayPixel(angle, [&](Vec2 p, unsigned x, unsigned y)
  {
    auto const r = Sqrt(p.x * p.x + p.y * p.y);
    constexpr auto midpoint = (SCENE.R_OUT + SCENE.R_IN) / 2;
    if (absFixed(r - midpoint) <= PIXEL_STEP * 2)
    {
      led_module.writePixel(x, HEIGHT - y, GRAPHICS_NORMAL, 1);
    }
  });
}
