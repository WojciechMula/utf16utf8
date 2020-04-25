#include "random_utf16.h"
#include "scalar_utf16.h"

RandomUTF16::RandomUTF16(std::random_device& rd, int prob_1byte, int prob_2bytes, int prob_3bytes, int prob_4bytes)
    : gen(rd())
    , utf8_length({2*prob_1byte,
                   2*prob_2bytes,
                     prob_3bytes,
                     prob_3bytes,
                   2*prob_4bytes}) {}

uint32_t RandomUTF16::generate() {
    switch (utf8_length(gen)) {
        case 0: return val_1byte(gen);
        case 1: return val_2bytes(gen);
        case 2: return val_3bytes_a(gen);
        case 3: return val_3bytes_b(gen);
        case 4: return val_4bytes(gen);
        default:
            return val_1byte(gen);
    }
}

std::vector<uint16_t> RandomUTF16::generate(size_t count)
{
    std::vector<uint16_t> result;
    result.reserve(count);
    while (result.size() < count) {
        const uint32_t value = generate();
        uint16_t W1;
        uint16_t W2;
        if (utf16::encode(value, W1, W2) == 1)
            result.push_back(W1);
        else {
            result.push_back(W1);
            result.push_back(W2);
        }
    }

    result.push_back(0); // EOS for scalar code

    return result;
}

std::vector<uint16_t> RandomUTF16::generate(size_t size, long seed) {
    gen.seed(seed);
    return generate(size);
}
