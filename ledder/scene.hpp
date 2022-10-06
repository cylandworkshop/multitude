#pragma once

#include "math/fp.hpp"
#include "math/vec.hpp"
#include "math/sin.hpp"
#include "math/rot.hpp"

constexpr fp_t TABLE_HEIGHT = 0.103;
constexpr fp_t TABLE_WIDTH = TABLE_HEIGHT / 2;
constexpr fp_t PIXEL_STEP = TABLE_HEIGHT / 31;
constexpr fp_t PIXEL_HALF_STEP = PIXEL_STEP / 2;

constexpr struct
{
    fp_t R_TABLE = 0.8333 - TABLE_HEIGHT;
    fp_t R_IN = R_TABLE + PIXEL_HALF_STEP;
    fp_t R_OUT = R_TABLE + TABLE_HEIGHT - PIXEL_HALF_STEP;
} SCENE;

Vec2 DisplayPixel(Rot2 r, int x, int y)
{
    auto pc = Vec2{x * PIXEL_STEP - TABLE_WIDTH / 2, y * PIXEL_STEP + SCENE.R_TABLE};
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

    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {
            auto pos = DisplayPixel(r, i, j);
            fn(pos);
        }
    }
}