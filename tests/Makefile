# Makefile for tests

all: tests.test

# Compile the tests
tests.exe: *.cpp
	$(CXX) -std=c++11 -g -O3 -Wall -I. -o tests.exe tests.cpp -lboost_unit_test_framework

# Run the tests
tests.test : tests.exe
	./tests.exe 2>&1 | tee -a tests.test

clean:
	rm -f *.o *.exe *.test
