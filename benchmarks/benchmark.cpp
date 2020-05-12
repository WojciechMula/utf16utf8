#include "benchmark.h"
#include "sse.h"
#include "random_utf16.h"
#include "random_utf8.h"
#include "reference.h"
#include "event_counter.h"

class Benchmark {

    size_t size;
    std::random_device rd{};

public:
    Benchmark(const size_t tsize) : size(tsize) {}

    void run() {
        printf("\n");
        printf("Running UTF16 => UTF8 benchmark.\n");
        printf("The speed is normalized by the number of code points.\n");

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
        run(gen_1byte, repeat);

        puts("- Output exactly 2 UTF8 bytes");
        run(gen_2bytes, repeat);

        puts("- Output exactly 3 UTF8 bytes");
        run(gen_3bytes, repeat);

        //puts("- Output exactly 4 UTF8 bytes");
        // Given that we do not optimize this case at
        // all, it is not a relevant benchmark.
        //run(gen_4bytes, repeat);

        puts("- Output 1 or 2 UTF8 bytes");
        run(gen_1_2, repeat);

        puts("- Output 1, 2 or 3 UTF8 bytes");
        run(gen_1_2_3, repeat);

        // Results from this benchmark appear bogus: SSE is too fast!!!
        //puts("- Output 1, 2, 3 or 4 UTF8 bytes");
        //run(anylength, repeat);
    }


    void run(RandomUTF16& generator, size_t repeat) {

        const auto UTF16 = generator.generate(size);

        std::vector<uint8_t> scalar_out;
        std::vector<uint8_t> sse_out;
        scalar_out.resize(4 * size + 32);
        sse_out.resize(4 * size + 32);

        auto scalar = [&UTF16, &scalar_out]() {
            utf16_to_utf8(UTF16.data(), (uint8_t*)scalar_out.data());
        };

        auto sse = [&UTF16, &sse_out, size=size]() {
            sse_convert_utf16_to_utf8(UTF16.data(), size, (uint8_t*)sse_out.data());
        };

        auto sse_hybrid= [&UTF16, &sse_out, size=size]() {
            sse_convert_utf16_to_utf8_hybrid(UTF16.data(), size, (uint8_t*)sse_out.data());
        };
#define RUNINS( procedure) \
        {\
        event_collector collector;\
        event_aggregate all{};\
        for(size_t i = 0; i < repeat; i++) {\
          collector.start();\
          procedure();\
          event_count allocate_count = collector.end();\
          all << allocate_count;\
        }\
        double freq = (all.best.cycles() / all.best.elapsed_sec()) / 1000000000.0;\
        double insperunit = all.best.instructions() / double(size);\
        double gbs = double(size) * double(sizeof(sse_out[0])) / all.best.elapsed_ns();\
        printf("                               %8.3f ins/codepoint, %8.3f GHz, %8.3f GB/s \n", insperunit, freq, gbs);\
        }
#define RUN(name, procedure) \
    BEST_TIME(/**/, procedure(), name, repeat, size);\
    RUNINS(procedure)

        RUN("scalar", scalar);
        RUN("SSE",    sse);
        RUN("SSEH",   sse_hybrid);
    }


    void run_from_utf8() {
        printf("\n");
        printf("Running UTF8 => UTF16 benchmark.\n");
        printf("The speed is normalized by the number of code points.\n");
        const size_t repeat = 10000;
        RandomUTF8 gen_1byte (rd, 1, 0, 0, 0);
        RandomUTF8 gen_2bytes(rd, 0, 1, 0, 0);
        RandomUTF8 gen_1_2(rd, 1, 1, 0, 0);

        printf("Input size: (UTF8) %lu\n", size);

        puts("- Output ASCII characters");
        run_from_utf8(gen_1byte, repeat);

        puts("- Output exactly 2 UTF8 bytes");
        run_from_utf8(gen_2bytes, repeat);

        puts("- Output 1 or 2 UTF8 bytes");
        run_from_utf8(gen_1_2, repeat);

    }


    void run_from_utf8(RandomUTF8& generator, size_t repeat) {

        const auto UTF8 = generator.generate(size);

        std::vector<uint16_t> scalar_out;
        std::vector<uint16_t> sse_out;
        scalar_out.resize(4 * size + 32);
        sse_out.resize(4 * size + 32);

        auto scalar = [&UTF8, &scalar_out]() {
            utf8_to_utf16(UTF8.data(), scalar_out.data());
        };

        auto sse = [&UTF8, &sse_out, size=size]() {
            sse_convert_valid_utf8_to_utf16_lemire(UTF8.data(), size, sse_out.data());
        };

        RUN("scalar", scalar);
        RUN("SSE",    sse);

    }

};

int main() {
    // todo: Daniel finds the endless stream of numbers too much
    std::vector<size_t> input_size{4096};//256 , 512, 1024, 2048, 4096};
    for (const size_t size: input_size) {
        Benchmark bench(size);
        bench.run();
        bench.run_from_utf8();
    }
}
