CXX = g++
CXXFLAGS = -Wall -std=c++11 -O2

flynn: flynn.o tron.o
	$(CXX) $(CXXFLAGS) -o flynn flynn.o tron.o

flynn.o: tron.hpp

carlos: carlos.o tron.o
	$(CXX) $(CXXFLAGS) -o carlos carlos.o tron.o

carlos.o: tron.hpp

tron.o: tron.hpp

clean:
	rm -f flynn
	