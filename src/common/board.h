#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <stack>
#include <tuple>

#include <cstring>
#include <climits>

#include "score.h"
#include "move.h"

#define CURRENT(myTurn) ((myTurn) ? PLAYER_0 : PLAYER_1)

#define EMPTY_SCORE {0, 0, 0, 0, 0, 0, 0, 0, 0}
#define AT_9(s, y, x) (s[y*3 + x])
#define AT_9m(s, m) (s[((Move) m).j/9])

// GLOBAL_VICTORY0_SCORE+1 and -GLOBAL_VICTORY0_SCORE-1 must be valid numbers !!!
#define GLOBAL_VICTORY0_SCORE (std::numeric_limits<score_t>::max()-2)
#define MIN_SCORE (-GLOBAL_VICTORY0_SCORE-1)
#define MAX_SCORE (GLOBAL_VICTORY0_SCORE+1)

struct State {
	std::array<ttt_t, 9> board;
	ttt_t macro_board;
	score_t nones_sum;
	player_t winner;
};

class Board {
public:
	Board() {
		state.board = {EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT};
		state.macro_board = EMPTY_TTT;
		state.winner = NONE;
		state.nones_sum = 9*9;
	 }

	Board( const std::string& in) : Board() {
		int cpt = 0;

		for (int Y = 0; Y < 3; Y++)
		for (int y = 0; y < 3; y++)
		for (int X = 0; X < 3; X++)
		for (int x = 0; x < 3; x++) {
			if (in[cpt] == ',') cpt++;
			set_ttt_int(get_ttt(Y, X), y, x, from_char(in[cpt++]));
			if (get_ttt_int(get_ttt(Y, X), y, x) != NONE) {
				state.nones_sum--;
			}
		}

		for (int Y = 0; Y < 3; Y++)
		for (int X = 0; X < 3; X++) {
			const auto ttt = get_ttt(Y, X);

			if (win(ttt, PLAYER_0) || win(ttt, PLAYER_1)) {
				state.nones_sum -= nones(ttt);
			}
		}

		state.macro_board = macroBoardFromBoard();

		if (win(state.macro_board, PLAYER_0))
			state.winner = PLAYER_0;
		if (win(state.macro_board, PLAYER_1))
			state.winner = PLAYER_1;
	}

	inline ttt_t get(const Move& move) const {
		return get_ttt_int(AT_9m(state.board, move), move.j%9);
	}

	inline ttt_t get(uint8_t index, uint8_t j) const {
		return get_ttt_int(state.board[index], j);
	}

	inline ttt_t& get_ttt(int Y, int X) {
		return AT_9(state.board, Y, X);
	}

	inline const ttt_t& get_ttt(int Y, int X) const {
		return AT_9(state.board, Y, X);
	}

	inline bool isWonOrFull_d(uint8_t m) const {
		return win(state.board[m], PLAYER_0) || win(state.board[m], PLAYER_1) || nones(state.board[m]) == 0;
	}

	inline bool isWonOrFull(Move m) const {
		return win(AT_9m(state.board, m), PLAYER_0) || win(AT_9m(state.board, m), PLAYER_1) || nones(AT_9m(state.board, m)) == 0;
	}

	inline void possibleMoves(std::array<MoveValued, 9*9+1>& moves, const Move& moveGenerator) const {
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
		return (lastMove == Move::any || (lastMove.y() == move.Y() && lastMove.x() == move.X()))
				&& !isWonOrFull(move) && get(move) == NONE;
	}

	// name of winner in case of victory or draw, NONE otherwise
	inline player_t winnerOrDraw() const {
		if (state.winner != NONE)
			return state.winner;
		if (state.nones_sum == 0)
			return DRAW;
		return NONE;
	}

	void action(const Move& move, bool myTurn) {
		// save informations
		actions[actions_size++] = state;

		// actions here
		set_ttt_int(AT_9m(state.board, move), move.j%9, CURRENT(myTurn));

		const auto ttt = AT_9m(state.board, move);

		state.nones_sum--;

		if (nones(ttt) == 0) {
			set_ttt_int(state.macro_board, move.j/9, DRAW);
		}
		else if (win(ttt, PLAYER_0)) {
			set_ttt_int(state.macro_board, move.j/9, PLAYER_0);
		}
		else if (win(ttt, PLAYER_1)) {
			set_ttt_int(state.macro_board, move.j/9, PLAYER_1);
		}
		else {
			return; // no macro update needed
		}

		// macro board update
		state.nones_sum -= nones(ttt); // remove nones in completed ttt

		if (win(state.macro_board, PLAYER_0))
			state.winner = PLAYER_0;
		else if (win(state.macro_board, PLAYER_1))
			state.winner = PLAYER_1;
		else if (state.nones_sum == 0)
			state.winner = DRAW;
	}

	void cancel() {
		state = actions[--actions_size];
	}

	inline score_t score(const Scoring& scoring) const {
		if (state.winner == PLAYER_0) return +(GLOBAL_VICTORY0_SCORE - actions_size);
		if (state.winner == PLAYER_1) return -(GLOBAL_VICTORY0_SCORE - actions_size);
		if (state.winner == DRAW) return 0; // draw

		return _score(scoring, PLAYER_0) - _score(scoring, PLAYER_1);
	}

	const std::array<ttt_t, 9>& getBoard() const {
		return state.board;
	}

	friend std::ostream& operator<<(std::ostream& os, const Board& that);

private:
	__attribute__((optimize("unroll-loops")))
	score_t _score(const Scoring& scoring, int player) const {
		score_t s = 0;

		for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
			const score_t s0 = scoring.score(state.board[std::get<0>(line)], player);
			const score_t s1 = scoring.score(state.board[std::get<1>(line)], player);
			const score_t s2 = scoring.score(state.board[std::get<2>(line)], player);

			s += std::min(s0, std::min(s1, s2))*(s0+s1+s2);
		}

		return s;
	}

	int macroBoardFromBoard() const {
		auto macro_board = EMPTY_TTT;

		for (int i = 0; i < 9; i++)
		{
			const auto ttt = state.board[i];

			if (win(ttt, PLAYER_0))
				set_ttt_int(macro_board, i, PLAYER_0);

			else if (win(ttt, PLAYER_1))
				set_ttt_int(macro_board, i, PLAYER_1);

			else if (nones(ttt) == 0)
				set_ttt_int(macro_board, i, DRAW);
		}

		return macro_board;
	}

private:
	State state;

	int actions_size = 0;
	std::array<State, 9*9> actions;
};

std::ostream& operator<<(std::ostream& os, const Board& that) {
	for (int Y = 0; Y < 3; Y++) {
		for (int y = 0; y < 3; y++) {
			for (int X = 0; X < 3; X++)
			for (int x = 0; x < 3; x++)
				std::cerr << to_char(that.get(Move(Y, X, y, x))) << ' ';
			std::cerr << std::endl;
		}
	}

	print_ttt(that.state.macro_board);

	switch (that.state.winner) {
	case NONE:
		std::cerr << "NONE" << std::endl; break;
	case PLAYER_0:
		std::cerr << "PLAYER_0" << std::endl; break;
	case PLAYER_1:
		std::cerr << "PLAYER_1" << std::endl; break;
	case DRAW:
		std::cerr << "DRAW" << std::endl; break;
	}

	std::cerr << "#nones: " << that.state.nones_sum << std::endl;
	std::cerr << "#actions: " << that.actions_size << std::endl;
	return os;
}