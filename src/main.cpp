#include <iostream>
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
#include "table.h"

// Constants and types ////////////////////////////////////////

#define MIN_DEPTH (1)
#define MAX_DEPTH (20)

#define BUDGET_TIME (0.450) // ms
#define FAILSAFE_TIME (0.100) // ms
#define SAFE_TIME (2*(BUDGET_TIME)) // ms

#define TABLE_CUTOFF (2)

using namespace std;

// Program  ///////////////////////////////////////////////////

array< array< array<double, 2>, 9 >, 9 > historyCuts;

void clear_history() {
	for (int Y = 0; Y<9; Y++)
		for (int X = 0; X<9; X++)
		{
			historyCuts[Y][X][0] = 0.;
			historyCuts[Y][X][1] = 0.;
		}
}

Move givenMoveGenerator;
array<Move, MAX_DEPTH+1> movesGenerator;
array<array<MoveValued, 9*9+1>, MAX_DEPTH+1> moves;

double BUDGET;
int positions;

TTable<TABLE_SIZE> ttable;

auto start = chrono::steady_clock::now();

double temps = 0;

MoveValued backtracking(Board& board, int depth, int maxDepth, bool myTurn, score_t A, score_t B) {
	positions++;

	static int tick = 0;
	if (tick++ >= 30000) {
		const auto diff = chrono::steady_clock::now() - start;
		tick = 0;
		const double dt = chrono::duration <double, std::ratio<1>> (diff).count();
		if (dt >= BUDGET)
			throw 0;
	}

	ValueType type = ValueType::UPPER;
	MoveValued best = {Move::end, -GLOBAL_VICTORY0_SCORE-1};
	if (board.winnerOrDraw() != NONE || depth == maxDepth) {
		best.value = (myTurn ? 1 : -1) * board.boardScore();
		return best; // no need to save this position
	}
	else {
		// try to find position in transposition table
		const ExploredPosition* pos = (maxDepth - depth >= TABLE_CUTOFF) ? ttable.get(board.getBoard(), myTurn, movesGenerator[depth]==Move::any) : nullptr;
		MoveValued hashMove = {Move::end, -1};
		if (pos != nullptr) {
			// saved move heuristic
			hashMove.move = pos->best_move;
			hashMove.value = pos->value;

			// hash move existence confirmed
			if (board.isValidMove(movesGenerator[depth], hashMove.move)) {
				// stored result is relevant
				if ((maxDepth-depth) <= pos->depthBelow
						/*|| ((pos->type == ValueType::EXACT || pos->type == ValueType::LOWER)
							&& pos->value == GLOBAL_VICTORY0_SCORE)*/) {

					if (pos->type == ValueType::EXACT)
						return hashMove;

					else if (pos->type == ValueType::LOWER) {
						if (hashMove.value > best.value) {
							best = hashMove;

							if (best.value > A) {
								type = ValueType::EXACT;
								A = best.value;
								if (A >= B) { // alpha beta pruning
									type = ValueType::LOWER;
									historyCuts[best.move.Y()*3 + best.move.y()][best.move.X()*3 + best.move.x()][myTurn ? 1 : 0] += (double) (1 << 2*(maxDepth-depth));
									goto return_pos;
								}
							}
						}
					}
				}
			}
			else {
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
				if ((maxDepth-depth) <= pos->depthBelow && pos->type != ValueType::UPPER) {
					mv.move = Move::skip;
				}
				// irrelevant, but first to check
				else {
					swap(moves[depth][0].move, mv.move);
				}
				continue;
			}

			// heuristic value for move ordering
			if (maxDepth-depth >= 3) {
				mv.value = historyCuts[mv.move.Y()*3 + mv.move.y()][mv.move.X()*3 + mv.move.x()][myTurn ? 1 : 0];
			}
			else {
				const int ttt1 = board.get_ttt(mv.move.Y(), mv.move.X());
				int ttt2 = ttt1;
				set_ttt_int(ttt2, mv.move.y(), mv.move.x(), CURRENT(myTurn));
				mv.value = (myTurn ? 1 : -1) * score(ttt2, myTurn ? PLAYER_0 : PLAYER_1);
			}
		}

		sort(moves[depth].begin() + (found ? 1 : 0), moves[depth].begin()+nbMoves,
			 [](const MoveValued& m1, const MoveValued& m2){ return m1.value > m2.value; });

		// for every possible move
		for (const MoveValued& mv : moves[depth]) {
			if (mv.move == Move::end) break;
			if (mv.move == Move::skip) continue;

			board.action(mv.move, myTurn);

			movesGenerator[depth+1] = board.isWonOrFull_d(mv.move.j%9) ? Move::any : mv.move;

			MoveValued current;
			try {
				current = backtracking(board, depth+1, maxDepth, !myTurn, -B, -A);
			}
			catch (int) { board.cancel(); throw; }

			board.cancel();

			current.value *= -1; // negamax

	//			if (depth == 0)
	//				cerr << current.value << " - " << mv.move.Y() << " " << mv.move.X() << " " << mv.move.y() << " " << mv.move.x() << " " << current.move.Y() << " " << current.move.X() << " " << current.move.y() << " " << current.move.x() << endl;


			if (current.value > best.value) {
				best.value = current.value;
				best.move = mv.move;

				if (best.value > A) {
					type = ValueType::EXACT;
					A = best.value;

					if (A >= B) { // alpha beta pruning
						type = ValueType::LOWER;
						historyCuts[best.move.Y()*3 + best.move.y()][best.move.X()*3 + best.move.x()][myTurn ? 1 : 0] += (double) (1 << 2*(maxDepth-depth));
						break;
					}
				}
			}
		}
	}
	return_pos:

	// save position in transposition table
	if (type != ValueType::UPPER && maxDepth - depth >= TABLE_CUTOFF) {
		ExploredPosition pos;
		pos.type = type;
		pos.depthBelow = maxDepth - depth;
		pos.fullMoves = (movesGenerator[depth] == Move::any);
		pos.best_move = best.move;
		pos.my_turn = myTurn;
		pos.value = A;
		ttable.put(board.getBoard(), pos);
	}

	return best;
}

