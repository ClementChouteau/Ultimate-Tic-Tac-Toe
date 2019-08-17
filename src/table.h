#ifndef TABLE_H
#define TABLE_H

#include <array>
#include <bitset>

#include <cstring>
#include <cassert>
#include <cstdint>

#include "zobrist.h"
#include "score.h"
#include "move.h"

#define TABLE_SIZE (1 << 24)

enum ValueType {
	UNKWN = 0,
	LOWER,
	UPPER,
	EXACT
};

class ExploredPosition {
public:
	hash_t hash;
	score_t value;
	unsigned int depthBelow:5;
	bool fullMoves:1;
	unsigned int type:2;
	bool my_turn:1;
	Move best_move;

	ExploredPosition() { }

	void print() const {
		std::cerr << '{';
		std::cerr << value << ',';
		std::cerr << std::bitset<8*sizeof(hash_t)>(hash) << ',';
		std::cerr << 'D' << (int) depthBelow << ',';

		if (my_turn)
			std::cerr << "PLAYER_0" << ',';
		else
			std::cerr << "PLAYER_1" << ',';

		switch (type) {
		case ValueType::LOWER:
			std::cerr << "ALPHA" << ','; break;
		case ValueType::UPPER:
			std::cerr << "BETA" << ','; break;
		case ValueType::EXACT:
			std::cerr << "EXACT" << ','; break;
		default:
			std::cerr << "UNKWN" << ','; break;
		}

		if (fullMoves) {
			std::cerr << "any";
		}
		else {
			std::cerr << '(';
			std::cerr << (int) best_move.Y() << ',';
			std::cerr << (int) best_move.X() << ',';
			std::cerr << (int) best_move.y() << ',';
			std::cerr << (int) best_move.x();
			std::cerr << ')';
		}
		std::cerr << '}';
	}
}  __attribute__((packed));

template<int N>
class TTable {
public:
	TTable<N>() {
		for (ExploredPosition& pos : ttable)
			pos.type = ValueType::UNKWN;
	}

	const ExploredPosition* get(const std::array<int, 9>& board, bool my_turn, const Move& moveGenerator) const {
		const auto fullMoves = (moveGenerator==Move::any);
		const auto mov = fullMoves ? 0 : moveGenerator.yx();

		const auto h0 = pos_hash<0>(board, my_turn, fullMoves, mov);
		const auto h1 = pos_hash<1>(board, my_turn, fullMoves, mov);

		const auto ttable_h0 = ttable[h0%TABLE_SIZE];
		if (ttable_h0.hash == h1 && ttable_h0.my_turn == my_turn
				&& ttable_h0.fullMoves == fullMoves
				&& (fullMoves || ttable_h0.best_move.YX() == mov)) {
			hit++;
			return &ttable[h0%TABLE_SIZE];
		}

		const auto ttable_h1 = ttable[h1%TABLE_SIZE];
		if (ttable_h1.hash == h0 && ttable_h1.my_turn == my_turn
				&& ttable_h1.fullMoves == fullMoves
				&& (fullMoves || ttable_h1.best_move.YX() == mov)) {
			hit++;
			return &ttable[h1%TABLE_SIZE];
		}

		// une position avec un move optimal compatible est optimale
		if (!fullMoves) {
			const ExploredPosition* pos = get(board, my_turn, Move::any);
			if (pos != nullptr && pos->best_move.YX() == mov)
				return pos;
			else
				miss--;
		}

		miss++;
		return nullptr;
	}

	void put(const std::array<int, 9>& board, ExploredPosition& pos) {
		insertions++;

		const auto mov = pos.fullMoves ? 0 : pos.best_move.YX();

		const auto h0 = pos_hash<0>(board, pos.my_turn, pos.fullMoves, mov);
		const auto h1 = pos_hash<1>(board, pos.my_turn, pos.fullMoves, mov);

		const auto addr0 = &ttable[h0%TABLE_SIZE];
		const auto addr1 = &ttable[h1%TABLE_SIZE];

		const auto ttable_h0 = ttable[h0%TABLE_SIZE];
		const bool equals0 = (ttable_h0.hash == h1 && ttable_h0.my_turn == pos.my_turn
							  && ttable_h0.fullMoves == pos.fullMoves && (pos.fullMoves || ttable_h0.best_move == pos.best_move));

		const auto ttable_h1 = ttable[h1%TABLE_SIZE];
		const bool equals1 = (ttable_h1.hash == h0 && ttable_h1.my_turn == pos.my_turn
							  && ttable_h1.fullMoves == pos.fullMoves && (pos.fullMoves || ttable_h1.best_move == pos.best_move));

		ExploredPosition* where = addr0;

		// keep best
		if (equals0) {
			if (ttable_h0.depthBelow > pos.depthBelow)
				return;
			else
				where = addr0;
		}
		else if (equals1) {
			if (ttable_h1.depthBelow >= pos.depthBelow)
				return;
			else
				where = addr1;
		}
		// free space
		else if (ttable_h0.type == ValueType::UNKWN) {
			inUse++;
			where = addr0;
		}
		else if (ttable_h1.type == ValueType::UNKWN) {
			inUse++;
			where = addr1;
		}
		// collision
		else {
			collisions++;
			// smaller tree
			where = (ttable_h0.depthBelow <= ttable_h1.depthBelow) ? addr0 : addr1;
		}

		pos.hash = (where == addr0) ? h1 : h0;
		memcpy(where, &pos, sizeof(ExploredPosition));
	}

	void resetCounters() {
		hit = 0;
		miss = 0;
		insertions = 0;
		collisions = 0;
	}

	float hitRate() {
		return ((float)hit/((float)hit+miss));
	}

	float collisionRate() {
		return ((float)collisions/((float)insertions));
	}

	float useRate() {
		return ((float)inUse/((float)TABLE_SIZE));
	}

	void print() {
		for (unsigned int i = 0; i < ttable.size(); i++) {
			const ExploredPosition& pos = ttable[i];
			if (pos.type != ValueType::UNKWN) {
				std::cerr << "ttable[" << i <<"] = ";
				pos.print();
				std::cerr << ";" << std::endl;
			}
		}
	}

private:
	template<int Hash>
	inline static hash_t pos_hash(const std::array<int, 9>& board, bool my_turn, bool fullMoves, unsigned int mov) {
		const auto h0 = zhash<Hash>(board[0], 0);
		const auto h1 = zhash<Hash>(board[1], 1);
		const auto h2 = zhash<Hash>(board[2], 2);
		const auto h3 = zhash<Hash>(board[3], 3);
		const auto h4 = zhash<Hash>(board[4], 4);
		const auto h5 = zhash<Hash>(board[5], 5);
		const auto h6 = zhash<Hash>(board[6], 6);
		const auto h7 = zhash<Hash>(board[7], 7);
		const auto h8 = zhash<Hash>(board[8], 8);

		const auto hT = zhash_myturn<Hash>(my_turn);
		const auto hF = zhash_fullmoves<Hash>(fullMoves);
		const auto hM = zhash_mov<Hash>(mov); // assuming fullMoves=true => mov=0

		return h0 ^ h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6 ^ h7 ^ h8 ^ hT ^ hF ^ hM;
	}

private:
	std::array<ExploredPosition, N> ttable;
	mutable int hit = 0;
	mutable int miss = 0;
	mutable int insertions = 0;
	mutable int collisions = 0;
	mutable int inUse = 0;
};

#endif // TABLE_H
