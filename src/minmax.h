#pragma once

#include <array>
#include <algorithm>
#include <iostream>
#include <chrono>

#include "common/move.h"
#include "common/board.h"
#include "score.h"
#include "transposition_table.h"

#define MIN_DEPTH (1)
#define MAX_DEPTH (81)

#define TIME_CHECK_EVERY_N_POSITIONS (30000)

#define TABLE_CUTOFF (2)

template<int TableSize>
class MinMaxBasedAI {
public:
    MinMaxBasedAI(const Scoring& scoring) : scoring(scoring) {        
    }

    Move play(Board& board, player_t startingPlayer, const Move& givenMoveGenerator, double timeBudget) {
        start = std::chrono::steady_clock::now();
        this->timeBudget = timeBudget/1000.;
        movesGenerator[0] = givenMoveGenerator;

        exploredPositions = 0;
        MoveValued bestMoveValued = {Move::end, -1};
        maxDepth = MIN_DEPTH;
        try {
            iterativeDeepening(bestMoveValued, board, startingPlayer);
        }
        // last deepening was aborted
        catch (int) {
            maxDepth--;
            std::cerr << "aborting next deepening" << std::endl;
        }

        const auto dt = elapsedInMs();
        std::cerr << std::fixed
            << "score: " << decodeDraw(scoring.score(board)) << ", best: " << bestMoveValued.value << ", elapsed : " << dt << " ms" << ", positions: " << exploredPositions << ", positions/s: " << exploredPositions/dt << std::endl
            << "choice D" << maxDepth << " (Y, X, y, x): " << bestMoveValued.move.Y() << ' ' << bestMoveValued.move.X() << ' ' << bestMoveValued.move.y() << ' ' << bestMoveValued.move.x() << std::endl
            << std::endl;

        return bestMoveValued.move;
    }

private:
    double elapsedInMs() const {
        const auto now = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration <double, std::ratio<1>> (now - start).count();
        return dt;
    }

    bool timeBudgetExceeded() const {
        return elapsedInMs() >= timeBudget;
    }

    void iterativeDeepening(MoveValued& best, Board& board, player_t startingPlayer) {
        // while we don't have a win/loss
        while (!isDraw(best.value) && std::abs(best.value) < GLOBAL_VICTORY0_SCORE-MAX_DEPTH) {
            previousExploredPositions = exploredPositions;

            best = minmax(board, 0, maxDepth, startingPlayer, MIN_NEGATABLE_SCORE, MAX_NEGATABLE_SCORE);

            printStatistics();

            maxDepth++; // explore one level deeper
        }
    }

    void printStatistics() {
        const auto& counters = ttable.counters;
        auto hitRatio = (counters.get != 0 ? ((double)counters.hit/counters.get) : 1) * 100.;
        auto missRatio = (counters.get != 0 ? ((double)counters.miss/counters.get) : 1) * 100.;
        auto collisionsRatio = (counters.get != 0 ? ((double)counters.collisions/counters.get) : 0) * 100.;
        auto usageRatio = (counters.capacity != 0 ? ((double)counters.count/counters.capacity) : 1) * 100.;
        
        std::cerr << std::setprecision(3)
            << 'D' << maxDepth << " cost: " << (exploredPositions - previousExploredPositions)
            << ", hit%: " << hitRatio
            << ", miss%: " << missRatio
            << ", collisions%: " << collisionsRatio
            << ", use%: " << usageRatio << std::endl;
    }

