#pragma once

#include <array>
#include <algorithm>

#include <cmath>
#include <climits>
#include <cassert>

#include "common/types.h"
#include "common/ttt_utils.h"
#include "common/global_score.h"
#include "common/board.h"

#define VICTORY_POINTS (15)

class Scoring {
public:
	Scoring() {
		for (ttt_t ttt = 0; ttt < NUMBER_OF_TTT; ttt++) {
			_score[2*ttt+PLAYER_0-1] = _compute_score(ttt, PLAYER_0);
			_score[2*ttt+PLAYER_1-1] = _compute_score(ttt, PLAYER_1);
		}
	}

	inline score_t score(ttt_t ttt, player_t player) const {
		return _score[2*ttt+player-1];
	}

	inline score_t score(const Board& board) const {
		if (board.winner() == NONE)
			return _board_score(board.getBoard());

		// we remove the explored depth to the score to choose closest victory (or farthest defeat)
		else if (board.winner() == PLAYER_0) return +(GLOBAL_VICTORY0_SCORE - board.actionsSize());
		else if (board.winner() == PLAYER_1) return -(GLOBAL_VICTORY0_SCORE - board.actionsSize());
		else if (board.winner() == DRAW) return DRAW_SCORE;
	}

private:
	score_t _compute_score(ttt_t ttt, player_t player) const {
		// victory
		if (win(ttt, player)) return VICTORY_POINTS;
		if (win(ttt, OTHER(player))) return 0;
	
		// draw (not winnable)
		if (!winnable(ttt, player)) return 0;
	
		// score based on number of possible ways to win
		switch (number_of_ways_to_win(ttt, player)) {
		case 5: return VICTORY_POINTS - 1;
		case 4: return VICTORY_POINTS - 2;
		case 3: return VICTORY_POINTS - 3;
		case 2: return VICTORY_POINTS - 4;
		case 1: return VICTORY_POINTS - 5;
		case 0: break;
		default: assert(0);
		}
	
		// score based on number of threats (line started that could be completed)
		switch (number_of_unique_threats(ttt, player)) {
		case 4: return 6;
		case 3: return 5;
		case 2: return 4;
		case 1: return 2;
		case 0: return 1;
		default: assert(0);
		}
	}

	score_t _board_score(const std::array<ttt_t, 9>& board) const {
		float s = 0;

		s += _line_score(board, ttt_possible_lines[0]);
		s += _line_score(board, ttt_possible_lines[1]);
		s += _line_score(board, ttt_possible_lines[2]);
		s += _line_score(board, ttt_possible_lines[3]);
		s += _line_score(board, ttt_possible_lines[4]);
		s += _line_score(board, ttt_possible_lines[5]);
		s += _line_score(board, ttt_possible_lines[6]);
		s += _line_score(board, ttt_possible_lines[7]);

		return s;
	}

	inline float _line_score(const std::array<ttt_t, 9>& board, const std::tuple<int, int, int> line) const {
		const float s00 = score(board[std::get<0>(line)], PLAYER_0);
		const float s01 = score(board[std::get<0>(line)], PLAYER_1);
		const float s10 = score(board[std::get<1>(line)], PLAYER_0);
		const float s11 = score(board[std::get<1>(line)], PLAYER_1);
		const float s20 = score(board[std::get<2>(line)], PLAYER_0);
		const float s21 = score(board[std::get<2>(line)], PLAYER_1);

		return (((s00 * s10 * s20) - (s01 * s11 * s21)) / ((s00 + s01) * (s10 + s11) * (s20 + s21))) * 15 * 15 * 15;
	}

private:
	std::array<score_t, 2*NUMBER_OF_TTT> _score;
};

