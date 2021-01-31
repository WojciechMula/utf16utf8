#pragma once

#include <cstdint>
#include <cstddef>

size_t pext_convert_utf8_to_utf16(const uint8_t* input, size_t size, uint16_t* output);
