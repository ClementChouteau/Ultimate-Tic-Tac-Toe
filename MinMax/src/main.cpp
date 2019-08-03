#include <iostream>
#include <iomanip> 
#include <sstream>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <utility>
#include <chrono>

#include <climits>
#include <cassert>

#include "board.h"
#include "transposition_table.h"

// Constants and types ////////////////////////////////////////

#define MIN_DEPTH (1)
#define MAX_DEPTH (81)

#define BUDGET_TIME (0.450) // ms
#define FAILSAFE_TIME (0.100) // ms
#define SAFE_TIME (2*(BUDGET_TIME)) // ms

#define TABLE_CUTOFF (2)
#define TABLE_SIZE (1 << 24)

// Program  ///////////////////////////////////////////////////

std::array< std::array< std::array<double, 2>, 9 >, 9 > historyCuts;

void clear_history() {
	for (int Y = 0; Y<9; Y++)
		for (int X = 0; X<9; X++)
		{
			historyCuts[Y][X][0] = 0.;
			historyCuts[Y][X][1] = 0.;
		}
}

Move givenMoveGenerator;
std::array<Move, MAX_DEPTH+1> movesGenerator;
std::array<std::array<MoveValued, 9*9+1>, MAX_DEPTH+1> moves;

double BUDGET;
int positions;

TranspositionTable<TABLE_SIZE> ttable;

auto start = std::chrono::steady_clock::now();

double temps = 0;

MoveValued backtracking(Board& board, const Scoring& scoring, int depth, int maxDepth, bool myTurn, score_t A, score_t B) {
	positions++;

	static int tick = 0;
	if (tick++ >= 30000) {
		const auto diff = std::chrono::steady_clock::now() - start;
		tick = 0;
		const double dt = std::chrono::duration <double, std::ratio<1>> (diff).count();
		if (dt >= BUDGET)
			throw 0;
	}

	ExploredPositionType type = ExploredPositionType::UPPER;
	MoveValued best = {Move::end, -GLOBAL_VICTORY0_SCORE-1};
	if (board.winnerOrDraw() != NONE || depth == maxDepth) {
		best.value = (myTurn ? 1 : -1) * board.score(scoring);
		return best; // no need to save this position
	}
	else {
		// try to find position in transposition table
		const ExploredPosition* pos = (maxDepth - depth >= TABLE_CUTOFF)
				? ttable.get(board.getBoard(), myTurn, movesGenerator[depth])
				: nullptr;
		MoveValued hashMove = {Move::end, -1};
		if (pos != nullptr) {
			// saved move heuristic
			hashMove.move = pos->bestMove;
			hashMove.value = pos->value;

			// hash move existence confirmed
			if (board.isValidMove(movesGenerator[depth], hashMove.move)) {
				// stored result is relevant
				if ((maxDepth-depth) <= pos->depthBelow) {

					if (pos->type == ExploredPositionType::EXACT)
						return hashMove;

					else if (pos->type == ExploredPositionType::LOWER) {
						if (hashMove.value > best.value) {
							best = hashMove;

							if (best.value > A) {
								type = ExploredPositionType::EXACT;
								A = best.value;
								if (A >= B) { // alpha beta pruning
									type = ExploredPositionType::LOWER;
									historyCuts[best.move.Y()*3 + best.move.y()][best.move.X()*3 + best.move.x()][myTurn ? 1 : 0] += (double) (1 << 2*(maxDepth-depth));
									goto return_pos;
								}
							}
						}
					}
				}
			}
			else {
				std::cerr << "GRAVE COLLISION" << std::endl;
				pos = nullptr;
			}
		}
		// generate moves
		board.possibleMoves(moves[depth], movesGenerator[depth]);

		// order moves
		bool found = false;
		int nbMoves = 0;
		for (MoveValued& mv : moves[depth]) {
			if (mv.move == Move::end) break;
			nbMoves++;

			// hash move found
			if (pos != nullptr && mv.move == hashMove.move) {
				found = true;
				// stored result is relevant
				if ((maxDepth-depth) <= pos->depthBelow && pos->type != ExploredPositionType::UPPER) {
					mv.move = Move::skip;
				}
				// irrelevant, but first to check
				else {
					std::swap(moves[depth][0].move, mv.move);
				}
				continue;
			}

			// heuristic value for move ordering
			if (maxDepth-depth >= 3) {
				mv.value = historyCuts[mv.move.Y()*3 + mv.move.y()][mv.move.X()*3 + mv.move.x()][myTurn ? 1 : 0];
			}
			else {
				const auto ttt1 = board.get_ttt(mv.move.Y(), mv.move.X());
				auto ttt2 = ttt1;
				set_ttt_int(ttt2, mv.move.y(), mv.move.x(), CURRENT(myTurn));
				mv.value = (myTurn ? 1 : -1) * scoring.score(ttt2, myTurn ? PLAYER_0 : PLAYER_1);
			}
		}

		std::sort(moves[depth].begin() + (found ? 1 : 0), moves[depth].begin()+nbMoves,
			 [](const MoveValued& m1, const MoveValued& m2){ return m1.value > m2.value; });

		// for every possible move
		for (const MoveValued& mv : moves[depth]) {
			if (mv.move == Move::end) break;
			if (mv.move == Move::skip) continue;

			board.action(mv.move, myTurn);

			movesGenerator[depth+1] = board.isWonOrFull_d(mv.move.j%9) ? Move::any : mv.move;

			MoveValued current;
			try {
				current = backtracking(board, scoring, depth+1, maxDepth, !myTurn, -B, -A);
			}
			catch (int) { board.cancel(); throw; }

			board.cancel();

			current.value *= -1; // negamax

	//			if (depth == 0)
	//				std::cerr << current.value << " - " << mv.move.Y() << " " << mv.move.X() << " " << mv.move.y() << " " << mv.move.x() << " " << current.move.Y() << " " << current.move.X() << " " << current.move.y() << " " << current.move.x() << std::endl;


			if (current.value > best.value) {
				best.value = current.value;
				best.move = mv.move;

				if (best.value > A) {
					type = ExploredPositionType::EXACT;
					A = best.value;

					if (A >= B) { // alpha beta pruning
						type = ExploredPositionType::LOWER;
						historyCuts[best.move.Y()*3 + best.move.y()][best.move.X()*3 + best.move.x()][myTurn ? 1 : 0] += (double) (1 << 2*(maxDepth-depth));
						break;
					}
				}
			}
		}
	}
	return_pos:

	// save position in transposition table
	if (type != ExploredPositionType::UPPER && (maxDepth - depth) >= TABLE_CUTOFF) {
		ExploredPosition pos;
		pos.type = type;
		pos.depthBelow = maxDepth - depth;
		pos.fullMoves = (movesGenerator[depth] == Move::any);
		pos.bestMove = best.move.j;
		pos.myTurn = myTurn;
		pos.value = A;

		ttable.put(board.getBoard(), pos);
	}

	return best;
}

