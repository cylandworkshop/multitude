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
#include "math/inv.hpp"
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
  ENCODER.Update(millis(), dt, abs(position) % ENCODER_STEPS);

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

bool DoSampleFont(char c, fp_t x, fp_t y)
{
    // uint8_t const width = pgm_read_byte(FONT + FONT_WIDTH_OFFSET);
    // uint8_t const height = pgm_read_byte(FONT + FONT_HEIGHT_OFFSET);

    // return SampleFont(c, roundFixed(x * width).getInteger(), roundFixed(y * height).getInteger());
    constexpr fp_t FONT_STEP_X = 1.0 / FONT_H;
    constexpr fp_t FONT_STEP_Y = 1.0 / FONT_W;

    constexpr auto FONT_STEP_X_INT = FONT_STEP_X.getFraction();
    constexpr auto FONT_STEP_Y_INT = FONT_STEP_Y.getFraction();

    constexpr auto FONT_STEP_X_INV = FONT_STEP_X_INT % 2 == 0 ? MulInverse(FONT_STEP_X_INT - 1) : MulInverse(FONT_STEP_X_INT);
    constexpr auto FONT_STEP_Y_INV = FONT_STEP_Y_INT % 2 == 0 ? MulInverse(FONT_STEP_Y_INT - 1) : MulInverse(FONT_STEP_Y_INT);

    auto const ix = x.getFraction();
    auto const iy = y.getFraction();

    auto fx = iy * FONT_STEP_Y_INV;
    auto fy = ix * FONT_STEP_X_INV;

    if (fx >= FONT_W)
    {
      fx = FONT_W - 1;
    }
    if (fy >= FONT_H)
    {
      fy = FONT_H - 1;
    }

    auto wordIdx = fx;
    auto bitIdx = fy;

    uint8_t b0 = NthBit(DISTORTION_MASKS[0][wordIdx], bitIdx);
    uint8_t b1 = NthBit(DISTORTION_MASKS[1][wordIdx], bitIdx);
    uint8_t b2 = NthBit(DISTORTION_MASKS[2][wordIdx], bitIdx);

    uint8_t cw = b0 | (b1 << 1) | (b2 << 2);

    // Serial.println(int(cw));

    switch (cw)
    {
      case 0:
      case 1:
        fx = fx > 1 ? fx - 2 : FONT_W - 2;
        break;
      case 2:
      case 5:
        fy = fy > 1 ? fy - 2 : FONT_H - 2;
        break;
      case 3:
      case 6:
        fx = fx < FONT_W - 2 ? fx + 2 : 0;
        break;
      case 4:
        fy = fy < FONT_W - 2 ? fy + 2 : 0;
        break;
      default:
        break;
    }

    return SampleFont(c, fx, fy);
}

void TryUpdateFakeEncoder()
{
  static uint32_t LAST_FAKE_ENCODER = 0;
  static uint8_t LAST_FAKE_ENCODER_STATE = 0;
  uint32_t const t = millis();
  auto const dt = Duration(LAST_FAKE_ENCODER, t);
  if (dt > 2000)
  {
    if (LAST_FAKE_ENCODER)
    {
      ENCODER.Update(t, dt, LAST_FAKE_ENCODER_STATE);
    }
    LAST_FAKE_ENCODER_STATE = (LAST_FAKE_ENCODER_STATE + 1) % ENCODER_STEPS;
    LAST_FAKE_ENCODER = t;
  }
}

uint16_t Rng()
{
  static uint32_t state = 0x13371337;
  uint32_t x = state;
  x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
  state = x;
  return static_cast<uint16_t>(state >> 8);
}

void loop() {
  TryUpdateEncoder();

  // TryUpdateFakeEncoder();
  uint32_t const t = millis();

  static uint32_t dmaskTs = 0;

  if (Duration(dmaskTs, t) > 1)
  {
    dmaskTs = t;
    auto maskIdx = Rng() % 3;
    auto bitIdx = Rng() % (uint16_t(FONT_H) * FONT_W);
    auto wordIdx = bitIdx / 32;
    bitIdx = bitIdx % 32;

    DISTORTION_MASKS[maskIdx][wordIdx] ^= uint32_t(1) << bitIdx;
    // Serial.println(DISTORTION_MASKS[maskIdx][wordIdx]);
  }
  // if (Duration(LAST_REDRAW_TS, t) < 10)
  // {
  //   return;
  // }

  // LAST_REDRAW_TS = t;

  auto const angle = ENCODER.GetAngle(t);
  // if (angle == ENCODER_PREV_ANGLE)
  // {
  //   return;
  // }

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
      // Vec2 ystart = start;
      Vec2 iystart = istart;
      for (unsigned j = 0; j < 32; ++j)
      {
          {
            // auto const p = ystart;
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
                // auto const bc = ToBox(tp, SCENE.LL_TEXT, SCENE.UR_TEXT);
                bool const isLetter = sector % 3 == 0;
                auto const bc = ToBoxFont(tp, SCENE.LL_TEXT, SCENE.UR_TEXT, isLetter);

                if (isLetter)
                {
                  if (SampleFont('A' + sector / 3, bc.x, bc.y))
                  {
                      led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
                  }
                }
                else
                {
                  if (SampleFont('S', bc.x, bc.y))
                  {
                      led_module.writePixel(y, x, GRAPHICS_NORMAL, 1);
                  }
                }
            }
          }
          // ystart = ystart + psx;
          iystart = iystart + ipsx;
      }

      // start = start + psy;
      istart = istart + ipsy;
  }

  // char buf[10];
  // auto const chars = sprintf(buf, "%d", int(sector));

  // led_module.drawString(0, 0, buf, chars, GRAPHICS_TOGGLE);

  auto const tEnd = millis();
  // Serial.println(Duration(tStart, tEnd));
  // Serial.print('#'); Serial.print(numInBox);
  // Serial.println(int(SCENE.PIXEL_HALF_STEP > 0));
  // led_module.scanDisplayBySPI();
  // delay(1);
}
