#pragma once

#include <array>
#include <random>
#include <algorithm>

#include <cstdint>

#include "ttt.h"
#include "score.h"

using hash_t = std::uint32_t;

std::mt19937 rd;
std::uniform_int_distribution<hash_t> dist;

/** This is a class to generate automatically a high quality hash function.
  * The hash are randomly generated and stored.
  * The values must be between 0 and COUNT-1
  * It is good for small COUNT, but cache misses become an issue for big COUNT
  */
template<typename T, int COUNT>
class ZobristHasher {
public:
	ZobristHasher() {
		std::generate(_hash.begin(), _hash.end(), [&]() { return dist(rd); });
	}

	hash_t hash(T t = 0) const {
		return _hash[t];
	}

private:
	std::array<hash_t, COUNT> _hash;
};
