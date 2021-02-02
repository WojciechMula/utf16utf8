#include "random_utf8.h"
#include "event_counter.h"
#include "benchmark.h"
#include "macros.h"

#include "sse.h"
#include "reference.h"
#include "utf8.h"
#include "pext.h"


class Benchmark {

    size_t size;
    std::random_device rd{};

public:
    Benchmark(const size_t tsize) : size(tsize) {}

    void run_from_utf8() {
        printf("\n");
        printf("Running UTF8 => UTF16 benchmark.\n");
        printf("The speed is normalized by the number of input bytes.\n");
        const size_t repeat = 10000;
        RandomUTF8 gen_1byte (rd, 1, 0, 0, 0);
        RandomUTF8 gen_2bytes(rd, 0, 1, 0, 0);
        RandomUTF8 gen_3bytes(rd, 0, 0, 1, 0);
        RandomUTF8 gen_4bytes(rd, 0, 0, 0, 1);

        RandomUTF8 gen_1_2(rd, 1, 1, 0, 0);
        RandomUTF8 gen_1_2_3(rd, 1, 1, 1, 0);
        RandomUTF8 gen_1_2_3_4(rd, 1, 1, 1, 1);

        printf("Input size: (UTF8) %lu\n", size);

        puts("- Input ASCII characters");
        run_from_utf8(gen_1byte, repeat);

        puts("- Input exactly 2 UTF8 bytes");
        run_from_utf8(gen_2bytes, repeat);

        puts("- Input exactly 3 UTF8 bytes");
        run_from_utf8(gen_3bytes, repeat);

        puts("- Input exactly 4 UTF8 bytes");
        run_from_utf8(gen_3bytes, repeat);

        puts("- Input 1 or 2 UTF8 bytes");
        run_from_utf8(gen_1_2, repeat);

        puts("- Input 1 or 2 or 3 UTF8 bytes");
        run_from_utf8(gen_1_2_3, repeat);

        puts("- Input 1 or 2 or 3 or 4 UTF8 bytes");
        run_from_utf8(gen_1_2_3_4, repeat);
    }


    void run_from_utf8(RandomUTF8& generator, size_t repeat) {

        const auto UTF8 = generator.generate(size);
        size_t volume = UTF8.size() * sizeof(UTF8[0]);

        std::vector<uint16_t> scalar_out;
        std::vector<uint16_t> sse_out;
        std::vector<uint16_t> pext_out;
        scalar_out.resize(4 * size + 32);
        sse_out.resize(4 * size + 32);
        pext_out.resize(4 * size + 32);

        auto scalar = [&UTF8, &scalar_out]() {
            utf8_to_utf16(UTF8.data(), scalar_out.data());
        };

        auto scalar_utf8lib = [&UTF8, &scalar_out]() {
            scalar_out.clear();
            utf8::utf8to16(UTF8.begin(), UTF8.end(), back_inserter(scalar_out));
        };

        auto sse = [&UTF8, &sse_out, size=size]() {
            sse_convert_valid_utf8_to_utf16_lemire(UTF8.data(), size, sse_out.data());
        };

        auto pext = [&UTF8, &pext_out, size=size]() {
            pext_convert_utf8_to_utf16(UTF8.data(), size, pext_out.data());
        };

        RUN("scalar", scalar);
        RUN("scalar_utf8lib", scalar_utf8lib);
        RUN("SSE",    sse);
        RUN("PEXT",   pext);
    }
};

int main() {
    // todo: Daniel finds the endless stream of numbers too much
    // Because some schemes use branching, you need to have a sizeable input if you
    // want fair results, given that we repeat the tests many times (we need to account
    // for branch predictors learning the branches).
    std::vector<size_t> input_size{16384};
    for (const size_t size: input_size) {
        Benchmark bench(size);
        bench.run_from_utf8();
    }
}
