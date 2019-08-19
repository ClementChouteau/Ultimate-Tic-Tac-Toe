#pragma once

#include <climits>

#include "types.h"

// int16_t ranges from -32768 to +32767
#define MAX_NEGATABLE_SCORE (std::numeric_limits<score_t>::max()) // used to initialize the score to an impossible value
#define MIN_NEGATABLE_SCORE (-MAX_NEGATABLE_SCORE) // used to initialize the score to an impossible value
#define GLOBAL_VICTORY0_SCORE (MAX_NEGATABLE_SCORE-1) // score of won board at depth 0
#define DRAW_SCORE (std::numeric_limits<score_t>::min()) // draw is coded as one separate value

inline bool isDraw(score_t score) {
	return score == DRAW_SCORE;
}

inline score_t decodeDraw(score_t score) {
	if (score == DRAW_SCORE) {
		return 0;
	}

	return score;
}
