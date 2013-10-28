# Makefile for tests

all: tests.out

# Compile the tests
tests.exe: *.cpp $(shell find .. -name *.hpp)
	$(CXX) -std=c++11 -g -O3 -Wall -I. -o tests.exe tests.cpp -lboost_unit_test_framework

# Run the tests
tests.out : tests.exe
	./tests.exe 2>&1 | tee tests.out

clean:
	rm -f *.o *.exe *.out
