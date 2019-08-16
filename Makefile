CXXFLAGS = -Wall -O2 -mpopcnt -std=c++11

.PHONY: test report clean

all: minmax mcts

minmax: bin/minmax

mcts: bin/mcts

bin/minmax: src/*
	g++ ${CXXFLAGS} src/main_minmax.cpp -o bin/main_minmax

bin/mcts: src/*
	g++ ${CXXFLAGS} src/main_mcts.cpp -o bin/main_mcts

test: all
	./bin/main_minmax < in/test.in
	./bin/main_mcts < in/test.in

report: minmax
	python evaluator.py ./bin/main_minmax --bench
	/usr/bin/time --verbose sh -c './bin/main_minmax < in/begin0.in  >/dev/null 2>/dev/null'
	valgrind --tool=cachegrind ./bin/main_minmax < in/begin0.in

clean:
	rm -f bin/*
