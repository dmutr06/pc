CXX      = g++
CXXFLAGS = -std=c++23 -O2 -pthread
BUILD    = build

SRCS     = matrix.cpp
MAIN     = main.cpp
BENCH    = benchmark.cpp

all: $(BUILD)/main $(BUILD)/benchmark

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/main: $(MAIN) $(SRCS) matrix.h | $(BUILD)
	$(CXX) $(CXXFLAGS) $(MAIN) $(SRCS) -o $@

$(BUILD)/benchmark: $(BENCH) $(SRCS) matrix.h | $(BUILD)
	$(CXX) $(CXXFLAGS) $(BENCH) $(SRCS) -o $@

benchmark: $(BUILD)/benchmark
	.venv/bin/python3 benchmark.py

clean:
	rm -rf $(BUILD)

.PHONY: all benchmark clean
