#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>

#include "reference.h"

#include "sse.h"
#include "random_utf8.h"
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

template <class T>
void display(const std::vector<T>& input) {
    for(size_t i = 0; i < input.size(); i++) {
        std::cout << input[i] << " ";
    }
    std::cout << "\n";

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

    if(! compare_strings(reference, sse) ) {
        display<uint16_t>(input);
        return false;
    }
    return true;
}


bool validate_hybrid(const std::vector<uint16_t>& input, size_t size) {
    // reference
    std::string tmp;
    tmp.resize(4 * size);
    utf16_to_utf8(input.data(), (uint8_t*)tmp.data());
    const std::string reference{tmp.c_str()};

    // SSE
    tmp.assign("");
    tmp.resize(8 * size);
    const size_t output_size = sse_convert_utf16_to_utf8_hybrid(input.data(), size, (uint8_t*)tmp.data());
    const std::string sse{tmp.data(), output_size};

    if(! compare_strings(reference, sse) ) {
        display<uint16_t>(input);
        return false;
    }
    return true;
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
    for(size_t t = 0; t < 10000; t++) {
      const auto UTF16 = generator.generate(512);
      const size_t size = UTF16.size() - 1;
      if(!validate(UTF16, size)) {
          return false;
      }
      if(!validate_hybrid(UTF16, size)) {
          return false;
      }
    }
    return true;
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
    for(size_t t = 0; t < 10000; t++) {
      const long seed = 1;
      const auto UTF16 = generator.generate(512, seed);
      const size_t size = UTF16.size() - 1;

      return validate(UTF16, size) && validate_hybrid(UTF16, size);
    }
    return true;
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
        if (!validate_hybrid(input, 8)) {
            printf("Failed for %04x\n", value);
            return false;
        }
    }

    return true;
}


bool validate_from_utf8(const std::vector<uint8_t>& input) {
    size_t actual_length = input.size();
    for(size_t i = 0; i < input.size(); i++) {
        if(input[i] ==0) {
            actual_length = i;
            break;
        }
    }
    // reference
    std::vector<uint16_t> tmp;
    tmp.resize(4 * input.size());
    size_t len = strlen_utf8_to_utf16(input.data());
    utf8_to_utf16(input.data(), (uint16_t*)tmp.data());
    const std::vector<uint16_t> reference(tmp.data(), tmp.data() + len);

    // SSE
    tmp.clear();
    tmp.resize(8 * input.size());
    const size_t output_size = sse_convert_valid_utf8_to_utf16_lemire(input.data(), actual_length, (uint16_t*)tmp.data());
    const std::vector<uint16_t> sse(tmp.data(), tmp.data() + output_size);

    if( sse != reference ) {
        printf("input:");display<uint8_t>(input);
        printf("ref:");display<uint16_t>(reference);
        printf("sse:");display<uint16_t>(sse);
        printf("BUG\n");
        return false;
    }
    return true;
}
bool validate_no_surrogates_from_utf8() {
    puts("Test transcoding random string from utf8 (without surrogates)");
    std::random_device rd{};
    RandomUTF8 generator(rd,
        /* 1 byte */  1,
        /* 2 bytes */ 1, //1,
        /* 3 bytes */ 0,//1,
        /* 4 bytes */ 0
    );
    for(size_t t = 0; t < 10000; t++) {
      const auto UTF8 = generator.generate(16);
      if(!validate_from_utf8(UTF8)) {
          return false;
      }
    }
    return true;
}
int main() {

    if(!validate_no_surrogates_from_utf8())
        return EXIT_FAILURE;

    if (!validate_all())
        return EXIT_FAILURE;

    if (!validate_no_surrogates())
        return EXIT_FAILURE;

    if (!validate_surrogates())
        return EXIT_FAILURE;

    puts("All OK");
    return EXIT_SUCCESS;
}

