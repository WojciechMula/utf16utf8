#include "random_utf16.h"
#include "event_counter.h"
#include "benchmark.h"
#include "macros.h"

#include "utf8.h"
#include "sse.h"
#include "reference.h"

class Benchmark {

    size_t size;
    std::random_device rd{};

public:
    Benchmark(const size_t tsize) : size(tsize) {}

    void run_from_utf16() {
        printf("\n");
        printf("Running UTF16 => UTF8 benchmark.\n");
        printf("The speed is normalized by the number of input bytes.\n");

        const size_t repeat = 10000;
        RandomUTF16 gen_1byte (rd, 1, 0, 0, 0);
        RandomUTF16 gen_2bytes(rd, 0, 1, 0, 0);
        RandomUTF16 gen_3bytes(rd, 0, 0, 1, 0);
        RandomUTF16 gen_4bytes(rd, 0, 0, 0, 1);
        RandomUTF16 gen_1_2(rd, 1, 1, 0, 0);
        RandomUTF16 gen_1_2_3(rd, 1, 1, 1, 0);
        RandomUTF16 anylength (rd, 1, 1, 1, 1);

        printf("Input size: (UTF16) %lu\n", size);

        puts("- Output ASCII characters");
        run_from_utf16(gen_1byte, repeat);

        puts("- Output exactly 2 UTF8 bytes");
        run_from_utf16(gen_2bytes, repeat);

        puts("- Output exactly 3 UTF8 bytes");
        run_from_utf16(gen_3bytes, repeat);

        //puts("- Output exactly 4 UTF8 bytes");
        // Given that we do not optimize this case at
        // all, it is not a relevant benchmark.
        //run(gen_4bytes, repeat);

        puts("- Output 1 or 2 UTF8 bytes");
        run_from_utf16(gen_1_2, repeat);

        puts("- Output 1, 2 or 3 UTF8 bytes");
        run_from_utf16(gen_1_2_3, repeat);

        // Results from this benchmark appear bogus: SSE is too fast!!!
        //puts("- Output 1, 2, 3 or 4 UTF8 bytes");
        //run(anylength, repeat);
    }


    void run_from_utf16(RandomUTF16& generator, size_t repeat) {

        const auto UTF16 = generator.generate(size);
        size_t volume = UTF16.size() * sizeof(UTF16[0]);


        std::vector<uint8_t> scalar_out;
        std::vector<uint8_t> sse_out;
        scalar_out.resize(4 * size + 32);
        sse_out.resize(4 * size + 32);

        auto scalar = [&UTF16, &scalar_out]() {
            utf16_to_utf8(UTF16.data(), (uint8_t*)scalar_out.data());
        };

        auto scalar_utf8lib = [&UTF16, &scalar_out]() {
            scalar_out.clear();
            utf8::utf16to8(UTF16.begin(), UTF16.end(), back_inserter(scalar_out));
        };

        auto sse = [&UTF16, &sse_out, size=size]() {
            sse_convert_utf16_to_utf8(UTF16.data(), size, (uint8_t*)sse_out.data());
        };

        auto sse_hybrid= [&UTF16, &sse_out, size=size]() {
            sse_convert_utf16_to_utf8_hybrid(UTF16.data(), size, (uint8_t*)sse_out.data());
        };

        RUN("scalar", scalar);
        RUN("scalar_utf8lib", scalar_utf8lib);

        RUN("SSE",    sse);
        RUN("SSEH",   sse_hybrid);
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
        bench.run_from_utf16();
    }
}
