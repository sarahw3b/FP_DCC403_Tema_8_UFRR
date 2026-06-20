CXX = g++
CXXFLAGS = -std=c++11 -Wall -I.

TARGET = formalrace-checker
SRCS = main.cpp src/parser.cpp src/cnf_gen.cpp src/solver.cpp src/reporter.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) tests/cenario_a.c

.PHONY: all clean run