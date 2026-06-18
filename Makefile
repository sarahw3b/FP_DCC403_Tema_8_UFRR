CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

SRC = main.cpp src/parser.cpp src/cnf_gen.cpp src/solver.cpp src/reporter.cpp
TARGET = formalrace-checker

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.cnf *.sat

.PHONY: all clean
