FLAGS=-Wall -Wextra -Wpedantic -std=c++17 -O3 -Wfatal-errors -Werror  -Wsign-compare -Wshadow -Wwrite-strings -Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion -march=native $(CXXFLAGS)

all: unittest benchmark

unittest: tests/unittest.cpp include/reference.h sse.o random_utf16.o scalar_utf16.o
	$(CXX) $(FLAGS) random_utf16.o scalar_utf16.o sse.o tests/unittest.cpp -o unittest -Iinclude

benchmark: benchmarks/benchmark.h benchmarks/benchmark.cpp sse.o random_utf16.o scalar_utf16.o
	$(CXX) $(FLAGS) random_utf16.o scalar_utf16.o sse.o benchmarks/benchmark.cpp -o benchmark -Iinclude -Ibenchmark

random_utf16.o: include/random_utf16.h src/random_utf16.cpp
	$(CXX) $(FLAGS)  -c src/random_utf16.cpp  -Iinclude

scalar_utf16.o: include/scalar_utf16.h src/scalar_utf16.cpp
	$(CXX) $(FLAGS) -c src/scalar_utf16.cpp -Iinclude

sse.o: include/sse.h src/sse.cpp src/sse_16bit_lookup.cpp src/sse_32bit_lookup.cpp include/scalar_utf8.h include/scalar_utf16.h
	$(CXX) $(FLAGS) -c src/sse.cpp -Iinclude

src/sse_16bit_lookup.cpp: scripts/sse_compress_16bit_words.py
	./scripts/sse_compress_16bit_words.py > src/sse_16bit_lookup.cpp 

src/sse_32bit_lookup.cpp: scripts/sse_compress_32bit_words.py
	./scripts/sse_compress_32bit_words.py > src/sse_32bit_lookup.cpp 

src/sse_utf16_to_utf8_simple.cpp : scripts/sse_utf16_to_utf8_simple.py
	./scripts/sse_utf16_to_utf8_simple.py > src/sse_utf16_to_utf8_simple.cpp

clean:
	rm -f random_utf16.o scalar_utf16.o   sse.o    unittest benchmark 