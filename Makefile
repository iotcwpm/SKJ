all: ioskj.debug

.PHONY: docs tests

HPPS := $(shell find . -name "*.hpp")
CPPS := $(shell find . -name "*.cpp")

CXX_FLAGS := -std=c++11 -Wall -Wno-unused-local-typedefs

LIBS := -lstencila -lboost_filesystem -lboost_system -lpugixml -ltidy-html5

# Include local Makefile for setting or overriding make variables
include Makefile.local

ioskj.exe: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -oioskj.exe ioskj.cpp $(LIB_DIRS) $(LIBS)

ioskj.debug: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -g -O0 $(INC_DIRS) -oioskj.debug ioskj.cpp $(LIB_DIRS) $(LIBS)

tests.exe: tests.cpp
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -otests.exe tests.cpp $(LIBS) -lboost_unit_test_framework

# Run the tests
tests.out : tests.exe
	./tests.exe 2>&1 | tee tests.out

run: ioskj.exe
	./ioskj.exe

docs:
	$(MAKE) -C docs

tests:
	$(MAKE) -C tests

clean:
	rm -f *.debug *.exe *.o