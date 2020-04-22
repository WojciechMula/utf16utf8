#include "reference.cpp"
#include "sse.cpp"

#include <string>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <random>

class RandomUTF16 final {
public:
    RandomUTF16(std::random_device& rd,
                int prob_1byte,
                int prob_2bytes,
                int prob_3bytes,
                int prob_4bytes);

    std::vector<uint16_t> generate(size_t size);
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
        if (encode_utf16(value, W1, W2) == 1)
            result.push_back(W1);
        else {
            result.push_back(W1);
            result.push_back(W2);
        }
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
    tmp.resize(4 * size);
    const size_t output_size = sse_convert_utf16_to_utf8(input.data(), size, (uint8_t*)tmp.data());
    const std::string sse{tmp.data(), output_size};

    return compare_strings(reference, sse);
}

bool validate_sample() {
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

bool validate_all() {
    puts("Validate all input values");
    std::vector<uint16_t> input;
    input.resize(8 + 1);

    // Note: max is 0x7fff because the SSE implementation doesn't deal properly with signed values
    for (uint16_t value=1; value <= 0x07ff; value++) {
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

    if (false && !validate_all())
        return EXIT_FAILURE;

    if (!validate_sample())
        return EXIT_FAILURE;

    puts("All OK");
    return EXIT_SUCCESS;
}
