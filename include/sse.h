#pragma once

#include <cstdint>
#include <cstddef>

size_t sse_convert_utf16_to_utf8(const uint16_t* input, size_t size, uint8_t* output);


// daniel reimplemented sse_convert_utf16_to_utf8 by accident
size_t sse_convert_utf16_to_utf8_lemire(const uint16_t* input, size_t size, uint8_t* output);