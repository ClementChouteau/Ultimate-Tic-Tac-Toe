#pragma once

#include <array>
#include <algorithm>

#include <cmath>
#include <climits>

#include "ttt.h"

#define VICTORY_POINTS (50)

using score_t = int16_t;

static std::array<score_t, 2*NUMBER_OF_TTT> _score;

inline score_t ttt_score(int ttt, int player) {
	return _score[2*ttt+player-1];
}

score_t _compute_score(int ttt, int player) {
	// double victory (impossible)
	if (win(ttt, PLAYER_0) && win(ttt, PLAYER_1)) return _score[2*ttt+player-1] = 0;
	// to ensure symmetry

	// victory
	if (win(ttt, player)) return _score[2*ttt+player-1] = VICTORY_POINTS;
	if (win(ttt, OTHER(player))) return _score[2*ttt+player-1] = 0;

	// draw (not winnable)
	if (!winnable(ttt, player)) return _score[2*ttt+player-1] = 0;

	// score based on number of possible ways to win
	const score_t wins = number_of_ways_to_wins(ttt, player); // max: 5

	switch (wins) {
	case 5: return _score[2*ttt+player-1] = VICTORY_POINTS-1;
	case 4: return _score[2*ttt+player-1] = VICTORY_POINTS-2;
	case 3: return _score[2*ttt+player-1] = VICTORY_POINTS-3;
	case 2: return _score[2*ttt+player-1] = 40;
	case 1: return _score[2*ttt+player-1] = 35;
	}

	// other metrics
	const score_t unique_threats = number_of_unique_threats(ttt, player); // max: 4 (when no wins)

	return _score[2*ttt+player-1] = 5*unique_threats+1;
}

void compute_scores() {
	for (int ttt = 0; ttt < NUMBER_OF_TTT; ttt++) {
		_compute_score(ttt, PLAYER_0);
		_compute_score(ttt, PLAYER_1);
	}
}
