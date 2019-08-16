CXXFLAGS = -Wall -O2 -mpopcnt -std=c++11

.PHONY: clean 

all: minmax mcts

minmax: src/*
	g++ ${CXXFLAGS} src/main_minmax.cpp -o main_minmax

mcts: src/*
	g++ ${CXXFLAGS} src/main_mcts.cpp -o main_mcts

test: all
	./main_minmax < in/test.in
	./main_mcts < in/test.in

report: all
	python evaluator.py ./main_minmax --bench
	/usr/bin/time --verbose sh -c './main_minmax < in/begin0.in  >/dev/null 2>/dev/null'
	valgrind --tool=cachegrind ./main_minmax < in/begin0.in

clean:
	rm -f main_minmax main_mcts
