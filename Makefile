all: src/main.cpp src/board.h src/score.h src/ttt.h src/table.h src/zobrist.h
	g++ -Wall -O2 -std=c++11 src/main.cpp -o main

test: all
	./main < in/test.in

clean:
	rm -f main
