#pragma once

#include "math/fp.hpp"
#include "math/vec.hpp"
#include "math/sin.hpp"
#include "math/rot.hpp"
#include "math/inv.hpp"
#include "font.hpp"

constexpr uint8_t NUM_SECTORS = 12;

using Vec2f = Vec2Base<float>;
using Vec2u = Vec2Base<uint8_t>;

struct SCENE_FLOAT
{
    static constexpr float MAX_R_MM = 2500;
    static constexpr float TABLE_HEIGHT_MM = 320;
    static constexpr float TABLE_R_MM = 1950;
    static constexpr float TABLE_HEIGHT = TABLE_HEIGHT_MM / MAX_R_MM;
    static constexpr float TABLE_WIDTH = TABLE_HEIGHT / 2;
    static constexpr float PIXEL_STEP = TABLE_HEIGHT / 31;
    static constexpr float PIXEL_HALF_STEP = PIXEL_STEP / 2;
    static constexpr float R_TABLE = TABLE_R_MM / MAX_R_MM + PIXEL_HALF_STEP;
    static constexpr float R_IN = R_TABLE + PIXEL_HALF_STEP;
    static constexpr float R_OUT = R_TABLE + TABLE_HEIGHT - PIXEL_HALF_STEP;
    static constexpr float R_TEXT = R_TABLE + 5 * PIXEL_STEP;
    static constexpr float H_TEXT = 26 * PIXEL_STEP;
    static constexpr float W_TEXT = float(FONT_W) / float(FONT_H) * H_TEXT;
    static constexpr Vec2f LL_TEXT = {R_TEXT, -W_TEXT / 2};
    static constexpr Vec2f UR_TEXT = {R_TEXT + H_TEXT, W_TEXT / 2};
    // static constexpr Vec2f TEXT_INV_SCALE = Vec2f{1.0, 1.0} / (H_TEXT - W_TEXT);
};

constexpr struct
{
    fp_t TABLE_HEIGHT = SCENE_FLOAT::TABLE_HEIGHT;
    fp_t TABLE_WIDTH = SCENE_FLOAT::TABLE_WIDTH;
    fp_t PIXEL_STEP = SCENE_FLOAT::PIXEL_STEP;
    fp_t PIXEL_HALF_STEP = SCENE_FLOAT::PIXEL_HALF_STEP;
    fp_t R_TABLE = SCENE_FLOAT::R_TABLE;
    fp_t R_IN = SCENE_FLOAT::R_IN;
    fp_t R_OUT = SCENE_FLOAT::R_OUT;
    fp_t R_TEXT = SCENE_FLOAT::R_TEXT;
    fp_t H_TEXT = SCENE_FLOAT::H_TEXT;
    fp_t W_TEXT = SCENE_FLOAT::W_TEXT;
    Vec2 LL_TEXT = SCENE_FLOAT::LL_TEXT;
    Vec2 UR_TEXT = SCENE_FLOAT::UR_TEXT;
    // Vec2 TEXT_INV_SCALE = {SCENE_FLOAT::TEXT_INV_SCALE.x / 100, SCENE_FLOAT::TEXT_INV_SCALE.y / 100};
} SCENE;

struct Rotations
{
    Rot2 forward[NUM_SECTORS];
    Rot2 backward[NUM_SECTORS];
};

Rotations FillRotations()
{
    Rotations rotations;
    float SECTOR_STEP = 1.0 / NUM_SECTORS;
    for (uint8_t i = 0; i < NUM_SECTORS; ++i)
    {
        float const angle = SECTOR_STEP * i;
        rotations.forward[i] = Rot2{angle};
        rotations.backward[i] = Rot2{-angle};
    }
    return rotations;
}

static const Rotations SECTOR_ROTATIONS = FillRotations();

