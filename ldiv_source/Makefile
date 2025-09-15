CXX = g++
CXXFLAGS = -Wall -O2

TARGET = ldiv
OBJS = main.o
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f $(TARGET) *.o