#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <vector>
#include <array>
#include <stack>
#include <tuple>

#include <cstring>
#include <climits>

#include "score.h"
#include "zobrist.h"
#include "move.h"

#define CURRENT(myTurn) ((myTurn) ? PLAYER_0 : PLAYER_1)

#define EMPTY_SCORE {0, 0, 0, 0, 0, 0, 0, 0, 0}
#define AT_9(s, y, x) (s[y*3 + x])
#define AT_9m(s, m) (s[((Move) m).j/9])

// GLOBAL_VICTORY0_SCORE+1 and -GLOBAL_VICTORY0_SCORE-1 must be valid numbers !!!
#define GLOBAL_VICTORY0_SCORE (std::numeric_limits<score_t>::max()-2)

using namespace std;

typedef struct Action {
	Move move;
	int macro_board;
	char winner;
	int nones_sum;
} Action;

class Board {
public:
	Board() { }

	Board(const string& in) {
		int cpt = 0;

		for (int Y = 0; Y < 3; Y++)
		for (int y = 0; y < 3; y++)
		for (int X = 0; X < 3; X++)
		for (int x = 0; x < 3; x++) {
			if (in[cpt] == ',') cpt++;
			set_ttt_int(get_ttt(Y, X), y, x, from_char(in[cpt++]));
			if (get_ttt_int(get_ttt(Y, X), y, x) != NONE) {
				nones_sum--;
			}
		}

		for (int Y = 0; Y < 3; Y++)
		for (int X = 0; X < 3; X++) {
			const auto ttt = get_ttt(Y, X);

			if (score(ttt, PLAYER_0) == VICTORY_POINTS || score(ttt, PLAYER_1) == VICTORY_POINTS) {
				nones_sum -= nones(ttt);
			}
		}

		macro_board = macroBoardFromBoard();

		if (score(macro_board, PLAYER_0) == VICTORY_POINTS)
			winner = PLAYER_0;
		if (score(macro_board, PLAYER_1) == VICTORY_POINTS)
			winner = PLAYER_1;
	}

	inline int get(const Move& move) const {
		return get_ttt_int(AT_9m(board, move), move.j%9);
	}

	inline int get(uint8_t index, uint8_t j) const {
		return get_ttt_int(board[index], j);
	}

	inline int& get_ttt(int Y, int X) {
		return AT_9(board, Y, X);
	}

	inline const int& get_ttt(int Y, int X) const {
		return AT_9(board, Y, X);
	}

	inline bool isWonOrFull_d(uint8_t m) const {
		return (score(board[m], PLAYER_0) == VICTORY_POINTS
				|| score(board[m], PLAYER_1) == VICTORY_POINTS
				|| nones(board[m]) == 0);
	}

	inline bool isWonOrFull(Move m) const {
		return (score(AT_9m(board, m), PLAYER_0) == VICTORY_POINTS
				|| score(AT_9m(board, m), PLAYER_1) == VICTORY_POINTS
				|| nones(AT_9m(board, m)) == 0);
	}

	inline void possibleMoves(array<MoveValued, 9*9+1>& moves, const Move& moveGenerator) const {
		assert(moveGenerator != Move::end);
		assert(moveGenerator != Move::skip);

		int cnt = 0;
		if (moveGenerator != Move::any) {
			uint8_t mov = moveGenerator.j%9;

			for (uint8_t m = 0; m < 9; m++) {
				if (get(mov, m) == NONE) {
					moves[cnt].move = (Move) (mov*9 + m);
					cnt++;
				}
			}
		} else {
			for(uint8_t m=0; m<9; ++m)
				if (!isWonOrFull_d(m)){
					for (uint8_t save = 0; save<9; save++)
						if (get(m, save) == NONE) {
							moves[cnt].move = (Move) (m*9 + save);
							cnt++;
						}
				}
		}
		moves[cnt].move = Move::end;
	}

	inline bool isValidMove(Move lastMove, Move move) const {
		if (!(!isWonOrFull(move) && get(move) == NONE)) {
			if (isWonOrFull(move))
				cerr << "FULL ";
			if (get(move) != NONE)
				cerr << "!NONE";

			cerr << endl;
		}

		return (lastMove == Move::any || (lastMove.y() == move.Y() && lastMove.x() == move.X()))
				&& !isWonOrFull(move) && get(move) == NONE;
	}

