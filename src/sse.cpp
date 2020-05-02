#include "sse.h"

#include <cstring>
#include <immintrin.h>

#include "scalar_utf8.h"
#include "scalar_utf16.h"
#include "sse_16bit_lookup.cpp"
#include "sse_32bit_lookup.cpp"
#include "sse_utf16_to_utf8_simple.cpp"

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
        const uint16_t surrogates_mask = uint16_t(_mm_movemask_epi8(any_surrogate));
        if (surrogates_mask) {
            // for now only scalar fallback   
            auto save_utf8 = [&output](uint32_t value) {
                auto save_bytes = [&output](int byte) { *output++ = uint8_t(byte); };
                encode_utf8(value, save_bytes);
            };

            bool malformed = false;
            int consumed = 8;
            auto on_error = [&consumed, &malformed](const uint16_t* data,
                                                    const uint16_t* current,
                                                    utf16::Error error)
            {
                const auto error_pos = (current - data);
                if (error == utf16::Error::missing_low_surrogate and error_pos == 7)
                    consumed = 7; // hi surrogate at the and of 8-byte block, would reprocess it again
                else
                    malformed = true;
            };

            utf16::decode(input, 8, save_utf8, on_error);
            if (malformed)
                return output - start;
            else
                input += consumed;

            continue;
        }

        // if no surrogates: we for sure process whole register
        input += 8;

        // 0. determine how many bytes each 16-bit value produces
        //      1 byte  =     (in < 0x0080)
        //      2 bytes = not (in < 0x0080) and (in < 0x0800)
        //      3 bytes = not (in < 0x0800)
        const __m128i lt0080 = _mm_cmpeq_epi16(_mm_setzero_si128(),
                                               _mm_and_si128(in, _mm_set1_epi16((int16_t)0xff80)));
        const __m128i lt0800 = _mm_cmpeq_epi16(_mm_setzero_si128(),
                                               _mm_and_si128(in, _mm_set1_epi16((int16_t)0xf800)));

        // a. store lt0080 and lt0800 as bitmask, interleaving bits from both vectors
        const __m128i t0 = _mm_blendv_epi8(lt0080, lt0800, _mm_set1_epi16((int16_t)0xff00));
        const uint16_t patterns = uint16_t(_mm_movemask_epi8(t0));

        if (patterns == 0xffff) { // condition: (in < 0x0080)
            // Fast path: only ASCII values

            // [0000|0000|0ccc|dddd] => [0ccc|dddd]
            const __m128i lt0 = _mm_packus_epi16(in, in);
            uint64_t tmp = _mm_cvtsi128_si64(lt0);
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
            const uint8_t  pattern = uint8_t(tmp | (tmp >> 7));

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
            const __m128i lookup = _mm_loadu_si128((const __m128i*)compress_16bit_lookup[pattern]);
            const __m128i utf8   = _mm_shuffle_epi8(utf8_t0, lookup);

            _mm_storeu_si128((__m128i*)output, utf8);
            output += compress_16bit_length[pattern];
        } else {
            // input in range 0x0000 .. 0xffff

            // 1. prepare UTF8 bytes

            // output 1 UTF8 byte  : [0000|0000|0ccc|dddd] => [0ccc|dddd]
            // output 2 UTF8 bytes : [0000|0bbb|cccc|dddd] => [110b|bbcc], [10cc|dddd]
            // output 3 UTF8 bytes : [aaaa|bbbb|cccc|dddd] => [1110|aaaa], [10bb|bbcc], [10cc|dddd]

            // a. 1 byte code equals to input
            __m128i word0_1 = in;

            // a. build 2 byte codes
            __m128i tmp;
            __m128i byte0_2;
            __m128i byte1_2;
            __m128i word0_2;


                                                                        // [0000|0bbb|cccc|dddd]
            byte0_2 = _mm_and_si128(in, _mm_set1_epi16(0x003f));        // [0000|0000|00cc|dddd]

            tmp     = _mm_slli_epi32(in, 2);                            // [000b|bbcc|ccdd|dd00] // reused later
            byte1_2 = _mm_and_si128(tmp, _mm_set1_epi16(0x1f00));       // [000b|bbcc|0000|0000]

            const __m128i loct0 = _mm_set1_epi16((int16_t)0xc080);
            word0_2 = _mm_or_si128(byte0_2, byte1_2);                   // [000b|bbcc|00cc|dddd]
            word0_2 = _mm_or_si128(word0_2, loct0);                     // [110b|bbcc|10cc|dddd]

            // b. build 3 byte codes
            __m128i byte0_3;
            __m128i byte1_3;
            __m128i byte2_3;
            __m128i word0_3;
            __m128i word1_3;

            byte0_3 = byte0_2; // reuse from 2-byte case                // [0000|0000|00cc|dddd]

            byte1_3 = _mm_and_si128(tmp, _mm_set1_epi16(0x3f00));       // [00bb|bbcc|0000|0000]

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
                const uint8_t pattern = uint8_t(patterns & 0x00ff);
                const __m128i lookup  = _mm_loadu_si128((const __m128i*)compress_32bit_lookup[pattern]);
                const __m128i utf8    = _mm_shuffle_epi8(dword_lo, lookup);

                _mm_storeu_si128((__m128i*)output, utf8);
                output += compress_32bit_length[pattern];
            }

            // b. compress hi dwords
            {
                const uint8_t pattern = uint8_t(patterns >> 8);
                const __m128i lookup  = _mm_loadu_si128((const __m128i*)compress_32bit_lookup[pattern]);
                const __m128i utf8    = _mm_shuffle_epi8(dword_hi, lookup);

                _mm_storeu_si128((__m128i*)output, utf8);
                output += compress_32bit_length[pattern];
            }
        }
    }

    // process tail
    return output - start;
}


