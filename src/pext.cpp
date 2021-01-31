#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>

#include "pext_utf8_to_utf16.cpp"

size_t pext_convert_utf8_to_utf16(const uint8_t* input, size_t size, uint16_t* output) {
    uint16_t* output_start = output;
    const uint8_t* end = input + size;
    const uint8_t* curr = input;
    while (curr + 4 < end)
    {
        if (int8_t(*curr) >= 0) {
            *output++ = *curr++;
            continue;
        }

        const auto& entry = pext_lookup[*curr];
        if (entry.utf8length == 0)
            abort();

        uint32_t value;
        memcpy(&value, input, 4);
        value = __builtin_bswap32(value);
        value = _pext_u32(value, entry.mask);

        if (value <= 0xffff) {
            *output++ = static_cast<uint16_t>(value);
        } else {
            value -= 0x10000;
            *output++ = uint16_t(0xd800 | ((value >> 10) & 0x03ff));
            *output++ = uint16_t(0xdc00 | (value & 0x03ff));
            return 2;
        }

        curr += entry.utf8length;
    }
    
    // TODO process the tail

    return output - output_start;
}
