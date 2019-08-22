#pragma once

#include <random>

#include "common/board.h"
#include "common/move.h"

class RandomAI {
public:
    RandomAI() : rng(rd()) {
    }

    Move play(Board& board, bool myTurn, const Move& givenMoveGenerator, double timeBudget) {
        board.possibleMoves(moves, givenMoveGenerator);

        return randomMove();
    }

private:
    int countMoves() const {
        int nrMoves = 0;
        for (const MoveValued& mv : moves) {
            if (mv.move == Move::end) {
                break;
            }
            nrMoves++;
        }
        return nrMoves;
    }

    Move randomMove() {
        int nrMoves  = countMoves();

        std::uniform_int_distribution<std::mt19937::result_type> dist(0, nrMoves - 1);
        return moves[dist(rng)].move;
    }

private:
    std::array<MoveValued, 9*9+1> moves;

    std::random_device rd;
    std::mt19937 rng;
};
