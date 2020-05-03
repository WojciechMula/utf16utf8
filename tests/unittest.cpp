#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "reference.h"

#include "sse.h"
#include "random_utf16.h"


void dump(const std::string& s) {
    for (size_t i=0; i < s.size(); i++)
        printf(" %02x", static_cast<uint8_t>(s[i]));
}

bool compare_strings(const std::string& reference, const std::string& result) {
    const size_t reference_size = reference.size();
    const size_t result_size = result.size();
    if (reference_size != result_size) {
        printf("Different lengths: %lu != %lu\n", reference_size, result_size);
        printf("reference = "); dump(reference); putchar('\n');
        printf("result    = "); dump(result); putchar('\n');
        return false;
    }

    for (size_t i=0; i < reference_size; i++) {
        if (reference[i] != result[i]) {
            printf("Difference at %lu\n", i);
            printf("reference = "); dump(reference); putchar('\n');
            printf("result    = "); dump(result); putchar('\n');
            return false;
        }
    }

    return true;
}

bool validate(const std::vector<uint16_t>& input, size_t size) {
    // reference
    std::string tmp;
    tmp.resize(4 * size);
    utf16_to_utf8(input.data(), (uint8_t*)tmp.data());
    const std::string reference{tmp.c_str()};

    // SSE
    tmp.assign("");
    tmp.resize(8 * size);
    const size_t output_size = sse_convert_utf16_to_utf8(input.data(), size, (uint8_t*)tmp.data());
    const std::string sse{tmp.data(), output_size};

    return compare_strings(reference, sse);
}

bool validate_no_surrogates() {
    puts("Test transcoding random string (without surrogates)");
    std::random_device rd{};
    RandomUTF16 generator(rd,
        /* 1 byte */  1,
        /* 2 bytes */ 1,
        /* 3 bytes */ 1,
        /* 4 bytes */ 0
    );

    const auto UTF16 = generator.generate(512);
    const size_t size = UTF16.size() - 1;

    return validate(UTF16, size);
}

bool validate_surrogates() {
    puts("Test transcoding random string (only surrogates)");
    std::random_device rd{};
    RandomUTF16 generator(rd,
        /* 1 byte */  0,
        /* 2 bytes */ 0,
        /* 3 bytes */ 0,
        /* 4 bytes */ 1
    );

    const long seed = 1;
    const auto UTF16 = generator.generate(512, seed);
    const size_t size = UTF16.size() - 1;

    return validate(UTF16, size);
}

bool validate_all() {
    puts("Validate all input values");
    std::vector<uint16_t> input;
    input.resize(8 + 1);

    for (uint16_t value=1; value <= 0x0800; value++) {
        if (value >= 0xd800 && value <= 0xdfff) // skip reserved values
            continue;

        for (int i=0; i < 8; i++)
            input[i] = value;

        if (!validate(input, 8)) {
            printf("Failed for %04x\n", value);
            return false;
        }
    }

    return true;
}

  #define UNUSED __attribute__((unused))

bool utf16decoding_ascii() {
    const uint8_t * ascii = (const uint8_t *)"abcdefghijklmanfdfdsfdsdsfsfds12";
    uint16_t  utf16data[1024]{};
    size_t utf16len = strlen_utf8_to_utf16(ascii);
    utf8_to_utf16(ascii, utf16data);
    uint8_t  newascii[1024];

    UNUSED size_t asciilen = sse_convert_utf16_to_utf8_lemire(utf16data, utf16len, newascii);
    if (strcmp((const char *)ascii, (const char *)newascii) != 0) {
        printf("bug!\n");
        printf("%s\n", newascii);
        return false;
    } 
    return true;
}


bool utf16decoding_latin() {
    const uint8_t * latin = (const uint8_t *)u8"abcdéfghijklmanfdfdsfêsdsfsfds12";
    uint16_t  utf16data[1024]{};
    size_t utf16len = strlen_utf8_to_utf16(latin);
    utf8_to_utf16(latin, utf16data);
    uint8_t  newlatin[1024];
    UNUSED size_t asciilen = sse_convert_utf16_to_utf8_lemire(utf16data, utf16len, newlatin);
    if (strcmp((const char *)latin, (const char *)newlatin) != 0) {
        printf("bug!\n");
        printf("%s\n", newlatin);
        return false;
    } 
    return true;

}
bool utf16decoding_threebytes() {
    const uint16_t * utf16data = (const uint16_t *) u"₼ - ₼ééâ ";
    const uint8_t * utf8data = (const uint8_t *) u8"₼ - ₼ééâ ";
    size_t utf16len = strlen_utf16_to_utf8(utf16data);
    uint8_t  newutf8[1024];


    UNUSED size_t utf8len = sse_convert_utf16_to_utf8_lemire(utf16data, utf16len, newutf8);
    if (strcmp((const char *)newutf8, (const char *)utf8data) != 0) {
        printf("bug!\n");
        printf("%s\n", newutf8);
        return false;
    }
    return true;
}


int main() {
    if (!utf16decoding_ascii())
        return EXIT_FAILURE;
    if (!utf16decoding_latin())
        return EXIT_FAILURE;
    if (!utf16decoding_threebytes())
        return EXIT_FAILURE;

    /*if (!validate_all())
        return EXIT_FAILURE;

    if (!validate_no_surrogates())
        return EXIT_FAILURE;

    if (!validate_surrogates())
        return EXIT_FAILURE;

    puts("All OK");
    return EXIT_SUCCESS;*/
}

