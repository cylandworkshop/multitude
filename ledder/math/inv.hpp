#pragma once

constexpr uint16_t MulInverseStep(uint16_t x, uint16_t y)
{
    return y * (2 - y * x);
}

constexpr uint16_t MulInverse(uint16_t x)
{
    return MulInverseStep(x, MulInverseStep(x, (3 * x) ^ 2));
}