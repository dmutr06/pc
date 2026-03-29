CC = g++
CFLAGS = -std=c++23 -pthread

all: main profile_test test_features test_features2

main: main.cpp thread_pool.cpp thread_pool.hpp
	$(CC) $(CFLAGS) main.cpp -o main

profile_test: profile_test.cpp thread_pool.cpp thread_pool.hpp
	$(CC) $(CFLAGS) profile_test.cpp -o profile_test

test_features: test_features.cpp
	$(CC) $(CFLAGS) test_features.cpp -o test_features

test_features2: test_features2.cpp
	$(CC) $(CFLAGS) test_features2.cpp -o test_features2

clean:
	rm -f main profile_test test_features test_features2
