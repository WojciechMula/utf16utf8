#include "random_utf8.h"

RandomUTF8::RandomUTF8(std::random_device& rd, int prob_1byte, int prob_2bytes, int prob_3bytes, int prob_4bytes)
    : gen(rd())
    , bytes_count({double(prob_1byte), double(prob_2bytes), double(prob_3bytes), double(prob_4bytes)}) {}

std::vector<uint8_t> RandomUTF8::generate(size_t output_bytes)
{
    std::vector<uint8_t> result;
    result.reserve(output_bytes);
    while (result.size() < output_bytes) {
        switch (bytes_count(gen)) {
            case 0: // 1 byte
                result.push_back(val_7bit(gen));
                break;
            case 1: // 2 bytes
                result.push_back(0xc0 | val_5bit(gen));
                result.push_back(0x80 | val_6bit(gen));
                break;
            case 3: // 3 bytes
                result.push_back(0xe0 | val_4bit(gen));
                result.push_back(0x80 | val_6bit(gen));
                result.push_back(0x80 | val_6bit(gen));
                break;
            case 4: // 4 bytes
                result.push_back(0xf0 | val_3bit(gen));
                result.push_back(0x80 | val_6bit(gen));
                result.push_back(0x80 | val_6bit(gen));
                result.push_back(0x80 | val_6bit(gen));
                break;
        }
    }

    result.push_back(0); // EOS for scalar code

    return result;
}

std::vector<uint8_t> RandomUTF8::generate(size_t output_bytes, long seed) {
    gen.seed(uint32_t(seed));
    return generate(output_bytes);
}
