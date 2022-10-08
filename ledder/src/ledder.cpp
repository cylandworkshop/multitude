#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"
#include "Arial_Black_16_ISO_8859_1.h"

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

#define FONT Arial_Black_16_ISO_8859_1

DMD led_module(ROW, COLUMN);

bool BUFFERS[2][WIDTH][HEIGHT] = {{}, {}};
volatile uint8_t CURRENT_BUFFER = 0;

void scan_module()
{
  led_module.scanDisplayBySPI();
}

void setup()
{
  Timer1.initialize(500);
  Timer1.attachInterrupt(scan_module);
  led_module.clearScreen(true);
  led_module.selectFont(FONT);

  analogWrite(9, 255);

  Serial.begin(9600);
}

constexpr size_t TOTAL_PIXELS = WIDTH * HEIGHT;

EncoderState ENCODER = {};
float ENCODER_PREV_ANGLE = 0;

uint32_t LAST_REDRAW_TS = 0;

static bool preamble = false;

void TryUpdateAfterPreamble()
{
  if (Serial.available() < 3)
  {
    return;
  }

  int8_t position = 0;
  uint16_t dt = 0;

  int pos = Serial.read();
  if (pos == -1)
  {
    preamble = false;
    return;
  }

  position = static_cast<int8_t>(pos);

  int dt1 = Serial.read();
  if (dt1 == -1)
  {
    preamble = false;
    return;
  }

  int dt2 = Serial.read();
  if (dt2 == -1)
  {
    preamble = false;
    return;
  }

  dt = (static_cast<uint16_t>(dt2) << 8) | static_cast<uint8_t>(dt1);
  ENCODER.Update(dt, abs(position) % ENCODER_STEPS);

  // Serial.print('$'); Serial.print(position); Serial.print(' '); Serial.print(dt1); Serial.print(' '); Serial.println(dt2);

  // char buf[10];
  // auto const chars = sprintf(buf, "%d", int(ENCODER.updateDuration));
  // led_module.clearScreen(true);
  // led_module.drawString(0, 0, buf, chars, GRAPHICS_NORMAL);

  preamble = false;
}

void TryUpdateEncoder()
{
  if (preamble)
  {
    TryUpdateAfterPreamble();
  }
  else if (Serial.available() >= 4)
  {
    if (Serial.read() == '$')
    {
      preamble = true;
      TryUpdateAfterPreamble();
    }
  }
}

bool SampleFont(char c, fp_t x, fp_t y)
{
    // uint8_t const width = pgm_read_byte(FONT + FONT_WIDTH_OFFSET);
    // uint8_t const height = pgm_read_byte(FONT + FONT_HEIGHT_OFFSET);

    // return SampleFont(c, roundFixed(x * width).getInteger(), roundFixed(y * height).getInteger());
    constexpr fp_t FONT_STEP_X = 1.0 / FONT_W;
    constexpr fp_t FONT_STEP_Y = 1.0 / FONT_H;

    constexpr auto FONT_STEP_X_INT = FONT_STEP_X.getFraction();
    constexpr auto FONT_STEP_Y_INT = FONT_STEP_Y.getFraction();

    auto const fx = x.getFraction();
    auto const fy = y.getFraction();

    return SampleFont(c, fy / FONT_STEP_Y_INT, fx / FONT_STEP_X_INT);
}

void loop() {
  TryUpdateEncoder();

  uint32_t const t = millis();

  // if (Duration(LAST_REDRAW_TS, t) < 10)
  // {
  //   return;
  // }

  // LAST_REDRAW_TS = t;

  auto const angle = ENCODER.GetAngle(t);
  if (angle == ENCODER_PREV_ANGLE)
  {
    return;
  }

  // Serial.print(">");
  // Serial.println(float(angle));

  // char buf[10];
  // auto const chars = sprintf(buf, "%d", int(angle * 100));
  // led_module.clearScreen(true);
  // led_module.drawString(0, 0, buf, chars, GRAPHICS_NORMAL);

  ENCODER_PREV_ANGLE = angle;

  auto const tStart = millis();

  auto const sector = AngleToSector(angle);

  led_module.clearScreen(true);

  Rot2 r{angle};
  Vec2 psx = r.Apply(Vec2{SCENE.PIXEL_STEP, 0});
  Vec2 psy = r.Apply(Vec2{0, SCENE.PIXEL_STEP});
  Vec2 start = r.Apply(Vec2{SCENE.R_TABLE, -SCENE.TABLE_WIDTH});

  auto const ir = SECTOR_ROTATIONS.backward[sector];
  Vec2 ipsx = ir.Apply(psx);
  Vec2 ipsy = ir.Apply(psy);
  Vec2 istart = ir.Apply(start);

  uint16_t numInBox = 0;

  for (unsigned i = 0; i < 16; ++i)
  {
      Vec2 ystart = start;
      Vec2 iystart = istart;
      for (unsigned j = 0; j < 32; ++j)
      {
          {
            auto const p = ystart;
            auto const tp = iystart;
            auto const x = i;
            auto const y = j;

            // if (absFixed(tp.y) <= 0.01)
            // {
            //   led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
            // }
            if (IsInBox(tp, SCENE.LL_TEXT, SCENE.UR_TEXT))
            {
                // ++numInBox;
                // led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
                auto const bc = ToBox(tp, SCENE.LL_TEXT, SCENE.UR_TEXT);
                if (SampleFont('A' + sector, bc.x, bc.y))
                {
                    // DrawPoint(renderer, p);

                    led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
                }
            }
          }
          ystart = ystart + psx;
          iystart = iystart + ipsx;
      }

      start = start + psy;
      istart = istart + ipsy;
  }
  // ForEachDisplayPixel(angle, [&](Vec2 p, unsigned x, unsigned y)
  // {
  //   if (y == 0)
  //   {
  //     iystart = istart;
  //   }

  //   // auto const r = Sqrt(p.x * p.x + p.y * p.y);
  //   // constexpr auto midpoint = (SCENE.R_OUT + SCENE.R_IN) / 2;
  //   // if (absFixed(r - SCENE.R_IN) <= PIXEL_STEP * 2)
  //   // {
  //   //   led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
  //   // }
  //   // else
  //   // {
  //     // auto const tp = SECTOR_ROTATIONS.backward[sector].Apply(p);
  //     // if (absFixed(tp.y) <= PIXEL_STEP * 2)
  //     // {
  //     //     DrawPoint(renderer, p);
  //     // }

  //     auto const tp = iystart;

  //     if (IsInBox(tp, SCENE.LL_TEXT, SCENE.UR_TEXT))
  //     {
  //         led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
  //         // auto const bc = ToBox(tp, SCENE.LL_TEXT, SCENE.UR_TEXT);
  //         // if (SampleFont('A' + sector, bc.x, bc.y))
  //         // {
  //         //     // DrawPoint(renderer, p);

  //         //     led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
  //         // }
  //     }

  //     if (y < 31)
  //     {
  //       iystart = iystart + ipsx;
  //     }
  //     else
  //     {
  //       istart = istart + ipsy;
  //     }
  //     // auto const tp = SECTOR_ROTATIONS.backward[sector].Apply(p);
  //     // if (absFixed(tp.y) <= 0.01)
  //     // {
  //     //   led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
  //     //   // asm volatile ("");
  //     // }
  //   // }

  // });

  auto const tEnd = millis();
  Serial.println(Duration(tStart, tEnd));
  // Serial.print('#'); Serial.print(numInBox);
  // Serial.println(int(SCENE.PIXEL_HALF_STEP > 0));
  // led_module.scanDisplayBySPI();
  // delay(1);
}
