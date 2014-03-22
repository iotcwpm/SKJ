all: ioskj.exe ioskj.debug

.PHONY: docs tests

HPPS := $(shell find . -name "*.hpp")
CPPS := $(shell find . -name "*.cpp")

INCLUDES := -I. -I/home/nbentley/Trophia/P12013_Trident/Code/fsl/

LIBS := -lboost_filesystem -lboost_system

ioskj.exe: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -O3 -Wall $(INCLUDES) -oioskj.exe ioskj.cpp $(LIBS)

ioskj.debug: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -g -O0 -Wall $(INCLUDES) -oioskj.debug ioskj.cpp $(LIBS)

tests.exe: tests.cpp
	$(CXX) -std=c++11 -O3 -Wall $(INCLUDES) -otests.exe tests.cpp $(LIBS) -lboost_unit_test_framework

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