/***
 * For UTF-16, we have to worry about three ranges of codepoints.
 *
 * (1) U+0000 to U+D7FF 16-bit code units that are numerically equal to the corresponding code points.
 * 
 *    Mapped to one or two bytes.
 * 
 *  - One byte (ASCII) : U+0000 to U+007F
 *  - Two bytes        : U+0080 to U+07FF
 * 
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+007F     00..7F
 * U+0080..U+07FF     C2..DF   80..BF
 * 
 * (2) U+E000 to U+FFFF : 16-bit code units that are numerically equal to the corresponding code points.
 *    Mapped to three bytes.
 *   U+E000..U+FFFF     EE..EF   80..BF   80..BF
 * 
 * 
 * (3) Code points from U+010000 to U+10FFFF : coded using surrogate pairs.
 * Mapped to four bytes. This is outside of the Basic Multilingual Plane.
 *
 * U+100000..U+10FFFF F4       80..8F   80..BF   80..BF
 **********************************
 *
 * https://tools.ietf.org/html/rfc2781
 * Decoding of a single character from UTF-16 to an ISO 10646 character
 * value proceeds as follows. Let W1 be the next 16-bit integer in the
 * sequence of integers representing the text. Let W2 be the (eventual)
 * next integer following W1.
 * 
 * 1) If W1 < 0xD800 or W1 > 0xDFFF, the character value U is the value
 *    of W1. Terminate.
 * 
 * 2) Construct a 20-bit unsigned integer U', taking the 10 low-order
 *     bits of W1 as its 10 high-order bits and the 10 low-order bits of
 *     W2 as its 10 low-order bits.
 * 
 */


// used by sse_convert_utf8_to_utf16
static inline __m128i sse_convert_utf8_to_utf16_three_byte_blend(__m128i in) {
                // in is
                // LLLLLLLL HHHHHHHH 00000000 00000000
                // shifthigh1 is 
                // 00000000 00000000 LLLLLL00 HHHHHHLL
                const __m128i shifthigh1 = _mm_slli_epi16(in, 18);
                // blend
                // LLLLLLLL HHHHHHHH  LLLLLL00 HHHHHHLL
                const __m128i inshifthigh1 = _mm_blend_epi16(in, shifthigh1, 0xaaaaaaaa);
                // shifthigh2 is 
                // 00000000 0000HHHH  00000000 00000000
                const __m128i constant_hollow = _mm_set1_epi32(0x00FFFF00);
                const __m128i shifthigh2 = _mm_and_si128(constant_hollow,_mm_srli_epi16(in, 4));
                // final blend 
                // LLLLLLLL 0000HHHH  00000000  HHHHHHLL
                const __m128i blendedin = _mm_or_si128(_mm_andnot_si128(constant_hollow,inshifthigh1), shifthigh2);
                //
                // It is likely we can save a couple of instructions above...
                //
                return blendedin;
}

