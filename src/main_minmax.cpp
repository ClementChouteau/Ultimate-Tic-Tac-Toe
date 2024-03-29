#include <iostream>
#include <iomanip> 
#include <sstream>
#include <string>
#include <chrono>

#include "minmax.h"
#include "common/board.h"

// Constants and types ////////////////////////////////////////

#define BUDGET_TIME (450) // ms
#define FAILSAFE_TIME (100) // ms
#define SAFE_TIME (2*(BUDGET_TIME)) // ms

#define TABLE_SIZE (1 << 24)

void outputMove(const Move& move) {
	if (move == Move::end || move == Move::skip) {
		std::cout << "no_moves" << std::endl;
	}
	else {
		std::cout << "place_move "
			 << move.X()*3 + move.x() << ' '
			 << move.Y()*3 + move.y() << std::endl;
	}
}

double computeTimeBudget(double availableTimeInMs) {
	return (availableTimeInMs > SAFE_TIME) ? BUDGET_TIME : FAILSAFE_TIME;
}

int main() {
	std::ios::sync_with_stdio(false);

	player_t myPlayer = Owner::Player0;

	Move givenMoveGenerator;

	const Scoring scoring;
	MinMaxBasedAI<TABLE_SIZE> ai(scoring);

	Board board;

	while (true) {
		std::string line;
		std::getline(std::cin, line);
		std::stringstream ss;
		ss << line;

		std::string op;
		ss >> op;
		if (op[0] == 's') {
			std::string your_botid;
			ss >> your_botid;
			if (your_botid == "your_botid") {
				char c;
				ss >> c;
				myPlayer = from_char(c);
			}
			continue;
		}
		else if (op[0] == 'u') {
			std::string game;
			ss >> game;

			std::string op;
			ss >> op;

			if (game == "game" && op[0] == 'f') {
				std::string new_board;
				ss >> new_board;

				board = Board(new_board);
			}
			else if (game == "game" && op[0] == 'm') {
				givenMoveGenerator = Move::any;

				for (int Y = 0; Y < 3; Y++)
				for (int X = 0; X < 3; X++)
				{
					std::string num;
					std::getline(ss, num, ',');

					if (num.find("-1") != std::string::npos) {
						if (givenMoveGenerator == Move::any) {
							givenMoveGenerator = Move(0, 0, Y, X);
						}
						else {
							givenMoveGenerator = Move::any;
							goto possibleMovesFound;
						}
					}
				}
			}

			possibleMovesFound:;
			continue;
		}
		else if (op[0] == 'a') {
			std::string move;
			ss >> move;

			int availableTimeInMs;
			ss >> availableTimeInMs;

			const auto bestMove = ai.play(board, myPlayer, givenMoveGenerator, computeTimeBudget(availableTimeInMs));

			outputMove(bestMove);
		}
		else if (op[0] == 'e')
			break;
	}

	return 0;
}