int main() {
	std::ios::sync_with_stdio(false);

	const Scoring scoring;
	Board board;
	int myBot = PLAYER_0;

	int turn = 0;

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
				myBot = from_char(c);
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
					getline(ss, num, ',');

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

			std::string time;
			ss >> time;

			const int t = atoi(time.c_str());

			int maxDepth = MIN_DEPTH;
			BUDGET = (t > SAFE_TIME) ? BUDGET_TIME : FAILSAFE_TIME;

			start = std::chrono::steady_clock::now();

			clear_history();

			positions = 0;
			MoveValued best = {Move::end, -1};
			try {
				while (std::abs(best.value) < GLOBAL_VICTORY0_SCORE-MAX_DEPTH) {
					int prev_positions = positions;
					movesGenerator[0] = givenMoveGenerator;
					best = backtracking(board, scoring, 0, maxDepth, (myBot == PLAYER_0) ? true : false, -GLOBAL_VICTORY0_SCORE-1, GLOBAL_VICTORY0_SCORE+1);
					auto hitRatio = (ttable.counters.get != 0 ? ((double)ttable.counters.hit/ttable.counters.get) : 1)*100.;
					auto missRatio = (ttable.counters.get != 0 ? ((double)ttable.counters.miss/ttable.counters.get) : 1)*100.;
					auto collisionsRatio = (ttable.counters.get != 0 ? ((double)ttable.counters.collisions/ttable.counters.get) : 0)*100.;
					auto usageRatio = (ttable.counters.capacity != 0 ? ((double)ttable.counters.count/ttable.counters.capacity) : 1)*100.;
					std::cerr << std::setprecision(3)
						<< 'D' << maxDepth << " cost: " << (positions - prev_positions)
						<< ", hit%: " << hitRatio
						<< ", miss%: " << missRatio
						<< ", collisions%: " << collisionsRatio
						<< ", use%: " << usageRatio << std::endl;
					maxDepth++;

					const double dt = std::chrono::duration <double, std::ratio<1>> (std::chrono::steady_clock::now() - start).count();
					if (dt >= BUDGET)
						throw 0;
				}
			}
			catch (int) {
				std::cerr << "aborting next deepening" << std::endl;
			}

			const auto diff = std::chrono::steady_clock::now() - start;
			const double dt = std::chrono::duration <double, std::ratio<1>> (diff).count();
			std::cerr << "score: " << board.score(scoring) << ", best: " << best.value << ", total cost: " << positions << ", total time: " << dt << std::endl;
			std::cerr << "choice D" << (maxDepth-1) << " (Y, X, y, x): " << best.move.Y() << ' ' << best.move.X() << ' ' << best.move.y() << ' ' << best.move.x() << std::endl;
			std::cerr << std::endl;

			if (best.move == Move::end || best.move == Move::skip)
				std::cout << "no_moves" << std::endl;
			else
				std::cout << "place_move "
					 << best.move.X()*3 + best.move.x() << ' '
					 << best.move.Y()*3 + best.move.y() << std::endl;
			turn += 2;
		}
		else if (op[0] == 'e')
			break;
	}

	return 0;
}
