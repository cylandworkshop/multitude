#pragma once

#include "./fp.hpp"
// #include "sin-table.hpp"

constexpr fp_t Sin(fp_t x)
{
    return sinf(float(x) * M_PI * 2);
    // constexpr fp_t half = 0.5;
    // bool isNegative = signbitFixed(x);

    // auto idx = absFixed(x);
    // if (idx >= 0.5)
    // {
    //     idx = 1 - idx;
    //     isNegative = !isNegative;
    // }

    // if (idx >= 0.25)
    // {
    //     idx = 0.5 - idx;
    // }

    // auto const sinIdx = idx.getFraction();

    // auto const sinabs = SIN_LOOKUP_TABLE[sinIdx];
    // return (isNegative) ? -sinabs : sinabs;
}

constexpr fp_t Cos(fp_t x)
{
    return Sin(x + 0.25);
}