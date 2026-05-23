CXX ?= g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O0 -g -I. -I../src
TARGET = run_tests
SRC = tests.cc
# Pull in every game source except main.cc so the test runner sees Board, etc.
GAME_SRCS = $(filter-out ../src/main.cc,$(wildcard ../src/*.cc))

$(TARGET): $(SRC) $(GAME_SRCS) third_party/doctest.h
	$(CXX) $(CXXFLAGS) $(SRC) $(GAME_SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
