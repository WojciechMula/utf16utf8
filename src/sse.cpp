#include "sse.h"

#include <cstring>
#include <immintrin.h>

#include "scalar_utf8.h"
#include "scalar_utf16.h"
#include "sse_16bit_lookup.cpp"
#include "sse_32bit_lookup.cpp"
#include "sse_utf8_to_utf16.cpp"

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


/**
 * Todo: 
 *   - deal with BOM, LE/BE
 */




size_t sse_convert_utf16_to_utf8_hybrid(const uint16_t *input, size_t size,
                                        uint8_t *output) {

  const uint16_t *end = input + (size & ~0x7); // round down size to 8
  uint8_t *start = output;
  while (input != end) {
    const __m128i in = _mm_loadu_si128((__m128i *)input);
    // 1. test for ASCII

    const __m128i lt0080 =
        _mm_cmpeq_epi16(_mm_setzero_si128(),
                        _mm_and_si128(in, _mm_set1_epi16((int16_t)0xff80)));
    uint16_t ascii_patterns = uint16_t(_mm_movemask_epi8(lt0080));

    if (ascii_patterns == 0xFFFF) {
      // Fast path: only ASCII values

      // [0000|0000|0ccc|dddd] => [0ccc|dddd]
      const __m128i lt0 = _mm_packus_epi16(in, in);
      uint64_t tmp = _mm_cvtsi128_si64(lt0);
      memcpy(output, &tmp, 8);
      output += 8;
      input += 8;
      continue;
    }

    ascii_patterns &= 0x5555;
       
    // 0. determine how many bytes each 16-bit value produces
    //      1 byte  =     (in < 0x0080)
    //      2 bytes = not (in < 0x0080) and (in < 0x0800)
    //      3 bytes = not (in < 0x0800)

    const __m128i lt0800 =
        _mm_cmpeq_epi16(_mm_setzero_si128(),
                        _mm_and_si128(in, _mm_set1_epi16((int16_t)0xf800)));


    const uint16_t twobyte_patterns = uint16_t(_mm_movemask_epi8(lt0800));
    // 2. test for two-byte
    if (twobyte_patterns == 0xFFFF) {

      // twobyte_patterns
      // Fast path: values are in range 0x0000 ... 0x07ff
      // UTF16 codeword is valid and is expanded to either 1 or 2 bytes of UTF8

      // a. for values 00 .. 7f we have transformation (two UTF16 bytes -> one
      // UTF8 byte):
      //    [0000|0000|0ccc|dddd] => [0ccc|dddd]
      // b. for value  0080 .. 07ff we have (two UTF16 bytes -> two UTF8 bytes)
      //    [0000|0bbb|cccc|dddd] => [110b|bbcc|10cc|dddd]

      // tmp      = [0g0h|0i0j|0k0l|0m0n]
      // pattern  =           [gkhl|imjn]

      const uint8_t pattern = uint8_t(ascii_patterns | (ascii_patterns >> 7));

      // [0000|0000|0ccc|dddd]
      const __m128i utf8_1byte = in;

      __m128i byte0;
      __m128i byte1;
      __m128i word0;

      byte0 =
          _mm_and_si128(in, _mm_set1_epi16(0x003f)); // [0000|0000|00cc|dddd]
      byte1 = _mm_slli_epi16(in, 2);                 // [000b|bbcc|xxxx|xxxx]
      byte1 = _mm_and_si128(
          byte1, _mm_set1_epi16((int16_t)0x1f00)); // [000b|bbcc|0000|0000]

      word0 = _mm_or_si128(byte0, byte1); // [000b|bbcc|00cc|dddd]

      // update UTF8 markers:
      __m128i utf8_2bytes;
      utf8_2bytes = _mm_or_si128(
          word0, _mm_set1_epi16((int16_t)0xc080)); // [110b|bbcc|10cc|dddd]

      // keep in 16-bits proper UTF8 variants
      const __m128i utf8_t0 = _mm_blendv_epi8(utf8_2bytes, utf8_1byte, lt0080);

      // compress zeros from 1-byte words
      const __m128i lookup =
          _mm_loadu_si128((const __m128i *)compress_16bit_lookup[pattern]);
      const __m128i utf8 = _mm_shuffle_epi8(utf8_t0, lookup);

      _mm_storeu_si128((__m128i *)output, utf8);
      output += compress_16bit_length[pattern];
      input += 8;
      continue;
    }

    // 3. test if there are any surrogates

    __m128i basicplane =
        _mm_cmplt_epi16(_mm_sub_epi16(in, _mm_set1_epi16(uint16_t(0x5800))),
                        _mm_set1_epi16(uint16_t(0x8800)));

    if (_mm_movemask_epi8(basicplane)) {

      // for now only scalar fallback
      auto save_utf8 = [&output](uint32_t value) {
        auto save_bytes = [&output](int byte) { *output++ = uint8_t(byte); };
        encode_utf8(value, save_bytes);
      };

      bool malformed = false;
      int consumed = 8;
      auto on_error = [&consumed, &malformed](const uint16_t *data,
                                              const uint16_t *current,
                                              utf16::Error error) {
        const auto error_pos = (current - data);
        if (error == utf16::Error::missing_low_surrogate and error_pos == 7)
          consumed = 7; // hi surrogate at the and of 8-byte block, would
                        // reprocess it again
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
    // 4. finally, three-byte pattern

    {
      const uint16_t patterns = (ascii_patterns) | (twobyte_patterns & 0xaaaa);

      // input in range 0x0000 .. 0xffff

      // 1. prepare UTF8 bytes

      // output 1 UTF8 byte  : [0000|0000|0ccc|dddd] => [0ccc|dddd]
      // output 2 UTF8 bytes : [0000|0bbb|cccc|dddd] => [110b|bbcc], [10cc|dddd]
      // output 3 UTF8 bytes : [aaaa|bbbb|cccc|dddd] => [1110|aaaa],
      // [10bb|bbcc], [10cc|dddd]

      // a. 1 byte code equals to input
      __m128i word0_1 = in;

      // a. build 2 byte codes
      __m128i tmp;
      __m128i byte0_2;
      __m128i byte1_2;
      __m128i word0_2;

      // [0000|0bbb|cccc|dddd]
      byte0_2 =
          _mm_and_si128(in, _mm_set1_epi16(0x003f)); // [0000|0000|00cc|dddd]

      tmp = _mm_slli_epi32(in, 2); // [000b|bbcc|ccdd|dd00] // reused later
      byte1_2 =
          _mm_and_si128(tmp, _mm_set1_epi16(0x1f00)); // [000b|bbcc|0000|0000]

      const __m128i loct0 = _mm_set1_epi16((int16_t)0xc080);
      word0_2 = _mm_or_si128(byte0_2, byte1_2); // [000b|bbcc|00cc|dddd]
      word0_2 = _mm_or_si128(word0_2, loct0);   // [110b|bbcc|10cc|dddd]

      // b. build 3 byte codes
      __m128i byte0_3;
      __m128i byte1_3;
      __m128i byte2_3;
      __m128i word0_3;
      __m128i word1_3;

      byte0_3 = byte0_2; // reuse from 2-byte case                //
                         // [0000|0000|00cc|dddd]

      byte1_3 =
          _mm_and_si128(tmp, _mm_set1_epi16(0x3f00)); // [00bb|bbcc|0000|0000]

      const __m128i t1 = _mm_set1_epi16((int16_t)0x8080);
      word0_3 = _mm_or_si128(byte0_3, byte1_3); // [00bb|bbcc|00cc|dddd]
      word0_3 = _mm_or_si128(word0_3, t1);      // [10bb|bbcc|10cc|dddd]

      byte2_3 = _mm_srli_epi16(in, 12);
      byte2_3 = _mm_or_si128(byte2_3,
                             _mm_set1_epi16(0x00e0)); // [0000|0000|1110|aaaa]
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
        const __m128i lookup =
            _mm_loadu_si128((const __m128i *)compress_32bit_lookup[pattern]);
        const __m128i utf8 = _mm_shuffle_epi8(dword_lo, lookup);

        _mm_storeu_si128((__m128i *)output, utf8);
        output += compress_32bit_length[pattern];
      }

      // b. compress hi dwords
      {
        const uint8_t pattern = uint8_t(patterns >> 8);
        const __m128i lookup =
            _mm_loadu_si128((const __m128i *)compress_32bit_lookup[pattern]);
        const __m128i utf8 = _mm_shuffle_epi8(dword_hi, lookup);

        _mm_storeu_si128((__m128i *)output, utf8);
        output += compress_32bit_length[pattern];
      }
      input += 8;
    }
  }

  // process tail
  return output - start;
}

// This method asserts that utf8 input is valid
size_t sse_convert_valid_utf8_to_utf16_wmu(const uint8_t* input, size_t size, uint16_t* output) {
    
    const uint8_t* end = input + size - 8;
    const __m128i zero = _mm_setzero_si128();

    while (input < end) {
        const __m128i in = _mm_loadu_si128((__m128i*)input);

        // 0. gather MSB (7th bits)
        const uint16_t input_bit7 = uint16_t(_mm_movemask_epi8(in));

        if (input_bit7 == 0) {
            // fast path: ASCII
            _mm_storeu_si128((__m128i*)(output + 0), _mm_unpacklo_epi8(in, zero));
            _mm_storeu_si128((__m128i*)(output + 4), _mm_unpackhi_epi8(in, zero));
            input += 8;
            output += 8;
            continue;
        }

        // 1. gather 6th bits
        const uint16_t input_bit6 = uint16_t(_mm_movemask_epi8(_mm_add_epi8(in, in)));

        // 2. now use lower helves of these two masks (input_bit{6,7}) to get proper data
        const uint16_t index = uint16_t(input_bit7 << 8) | uint16_t(input_bit6 & 0x00ff);

        using namespace utf8_to_utf16;
        const Parameters& params = parameters[lookup[index]];
        if (params.char_lengths == (has_1byte_chars | has_2byte_chars)) {
            // do something
        } else
        if (params.char_lengths == (has_1byte_chars | has_2byte_chars | has_3byte_chars)) {
            // do something else
        } else {
            // scalar fallback
        }
        
       

    }
}

namespace {
#include "reference.h"


static size_t strlen_utf8_to_utf16_with_length(const uint8_t *str, size_t len)
{
	size_t i, count;
	uint32_t c;

	for (i=0, count=0;i < len; i++)
	{

		c = utf8_to_unicode32(&str[i], &i);
		count += codepoint_utf16_size(c);
	}
	return count;
}



void utf8_to_utf16_with_length(const uint8_t *utf8, size_t len, uint16_t *utf16)
{
	int j;
	uint32_t c;
    size_t i;


	for (i=0, j=0, c=1; i < len; i++)
	{
		c = utf8_to_unicode32(&utf8[i], &i);
		sprint_utf16(&utf16[j], c);
		j += codepoint_utf16_size(c);
	}
}
}
#include <stdio.h>

size_t sse_convert_valid_utf8_to_utf16_lemire(const uint8_t* input, size_t size, uint16_t* output) {

  size_t pos = 0;
  uint16_t *start = output;
  while (pos + 16 <= size) {
    //////////////////////////////
    // Can go faster if we grab large
    // blocks of data, larger than
    // 16 bytes.
    //////////////////////////////
    const __m128i in = _mm_loadu_si128((__m128i *)(input + pos));
    const uint16_t non_ascii_chars = uint16_t(_mm_movemask_epi8(in));
    // ASCII is likely common in many cases, we want a fast path.
    if(non_ascii_chars == 0) {
        // could use _mm_cvtepi8_epi16
        const __m128i out1 = _mm_unpacklo_epi8(in, _mm_setzero_si128());// order of parameter determines endianness
        _mm_storeu_si128((__m128i*)output, out1);
        output += 8;
        const __m128i out2 = _mm_unpackhi_epi8(in, _mm_setzero_si128());
        _mm_storeu_si128((__m128i*)output, out2);
        output += 8;
        pos += 16;
        continue;
    }
    const __m128i maxtwobyte = _mm_set1_epi8(uint8_t(0xdf));
    uint16_t istwobytes = uint16_t(_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_max_epu8(maxtwobyte, in), maxtwobyte)));
    // The most significant 4 bits (high nibble) can be used to classify the bytes, as
    // to whether they start a new code point.
    const __m128i hn = _mm_set1_epi8(uint8_t(0xF));
    const __m128i in_high_nibbles = _mm_and_si128(hn, _mm_srli_epi16(in, 4));
    //const __m128i masks = _mm_shuffle_epi8(_mm_set_epi8(0xf0, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0), in_high_nibbles);
    const uint16_t start_of_chars = uint16_t(_mm_movemask_epi8(_mm_shuffle_epi8(_mm_set_epi8(0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff), in_high_nibbles)));
    // We want to spot the easy two-byte scenario, it is easy! Just 
    // do a little comparison.
    if(istwobytes == 0xFFFF) {
        // this is a relatively easy scenario
    }



	// combine index and bytes consumed into a single lookup
//	index_bytes_consumed combined = combined_lookup[start_of_chars];
//	uint64_t consumed = combined.bytes_consumed;
//	uint8_t index = combined.index;

//	__m128i shuffle_vector = vectors[index];
    /// Then we must detect the case where all UTF8 code points
    /// span at most two bytes.
      //  const __m128i bytes_to_decode = _mm_shuffle_epi8(shuffle_vector,in);
	//	__m128i low_bytes = _mm_and_si128(bytes_to_decode,
	//			_mm_set1_epi16(0x007F));
	//	__m128i high_bytes = _mm_and_si128(bytes_to_decode,
	//			_mm_set1_epi16(0x1F00));//110xxxxx
	//	__m128i high_bytes_shifted = _mm_srli_epi16(high_bytes, 2);
  //      __m128i packed_result = _mm_or_si128(low_bytes, high_bytes_shifted);

    /// Then we must detect the case where all UTF8 code points
    /// span at most three bytes.
//put the location of the third byte in the high bits of shuffle_vector
    /// Finally, we have to deal  with 4-byte cases.
  }
  size_t len = strlen_utf8_to_utf16_with_length(input + pos, size - pos);
  utf8_to_utf16_with_length(input + pos, size - pos, output);
  output += len;
  return output - start;
}

