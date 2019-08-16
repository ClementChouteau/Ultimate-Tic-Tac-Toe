#pragma once

#include <iostream>
#include <array>
#include <tuple>

#include <cmath>

#include "types.h"

#define NONE ((player_t)0)
#define PLAYER_0 ((player_t)1)
#define PLAYER_1 ((player_t)2)
// macro board only
#define DRAW ((player_t)3)

#define EMPTY_TTT ((ttt_t)0)
#define NUMBER_OF_TTT ((long)std::pow(4, 3*3))

#define OTHER(player) (3-(player))

#define POS_TO_I(y, x) ((y)*3+(x))
static constexpr std::array<std::tuple<int, int, int>, 8> ttt_possible_lines =
{
	// lines
	std::make_tuple(POS_TO_I(0, 0), POS_TO_I(0, 1), POS_TO_I(0, 2)),
	std::make_tuple(POS_TO_I(1, 0), POS_TO_I(1, 1), POS_TO_I(1, 2)),
	std::make_tuple(POS_TO_I(2, 0), POS_TO_I(2, 1), POS_TO_I(2, 2)),
	// columns
	std::make_tuple(POS_TO_I(0, 0), POS_TO_I(1, 0), POS_TO_I(2, 0)),
	std::make_tuple(POS_TO_I(0, 1), POS_TO_I(1, 1), POS_TO_I(2, 1)),
	std::make_tuple(POS_TO_I(0, 2), POS_TO_I(1, 2), POS_TO_I(2, 2)),
	// diagonals
	std::make_tuple(POS_TO_I(0, 0), POS_TO_I(1, 1), POS_TO_I(2, 2)),
	std::make_tuple(POS_TO_I(0, 2), POS_TO_I(1, 1), POS_TO_I(2, 0))
};

inline ttt_t get_ttt_int(ttt_t ttt, int i) {
	return (ttt >> 2*i) & 3;
}

inline void set_ttt_int(ttt_t& ttt, int i, int c) {
	const ttt_t mask = 3 << 2*i;
	const ttt_t s = c << 2*i;
	ttt = (ttt & ~mask) | s;
}

inline ttt_t get_ttt_int(ttt_t ttt, int y, int x) {
	return (ttt >> 2*(y*3+x)) & 3;
}

inline void set_ttt_int(ttt_t& ttt, int y, int x, int c) {
	const ttt_t mask = 3 << 2*(y*3+x);
	const ttt_t s = c << 2*(y*3+x);
	ttt = (ttt & ~mask) | s;
}

inline char to_char(ttt_t t) {
	switch (t) {
	case NONE: return '.';
	case PLAYER_0: return '0';
	case PLAYER_1: return '1';
	case DRAW: return 'X';
	}
	return -1;
}

inline ttt_t from_char(char c) {
	switch (c) {
	case '.': return NONE;
	case '0': return PLAYER_0;
	case '1': return PLAYER_1;
	case 'X': return DRAW;
	}
	return -1;
}

void print_ttt(ttt_t ttt) {
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++)
			std::cerr << to_char(get_ttt_int(ttt, y, x)) << ' ';
		std::cerr << std::endl;
	}
}

constexpr ttt_t BIT0_IN_EACH = 0b010101010101010101;
constexpr ttt_t BIT1_IN_EACH = 0b101010101010101010;

inline score_t nones(ttt_t ttt) {
	ttt = ~ttt;
	return __builtin_popcount((ttt >> 1) & ttt & BIT0_IN_EACH);
}

constexpr ttt_t LINES_OF_PLAYER0[] = {
	// lines
	0b010101000000000000,
	0b000000010101000000,
	0b000000000000010101,
	// columns
	0b010000010000010000,
	0b000100000100000100,
	0b000001000001000001,
	// diagonals
	0b010000000100000001,
	0b000001000100010000,
};

/// returns ttt where positions 0b11 (DRAW) are transformed in 0b00 (NONE)
inline ttt_t remove_draw(ttt_t ttt) {
	return (ttt & (~(ttt << 1) & BIT1_IN_EACH))
	     | (ttt & (~(ttt >> 1) & BIT0_IN_EACH));
}

