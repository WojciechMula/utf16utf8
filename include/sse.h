#pragma once

#include <cstdint>
#include <cstddef>

size_t sse_convert_utf16_to_utf8(const uint16_t* input, size_t size, uint8_t* output);

size_t sse_convert_utf16_to_utf8_hybrid(const uint16_t* input, size_t size, uint8_t* output);

size_t sse_convert_valid_utf8_to_utf16_lemire(const uint8_t* input, size_t size, uint16_t* output);