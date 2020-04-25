#include "scalar_utf16.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace utf16 {

    int encode(uint32_t value, uint16_t& W1, uint16_t& W2) {
        assert(value <= 0xd800 || value > 0xdfff);
        assert(value <= 0x10ffff);
        if (value <= 0xffff) {
            W1 = value;
            return 1;
        } else {
            value -= 0x10000;
            W1 = 0xd800 | ((value >> 10) & 0x03ff);
            W2 = 0xdc00 | (value & 0x03ff);
            return 2;
        }
    }

    void test_utf16_encode_and_decode() {
        uint16_t array[2];
        for (uint32_t v = 0x10000; v <= 0x10ffff; v++) {
            uint32_t decoded = 0;
            const int n = encode(v, array[0], array[1]);

            decode(array, n,
                [&decoded](uint32_t v) { decoded = v; },
                [](const uint16_t*, const uint16_t*, Error) { puts("critical error"); abort(); });

            if (decoded != v)
                printf("%06x != %06x\n", v, decoded);
        }
    }

}