inline bool win(ttt_t ttt, player_t player) {
	// remove DRAW
	ttt = remove_draw(ttt); // 0b11 => 0b00

	if (player == PLAYER_1) {
		ttt = (ttt >> 1) & BIT0_IN_EACH; // 0b10 => 0b01, 0b01 => 0b00
	}

	return
		__builtin_popcount(ttt & LINES_OF_PLAYER0[0]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[1]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[2]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[3]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[4]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[5]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[6]) == 3 ||
		__builtin_popcount(ttt & LINES_OF_PLAYER0[7]) == 3
	;
}

inline ttt_t transform_adversary_to_draw(ttt_t ttt, player_t player) {
	if (player == PLAYER_0) {
		return ttt | (BIT0_IN_EACH & (~ttt & (ttt >> 1)));
	}
	if (player == PLAYER_1) {
		return ttt | (BIT1_IN_EACH & (~ttt & (ttt << 1)));
	}
	return ttt;
}

inline score_t number_of_ways_to_win(ttt_t ttt, player_t player) {
	ttt = transform_adversary_to_draw(ttt, player);

	return
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[0]) ^ ((ttt & (LINES_OF_PLAYER0[0] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[1]) ^ ((ttt & (LINES_OF_PLAYER0[1] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[2]) ^ ((ttt & (LINES_OF_PLAYER0[2] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[3]) ^ ((ttt & (LINES_OF_PLAYER0[3] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[4]) ^ ((ttt & (LINES_OF_PLAYER0[4] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[5]) ^ ((ttt & (LINES_OF_PLAYER0[5] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[6]) ^ ((ttt & (LINES_OF_PLAYER0[6] << 1)) >> 1)) == 2) +
		(__builtin_popcount((ttt & LINES_OF_PLAYER0[7]) ^ ((ttt & (LINES_OF_PLAYER0[7] << 1)) >> 1)) == 2)
	;
}

inline bool winnable(ttt_t ttt, player_t player) {
	ttt = transform_adversary_to_draw(ttt, player);

	return
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[0]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[0] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[1]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[1] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[2]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[2] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[3]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[3] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[4]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[4] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[5]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[5] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[6]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[6] << 1)) == 0)) ||
		((__builtin_popcount(ttt & LINES_OF_PLAYER0[7]) == 0) || (__builtin_popcount(ttt & (LINES_OF_PLAYER0[7] << 1)) == 0))
	;
}

inline bool draw(ttt_t ttt) {
	return !winnable(ttt, PLAYER_0) && !winnable(ttt, PLAYER_1);
}

inline score_t number_of_threats(ttt_t ttt, player_t player) {
	int threats = 0;
	ttt = transform_adversary_to_draw(ttt, player);

	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if ((c0 == player || c1 == player || c2 == player)
				&& (c0 == player || c0 == NONE)
				&& (c1 == player || c1 == NONE)
				&& (c2 == player || c2 == NONE))
			threats++;
	}

	return threats;
}

inline score_t number_of_unique_threats(ttt_t ttt, player_t player) {
	ttt = transform_adversary_to_draw(ttt, player);
	int threats = 0;

	// if the line is a threat, then there is only "player" and '.'
    // we need to check if there is at least one position different than '.'
    for (const auto LINE : LINES_OF_PLAYER0) {
		const auto cnt0 = __builtin_popcount(ttt & LINE);
		const auto cnt1 = __builtin_popcount(ttt & (LINE << 1));

        if ((cnt0 == 0 || cnt1 == 0) && (cnt0 != 0 || cnt1 != 0)) {
            threats++;
            auto draw = ((~ttt >> 1) & ~ttt) & LINE;
            draw |= draw << 1;
            ttt |= draw;
        }
    }
	return threats;
}

inline score_t number_of_blockages(ttt_t ttt, player_t player) {
	score_t blockages = 0;

	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if (!(c0 == DRAW || c1 == DRAW || c2 == DRAW)
				&& (c0 == player || c1 == player || c2 == player)) {
				blockages++;
		}
	}

	return blockages;
}
