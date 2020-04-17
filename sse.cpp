#include <cstdint>
#include <cstddef>
#include <immintrin.h>

#include "sse_lookup.cpp"

void convert_utf16_to_utf8(const uint16_t* input, size_t size, uint8_t* output) {
    const uint16_t* end = input + (size & 0x7); // round down size to 8
    while (input != end) {
        const __m128i in = _mm_loadu_si128((__m128i*)input);
        input += 8;
 
        // 1. Fast path: values are in range 0x0000 ... 0x0800
        //    UTF16 codeword is valid and is expanded to either 1 or 2 bytes of UTF8
        const bool fast_path = true; // pretend for now...
        if (fast_path) {
            // a. for values 00 .. 7f we have transformation (two UTF16 bytes -> one UTF8 byte):
            //    [0000|0000|0ccc|dddd] => [0ccc|dddd]
            // b. for value  0080 .. 8000 we have (two UTF16 bytes -> one UTF8 bytes
            //    [0000|0bbb|cccc|dddd] => [110b|bbcc|10cc|dddd]
            const __m128i t0 = _mm_cmplt_epi16(in, _mm_set1_epi16(0x007f));

            // Note: this packs suggest we should process two SSE regs in a loop
            uint16_t pattern = _mm_movemask_epi8(_mm_packus_epi16(t0, t0));

            // [0000|0000|0ccc|dddd]
            const __m128i utf8_1byte = _mm_and_si128(in, _mm_set1_epi16(0x007f));

            __m128i byte0;
            __m128i byte1;

            // byte0 = [0000|0000|00cc|dddd]
            byte0 = _mm_and_si128(in, _mm_set1_epi16(0x003f));

            // byte1 = [000b|bbcc|xxxx|xxxx]
            byte1 = _mm_srli_epi16(in, 2);
            // byte1 = [000b|bbcc|0000|0000]
            byte1 = _mm_and_si128(byte1, _mm_set1_epi16((int16_t)0xff00));

            // update UTF8 markers:
            // byte0 = [0000|0000|10cc|dddd]
            byte0 = _mm_or_si128(byte0, _mm_set1_epi16(0x0080));
            // byte1 = [110b|bbcc|0000|0000]
            byte1 = _mm_or_si128(byte1, _mm_set1_epi16((int16_t)0xc000));

            const __m128i utf8_2bytes = _mm_or_si128(byte0, byte1);

            // keep in 16-bits proper UTF8 variants
            const __m128i utf8_t0 = _mm_blendv_epi8(utf8_2bytes, utf8_1byte, t0);

            // compress zeros from 1-byte
            const __m128i lookup = _mm_loadu_si128((const __m128i*)utf8_compress_lookup[pattern]);
            const __m128i utf8   = _mm_shuffle_epi8(utf8_t0, lookup);

            _mm_store_si128((__m128i*)output, utf8);
            output += utf8_compress_length[pattern];
        } else {
            // TODO
        }
    }

    // process tail
}
