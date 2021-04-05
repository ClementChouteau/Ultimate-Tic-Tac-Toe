#pragma once

#include <array>
#include <tuple>
#include <cmath>

#include "types.h"

enum Owner : player_t
{
  None    = 0, // 0b00
  Player0 = 1, // 0b01
  Player1 = 2, // 0b10
  Draw    = 3  // 0b11 (macro board only)
};

inline bool encodePlayerAsBool(player_t player) {
	return player == static_cast<player_t>(Owner::Player0);
}

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
	case static_cast<player_t>(Owner::None): return '.';
	case static_cast<player_t>(Owner::Player0): return '0';
	case static_cast<player_t>(Owner::Player1): return '1';
	case static_cast<player_t>(Owner::Draw): return 'X';
	}
	return -1;
}

inline ttt_t from_char(char c) {
	switch (c) {
	case '.': return static_cast<player_t>(Owner::None);
	case '0': return static_cast<player_t>(Owner::Player0);
	case '1': return static_cast<player_t>(Owner::Player1);
	case 'X': return static_cast<player_t>(Owner::Draw);
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

// Number of Owner::None
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
