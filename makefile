# build config
# =================================================================================================================== #
# build components
dirs := . vm pcb mmu 
includes := $(foreach x, $(dirs), -I$(x))
c_sources := $(foreach x, $(dirs), $(wildcard $(x)/*.c))
cpp_sources := $(foreach x, $(dirs), $(wildcard $(x)/*.cpp))
sources := $(c_sources) $(cpp_sources)
objects := $(patsubst %.c, %.o, $(c_sources)) $(patsubst %.cpp, %.o, $(cpp_sources))
source_files = $(foreach x, $(sources), $(notdir $(x)))
object_files := $(foreach x, $(objects), $(notdir $(x)))

# make search config
# VPATH = $(dirs)
vpath %.cpp $(dirs)
vpath %.c $(dirs)
vpath %.h $(dirs)
vpath %.o $(dirs)

# target parameters
target := vm

# compilation parameters
CXX := g++
CPPFLAGS :=
CXXFLAGS := -g -std=c++23
main_flags := -D VERBOSE -D DEBUG


# general rules
# =================================================================================================================== #
# generate object files from their respective source files
%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(includes) -o $@ -c $<

%.o : %.c
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(includes) -o $@ -c $<


# specific rules
# =================================================================================================================== #
# compile main.cpp with its custom flags
main.o : main.cpp vm.h pcb.h mmu.h page.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(main_flags) $(includes) -o $@ -c $<


# dependency rules
# =================================================================================================================== #
# main.o, vm.o need to be recompiled when vm.h changes
main.o vm.o : vm.h
main.o mmu.o : mmu.h
main.o pcb.o : pcb.h


# build rules
# =================================================================================================================== #
# each object file will be recompiled only when necessary
$(target) : $(object_files)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(includes) -o $@ $^


# phony rules
# =================================================================================================================== #
# clean, rebuild, or print some basic build diagnostics
.PHONY : clean, re, debug
clean :
	-rm $(target) $(object_files)

re : clean $(target)

debug :
	@echo "dirs = " $(dirs)
	@echo "includes = " $(includes)
	@echo "c_sources = " $(c_sources)
	@echo "cpp_sources = " $(cpp_sources)
	@echo "sources = " $(sources)
	@echo "objects = " $(objects)
