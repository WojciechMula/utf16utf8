FLAGS=-Wall -Wextra -Wpedantic -std=c++17 -O2 -Wfatal-errors -march=native $(CXXFLAGS)

unittest: unittest.cpp reference.cpp sse.cpp
	$(CXX) $(FLAGS) unittest.cpp -o unittest

sse.o: sse.cpp sse_lookup.cpp
	$(CXX) $(FLAGS) -c sse.cpp

sse_lookup.cpp: scripts/expand_utf8.py
	./scripts/expand_utf8.py > sse_lookup.cpp

sscalar_test: scalar_test.cpp scalar.h
	$(CXX) $(FLAGS) scalar_test.cpp -o scalar_test
