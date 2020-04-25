FLAGS=-Wall -Wextra -Wpedantic -std=c++17 -O2 -Wfatal-errors -march=native $(CXXFLAGS)

unittest: unittest.cpp reference.cpp sse.cpp sse_lookup.cpp sse_dword_lookup.cpp random_utf16.o scalar_utf16.o
	$(CXX) $(FLAGS) random_utf16.o scalar_utf16.o unittest.cpp -o unittest

random_utf16.o: random_utf16.h random_utf16.cpp scalar_utf16.o
	$(CXX) $(FLAGS) scalar_utf16.o -c random_utf16.cpp 

scalar_utf16.o: scalar_utf16.h scalar_utf16.cpp
	$(CXX) $(FLAGS) -c scalar_utf16.cpp

sse.o: sse.cpp sse_lookup.cpp sse_dword_lookup.cpp
	$(CXX) $(FLAGS) -c sse.cpp

sse_lookup.cpp: scripts/expand_utf8.py
	./scripts/expand_utf8.py > sse_lookup.cpp

sse_dword_lookup.cpp: scripts/compress_dwords_utf8.py
	./scripts/compress_dwords_utf8.py > sse_dword_lookup.cpp

sscalar_test: scalar_test.cpp scalar.h
	$(CXX) $(FLAGS) scalar_test.cpp -o scalar_test
