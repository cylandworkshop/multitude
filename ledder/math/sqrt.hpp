#pragma once

#include "./fp.hpp"

fp_t Sqrt(fp_t x)
{
    constexpr size_t SQRT_NUM_ITERATIONS = 4;

    constexpr fp_t STEP = fp_t::Epsilon * 2;

    auto x0 = x / 2;
    for (size_t i = 0; i < SQRT_NUM_ITERATIONS; ++i)
    {
        auto const delta = x0 * x0 - x;

        if (absFixed(delta) <= STEP)
        {
            break;
        }

        x0 = x0 - delta / (2 * x0);
    }

    return x0;
}