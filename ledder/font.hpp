#pragma once

#include "pgm.hpp"

#include "armenian32.hpp"

constexpr uint16_t FONT_WIDTH_OFFSET = 2;
constexpr uint16_t FONT_HEIGHT_OFFSET = 3;
constexpr uint16_t FONT_FIRST_CHAR_OFFSET =4;
constexpr uint16_t FONT_CHAR_COUNT_OFFSET = 5;
constexpr uint16_t FONT_DATA_OFFSET = 6;

bool SampleFont(char c, uint8_t x, uint8_t y)
{
    uint16_t const width = pgm_read_byte(FONT + FONT_WIDTH_OFFSET);
    uint16_t const height = pgm_read_byte(FONT + FONT_HEIGHT_OFFSET);
    uint8_t const first = pgm_read_byte(FONT + FONT_FIRST_CHAR_OFFSET);

    auto const charOffset = c - first;
    auto const charDataOffset = FONT_DATA_OFFSET + (width * (height / 8)) * charOffset;

    auto const byteOffset = charDataOffset + (y / 8) * width + x;
    auto const bitOffset = y % 8;

    auto const byte = pgm_read_byte(FONT + byteOffset);

    return byte & (1 << bitOffset);
}