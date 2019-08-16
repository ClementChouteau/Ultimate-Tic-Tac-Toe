#pragma once

#include <array>
#include <algorithm>

#include <cmath>
#include <climits>

#include "types.h"
#include "ttt.h"

#define VICTORY_POINTS (50)

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
		const score_t wins = number_of_ways_to_win(ttt, player); // max: 5

		switch (wins) {
		case 5: return VICTORY_POINTS-1;
		case 4: return VICTORY_POINTS-2;
		case 3: return VICTORY_POINTS-3;
		case 2: return 40;
		case 1: return 35;
		}
	
		// other metrics
		const score_t unique_threats = number_of_unique_threats(ttt, player); // max: 4 (when no wins)
	
		return 5*unique_threats+1;
	}

private:
	std::array<score_t, 2*NUMBER_OF_TTT> _score;
};
