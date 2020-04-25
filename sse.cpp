#include <cstdint>
#include <cstddef>
#include <cstring>
#include <immintrin.h>

#include "scalar.h"
#include "sse_lookup.cpp"
#include "sse_dword_lookup.cpp"

namespace nonstd {

    static __m128i _mm_or_si128(const __m128i v1, const __m128i v2, const __m128i v3) {
        return ::_mm_or_si128(v1, ::_mm_or_si128(v2, v3));
    }

}

size_t sse_convert_utf16_to_utf8(const uint16_t* input, size_t size, uint8_t* output) {
    const uint16_t* end = input + (size & ~0x7); // round down size to 8
    uint8_t* start = output;
    while (input != end) {
        const __m128i in = _mm_loadu_si128((__m128i*)input);

        // 0. test if there are any surrogates
        const __m128i msn = _mm_and_si128(in, _mm_set1_epi16(int16_t(0xf800)));
        const __m128i hi_surrogates = _mm_cmpeq_epi16(msn, _mm_set1_epi16(int16_t(0xd800))); // higher byte: 0b1101_10xx
        const __m128i lo_surrogates = _mm_cmpeq_epi16(msn, _mm_set1_epi16(int16_t(0xdc00))); // higher byte: 0b1101_11xx
        const __m128i any_surrogate = _mm_or_si128(lo_surrogates, hi_surrogates);
        const uint16_t surrogates_mask = _mm_movemask_epi8(any_surrogate);
        if (surrogates_mask) {
            // for now only scalar fallback   
            auto save_utf8 = [&output](uint32_t value) {
                auto save_bytes = [&output](uint8_t byte) { *output = byte; };
                output += encode_utf8(value, save_bytes);
            };

            bool malformed = false;
            int consumed = 8;
            auto on_error = [&consumed, &malformed](const uint16_t* data,
                                                    const uint16_t* current,
                                                    UTF16_Error error)
            {
                const auto error_pos = (current - data);
                if (error == UTF16_Error::missing_low_surrogate and error_pos == 7)
                    consumed = 7; // hi surrogate at the and of 8-byte block, would reprocess it again
                else
                    malformed = true;
            };

            decode_utf16(input, 8, save_utf8, on_error);
            if (malformed)
                return output - start;
            else
                input += consumed;
        } else 
            // if no surrogates: we for sure process whole register
            input += 8;

        // 0. determine how many bytes each 16-bit value produces
        //      1 byte  =     (in < 0x0080)
        //      2 bytes = not (in < 0x0080) and (in < 0x0800)
        //      3 bytes = not (in < 0x0800)
        const __m128i lt0080 = _mm_cmpeq_epi16(_mm_setzero_si128(),
                                               _mm_and_si128(in, _mm_set1_epi16((int16_t)0xff80)));
        const __m128i lt0800 = _mm_cmpeq_epi16(_mm_setzero_si128(),
                                               _mm_and_si128(in, _mm_set1_epi16((int64_t)0xf800)));

        // a. store lt0080 and lt0800 as bitmask, interleaving bits from both vectors
        const __m128i t0 = _mm_blendv_epi8(lt0080, lt0800, _mm_set1_epi16((int16_t)0xff00));
        const uint16_t patterns = _mm_movemask_epi8(t0);

        if (patterns == 0xffff) { // condition: (in < 0x0080)
            // Fast path: only ASCII values

            // [0000|0000|0ccc|dddd] => [0ccc|dddd]
            const __m128i t0 = _mm_packus_epi16(in, in);
            uint64_t tmp = _mm_cvtsi128_si64(t0);
            memcpy(output, &tmp, 8);
            output += 8;
        }
        else if ((patterns & 0xaaaa) == 0xaaaa) { // condition: (in < 0x0800)
            // Fast path: values are in range 0x0000 ... 0x07ff
            // UTF16 codeword is valid and is expanded to either 1 or 2 bytes of UTF8

            // a. for values 00 .. 7f we have transformation (two UTF16 bytes -> one UTF8 byte):
            //    [0000|0000|0ccc|dddd] => [0ccc|dddd]
            // b. for value  0080 .. 07ff we have (two UTF16 bytes -> two UTF8 bytes)
            //    [0000|0bbb|cccc|dddd] => [110b|bbcc|10cc|dddd]

            // tmp      = [0g0h|0i0j|0k0l|0m0n]
            // pattern  =           [gkhl|imjn]
            const uint16_t tmp     = patterns & 0x5555;
            const uint8_t  pattern = (tmp | (tmp >> 7));

            // [0000|0000|0ccc|dddd]
            const __m128i utf8_1byte = in;

            __m128i byte0;
            __m128i byte1;
            __m128i word0;

            byte0 = _mm_and_si128(in, _mm_set1_epi16(0x003f));                  // [0000|0000|00cc|dddd]
            byte1 = _mm_slli_epi16(in, 2);                                      // [000b|bbcc|xxxx|xxxx]
            byte1 = _mm_and_si128(byte1, _mm_set1_epi16((int16_t)0x1f00));      // [000b|bbcc|0000|0000]

            word0 = _mm_or_si128(byte0, byte1);                                 // [000b|bbcc|00cc|dddd]

            // update UTF8 markers:
            __m128i utf8_2bytes;
            utf8_2bytes = _mm_or_si128(word0, _mm_set1_epi16((int16_t)0xc080)); // [110b|bbcc|10cc|dddd]

            // keep in 16-bits proper UTF8 variants
            const __m128i utf8_t0 = _mm_blendv_epi8(utf8_2bytes, utf8_1byte, lt0080);

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

            // a. 1 byte code equals to input
            __m128i word0_1 = in;

            // a. build 2 byte codes
            __m128i byte0_2;
            __m128i byte1_2;
            __m128i word0_2;
                                                                        // [0000|0bbb|cccc|dddd]
            byte0_2 = _mm_and_si128(in, _mm_set1_epi16(0x003f));        // [0000|0000|00cc|dddd]

            byte1_2 = _mm_and_si128(in, _mm_set1_epi16(0x07c0));        // [0000|0bbb|cc00|0000]
            byte1_2 = _mm_slli_epi32(byte1_2, 2);                       // [000b|bbcc|0000|0000]

            const __m128i t0 = _mm_set1_epi16((int16_t)0xc080);
            word0_2 = _mm_or_si128(byte0_2, byte1_2);                   // [000b|bbcc|00cc|dddd]
            word0_2 = _mm_or_si128(word0_2, t0);                        // [110b|bbcc|10cc|dddd]

            // b. build 3 byte codes
            __m128i byte0_3;
            __m128i byte1_3;
            __m128i byte2_3;
            __m128i word0_3;
            __m128i word1_3;

            byte0_3 = byte0_2; // reuse from 2-byte case                // [0000|0000|00cc|dddd]

                                                                        // [aaaa|bbbb|cccc|dddd]
            byte1_3 = _mm_and_si128(in, _mm_set1_epi16(0x0fc0));        // [0000|bbbb|cc00|0000]
            byte1_3 = _mm_slli_epi32(byte1_3, 2);                       // [00bb|bbcc|0000|0000]

            const __m128i t1 = _mm_set1_epi16((int16_t)0x8080);
            word0_3 = _mm_or_si128(byte0_3, byte1_3);                   // [00bb|bbcc|00cc|dddd]
            word0_3 = _mm_or_si128(word0_3, t1);                        // [10bb|bbcc|10cc|dddd]

            byte2_3 = _mm_srli_epi16(in, 12);
            byte2_3 = _mm_or_si128(byte2_3, _mm_set1_epi16(0x00e0));    // [0000|0000|1110|aaaa]
            word1_3 = byte2_3;

            word0_1 = _mm_and_si128(lt0080, word0_1);

            const __m128i m2 = _mm_andnot_si128(lt0080, lt0800);
            word0_2 = _mm_and_si128(m2, word0_2);

            word0_3 = _mm_andnot_si128(lt0800, word0_3);
            word1_3 = _mm_andnot_si128(lt0800, word1_3);

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
