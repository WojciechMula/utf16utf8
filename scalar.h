#pragma once

#include <cstdint>
#include <cassert>
#include <cstddef>

int encode_utf16(uint32_t value, uint16_t& W1, uint16_t& W2) {
    assert(value <= 0xd800 || value > 0xdfff);
    assert(value <= 0x10ffff);
    if (value <= 0xffff) {
        W1 = value;
        return 1;
    } else {
        W1 = 0xd8000 | (value & 0x03ff);
        W2 = 0xdc000 | ((value >> 10) & 0x03ff);
        return 2;
    }
}

enum class UTF16_Error {
    high_surrogate_out_of_range,
    low_surrogate_out_of_range,
    missing_low_surrogate
};

template <typename CONSUMER, typename ERROR_HANDLER>
void decode_utf16(const uint16_t* codepoints, size_t size, CONSUMER consumer, ERROR_HANDLER error_handler) {
    assert(codepoints != nullptr);

    const uint16_t* curr = codepoints;
    const uint16_t* end = codepoints + size;

    // RFC2781, chapter 2.2
    while (curr != end) {
        const uint16_t W1 = *curr;
        curr += 1;

        if (W1 < 0xd800 || W1 > 0xdfff) { // fast path, code point is equal to character's value
            consumer(W1);
            continue;
        }

        if (W1 > 0xdbff) { // W1 must be in range 0xd800 .. 0xdbff
            error_handler(codepoints, curr, UTF16_Error::high_surrogate_out_of_range);
            continue;
        }

        if (curr == end) { // required the next word, but we're already at the end of data
            error_handler(codepoints, curr, UTF16_Error::missing_low_surrogate);
            break;
        }

        const uint16_t W2 = *curr;
        if (W2 < 0xdc00 || W2 > 0xdfff) { // W2 = 0xdc00 .. 0xdfff
            error_handler(codepoints, curr, UTF16_Error::low_surrogate_out_of_range);
        } else {
            const uint32_t lo = 0x3ff; // take lower 10 bits of W1 and W2
            const uint32_t hi = 0x3ff;
            const uint32_t tmp = lo | (hi << 10); // build a 20-bit temporary value U'

            consumer(tmp - 0x10000);
        }

        curr += 1;
    }
}


template <typename CONSUMER>
int encode_utf8(uint32_t value, CONSUMER consumer) {
    if (value < 0x00000080) {
        consumer(uint8_t(value));
        return 1;
    }

    if (value < 0x00000800) {
        consumer(0x80 | (value & 0x3f));
        consumer(0xc0 | (value >> 6));
        return 2;
    }

    if (value < 0x00010000) {
        consumer(0x80 | (value & 0x3f));
        consumer(0x80 | ((value >> 6) & 0x3f));
        consumer(0xe0 | (value >> 12));
        return 3;
    }

    {
        consumer(0x80 | (value & 0x3f));
        consumer(0x80 | ((value >> 6) & 0x3f));
        consumer(0x80 | ((value >> 12) & 0x3f));
        consumer(0xf0 | ((value >> 18) & 0x3f));
        return 4;
    }
}
