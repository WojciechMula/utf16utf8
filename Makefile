FLAGS=-Wall -Wextra -Wpedantic -std=c++17 -O2 -Wfatal-errors -march=native $(CXXFLAGS)

unittest: unittest.cpp reference.cpp sse.cpp sse_16bit_lookup.cpp sse_32bit_lookup.cpp random_utf16.o scalar_utf16.o
	$(CXX) $(FLAGS) random_utf16.o scalar_utf16.o unittest.cpp -o unittest

random_utf16.o: random_utf16.h random_utf16.cpp scalar_utf16.o
	$(CXX) $(FLAGS) scalar_utf16.o -c random_utf16.cpp 

scalar_utf16.o: scalar_utf16.h scalar_utf16.cpp
	$(CXX) $(FLAGS) -c scalar_utf16.cpp

sse.o: sse.cpp sse_16bit_lookup.cpp sse_32bit_lookup.cpp
	$(CXX) $(FLAGS) -c sse.cpp

sse_16bit_lookup.cpp: scripts/sse_compress_16bit_words.py
	./scripts/sse_compress_16bit_words.py > sse_16bit_lookup.cpp

sse_32bit_lookup.cpp: scripts/sse_compress_32bit_words.py
	./scripts/sse_compress_32bit_words.py > sse_32bit_lookup.cpp

sscalar_test: scalar_test.cpp scalar.h
	$(CXX) $(FLAGS) scalar_test.cpp -o scalar_test
