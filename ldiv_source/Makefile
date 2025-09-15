CXX = g++
CXXFLAGS = -Wall -g

all: ldiv

ldiv: main.o l-div.o
	$(CXX) $(CXXFLAGS) -o ldiv main.o l-div.o

main.o: main.cpp l-div.h
	$(CXX) $(CXXFLAGS) -c main.cpp

l-div.o: l-div.cpp l-div.h
	$(CXX) $(CXXFLAGS) -c l-div.cpp

clean:
	rm -f *.o ldiv