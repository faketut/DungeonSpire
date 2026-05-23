CXX ?= g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O0 -g -I.
TARGET = run_tests
SRC = tests.cc

$(TARGET): $(SRC) third_party/doctest.h
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
