all: main.exe main.debug

.PHONY: docs tests

HPPS := $(shell find . -name "*.hpp")
CPPS := $(shell find . -name "*.cpp")

LIBS := -lboost_filesystem -lboost_system

main.exe: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -O3 -Wall -I. -I/home/nbentley/Trophia/P12013_Trident/Code/fsl/ -omain.exe main.cpp $(LIBS)

main.debug: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -g -O0 -Wall -I. -I/home/nbentley/Trophia/P12013_Trident/Code/fsl/ -omain.debug main.cpp $(LIBS)

run: main.exe
	./main.exe

docs:
	$(MAKE) -C docs

tests:
	$(MAKE) -C tests

clean:
	rm -f *.debug *.exe *.o