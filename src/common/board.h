#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <stack>
#include <tuple>

#include <cstring>

#include "score.h"
#include "move.h"
#include "global_score.h"

#define CURRENT(myTurn) ((myTurn) ? PLAYER_0 : PLAYER_1)

#define AT_9(s, y, x) (s[y*3 + x])
#define AT_9m(s, m) (s[((Move) m).j/9])

struct State {
	std::array<ttt_t, 9> board;
	ttt_t macro_board;
	score_t nones_sum;
	player_t winner;
};

class Board {
public:
	Board(const Scoring* given_scoring) : scoring(given_scoring) {
		state.board = {EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT, EMPTY_TTT};
		state.macro_board = EMPTY_TTT;
		state.winner = NONE;
		state.nones_sum = 9*9;
	 }

	Board(const Scoring* given_scoring, const std::string& in) : Board(given_scoring) {
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
			auto& ttt = get_ttt(Y, X);

			if (win(ttt, PLAYER_0) || win(ttt, PLAYER_1)) {
				state.nones_sum -= nones(ttt);
			}

			ttt = normalize(scoring, ttt);
		}

		state.macro_board = macroBoardFromBoard();

		if (win(state.macro_board, PLAYER_0))
			state.winner = PLAYER_0;
		if (win(state.macro_board, PLAYER_1))
			state.winner = PLAYER_1;
		if (state.nones_sum == 0)
			state.winner = DRAW;
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
		return scoring->score(state.board[m], PLAYER_0) == VICTORY_POINTS || scoring->score(state.board[m], PLAYER_1) == VICTORY_POINTS || nones(state.board[m]) == 0;
	}

	inline bool isWonOrFull(Move m) const {
		return scoring->score(AT_9m(state.board, m), PLAYER_0) == VICTORY_POINTS || scoring->score(AT_9m(state.board, m), PLAYER_1) == VICTORY_POINTS || nones(AT_9m(state.board, m)) == 0;
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
		auto& ttt = AT_9m(state.board, move);
		set_ttt_int(ttt, move.j%9, CURRENT(myTurn));
		const auto nones_to_remove = nones(ttt);

		ttt = normalize(scoring, ttt);

		state.nones_sum--; // one none was removed of the ttt

		// macro board update if necessary
		if (scoring->score(ttt, PLAYER_0) == VICTORY_POINTS)
			set_ttt_int(state.macro_board, move.j/9, PLAYER_0);
		else if (scoring->score(ttt, PLAYER_1) == VICTORY_POINTS)
			set_ttt_int(state.macro_board, move.j/9, PLAYER_1);
		else if (nones(ttt) == 0)
			set_ttt_int(state.macro_board, move.j/9, DRAW);
		else
			return; // no winner state update needed

		state.nones_sum -= nones_to_remove; // remove nones of the (now completed) ttt

		// winner state update
		if (scoring->score(state.macro_board, PLAYER_0) == VICTORY_POINTS)
			state.winner = PLAYER_0;
		else if (scoring->score(state.macro_board, PLAYER_1) == VICTORY_POINTS)
			state.winner = PLAYER_1;
		else if (state.nones_sum == 0)
			state.winner = DRAW;
	}

	void cancel() {
		state = actions[--actions_size];
	}

	inline score_t score(const Scoring& scoring) const {
		if (state.winner == NONE)
			return _score(scoring);

		// we remove the explored depth to the score to choose closest victory (or farthest defeat)
		else if (state.winner == PLAYER_0) return +(GLOBAL_VICTORY0_SCORE - actions_size);
		else if (state.winner == PLAYER_1) return -(GLOBAL_VICTORY0_SCORE - actions_size);
		else if (state.winner == DRAW) return DRAW_SCORE; // draw
	}

	const std::array<ttt_t, 9>& getBoard() const {
		return state.board;
	}

	friend std::ostream& operator<<(std::ostream& os, const Board& that);

private:
	__attribute__((optimize("unroll-loops")))
	score_t _score(const Scoring& scoring) const {
		float s = 0;

		for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
			const float s00 = scoring.score(state.board[std::get<0>(line)], PLAYER_0);
			const float s01 = scoring.score(state.board[std::get<0>(line)], PLAYER_1);
			const float s10 = scoring.score(state.board[std::get<1>(line)], PLAYER_0);
			const float s11 = scoring.score(state.board[std::get<1>(line)], PLAYER_1);
			const float s20 = scoring.score(state.board[std::get<2>(line)], PLAYER_0);
			const float s21 = scoring.score(state.board[std::get<2>(line)], PLAYER_1);

			s += (s00 / (s00 + s01)) * 15 * (s10 / (s10 + s11)) * 15 * (s20 / (s20 + s21)) * 15;
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
	const Scoring* scoring;

	int actions_size = 0;
	std::array<State, 9*9> actions;
};

std::ostream& operator<<(std::ostream& os, const Board& that) {
	for (int Y = 0; Y < 3; Y++) {
		for (int y = 0; y < 3; y++) {
			for (int X = 0; X < 3; X++) {
				for (int x = 0; x < 3; x++) {
					std::cerr << to_char(that.get(Move(Y, X, y, x))) << ' ';
				}
				std::cerr << ' ';
			}
			std::cerr << std::endl;
		}
		std::cerr << std::endl;
	}

	print_ttt(that.state.macro_board);
	std::cerr << "winner: ";

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