	// name of winner in case of victory or draw
	inline int winnerOrDraw() const {
		return (winner != NONE) ? (winner) : ((nones_sum == 0) ? DRAW : NONE);
	}

	void action(const Move& move, bool myTurn) {
		// save informations
		actions[actions_size].move = move;
		actions[actions_size].macro_board = macro_board;
		actions[actions_size].winner = winner;
		actions[actions_size].nones_sum = nones_sum;

		actions_size++;

		// actions here
		set_ttt_int(AT_9m(board, move), move.j%9, CURRENT(myTurn));

		const auto ttt = AT_9m(board, move);

		nones_sum--;

		// more actions if macro update
		if (score(ttt, PLAYER_0) == VICTORY_POINTS || score(ttt, PLAYER_1) == VICTORY_POINTS || nones(ttt) == 0) {
			nones_sum -= nones(ttt);

			if (score(ttt, PLAYER_0) == VICTORY_POINTS)
				set_ttt_int(macro_board, move.j/9, PLAYER_0);
			else if (score(ttt, PLAYER_1) == VICTORY_POINTS)
				set_ttt_int(macro_board, move.j/9, PLAYER_1);
			else if (nones(ttt) == 0)
				set_ttt_int(macro_board, move.j/9, DRAW);

			if (score(macro_board, PLAYER_0) == VICTORY_POINTS)
				winner = PLAYER_0;
			else if (score(macro_board, PLAYER_1) == VICTORY_POINTS)
				winner = PLAYER_1;
			else if (nones_sum == 0)
				winner = DRAW;
		}
	}

	void cancel() {
		const Action& act = actions[--actions_size];

		set_ttt_int(AT_9m(board, act.move), act.move.j%9, NONE);

		macro_board = act.macro_board;
		winner = act.winner;
		nones_sum = act.nones_sum;
	}

	inline score_t boardScore() const {
		if (winner == PLAYER_0) return GLOBAL_VICTORY0_SCORE - actions_size;
		if (winner == PLAYER_1) return -(GLOBAL_VICTORY0_SCORE) + actions_size;
		if (winner == DRAW) return 0; // draw

		return _score(PLAYER_0) - _score(PLAYER_1);
	}

	void print() const {
		for (int Y = 0; Y < 3; Y++) {
			for (int y = 0; y < 3; y++) {
				for (int X = 0; X < 3; X++)
				for (int x = 0; x < 3; x++)
					cerr << to_char(get(Move(Y, X, y, x))) << ' ';
				cerr << endl;
			}
		}
		print_ttt(macro_board);
		switch (winner) {
		case NONE: cerr << "NONE" << endl; break;
		case PLAYER_0: cerr << "PLAYER_0" << endl; break;
		case PLAYER_1: cerr << "PLAYER_1" << endl; break;
		case DRAW: cerr << "DRAW" << endl; break;
		}
		cerr << "#nones: " << nones_sum << endl;
		cerr << "#actions: " << actions_size << endl;
	}

	const std::array<int, 9>& getBoard() const {
		return board;
	}

private:
	__attribute__((optimize("unroll-loops")))
	inline score_t _score(int player) const {
		long s = 0;

		for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
			const long s0 = score(board[std::get<0>(line)], player);
			const long s1 = score(board[std::get<1>(line)], player);
			const long s2 = score(board[std::get<2>(line)], player);

			s += std::min(s0, std::min(s1, s2))*(s0+s1+s2);
		}

		//assert(s >= std::numeric_limits<score_t>::min() && s <= std::numeric_limits<score_t>::max());

		return s;
	}

	int macroBoardFromBoard() const {
		auto macro_board = EMPTY_TTT;

		for (int i = 0; i < 9; i++)
		{
			const auto ttt = board[i];

			if (score(ttt, PLAYER_0) == VICTORY_POINTS)
				set_ttt_int(macro_board, i, PLAYER_0);

			else if (score(ttt, PLAYER_1) == VICTORY_POINTS)
				set_ttt_int(macro_board, i, PLAYER_1);

			else if (nones(ttt) == 0)
				set_ttt_int(macro_board, i, DRAW);
		}

		return macro_board;
	}

private:
	std::array<int, 9> board = {EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT};
	int macro_board = EMPTY_TTT;

	int winner = NONE;
	int nones_sum = 9*9;

	int actions_size = 0;
	std::array<Action, 128> actions; // 10% gain with fixed size stack
};

#endif // BOARD_H
