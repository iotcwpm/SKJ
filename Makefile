all: main.debug

%.exe: %.cpp
	$(CXX) -std=c++0x -O2 -Wall -I../parti/ -I.. -I. -o$*.exe $*.cpp

%.debug: %.cpp
	$(CXX) -std=c++0x -O0 -Wall -I../parti/ -I.. -I. -g -o$*.debug $*.cpp

clean:
	rm -f *.debug *.exe *.o