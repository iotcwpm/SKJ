all: main.exe main.debug

.PHONY: docs tests

HPPS := $(shell find . -name "*.hpp")
CPPS := $(shell find . -name "*.cpp")

main.exe: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -O2 -Wall -I. -omain.exe main.cpp

main.debug: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -g -O0 -Wall -I. -omain.debug main.cpp

run: main.exe
	./main.exe

docs:
	$(MAKE) -C docs

tests:
	$(MAKE) -C tests

clean:
	rm -f *.debug *.exe *.o