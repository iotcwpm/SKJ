all: ioskj.debug

.PHONY: docs tests

# Define options and required libraries
CXX_FLAGS := -std=c++11 -Wall -Wno-unused-function -Wno-unused-local-typedefs
#INC_DIRS := -I. -Istencila
INC_DIRS := -I. -I/home/nbentley/Stencila/source/stencila/cpp/stencila -I/home/nbentley/Stencila/source/stencila/build/current/cpp/requires/boost/include/ -I/home/nbentley/Stencila/source/stencila/build/current/cpp/requires/rapidjson/include/
#LIB_DIRS := -Lstencila
LIB_DIRS := -L. -L/home/nbentley/Stencila/source/stencila/build/current/cpp/library
LIBS := -lstencila

# Find all .hpp and .cpp files (to save time don't recurse into subdirectories)
HPPS := $(shell find . -maxdepth 1 -name "*.hpp")
CPPS := $(shell find . -maxdepth 1 -name "*.cpp")

ioskj.exe: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -o$@ ioskj.cpp $(LIB_DIRS) $(LIBS)

run: ioskj.exe
	./ioskj.exe

ioskj.debug: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -g -O0 $(INC_DIRS) -o$@ ioskj.cpp $(LIB_DIRS) $(LIBS)

# Generate an execuatable for profiling
ioskj.prof: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -pg -O3 $(INC_DIRS) -o$@ ioskj.cpp $(LIB_DIRS) $(LIBS)

# Profile
profile:
	./ioskj.prof
	gprof ioskj.prof gmon.out > profiling.txt

tests.exe: tests.cpp
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -otests.exe tests.cpp $(LIB_DIRS) $(LIBS) -lboost_unit_test_framework

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
	mkdir -p .pages/model/description ; cp -fr model/description/. .pages/model/description/
	mkdir -p .pages/parameters/description ; cp -fr parameters/description/. .pages/parameters/description/
	mkdir -p .pages/procedures/description ; cp -fr procedures/description/. .pages/procedures/description/
	mkdir -p .pages/model/display ; cp -fr model/display/. .pages/model/display/
	mkdir -p .pages/yield/display ; cp -fr yield/display/. .pages/yield/display/
	mkdir -p .pages/data/display ; cp -fr data/display/. .pages/data/display/
	mkdir -p .pages/feasible/display ; cp -fr feasible/display/. .pages/feasible/display/
	mkdir -p .pages/ss3/display ; cp -fr ss3/display/. .pages/ss3/display/
	mkdir -p .pages/evaluate/display ; cp -fr evaluate/display/. .pages/evaluate/display/
	mkdir -p .pages/doxygen  ; cp -fr doxygen/html/. .pages/doxygen/
	ghp-import -m "Updated pages" -p .pages

clean:
	rm -f *.debug *.exe *.o