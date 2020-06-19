FLAGS=-Wall -Wextra -Wpedantic -std=c++14 -O3 -Wno-overflow -Wfatal-errors -Wsign-compare -Wshadow -Wwrite-strings -Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion -march=native $(CXXFLAGS)

all: unittest benchmark

unittest: tests/unittest.cpp include/reference.h sse.o random_utf16.o scalar_utf16.o random_utf8.o
	$(CXX) $(FLAGS) random_utf16.o random_utf8.o scalar_utf16.o sse.o tests/unittest.cpp -o unittest -Iinclude

benchmark: benchmarks/benchmark.h benchmarks/benchmark.cpp sse.o random_utf16.o scalar_utf16.o random_utf8.o
	$(CXX) $(FLAGS) random_utf16.o random_utf8.o scalar_utf16.o sse.o benchmarks/benchmark.cpp -o benchmark -Iinclude -Ibenchmark  -Idependencies/utf8_v2_3_4/source

random_utf16.o: include/random_utf16.h src/random_utf16.cpp
	$(CXX) $(FLAGS)  -c src/random_utf16.cpp  -Iinclude

random_utf8.o: include/random_utf8.h src/random_utf8.cpp
	$(CXX) $(FLAGS)  -c src/random_utf8.cpp  -Iinclude

scalar_utf16.o: include/scalar_utf16.h src/scalar_utf16.cpp
	$(CXX) $(FLAGS) -c src/scalar_utf16.cpp -Iinclude

sse.o: include/sse.h src/sse.cpp src/sse_16bit_lookup.cpp src/sse_32bit_lookup.cpp src/sse_utf8_to_utf16.cpp include/scalar_utf8.h  src/sse_utf8_decoder.cpp include/scalar_utf16.h
	$(CXX) $(FLAGS) -c src/sse.cpp -Iinclude

src/sse_16bit_lookup.cpp: scripts/sse_compress_16bit_words.py
	./scripts/sse_compress_16bit_words.py > src/sse_16bit_lookup.cpp 

src/sse_32bit_lookup.cpp: scripts/sse_compress_32bit_words.py
	./scripts/sse_compress_32bit_words.py > src/sse_32bit_lookup.cpp 

src/sse_utf8_to_utf16.cpp: scripts/sse_utf8_to_utf16.py
	./scripts/sse_utf8_to_utf16.py > src/sse_utf8_to_utf16.cpp

src/sse_utf8_decoder.cpp:scripts/sse_utf8_utf16_decode.py
	./scripts/sse_utf8_utf16_decode.py > src/sse_utf8_decoder.cpp

clean:
	rm -f random_utf16.o scalar_utf16.o   sse.o    unittest benchmark 
