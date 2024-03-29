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
			_score[2*ttt+Owner::Player0-1] = _compute_score(ttt, Owner::Player0);
			_score[2*ttt+Owner::Player1-1] = _compute_score(ttt, Owner::Player1);
		}
	}

	inline score_t score(ttt_t ttt, player_t player) const {
		return _score[2*ttt+player-1];
	}

	inline score_t score(const Board& board) const {
		if (board.winner() == Owner::None)
			return _board_score(board.getBoard());

		// we remove the explored depth to the score to choose closest victory (or farthest defeat)
		else if (board.winner() == Owner::Player0) return +(GLOBAL_VICTORY0_SCORE - board.actionsSize());
		else if (board.winner() == Owner::Player1) return -(GLOBAL_VICTORY0_SCORE - board.actionsSize());
		else if (board.winner() == Owner::Draw) return DRAW_SCORE;
	}

private:
	score_t _compute_score(ttt_t ttt, Owner player) const {
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
		return _line_score(board, ttt_possible_lines[0])
		     + _line_score(board, ttt_possible_lines[1])
         + _line_score(board, ttt_possible_lines[2])
         + _line_score(board, ttt_possible_lines[3])
         + _line_score(board, ttt_possible_lines[4])
         + _line_score(board, ttt_possible_lines[5])
         + _line_score(board, ttt_possible_lines[6])
         + _line_score(board, ttt_possible_lines[7])
    ;
	}

	inline score_t _line_score(const std::array<ttt_t, 9>& board, const std::tuple<int, int, int> line) const {
		const auto s00 = score(board[std::get<0>(line)], Owner::Player0);
		const auto s01 = score(board[std::get<0>(line)], Owner::Player1);
		const auto s10 = score(board[std::get<1>(line)], Owner::Player0);
		const auto s11 = score(board[std::get<1>(line)], Owner::Player1);
		const auto s20 = score(board[std::get<2>(line)], Owner::Player0);
		const auto s21 = score(board[std::get<2>(line)], Owner::Player1);

		return (s00 * s10 * s20) - (s01 * s11 * s21);
	}

private:
	std::array<score_t, 2*NUMBER_OF_TTT> _score;
};

