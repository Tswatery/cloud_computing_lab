# CXXFLAGS+=-O2 -ggdb -DDEBUG
# CXXFLAGS+=-Wall -Wextra

# all: sudoku

# sudoku: main.cc neighbor.cc sudoku_basic.cc sudoku_min_arity.cc sudoku_min_arity_cache.cc sudoku_dancing_links.cc
# 	g++ -O2 -o $@ $^

CC = g++

CXXFLAGS = -std=c++17 -O2 -pthread

all: sudoku_solve

#a.out: ReadFromstdin.cpp
#	g++ $(CXXFLAGS) $^ -o $@

sudoku_solve: DLX.cpp
	g++ $(CXXFLAGS) $^ -o $@

# b.out: b.cpp
# 	g++ $(CXXFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f a.out, sudoku_solve 

