#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <array>
#include <random>

#include <cstdint>

#include "ttt.h"
#include "score.h"

#define HASH_POS(ttt, i, Hash) ((i)*(2*NUMBER_OF_TTT) + 2*(ttt) + (Hash))

using hash_t = std::uint32_t;

// 2 hash for each ttt at each (y, x) position
static std::array<hash_t, 2*9*NUMBER_OF_TTT> _hash;

static std::array<hash_t, 2> _1myTurn_hash;
static std::array<hash_t, 2> _0myTurn_hash;

static std::array<hash_t, 2> _1fullMoves_hash;
static std::array<hash_t, 2> _0fullMoves_hash;

static std::array<hash_t, 2*9> _mov_hash;

// hash of two won/lost/draw is equal
template<int Hash>
inline hash_t zhash(int ttt, int i) {
	return _hash[HASH_POS(ttt, i, Hash)];
}

template<int Hash>
inline hash_t zhash_myturn(bool myTurn) {
	return myTurn ? _1myTurn_hash[Hash] : _0myTurn_hash[Hash];
}

template<int Hash>
inline hash_t zhash_fullmoves(bool fullMoves) {
	return fullMoves ? _1fullMoves_hash[Hash] : _0fullMoves_hash[Hash];
}

template<int Hash>
inline hash_t zhash_mov(int mov) {
	return _mov_hash[2*mov+Hash];
}

// hash pour chaque composante du jeu
void compute_hashes() {
	std::mt19937 rd;
	std::uniform_int_distribution<hash_t> dist;

	_1myTurn_hash[0] = dist(rd);
	_1myTurn_hash[1] = dist(rd);
	_0myTurn_hash[0] = dist(rd);
	_0myTurn_hash[1] = dist(rd);

	_1fullMoves_hash[0] = dist(rd);
	_1fullMoves_hash[1] = dist(rd);
	_0fullMoves_hash[0] = dist(rd);
	_0fullMoves_hash[1] = dist(rd);

	for (unsigned int i = 0; i<_mov_hash.size(); i++)
		_mov_hash[i] = dist(rd);

	std::array<std::array<int, 9>, 2> win_hash;
	std::array<std::array<int, 9>, 2> lose_hash;
	std::array<std::array<int, 9>, 2> draw_hash;
	for (int i = 0; i<9; i++)
	{
		win_hash[0][i] = dist(rd);
		win_hash[1][i] = dist(rd);
		lose_hash[0][i] = dist(rd);
		lose_hash[1][i] = dist(rd);
		draw_hash[0][i] = dist(rd);
		draw_hash[1][i] = dist(rd);
	}

	for (int i = 0; i<9; i++)
		for (int ttt = 0; ttt<NUMBER_OF_TTT; ttt++)
			for (int Hash = 0; Hash<=1; Hash++)
			{
				hash_t h;

				if (score(ttt, PLAYER_0) == VICTORY_POINTS)
					h = win_hash[Hash][i];
				else if (score(ttt, PLAYER_1) == -VICTORY_POINTS)
					h = lose_hash[Hash][i];
				else if (nones(ttt) == 0)
					h = draw_hash[Hash][i];
				else
					h = dist(rd);

				_hash[HASH_POS(ttt, i, Hash)] = h;
			}
}

#endif // ZOBRIST_H
