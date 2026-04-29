CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pthread -O2
BUILD_DIR = build
TARGET = $(BUILD_DIR)/server

SRCS = main.cpp matrix.cpp server.cpp
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) server *.o

.PHONY: all clean