uint8_t AngleToSector(fp_t angle)
{
    constexpr fp_t SECTOR_STEP = 1.0 / NUM_SECTORS;
    constexpr fp_t SECTOR_HALF_STEP = 1.0 / NUM_SECTORS / 2;

    constexpr auto SECTOR_STEP_INT = SECTOR_STEP.getFraction();

    angle += SECTOR_HALF_STEP;

    if (angle < 0)
    {
        angle = 1.0 + angle;
    }

    auto const fangle = angle.getFraction();

    return fangle / SECTOR_STEP_INT;
}

bool IsInBox(Vec2 const& v, Vec2 const& ll, Vec2 const& ur)
{
    return v.x >= ll.x && v.x <= ur.x && v.y >= ll.y && v.y <= ur.y;
}

Vec2 ToBox(Vec2 const& v, Vec2 const& ll, Vec2 const& ur)
{
    fp_t x, y;
    x = v.x - ll.x;
    y = v.y - ll.y;

    return Vec2{x, y} / (SCENE.UR_TEXT - SCENE.LL_TEXT);
}

static uint32_t DISTORTION_MASKS[3][24] = {{}, {}, {}};

uint8_t NthBit(uint32_t word, uint8_t n)
{
  return ((uint32_t(1) << n) & word) >> n;
}

Vec2u ToBoxFont(Vec2 const& v, Vec2 const& ll, Vec2 const& ur, bool glitch)
{
    constexpr fp_t FONT_STEP_X = SCENE_FLOAT::H_TEXT / FONT_H;
    constexpr fp_t FONT_STEP_Y = SCENE_FLOAT::W_TEXT / FONT_W;

    fp_t x, y;
    x = v.x - ll.x;
    y = v.y - ll.y;

    int16_t fx = y.getFraction() / FONT_STEP_X.getFraction();
    int16_t fy = x.getFraction() / FONT_STEP_Y.getFraction();

    if (fx >= FONT_W)
    {
        fx = FONT_W - 1;
    }

    if (fy >= FONT_H)
    {
        fy = FONT_H -1;
    }

    if (glitch)
    {
        auto wordIdx = fx;
        auto bitIdx = fy;

        uint8_t b0 = NthBit(DISTORTION_MASKS[0][wordIdx], bitIdx);
        uint8_t b1 = NthBit(DISTORTION_MASKS[1][wordIdx], bitIdx);
        uint8_t b2 = NthBit(DISTORTION_MASKS[2][wordIdx], bitIdx);

        uint8_t cw = b0 | (b1 << 1) | (b2 << 2);
        switch (cw)
        {
        case 1:
            fx = fx - 1;
            break;
        case 2:
            fy = fy - 1;
            break;
        case 3:
            fx = fx + 1;
            break;
        case 4:
            fy = fy + 1;
            break;
        default:
            break;
        }

        if (fx < 0) fx = 0;
        if (fy < 0) fy = 0;
        if (fx >= FONT_W) fx = FONT_W - 1;
        if (fy >= FONT_H) fy = FONT_H - 1;
    }
    return Vec2u{static_cast<uint8_t>(fx), static_cast<uint8_t>(fy)};
}

Vec2 DisplayPixel(Rot2 r, int x, int y)
{
    auto pc = Vec2{y * SCENE.PIXEL_STEP + SCENE.R_TABLE, x * SCENE.PIXEL_STEP - SCENE.TABLE_WIDTH};
    return r.Apply(pc);
}

Vec2 DisplayPixel(fp_t angle, int x, int y)
{
    return DisplayPixel(Rot2{angle}, x, y);
}

template<typename Fn>
void ForEachDisplayPixel(fp_t angle, Fn&& fn)
{
    Rot2 r{angle};
    Vec2 psx = r.Apply(Vec2{SCENE.PIXEL_STEP, 0});
    Vec2 psy = r.Apply(Vec2{0, SCENE.PIXEL_STEP});
    Vec2 start = r.Apply(Vec2{SCENE.R_TABLE, -SCENE.TABLE_WIDTH / 2});

    for (unsigned i = 0; i < 16; ++i)
    {
        Vec2 ystart = start;
        for (unsigned j = 0; j < 32; ++j)
        {
            fn(ystart, i, j);
            ystart = ystart + psx;
        }

        start = start + psy;
    }
}
