#pragma once

#include <cstdint>

#include "score.h" // score_t
#include "zobrist.h" // hash_t

enum ExploredPositionType {
	UNKWN = 0,
	LOWER,
	UPPER,
	EXACT
};

struct alignas(8) ExploredPosition {
	hash_t otherHash; // biggest first
	score_t value;
	unsigned int depthBelow:5;
	bool fullMoves:1;
	unsigned int type:2;
	bool myTurn:1;
	uint8_t bestMove:7; // we can't put a Move here, it would use 8 bits
} __attribute__((packed));


// output utilities (for debug)
std::ostream& operator<<(std::ostream& os, const ExploredPosition& that) {
	os << '{';
	std::cerr << that.value << ',';
	os << std::bitset<8*sizeof(hash_t)>(that.otherHash) << ',';
	os << 'D' << (int) that.depthBelow << ',';
	if (that.myTurn)
		os << "PLAYER_0" << ',';
	else
		os << "PLAYER_1" << ',';
	switch (that.type) {
	case ExploredPositionType::LOWER:
		os << "ALPHA" << ','; break;
	case ExploredPositionType::UPPER:
		os << "BETA" << ','; break;
	case ExploredPositionType::EXACT:
		os << "EXACT" << ','; break;
	default:
		os << "UNKWN" << ','; break;
	}
	if (that.fullMoves) {
		os << "any";
	}
	else {
		os << '(';
		os << (int) Move(that.bestMove).Y() << ',';
		os << (int) Move(that.bestMove).X() << ',';
		os << (int) Move(that.bestMove).y() << ',';
		os << (int) Move(that.bestMove).x();
		os << ')';
	}
	os << '}';

	return os;
}