all: ioskj.exe ioskj.debug

.PHONY: docs tests

HPPS := $(shell find . -name "*.hpp")
CPPS := $(shell find . -name "*.cpp")

LIBS := -lboost_filesystem -lboost_system

ioskj.exe: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -O3 -Wall -I. -I/home/nbentley/Trophia/P12013_Trident/Code/fsl/ -oioskj.exe ioskj.cpp $(LIBS)

ioskj.debug: $(HPPS) $(CPPS)
	$(CXX) -std=c++11 -g -O0 -Wall -I. -I/home/nbentley/Trophia/P12013_Trident/Code/fsl/ -oioskj.debug ioskj.cpp $(LIBS)

run: ioskj.exe
	./ioskj.exe

docs:
	$(MAKE) -C docs

tests:
	$(MAKE) -C tests

clean:
	rm -f *.debug *.exe *.o