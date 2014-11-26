all: ioskj.debug

.PHONY: docs tests

# Define options and required libraries
CXX_FLAGS := -std=c++11 -Wall -Wno-unused-function -Wno-unused-local-typedefs
INC_DIRS := -I. -Irequires/stencila -Irequires/boost
LIB_DIRS := -Lrequires/stencila
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

############################################################################
# Documentation

# Generate Doxygen documentation
doxygen-html:
	doxygen doxygen/Doxyfile

# Stencil list for rendering
STENCILS := \
	model/description parameters/description procedures/description \
	model/display data/display yield/display \
	feasible/display ss3/display evaluate/display
# Stencil output list
STENCILS_HTML := $(patsubst %,%/index.html,$(STENCILS))
# Stencil `.pages` folders list
STENCILS_PAGES := $(patsubst %,.pages/%,$(STENCILS))

# Render a stencil: Cila -> HTML
%/index.html: %/stencil.cila
	cd $(dir $<) && Rscript -e "require(stencila); Stencil('.')$$ render()$$ export('index.html')"
# Copy a stencil to `.pages`
.pages/%: %/index.html
	mkdir -p $@  ; cp -fr $(dir $<). $@

# Render all stencils
stencils-html: $(STENCILS_HTML)

# Copy all stencils to `.pages` folder in preparation for publishing
stencils-pages: $(STENCILS_PAGES)

# Generate all documentation
docs: doxygen-html stencils-html

# Publish documentation
# Aggregates alternative documentation into a single place for publishing using Github pages 
# Requires the a branch called "gh-pages":
#	git branch gh-pages
# and the "ghp-import" script
# 	sudo pip install ghp-import
publish: stencils-pages
	mkdir -p .pages/doxygen  ; cp -fr doxygen/html/. .pages/doxygen/
	ghp-import -m "Updated pages" -p .pages

clean:
	rm -f *.debug *.exe *.o