size_t sse_convert_utf8_to_utf16(const uint8_t* input, size_t size, uint16_t* output) {
    // todo: should specify BOM, we assume LE for now?
    // Could flip them around  with 
    // _mm_or_si128(
	//	_mm_slli_epi16(x, 8),
	//	_mm_srli_epi16(x, 8));
    // Or be otherwise smarter.
    const uint8_t* end = input + (size & ~0xf); // round down size to 16
    uint16_t* start = output;
    while (input != end) {
        const __m128i in = _mm_loadu_si128((__m128i*)input);
        // ASCII might be common enough
        // in practice, to make a special case for it.
        const __m128i packedascii = _mm_packus_epi16(in,in);
        const auto nonascii_pattern = _mm_movemask_epi8(packedascii);
        // to 
        if(nonascii_pattern == 0) {
           // easy case
           // [0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a] => [aaaaaa....]
           _mm_storeu_si128((__m128i*)output, packedascii);
           output += 8;// wrote 8 ASCII characters, we are done!!!
           continue;
        }
        // We are ok whenever the most significant byte is < 0xD8 and > 0xDF. Otherwise, surrogates.
        // SSE is limited to *signed* comparisons and though there is a _mm_cmplt_epi16 intrinsic
        // the underlying instruction is pcmpgtw. So we want to use _mm_cmpgt_epi16 as much 
        // as possible for clarity. So the trick is to move the range [0xD800, 0xDFFF] up to the lowest
        // possible signed value... which is 0x8000. 
        // Solve for x in 0xD800 - x = 0x8000, get x = 0xD800 - 0x8000. The first
        // value in the basic plane will be 0xDFFF + 1 - x = 0xDFFF + 1 - (0xD800 - 0x8000)
        // Signed integers go from 0 to 0x7f (0 to 127) and from 0x80 (-128) to 0xff (-1).


        __m128i basicplane = _mm_cmpgt_epi16(_mm_set1_epi16(uint16_t(0x8800)), // 0x8800 = 0xDFFF + 1 - (0xD800 - 0x8000))
                   _mm_sub_epi16(in, _mm_set1_epi16(0xD800 - 0x8000)));
        if(_mm_movemask_epi8(basicplane) != 0xFFFF) {
           // have fun
           // we are outside of the  Basic Multilingual Plane
        } else {
            // path with no surrogates
            // each of the 16-bit words can :
            // map to 1 output byte if <=0x7f
            // map to 2 output bytes if <= 07FF
            // or three output bytes if in the range 0xE000 0xFFFF 
            const __m128i maxtwobytes = _mm_set1_epi16(0x07FF);
            const __m128i istwobytes = _mm_cmpeq_epi16(_mm_max_epu16(in,maxtwobytes), maxtwobytes);
            const uint16_t istwobytes_pattern = uint16_t(_mm_movemask_epi8(istwobytes));
            if(istwobytes_pattern == 0xFFFF) {
                // Each of the two bytes is mapped to either one byte or two bytes, so we effectively
                // are in compression mode.

                // we need to convert LLLLLLLL 00000HHH (little endian)  into 
                // - 2 byte character (11 bits):  110HHHLL 10LLLLLL
                // 
                // first we shift left by 2 to get 
                //  LLLLLL00 000HHHLL (little endian) 
                const __m128i shifthigh = _mm_slli_epi16(in, 2);
                // Then we can blend the two together
                // to get LLLLLLLL 000HHHLL
                const __m128i constant_high_byte = _mm_set1_epi16(0xFF00);
                const __m128i blendedin = _mm_blendv_epi8(in, shifthigh, constant_high_byte);
                const __m128i shufmaskandhighbits = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(simple_compress_16bit_to_8bit_lookup[nonascii_pattern]));
                const __m128i constant_high_nibble = _mm_set1_epi8(0xF0);

                const __m128i utf8highbits =  _mm_and_si128(shufmaskandhighbits, constant_high_nibble);
                const __m128i shufmask =  _mm_andnot_si128(shufmaskandhighbits, constant_high_nibble);
                const __m128i reshuffled = _mm_shuffle_epi8(blendedin, shufmask);
                const __m128i reshuffledmasked = _mm_andnot_si128(utf8highbits, reshuffled);
                const __m128i utf8highbitsshifted = _mm_add_epi16(utf8highbits, utf8highbits);
                const __m128i finaloutput = _mm_or_si128(reshuffledmasked, utf8highbitsshifted);
                _mm_storeu_si128((__m128i*)output, finaloutput);
                output += simple_compress_16bit_to_8bit_len[nonascii_pattern];
                continue;
            } else {
                // Having fun yet?
                // This time we want to convert LLLLLLLL HHHHHHHH
                // into
                // - 3 byte character (17 bits):  1110____ (4) 10______ (6) 10______ (6)
                // - 3 byte character (17 bits):  1110HHHH (4) 10HHHHLL (6) 10LLLLLL (6)
                //
                //  Move it to four bytes LLLLLLLL HHHHHHHH
                //  LLLLLLLL HHHHHHHH 00000000 00000000
                // We can right shift by 2 to get 
                //  LLLLLL00 HHHHHHLL 000000HH 00000000
                // We can right shift by 12 to get
                // 00000000 LLLL0000 HHHHLLLL 0000HHHH
                // Blending twice we can get 
                // LLLLLLLL  HHHHHHLL  <something> 0000HHHH

                __m128i blended1 = sse_convert_utf8_to_utf16_three_byte_blend(_mm_unpacklo_epi16(in, _mm_setzero_si128()));
                __m128i blended2 = sse_convert_utf8_to_utf16_three_byte_blend(_mm_unpackhi_epi16(in, _mm_setzero_si128()));


                const __m128i shufmaskandhighbits1 = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(simple_compress_16bit_to_8bit_lookup[nonascii_pattern]));
                const __m128i shufmaskandhighbits2 = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(simple_compress_16bit_to_8bit_lookup[nonascii_pattern]));

                const __m128i utf8highbits1 =  _mm_and_si128(shufmaskandhighbits1, _mm_set1_epi16(0xF0));
                const __m128i utf8highbits2 =  _mm_and_si128(shufmaskandhighbits2, _mm_set1_epi16(0xF0));

                const __m128i shufmask1 =  _mm_and_si128(shufmaskandhighbits1, _mm_set1_epi16(0x0F));
                const __m128i shufmask2 =  _mm_and_si128(shufmaskandhighbits2, _mm_set1_epi16(0x0F));

                const __m128i reshuffled1 = _mm_shuffle_epi8(blended1, shufmask1);
                const __m128i reshuffled2 = _mm_shuffle_epi8(blended2, shufmask2);

                const __m128i reshuffledmasked1 = _mm_andnot_si128(utf8highbits1, reshuffled1);
                const __m128i reshuffledmasked2 = _mm_andnot_si128(utf8highbits2, reshuffled2);

                const __m128i utf8highbitsshifted1 = _mm_add_epi16(utf8highbits1, utf8highbits1);
                const __m128i utf8highbitsshifted2 = _mm_add_epi16(utf8highbits2, utf8highbits2);

                const __m128i finaloutput1 = _mm_or_si128(reshuffledmasked1, utf8highbitsshifted1);
                const __m128i finaloutput2 = _mm_or_si128(reshuffledmasked2, utf8highbitsshifted2);

                _mm_storeu_si128((__m128i*)output, finaloutput1);
                output += simple_compress_16bit_to_8bit_len[nonascii_pattern];

                _mm_storeu_si128((__m128i*)output, finaloutput2);
                output += simple_compress_16bit_to_8bit_len[nonascii_pattern];
                continue;

            }



        }



    }
    // process tail
    return output - start;
}