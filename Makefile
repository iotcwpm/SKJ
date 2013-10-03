all: main.exe main.debug

%.exe: %.cpp
	$(CXX) -std=c++11 -O2 -Wall -I. -o$*.exe $*.cpp

%.debug: %.cpp
	$(CXX) -std=c++11 -g -O0 -Wall -I. -o$*.debug $*.cpp

clean:
	rm -f *.debug *.exe *.o