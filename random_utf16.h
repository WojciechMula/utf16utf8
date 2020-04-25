#pragma once

#include <cstdint>
#include <cstddef>
#include <random>

class RandomUTF16 final {
public:
    RandomUTF16(std::random_device& rd,
                int prob_1byte,
                int prob_2bytes,
                int prob_3bytes,
                int prob_4bytes);

    std::vector<uint16_t> generate(size_t size);
    std::vector<uint16_t> generate(size_t size, long seed);
private:
    uint32_t generate();

    std::mt19937 gen;
    std::discrete_distribution<> utf8_length;
    std::uniform_int_distribution<uint32_t> val_1byte   {0x0001, 0x007f};
    std::uniform_int_distribution<uint32_t> val_2bytes  {0x0080, 0x07ff};
    std::uniform_int_distribution<uint32_t> val_3bytes_a{0x0800, 0xd7ff};
    std::uniform_int_distribution<uint32_t> val_3bytes_b{0xe000, 0xffff};
    std::uniform_int_distribution<uint32_t> val_4bytes  {0x10000, 0x10ffff};
};
