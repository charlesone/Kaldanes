# Makefile for Kaldanes 11/27/18
# (CJohnson@member.fsf.org)
#
# This code was developed on gcc version 4.8.5 20150623 on Centos-7.
# Try "sudo yum install devtoolset-7" to get it, if that doesn't work.
# the compiler can be obtained from Software Collections "Developer Toolset 7",
# at //www.softwarecollections.org/en/scls/, browse for the latest version.
#
# "make all", and "make clean" do everything you need.
# Valid optimization settings are -O0 -O1 -O2 -O3 or -Ofast.
#
# For TableDemo and JoinDemo, optimization above -O1 produces
# a segfault after execution is finished with all outputs completed.
# It is presumed that the optimized code does not clean up after itself
# very well in termination, with all of the advanced features in use.
#
# Update: this code has since been modified to compile clean and work on
# gcc version 4.8.5 -std=c++11, but now also compiles clean and works on
# gcc version 8.2.1 -std=c++11 & -std=c++14 & -std=c++17 & -std=c++2a,
# which is the preliminary version of gcc -std=c++20 standard.
#
CXX = g++
CPPFLAGS = -std=c++11 -Ofast # put pre-processor settings (-I, -D, etc) here
CXXFLAGS = -Wall  # put compiler settings here
LDFLAGS =         # put linker settings here
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
BINS=$(SOURCES:.cpp=)

all: $(BINS)

SortBench: SortBench.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) SortBench.o

SortDemo: SortDemo.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) SortDemo.o

SortQATest: SortQATest.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) SortQATest.o

TableDemo: TableDemo.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) TableDemo.o

JoinDemo: JoinDemo.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) JoinDemo.o

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

clean:
	$(RM) $(OBJECTS) $(DEPS) $(BINS)
