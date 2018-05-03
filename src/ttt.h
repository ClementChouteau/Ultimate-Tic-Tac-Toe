#ifndef TTT_H
#define TTT_H

#include <iostream>
#include <vector>
#include <tuple>
#include <bitset>

#include <cmath>

#define NONE (0)
#define PLAYER_0 (1)
#define PLAYER_1 (2)
// macro board only
#define DRAW (3)

#define EMPTY_TTT (0)
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

inline int get_ttt_int(int ttt, int i) {
	return (ttt >> 2*i) & 3;
}

inline void set_ttt_int(int& ttt, int i, int c) {
	const int mask = 3 << 2*i;
	const int s = c << 2*i;
	ttt = (ttt & ~mask) | s;
}

inline int get_ttt_int(int ttt, int y, int x) {
	return (ttt >> 2*(y*3+x)) & 3;
}

inline void set_ttt_int(int& ttt, int y, int x, int c) {
	const int mask = 3 << 2*(y*3+x);
	const int s = c << 2*(y*3+x);
	ttt = (ttt & ~mask) | s;
}

inline char to_char(int t) {
	switch (t) {
	case NONE: return '.';
	case PLAYER_0: return '0';
	case PLAYER_1: return '1';
	case DRAW: return 'X';
	}
	return -1;
}

inline int from_char(char c) {
	switch (c) {
	case '.': return NONE;
	case '0': return PLAYER_0;
	case '1': return PLAYER_1;
	case 'X': return DRAW;
	}
	return -1;
}

void print_ttt(int ttt) {
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++)
			std::cerr << to_char(get_ttt_int(ttt, y, x)) << ' ';
		std::cerr << std::endl;
	}
}

inline int nones(int ttt) {
	return 9-__builtin_popcount(((ttt & 0x55555555) << 1) | (ttt & 0xAAAAAAAA));
}

int full_ttt(int c) {
	int ttt = 0;
	for (int i = 0; i<9; i++)
		set_ttt_int(ttt, i, c);

	return ttt;
}

bool win(int ttt, int player) {
	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if (c0 == player && c1 == player && c2 == player)
			return true;
	}

	return false;
}

bool winnable(int ttt, int player) {
	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if ((c0 == player || c0 == NONE) && (c1 == player || c1 == NONE) && (c2 == player || c2 == NONE))
			return true;
	}

	return false;
}

bool draw(int ttt) {
	return !winnable(ttt, PLAYER_0) && !winnable(ttt, PLAYER_1);
}

int number_of_ways_to_wins(int ttt, int player) {
	int wins = 0;

	for (int i = 0; i < 9; i++)
	{
		const char c = get_ttt_int(ttt, i);

		if (get_ttt_int(ttt, i) != NONE) continue;

		set_ttt_int(ttt, i, player);
		if (win(ttt, player)) wins++;

		set_ttt_int(ttt, i, c);
	}

	return wins;
}

int number_of_threats(int ttt, int player) {
	int threats = 0;

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

int number_of_unique_threats(int ttt, int player) {
	int threats = 0;

	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

		if ((c0 == player || c1 == player || c2 == player)
				&& (c0 == player || c0 == NONE)
				&& (c1 == player || c1 == NONE)
				&& (c2 == player || c2 == NONE))
		{
			threats++;

			if (c0 == NONE) set_ttt_int(ttt, std::get<0>(line), DRAW);
			if (c1 == NONE) set_ttt_int(ttt, std::get<1>(line), DRAW);
			if (c2 == NONE) set_ttt_int(ttt, std::get<2>(line), DRAW);
		}
	}

	return threats;
}

int number_of_blockages(int ttt, int player) {
	int blockages = 0;

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

#endif // TTT_H
