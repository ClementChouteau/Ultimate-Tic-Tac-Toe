#pragma once

#include <random>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cmath>

#include "common/board.h"
#include "common/move.h"

#define UCB_C (1)

typedef struct MCTSNode {
    Board board;
    bool myTurn;
    Move moveGenerator;

    int nrWin;
    int nrLoss;
    int nrDraw;
    int nrGames;

    std::unordered_map<Move, std::unique_ptr<MCTSNode>> sons;
    MCTSNode* parent;

    float UCB() const {
        int N = (parent != null) ? parent->nrGames : nrGames;

        return (float) nrWin / (float) nrGames + UCB_C * std::sqrt(std::log(N) / nrGames);
    }
};

bool operator<(const MCTSNode& a, const MCTSNode& b) {
    return a.UCB() < b.UCB();
}

bool operator<=(const MCTSNode& a, const MCTSNode& b) {
    return a.UCB() <= b.UCB();
}

class MCTSBasedAI {
public:
    MCTSBasedAI() {
    }

    Move play(Board& board, bool myTurn, const Move& givenMoveGenerator, double timeBudget) {
        this->timeBudget = timeBudget;

        movesGenerator[0] = givenMoveGenerator;

        exploredPositions = 0;
        MoveValued bestMoveValued = {Move::end, -1};

        MCTSNode root = {
            std::copy(board),
            myTurn,
            std::copy(givenMoveGenerator),
            0,
            0,
            0,
            0,
            std::unordered_map<Move, std::unique_ptr<MCTSNode>>()
        };

        std::priority_queue<MCTSNode*> nodesPQ;
    }

    // node is supposed to be non terminal
    void expand(MCTSNode& node) {
        //TODO à terminer pour clem120

        //TODO attention, peut on vraiment expand le node ?
    }

private:
    std::array<Move, MAX_DEPTH+1> movesGenerator; // move of the previous level
    std::array<std::array<MoveValued, 9*9+1>, MAX_DEPTH+1> moves;

    double timeBudget;
	std::chrono::time_point<std::chrono::steady_clock> start;

    int previousExploredPositions;
    int exploredPositions;


}
