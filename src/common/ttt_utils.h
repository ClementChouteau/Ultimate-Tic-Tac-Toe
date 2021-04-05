#pragma once

#include <cmath>
#include <iostream>

#include "ttt.h"
#include "precomputed_win.h"

const PrecomputedWin precomputedWin;

inline bool win(ttt_t ttt, player_t player) {
	return precomputedWin.isWon(ttt, player);
}

inline ttt_t transform_adversary_to_owner(ttt_t ttt, Owner player) {
	if (player == Owner::Player0) {
		return ttt | (BIT0_IN_EACH & (~ttt & (ttt >> 1)));
	}
	if (player == Owner::Player1) {
		return ttt | (BIT1_IN_EACH & (~ttt & (ttt << 1)));
	}
	return ttt;
}

score_t number_of_ways_to_win(ttt_t ttt, player_t player) {
	score_t wins = 0;

	for (int i = 0; i < 9; i++)
	{
		const auto c = get_ttt_int(ttt, i);

		if (get_ttt_int(ttt, i) != static_cast<player_t>(Owner::None)) continue;

		set_ttt_int(ttt, i, player);
		if (win(ttt, player)) wins++;

		set_ttt_int(ttt, i, c);
	}

	return wins;
}

score_t number_of_threats(ttt_t ttt, player_t player) {
	score_t threats = 0;

	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if ((c0 == player || c1 == player || c2 == player)
				&& (c0 == player || c0 == static_cast<player_t>(Owner::None))
				&& (c1 == player || c1 == static_cast<player_t>(Owner::None))
				&& (c2 == player || c2 == static_cast<player_t>(Owner::None)))
			threats++;
	}

	return threats;
}

score_t number_of_unique_threats(ttt_t ttt, player_t player) {
	score_t threats = 0;

	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if ((c0 == player || c1 == player || c2 == player)
				&& (c0 == player || c0 == Owner::None)
				&& (c1 == player || c1 == Owner::None)
				&& (c2 == player || c2 == Owner::None))
		{
			threats++;

			if (c0 == Owner::None) set_ttt_int(ttt, std::get<0>(line), Owner::Draw);
			if (c1 == Owner::None) set_ttt_int(ttt, std::get<1>(line), Owner::Draw);
			if (c2 == Owner::None) set_ttt_int(ttt, std::get<2>(line), Owner::Draw);
		}
	}

	return threats;
}

inline bool winnable(ttt_t ttt, Owner player) {
	ttt = transform_adversary_to_owner(ttt, player);

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
	return !winnable(ttt, Owner::Player0) && !winnable(ttt, Owner::Player1);
}

inline score_t number_of_blockages(ttt_t ttt, player_t player) {
	score_t blockages = 0;

	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if (!(c0 == Owner::Draw || c1 == Owner::Draw || c2 == Owner::Draw)
				&& (c0 == player || c1 == player || c2 == player)) {
				blockages++;
		}
	}

	return blockages;
}

/// ttt that are (won/lost/draw) are each transformed into one unique ttt
/// this is allow us to remember more positions in the table
inline ttt_t normalize(ttt_t ttt) {
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

	if (win(ttt, Owner::Player0))
		return WON0_TTT;
	if (win(ttt, Owner::Player1))
		return WON1_TTT;
	if (nones(ttt) == 0)
		return DRAW_TTT;

	return ttt;
}
