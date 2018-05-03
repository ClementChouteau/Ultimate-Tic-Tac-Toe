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

#define TABLE_SIZE (2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2)

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
	uint8_t depthBelow:5;
	bool fullMoves:1;
	uint8_t type:2;
	union { // 1 octet
		struct{
			bool my_turn:1;
			uint8_t padding:7;
		} __attribute__((packed));
		Move best_move;
	}__attribute__((packed));

	ExploredPosition() { }

	void print() const {
		std::cerr << '{';
		std::cerr << value << ',';
		std::cerr << std::bitset<8*sizeof(hash_t)>(hash) << ',';
		std::cerr << (int) depthBelow << ',';
		std::cerr << (my_turn ? 1 : 0) << ',';

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

		std::cerr << (int) best_move.Y() << ',';
		std::cerr << (int) best_move.X() << ',';
		std::cerr << (int) best_move.y() << ',';
		std::cerr << (int) best_move.x();
		std::cerr << "}";
	}
}  __attribute__((packed));

template<int N>
class TTable {
public:
	TTable<N>() {
		for (ExploredPosition& pos : ttable)
			pos.type = ValueType::UNKWN;
	}

	const ExploredPosition* get(const std::array<int, 9>& board, bool my_turn, bool fullMoves) {
		const auto h1 = pos_hash<0>(board, my_turn, fullMoves);
		const auto h2 = pos_hash<1>(board, my_turn, fullMoves);

		const auto pos_table1 = ttable[h1%TABLE_SIZE];
		if (pos_table1.hash == h2 && pos_table1.my_turn == my_turn && pos_table1.fullMoves == fullMoves) {
			hit++;
			return &ttable[h1%TABLE_SIZE];
		}

		const auto pos_table2 = ttable[h2%TABLE_SIZE];
		if (pos_table2.hash == h1 && pos_table2.my_turn == my_turn && pos_table2.fullMoves == fullMoves) {
			hit++;
			return &ttable[h2%TABLE_SIZE];
		}

		miss++;
		return nullptr;
	}

	void put(const std::array<int, 9>& board, ExploredPosition& pos) {
		insertions++;

		const auto h0 = pos_hash<0>(board, pos.my_turn, pos.fullMoves);
		const auto h1 = pos_hash<1>(board, pos.my_turn, pos.fullMoves);

		const auto pos_table0 = ttable[h0%TABLE_SIZE];
		const auto pos_table1 = ttable[h1%TABLE_SIZE];
		const auto addr0 = &ttable[h0%TABLE_SIZE];
		const auto addr1 = &ttable[h1%TABLE_SIZE];

		const bool equals0 = (pos_table0.hash == h1 && pos_table0.my_turn == pos.my_turn);
		const bool equals1 = (pos_table1.hash == h0 && pos_table1.my_turn == pos.my_turn);

		ExploredPosition* where;

		// keep best
		if (equals0) {
			if (pos_table0.depthBelow > pos.depthBelow)
				return;
			else
				where = addr0;
		}
		else if (equals1) {
			if (pos_table1.depthBelow >= pos.depthBelow)
				return;
			else
				where = addr1;
		}
		// free space
		else if (pos_table0.type == ValueType::UNKWN) {
			inUse++;
			where = addr0;
		}
		else if (pos_table1.type == ValueType::UNKWN) {
			inUse++;
			where = addr1;
		}
		// collision
		else {
			collisions++;
			// smaller tree
			where = (pos_table0.depthBelow <= pos_table1.depthBelow) ? addr0 : addr1;
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
	inline static hash_t pos_hash(const std::array<int, 9>& board, bool my_turn, bool fullMoves) {
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
		const auto hM = zhash_fullmoves<Hash>(fullMoves);

		return h0 ^ h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6 ^ h7 ^ h8 ^ hT ^ hM;
	}

private:
	std::array<ExploredPosition, N> ttable;
	int hit = 0;
	int miss = 0;
	int insertions = 0;
	int collisions = 0;
	int inUse = 0;
};

#endif // TABLE_H
