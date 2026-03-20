CXX = g++
CXXFLAGS = -std=c++23 -O0 -pthread

.PHONY: all clean run venv

all: main

main: main.cpp array_ops.h
	$(CXX) $(CXXFLAGS) main.cpp -o main

venv:
	if [ ! -d "venv" ]; then \
		python3 -m venv venv; \
		./venv/bin/pip install pandas matplotlib seaborn; \
	fi

run: main venv
	./main
	./venv/bin/python benchmark.py

clean:
	rm -f main benchmark_results.csv speedup_plot.png
	rm -rf venv
