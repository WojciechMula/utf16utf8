#include <cstdint>
#include <cstddef>
#include <immintrin.h>

#include "sse_lookup.cpp"
#include "sse_dword_lookup.cpp"

namespace nonstd {

    uint8_t _mm_movemask_epi16(const __m128i v) {
        const __m128i t0 = _mm_and_si128(v, _mm_set1_epi16(0x00ff)); // reset higher bytes
        const __m128i t1 = _mm_packus_epi16(t0, t0);

        return _mm_movemask_epi8(t1);
    }

    __m128i _mm_or_si128(const __m128i v1, const __m128i v2, const __m128i v3) {
        return ::_mm_or_si128(v1, ::_mm_or_si128(v2, v3));
    }

}

size_t sse_convert_utf16_to_utf8(const uint16_t* input, size_t size, uint8_t* output) {
    const uint16_t* end = input + (size & ~0x7); // round down size to 8
    uint8_t* start = output;
    while (input != end) {
        const __m128i in = _mm_loadu_si128((__m128i*)input);
        input += 8;

        // TODO: surrogates are not handled yet

        // 0. determine how many bytes each 16-bit value produces
        //      1 byte  =     (in < 0x0080)
        //      2 bytes = not (in < 0x0080) and not (in >= 0x0800)
        //      3 bytes =                           (in >= 0x0800)
        const __m128i lt0080 = _mm_cmplt_epi16(in, _mm_set1_epi16(0x0080));
        const __m128i ge0800 = _mm_cmplt_epi16(_mm_set1_epi16(0x07ff), in);

        // a. store lt0080 and lt0800 as bitmask, interleaving bits from both vectors
        const __m128i t0 = _mm_blendv_epi8(lt0080, ge0800, _mm_set1_epi16((int16_t)0xff00));
        const uint16_t patterns = _mm_movemask_epi8(t0);
 
        if ((patterns & 0xaaaa) == 0x0000) { // condition: not (in >= 0x08000)
            // 1. Fast path: values are in range 0x0000 ... 0x0800
            //    UTF16 codeword is valid and is expanded to either 1 or 2 bytes of UTF8

            // a. for values 00 .. 7f we have transformation (two UTF16 bytes -> one UTF8 byte):
            //    [0000|0000|0ccc|dddd] => [0ccc|dddd]
            // b. for value  0080 .. 8000 we have (two UTF16 bytes -> one UTF8 bytes
            //    [0000|0bbb|cccc|dddd] => [110b|bbcc|10cc|dddd]
            const __m128i t0 = _mm_cmplt_epi16(in, _mm_set1_epi16(0x0080));

            const uint8_t pattern = nonstd::_mm_movemask_epi16(t0);

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

            // compress zeros from 1-byte words
            const __m128i lookup = _mm_loadu_si128((const __m128i*)utf8_compress_lookup[pattern]);
            const __m128i utf8   = _mm_shuffle_epi8(utf8_t0, lookup);

            _mm_storeu_si128((__m128i*)output, utf8);
            output += utf8_compress_length[pattern];
        } else {
            // input in range 0x0000 .. 0xffff

            // 1. prepare UTF8 bytes

            // output 1 UTF8 byte  : [0000|0000|0ccc|dddd] => [0ccc|dddd]
            // output 2 UTF8 bytes : [0000|0bbb|cccc|dddd] => [110b|bbcc], [10cc|dddd]
            // output 3 UTF8 bytes : [aaaa|bbbb|cccc|dddd] => [1110|aaaa], [10bb|bbcc], [10cc|dddd]

            // a. 1 byte code equals input
            __m128i word0_1 = in;

            // a. build 2 byte codes
            __m128i byte0_2;
            __m128i byte1_2;
            __m128i word0_2;
                                                                        // [0000|0bbb|cccc|dddd]
            byte0_2 = _mm_and_si128(in, _mm_set1_epi16(0x003f));        // [0000|0000|00cc|dddd]
            byte0_2 = _mm_or_si128(byte0_2, _mm_set1_epi16(0x0080));    // [0000|0000|10cc|dddd]

            byte1_2 = _mm_and_si128(in, _mm_set1_epi16(0x07c0));        // [0000|0bbb|cc00|0000]
            byte1_2 = _mm_or_si128(byte1_2, _mm_set1_epi16(0x3000));    // [0011|0bbb|cc00|0000]
            byte1_2 = _mm_slli_epi32(byte1_2, 2);                       // [110b|bbcc|0000|0000]

            word0_2 = _mm_or_si128(byte0_2, byte1_2);                   // [110b|bbcc|10cc|dddd]

            // b. build 3 byte codes
            __m128i byte0_3;
            __m128i byte1_3;
            __m128i byte2_3;
            __m128i word0_3;
            __m128i word1_3;

            byte0_3 = byte0_2; // reuse from 2-byte case                // [0000|0000|10cc|dddd]

                                                                        // [aaaa|bbbb|cccc|dddd]
            byte1_3 = _mm_and_si128(in, _mm_set1_epi16(0x0fc0));        // [0000|bbbb|cc00|0000]
            byte1_3 = _mm_or_si128(byte1_3, _mm_set1_epi16(0x2000));    // [0010|bbbb|cc00|0000]
            byte1_3 = _mm_slli_epi32(byte1_3, 2);                       // [10bb|bbcc|0000|0000]

            word0_3 = _mm_or_si128(byte0_3, byte1_3);

            byte2_3 = _mm_srli_epi16(in, 12);
            byte2_3 = _mm_or_si128(byte2_3, _mm_set1_epi16(0x00e0));    // [0000|0000|1110|aaaa]
            word1_3 = byte2_3;

            word0_1 = _mm_and_si128(lt0080, word0_1);

            const __m128i not_m2 = _mm_or_si128(lt0080, ge0800);
            word0_2 = _mm_andnot_si128(not_m2, word0_2);

            word0_3 = _mm_and_si128(ge0800, word0_3);
            word1_3 = _mm_and_si128(ge0800, word1_3);

            // 2. expand 2-byte codes into 4-byte codes, possible dwords:
            //    - [0000|0000|0000|0000|0000|0000|0ccc|dddd]
            //    - [0000|0000|0000|0000|110b|bbcc|10cc|dddd]
            //    - [0000|0000|1110|aaaa|10bb|bbcc|10cc|dddd]
            
            __m128i word0;
            __m128i word1;
            word0 = nonstd::_mm_or_si128(word0_1, word0_2, word0_3);
            word1 = word1_3;

            __m128i dword_lo = _mm_unpacklo_epi16(word0, word1);
            __m128i dword_hi = _mm_unpackhi_epi16(word0, word1);

            // 3. compress bytes
            // a. compress lo dwords
            {
                const uint8_t pattern = patterns & 0x00ff;
                const __m128i lookup  = _mm_loadu_si128((const __m128i*)compress_dwords_utf8_lookup[pattern]);
                const __m128i utf8    = _mm_shuffle_epi8(dword_lo, lookup);

                _mm_storeu_si128((__m128i*)output, utf8);
                output += compress_dwords_utf8_length[pattern];
            }

            // b. compress hi dwords
            {
                const uint8_t pattern = patterns >> 8;
                const __m128i lookup  = _mm_loadu_si128((const __m128i*)compress_dwords_utf8_lookup[pattern]);
                const __m128i utf8    = _mm_shuffle_epi8(dword_hi, lookup);

                _mm_storeu_si128((__m128i*)output, utf8);
                output += compress_dwords_utf8_length[pattern];
            }
        }
    }

    // process tail
    return output - start;
}
