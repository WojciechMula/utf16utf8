#pragma once

#include <cstdint>
#include <cassert>
#include <cstddef>

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
            error_handler(codepoints, curr);
            continue;
        }

        if (curr == end) { // required the next word, but we're already at the end of data
            error_handler(codepoints, curr);
            break;
        }

        const uint16_t W2 = *curr;
        if (W2 < 0xdc00 || W2 > 0xdfff) { // W2 = 0xdc00 .. 0xdfff
            error_handler(codepoints, curr);
        } else {
            const uint32_t lo = 0x3ff; // take lower 10 bits of W1 and W2
            const uint32_t hi = 0x3ff;
            const uint32_t tmp = lo | (hi << 10); // build a 20-bit temporary value U'

            consumer(tmp - 0x10000);
        }

        curr += 1;
    }
}
