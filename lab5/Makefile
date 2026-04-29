CC=gcc
CFLAGS=-Wall -Wextra -pthread -O2
BUILD_DIR=build

all: $(BUILD_DIR) $(BUILD_DIR)/server

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/server: $(BUILD_DIR)/server.o $(BUILD_DIR)/cache.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/server.o: server.c cache.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c server.c -o $@

$(BUILD_DIR)/cache.o: cache.c cache.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c cache.c -o $@

clean:
	rm -rf $(BUILD_DIR)
