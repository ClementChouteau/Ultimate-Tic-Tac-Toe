all: src/main.cpp src/board.h src/score.h src/ttt.h src/transposition_table.h src/zobrist.h
	g++ -Wall -O2 -mpopcnt -std=c++11 src/main.cpp -o main

test: all
	./main < in/test.in

report: all
	python ../evaluator.py ./main --bench
	/usr/bin/time --verbose sh -c './main < in/begin0.in  >/dev/null 2>/dev/null'
	valgrind --tool=cachegrind ./main < in/begin0.in

clean:
	rm -f main
