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

run: ioskj.exe
	./ioskj.exe


tests.exe: tests.cpp
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -otests.exe tests.cpp $(LIBS) -lboost_unit_test_framework

# Run the tests
tests.out : tests.exe
	./tests.exe 2>&1 | tee tests.out

tests:
	$(MAKE) -C tests

# Generate documentation
docs:
	./render-stencils.R
	doxygen doxygen/Doxyfile

# Publish documentation
# Aggregates alternative documentation into a single place for publishing using Github pages 
# Requires the a branch called "gh-pages":
#	git branch gh-pages
# and the "ghp-import" script
# 	sudo pip install ghp-import
publish:
	mkdir -p pages/doxygen pages/model/description pages/model/display
	cp -fr model/description/. pages/model/description/
	cp -fr model/display/. pages/model/display/
	cp -fr doxygen/html/. pages/doxygen/
	ghp-import -m "Updated pages" -p pages

clean:
	rm -f *.debug *.exe *.o