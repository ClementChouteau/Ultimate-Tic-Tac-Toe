#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <ostream>

#include "wyhash/wyhash.h"

#include "explored_position.h"
#include "zobrist.h"
#include "common/move.h"

struct Hashers {
	ZobristHasher<bool, 1> seed;
	ZobristHasher<bool, 2> player;
	ZobristHasher<bool, 2> fullMoves;
	ZobristHasher<unsigned int, 2*9> move;
};

struct TranspositionTableCounters {
	long capacity = 0; /// physical size of the table
	long count = 0; /// non empty entries
	long hit = 0; /// number of get() successful
	long miss = 0; /// number of get() failed
	long get = 0; /// number of get() called
	long put = 0; /// number of put() called
	long collisions = 0; /// number of collisions that lead to an eviction
};

/** This is a table to store results of exploration.
  * Two different hash are used per position :
  * - the index of the entry in the table can be any one of the two hash.
  * - the hash stored is the other one (hence the name otherHash).
  */

template<int CAPACITY>
class TranspositionTable {
public:
	TranspositionTable<CAPACITY>() : positions(CAPACITY) {
		counters.capacity = CAPACITY;

		for (ExploredPosition& pos : positions)
			pos.type = ExploredPositionType::UNKWN;
	}

	const ExploredPosition* get(const std::array<ttt_t, 9>& board, player_t player, const Move& moveGenerator) const {
		counters.get++;
		
		const auto fullMoves = (moveGenerator==Move::any);
		const auto mov = fullMoves ? 0 : moveGenerator.yx();

		const auto h0 = pos_hash<0>(board, encodePlayerAsBool(player), fullMoves, mov);
		const auto h1 = pos_hash<1>(board, encodePlayerAsBool(player), fullMoves, mov);

		{
      const auto* ptr0 = &positions[h0%CAPACITY];
			const auto& p0 = *ptr0;
			if (p0.otherHash == h1 && p0.player == encodePlayerAsBool(player)
					&& p0.fullMoves == fullMoves
					&& (fullMoves || Move(p0.bestMove).YX() == mov)) {
				counters.hit++;
				return ptr0;
			}
		}
		{
      const auto* ptr1 = &positions[h1%CAPACITY];
			const auto& p1 = *ptr1;
			if (p1.otherHash == h0 && p1.player == encodePlayerAsBool(player)
					&& p1.fullMoves == fullMoves
					&& (fullMoves || Move(p1.bestMove).YX() == mov)) {
				counters.hit++;
				return ptr1;
			}
		}

		counters.miss++;
		return nullptr;
	}

	void put(const std::array<ttt_t, 9>& board, ExploredPosition& pos) {
		counters.put++;

		const auto mov = pos.fullMoves ? 0 : Move(pos.bestMove).YX();

		const auto h0 = pos_hash<0>(board, pos.player, pos.fullMoves, mov);
		const auto h1 = pos_hash<1>(board, pos.player, pos.fullMoves, mov);

		auto* ptr0 = &positions[h0%CAPACITY];
		auto* ptr1 = &positions[h1%CAPACITY];

		const bool equals0 = (ptr0->otherHash == h1 && ptr0->player == pos.player
							  && ptr0->fullMoves == pos.fullMoves
							  && (pos.fullMoves || ptr0->bestMove == pos.bestMove));

		const bool equals1 = (ptr1->otherHash == h0 && ptr1->player == pos.player
							  && ptr1->fullMoves == pos.fullMoves 
							  && (pos.fullMoves || ptr1->bestMove == pos.bestMove));

		auto* bucket = collision_resolution(pos, ptr0, ptr1, equals0, equals1);
		if (bucket == nullptr) // already there (or better)
			return;

		pos.otherHash = (bucket == ptr0) ? h1 : h0;
		*bucket = pos;
	}

	template<int _CAPACITY>
	friend std::ostream& operator<<(std::ostream& os, const TranspositionTable<_CAPACITY>& that);

private:
	/** Returns the address of a position available in the table, that can be either :
	  * - unused, of type ExploredPositionType::UNKWN
	  * - nullptr if a better position (more deeply searched) is already in the table
	  */
	ExploredPosition* collision_resolution(const ExploredPosition& pos, ExploredPosition* ptr0, ExploredPosition* ptr1, bool equals0, bool equals1) {
		// keep best or overwrite
		if (equals0) {
			if (ptr0->depthBelow > pos.depthBelow)
				return nullptr;
			else
				return ptr0;
		}
		else if (equals1) {
			if (ptr1->depthBelow >= pos.depthBelow)
				return nullptr;
			else
				return ptr1;
		}
		// free space
		else if (ptr0->type == ExploredPositionType::UNKWN) {
			counters.count++;
			return ptr0;
		}
		else if (ptr1->type == ExploredPositionType::UNKWN) {
			counters.count++;
			return ptr1;
		}

		// overwrite unrelated position, choose smaller tree
		counters.collisions++;
		return (ptr0->depthBelow <= ptr1->depthBelow) ? ptr0 : ptr1;
	}

	template<int Hash>
	inline hash_t pos_hash(const std::array<ttt_t, 9>& board, bool player, bool fullMoves, unsigned int move) const {
		return (hash_t) wyhash(board.data(), board.size() * sizeof(ttt_t), hashers[Hash].seed.hash())
			^ hashers[Hash].player.hash(player)
			^ hashers[Hash].fullMoves.hash(fullMoves)
			^ hashers[Hash].move.hash(move);
	}

private:
	std::vector<ExploredPosition> positions;

	std::array<Hashers, 2> hashers;

public:
	mutable TranspositionTableCounters counters;
};


// output utilities (for debug)
template<int CAPACITY>
std::ostream& operator<<(std::ostream& os, const TranspositionTable<CAPACITY>& that) {
	for (unsigned int i = 0; i < that.positions.size(); i++) {
		const ExploredPosition& pos = that.positions[i];
		if (pos.type != ExploredPositionType::UNKWN) {
			os << "positions[" << i <<"] = ";
			os << pos;
			os << ";" << std::endl;
		}
	}
	return os;
}
