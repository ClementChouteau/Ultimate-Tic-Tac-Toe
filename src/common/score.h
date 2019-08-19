#pragma once

#include <array>
#include <algorithm>

#include <cmath>
#include <climits>
#include <cassert>

#include "types.h"
#include "ttt.h"

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

private:
	std::array<score_t, 2*NUMBER_OF_TTT> _score;
};

/// ttt that are (won/lost/draw) are each transformed into one unique ttt
/// this is allow us to remember more positions in the table
inline ttt_t normalize(const Scoring* scoring, ttt_t ttt) {
	// 0 0 0
	// 0 0 0
	// 0 0 0
	const auto WON0_TTT = BIT0_IN_EACH;

	// 1 1 1
	// 1 1 1
	// 1 1 1
	const auto WON1_TTT = BIT1_IN_EACH;

	// 0 0 1
	// 1 1 0
	// 0 1 0
	const auto DRAW_TTT = 0b010110101001011001;

	if (scoring->score(ttt, PLAYER_0) == VICTORY_POINTS)
		return WON0_TTT;
	if (scoring->score(ttt, PLAYER_1) == VICTORY_POINTS)
		return WON1_TTT;
	if (nones(ttt) == 0)
		return DRAW_TTT;

	return ttt;
}
