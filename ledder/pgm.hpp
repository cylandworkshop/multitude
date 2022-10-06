#pragma once

#include <cstdint>

#ifndef __AVR

#define PROGMEM

uint8_t pgm_read_byte(void const* ptr)
{
    return *static_cast<uint8_t const*>(ptr);
}

#endif