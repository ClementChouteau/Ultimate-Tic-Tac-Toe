#pragma once

#include "types.h"
#include "ttt.h"

#include <bitset>

class PrecomputedWin {
public:
	PrecomputedWin() {
		for (ttt_t ttt = 0; ttt < NUMBER_OF_TTT; ttt++) {
			_isWon[2*ttt + PLAYER_0-1] = _win(ttt, PLAYER_0);
			_isWon[2*ttt + PLAYER_1-1] = _win(ttt, PLAYER_1);
		}
	}

	inline bool isWon(ttt_t ttt, player_t player) const {
		return _isWon[2*ttt + player-1];
	}

private:
    bool _win(ttt_t ttt, player_t player) const {
    	for (const std::tuple<int, int, int>& line : ttt_possible_lines) {
    		const auto c0 = get_ttt_int(ttt, std::get<0>(line));
    		const auto c1 = get_ttt_int(ttt, std::get<1>(line));
    		const auto c2 = get_ttt_int(ttt, std::get<2>(line));

    		if (c0 == player && c1 == player && c2 == player)
    			return true;
    	}

    	return false;
    }

private:
	std::bitset<2 * NUMBER_OF_TTT> _isWon;
};
