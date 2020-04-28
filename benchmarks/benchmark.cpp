#include "benchmark.h"
#include "sse.h"
#include "random_utf16.h"
#include "reference.h"

class Benchamark {

    size_t size;
    std::random_device rd{};

public:
    Benchamark(const size_t size) : size(size) {}

    void run() {
        const size_t repeat = 10000;
        RandomUTF16 gen_1byte (rd, 1, 0, 0, 0);
        RandomUTF16 gen_2bytes(rd, 0, 1, 0, 0);
        RandomUTF16 gen_3bytes(rd, 0, 0, 1, 0);
        RandomUTF16 gen_4bytes(rd, 0, 0, 0, 1);
        RandomUTF16 gen_1_2(rd, 1, 1, 0, 0);
        RandomUTF16 gen_1_2_3(rd, 1, 1, 1, 0);
        RandomUTF16 anylength (rd, 1, 1, 1, 1);

        printf("\n");
        printf("Input size: %lu\n", size);

        puts("- Output ASCII characters");
        run(gen_1byte, repeat);

        puts("- Output exactly 2 UTF8 bytes");
        run(gen_2bytes, repeat);

        puts("- Output exactly 3 UTF8 bytes");
        run(gen_3bytes, repeat);

        puts("- Output exactly 4 UTF8 bytes");
        run(gen_4bytes, repeat);

        puts("- Output 1 or 2 UTF8 bytes");
        run(gen_1_2, repeat);

        puts("- Output 1, 2 or 3 UTF8 bytes");
        run(gen_1_2_3, repeat);

        puts("- Output 1, 2, 3 or 4 UTF8 bytes");
        run(anylength, repeat);
    }


    void run(RandomUTF16& generator, size_t repeat) {

        const auto UTF16 = generator.generate(size);

        std::string scalar_out;
        std::string sse_out;
        scalar_out.reserve(4 * size + 32);
        sse_out.reserve(4 * size + 32);

        auto scalar = [&UTF16, &scalar_out]() {
            utf16_to_utf8(UTF16.data(), (uint8_t*)scalar_out.data());
        };

        auto sse = [&UTF16, &sse_out, size=size]() {
            sse_convert_utf16_to_utf8(UTF16.data(), size, (uint8_t*)sse_out.data());
        };

#define RUN(name, procedure) \
    BEST_TIME(/**/, procedure(), name, repeat, size);

        RUN("scalar", scalar);
        RUN("SSE",    sse);
    }

};

int main() {

    std::vector<size_t> input_size{256, 512, 1024, 2048, 4096};
    for (const size_t size: input_size) {
        Benchamark bench(size);
        bench.run();
    }
}
