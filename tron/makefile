CXX = llvm-g++
CXXFLAGS = -Wall -std=c++11

DEBUG = true

ifdef DEBUG
# for use with valgrind for eg
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O2
endif

all: flynn carlos

flynn: flynn.o tron.o
	$(CXX) $(CXXFLAGS) -o flynn flynn.o tron.o
ifdef DEBUG
	dsymutil flynn
endif

flynn.o: tron.hpp

carlos: carlos.o tron.o
	$(CXX) $(CXXFLAGS) -o carlos carlos.o tron.o
ifdef DEBUG
	dsymutil carlos
endif

carlos.o: tron.hpp

tron.o: tron.hpp

clean:
	rm -f *.o
	rm -rf *.dSYM
	rm -f flynn
	rm -f carlos