    MoveValued minmax(Board& board, int depth, int maxDepth, player_t player, score_t A, score_t B) {
        exploredPositions++;

        if (exploredPositions % TIME_CHECK_EVERY_N_POSITIONS == 0) {
            if (timeBudgetExceeded()) {
                throw 0;
            }
        }

        ExploredPositionType type = ExploredPositionType::UPPER;
        MoveValued best = {Move::end, -GLOBAL_VICTORY0_SCORE-1};

        if (board.winner() != Owner::None || depth == maxDepth) {
            const auto score = scoring.score(board);

            if (isDraw(score))
                best.value = score;
            else
                best.value = ((player == Owner::Player0) ? 1 : -1) * score;

            return best; // no need to save this position
        }
        else {
            // try to find current position in transposition table
            const ExploredPosition* pos = (maxDepth - depth >= TABLE_CUTOFF)
                ? ttable.get(board.getBoard(), player, movesGenerator[depth])
                : nullptr;
            
            MoveValued hashMove = {Move::end, -1};
            if (pos != nullptr) {
                // saved move heuristic
                hashMove.move = pos->bestMove;
                hashMove.value = pos->value;

                // hash move existence confirmed
                if (board.isValidMove(movesGenerator[depth], hashMove.move)) {
                    // stored result is relevant
                    if ((maxDepth - depth) <= pos->depthBelow) {

                        if (pos->type == ExploredPositionType::EXACT)
                            return hashMove;

                        else if (pos->type == ExploredPositionType::LOWER) {
                            if (decodeDraw(hashMove.value) > decodeDraw(best.value)) {
                                best = hashMove;

                                if (decodeDraw(best.value) > decodeDraw(A)) {
                                    type = ExploredPositionType::EXACT;
                                    A = best.value;
                                    if (decodeDraw(A) >= decodeDraw(B)) { // alpha beta pruning
                                        type = ExploredPositionType::LOWER;
                                        goto return_pos;
                                    }
                                }
                            }
                        }
                    }
                }
                // this should not happen
                else {
                    std::cerr << "A TRANSPOSITION TABLE COLLISION MADE IT RETURN AN IMPOSSIBLE MOVE" << std::endl;
                    pos = nullptr;
                }
            }
            // generate moves
            board.possibleMoves(moves[depth], movesGenerator[depth]);

            // order moves
            bool found = false;
            int nbMoves = 0;
            for (MoveValued& mv : moves[depth]) {
                if (mv.move == Move::end) {
                    break;
                }
                nbMoves++;

                // there was a hashmove corresponding to the current position and we found it in the possible moves
                if (pos != nullptr && mv.move == hashMove.move) {
                    found = true;
                    std::swap(moves[depth][0].move, mv.move); // relevant hashmove is checked first
                }

                // compute heuristic value for move ordering
                else {
                    const auto ttt1 = board.get_ttt(mv.move.Y(), mv.move.X());
                    auto ttt2 = ttt1;
                    set_ttt_int(ttt2, mv.move.y(), mv.move.x(), player);
                    mv.value = ((player == Owner::Player0) ? 1 : -1) * scoring.score(ttt2, player);
                }
            }

            std::sort(moves[depth].begin() + (found ? 1 : 0), moves[depth].begin() + nbMoves,
                [](const MoveValued& m1, const MoveValued& m2){ return m1.value > m2.value; });

            // for every possible move
            for (const MoveValued& mv : moves[depth]) {
                if (mv.move == Move::end) break;
                if (mv.move == Move::skip) continue;

                board.action(mv.move, player);

                movesGenerator[depth+1] = board.isWonOrFull_d(mv.move.j%9) ? Move::any : mv.move;

                MoveValued current;
                try {
                    current = minmax(board, depth+1, maxDepth, OTHER(player), -B, -A);
                }
                catch (int) { board.cancel(); throw; }

                board.cancel();

                if (!isDraw(current.value)) {
                    current.value *= -1; // negamax
                }

                if (decodeDraw(current.value) > decodeDraw(best.value)) {
                    best.value = current.value;
                    best.move = mv.move;

                    if (decodeDraw(best.value) > decodeDraw(A)) {
                        type = ExploredPositionType::EXACT;
                        A = best.value;

                        if (decodeDraw(A) >= decodeDraw(B)) { // alpha beta pruning
                            type = ExploredPositionType::LOWER;
                            break;
                        }
                    }
                }
            }
        }
        return_pos:

        // save position in transposition table
        if (type != ExploredPositionType::UPPER && (maxDepth - depth) >= TABLE_CUTOFF) {
            ExploredPosition pos;
            pos.type = type;
            pos.depthBelow = maxDepth - depth;
            pos.fullMoves = (movesGenerator[depth] == Move::any);
            pos.bestMove = best.move.j;
            pos.player = encodePlayerAsBool(player);
            pos.value = A;

            ttable.put(board.getBoard(), pos);
        }

        return best;
    }

private:
    TranspositionTable<TableSize> ttable;
    const Scoring& scoring;

    // these are used to avoid allocations for the moves to explore
    std::array<Move, MAX_DEPTH+1> movesGenerator; // move of the previous level
    std::array<std::array<MoveValued, 9*9+1>, MAX_DEPTH+1> moves;

    double timeBudget;
	std::chrono::time_point<std::chrono::steady_clock> start;

    int maxDepth;

    int previousExploredPositions;
    int exploredPositions;
};
