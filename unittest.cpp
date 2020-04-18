#include "reference.cpp"
#include "sse.cpp"

#include <string>
#include <vector>
#include <cstdlib>
#include <cassert>

uint32_t random_unicode(uint32_t min, uint32_t max) {
    return rand() % (max - min + 1) + min;
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

    puts("OK");
    return true;
}

int main() {
    const auto UTF16 = random_utf16(0x0001, 0x07ff, 256);
    const size_t size = UTF16.size() - 1;

    // reference
    std::string tmp;
    tmp.reserve(4 * size);
    utf16_to_utf8(UTF16.data(), (uint8_t*)tmp.data());
    const std::string reference{tmp.c_str()};

    // SSE
    tmp.assign("");
    tmp.reserve(4 * size);
    const size_t output_size = sse_convert_utf16_to_utf8(UTF16.data(), size, (uint8_t*)tmp.data());
    const std::string sse{tmp.data(), output_size};

    return compare_strings(reference, sse) ? 0 : 1;
}
