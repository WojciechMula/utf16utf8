FLAGS=-Wall -Wextra -Wpedantic -std=c++17 -O2 -march=native $(CXXFLAGS)

scalar_test: scalar_test.cpp scalar.h
	$(CXX) $(FLAGS) scalar_test.cpp -o scalar_test
