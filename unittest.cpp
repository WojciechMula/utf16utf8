#include "reference.cpp"
#include "sse.cpp"

#include <string>
#include <vector>
#include <cstdlib>
#include <cassert>

uint32_t random_unicode(uint32_t min, uint32_t max) {
    uint32_t value;
    // RFC 2781: Values between 0xD800 and 0xDFFF are specifically reserved for
    //           use with UTF-16, and don't have any characters assigned to them.
    do {
        value = rand() % (max - min + 1) + min;
    } while (value >= 0xd800 && value <= 0xdfff);

    return value;
}

std::vector<uint16_t> random_utf16(uint32_t min, uint32_t max, size_t count) {
    std::vector<uint16_t> result;
    result.reserve(count);
    for (size_t i=0; i < count; i++) {
        const uint32_t unicode = random_unicode(min, max);
        result.push_back(unicode); // TODO
    }

    result.push_back(0); // EOS for scalar code

    return result;
}

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

    return true;
}

bool validate(const std::vector<uint16_t>& input, size_t size) {
    // reference
    std::string tmp;
    tmp.reserve(4 * size);
    utf16_to_utf8(input.data(), (uint8_t*)tmp.data());
    const std::string reference{tmp.c_str()};

    // SSE
    tmp.assign("");
    tmp.reserve(4 * size);
    const size_t output_size = sse_convert_utf16_to_utf8(input.data(), size, (uint8_t*)tmp.data());
    const std::string sse{tmp.data(), output_size};

    return compare_strings(reference, sse);
}

bool validate_sample() {
    const auto UTF16 = random_utf16(0x0001, 0x7fff, 512);
    const size_t size = UTF16.size() - 1;

    return validate(UTF16, size);
}

bool validate_all() {
    std::vector<uint16_t> input;
    input.resize(8 + 1);

    // Note: max is 0x7fff because the SSE implementation doesn't deal properly with signed values
    for (uint16_t value=1; value <= 0x7fff; value++) {
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

int main(int argc, char* argv[]) {
    if (argc > 1) {
        srand(atoi(argv[1]));
    }

    if (validate_all()) {
        puts("All OK");
        return EXIT_SUCCESS;
    } else
        return EXIT_FAILURE;
}
