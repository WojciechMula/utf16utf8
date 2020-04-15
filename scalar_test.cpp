#include "scalar.h"
#include <cstdio>
#include <stdexcept>

void consumer(uint32_t codepoint) {
    printf("%04x [%c]\n", codepoint, codepoint);
}

void error_handler(const uint16_t* data, const uint16_t* errorpos) {
    printf("input error at %lu", errorpos - data);
    throw std::runtime_error("UTF16 stream is malformed");
}

int main() {
    const uint16_t test[] = {'k', 'i', 't', 't', 'e', 'n'};
    decode_utf16(test, 6, consumer, error_handler);
}