int main() {
	compute_scores();
	compute_hashes();

	std::ios::sync_with_stdio(false);

	Board board;
	int myBot = PLAYER_0;

	int turn = 0;

	while (true) {
		string line;
		getline(cin, line);
		stringstream ss;
		ss << line;

		string op;
		ss >> op;
		if (op[0] == 's') {
			string your_botid;
			ss >> your_botid;
			if (your_botid == "your_botid") {
				char c;
				ss >> c;
				myBot = from_char(c);
			}
			continue;
		}
		else if (op[0] == 'u') {
			string game;
			ss >> game;

			string op;
			ss >> op;

			if (game == "game" && op[0] == 'f') {
				string new_board;
				ss >> new_board;

				board = Board(new_board);
			}
			else if (game == "game" && op[0] == 'm') {
				givenMoveGenerator = Move::any;

				for (int Y = 0; Y < 3; Y++)
				for (int X = 0; X < 3; X++)
				{
					string num;
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
			string move;
			ss >> move;

			string time;
			ss >> time;

			const int t = atoi(time.c_str());

			int maxDepth = MIN_DEPTH;
			BUDGET = (t > SAFE_TIME) ? BUDGET_TIME : FAILSAFE_TIME;

			start = chrono::steady_clock::now();

			clear_history();

			board.print();

			positions = 0;
			MoveValued best = {Move::end, -1};
			try {
				while (maxDepth <= MAX_DEPTH) { // for end of games
					ttable.resetCounters();
					int prev_positions = positions;
					movesGenerator[0] = givenMoveGenerator;
					best = backtracking(board, 0, maxDepth, (myBot == PLAYER_0) ? true : false, -GLOBAL_VICTORY0_SCORE-1, GLOBAL_VICTORY0_SCORE+1);
					cerr << 'D' << maxDepth << " cost: " << (positions - prev_positions) << ", hit%: " << ttable.hitRate()*100. << ", collisions%: " << ttable.collisionRate()*100. << ", use%: " << ttable.useRate()*100. << endl;
					maxDepth++;
				}
			}
			catch (int) {
				cerr << "aborting next deepening" << endl;
			}

			const auto diff = chrono::steady_clock::now() - start;
			const double dt = chrono::duration <double, std::ratio<1>> (diff).count();
			cerr << "score: " << board.boardScore() << ", best: " << best.value << ", total cost: " << positions << ", total time: " << dt << endl;
			cerr << "choice D" << (maxDepth-1) << " (Y, X, y, x): " << best.move.Y() << ' ' << best.move.X() << ' ' << best.move.y() << ' ' << best.move.x() << endl;
			cerr << endl;

			if (best.move == Move::end || best.move == Move::skip)
				cout << "no_moves" << endl;
			else
				cout << "place_move "
					 << best.move.X()*3 + best.move.x() << ' '
					 << best.move.Y()*3 + best.move.y() << endl;
			turn += 2;
		}
		else if (op[0] == 'e')
			break;
	}

	return 0;
}
