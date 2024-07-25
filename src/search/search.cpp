/*
   Astra is a chess engine written in C++
   Copyright (C) 2024 Semih Özalp

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "search.h"

namespace Astra {
    /*
     * Global Constants
     */
    const int DELTA_MARGIN = 400; // used for delta pruning
    const int RAZOR_MARGIN = 129; // used for razor pruning
    const int FUTILITY_MARGIN = 68; // used for futility pruning

    const int DELTA_PIECE_VALUES[] = {114, 281, 297, 512, 936, 0};

    Search::Search(Board &board) : board(board), tt(16), searchedNodes(0), ply(0) {
        pvTable.reset();
        moveOrdering.clear();
    }

    int Search::quiesceSearch(int alpha, int beta) {
        // check if the search should be stopped
        if (timeManager.isTimeExceeded() && timePerMove != 0) {
            return 0;
        }

        bool pvNode = (beta - alpha) != 1;

        // Transposition Table Probing
        U64 hash = board.getHash();
        TTEntry entry;
        bool ttHit = tt.lookup(entry, hash, 0);

        if (ttHit && !pvNode) {
            if (entry.bound == EXACT_BOUND)
                return entry.score;
            if (entry.bound == LOWER_BOUND && entry.score >= beta)
                return entry.score;
            if (entry.bound == UPPER_BOUND && entry.score <= alpha)
                return entry.score;
        }

        Color stm = board.sideToMove();
        bool inCheck = board.inCheck();
        int bestScore = Eval::getEval(board);

        // Alpha-Beta Pruning
        if (bestScore >= beta)
            return bestScore;
        if (bestScore > alpha)
            alpha = bestScore;

        MoveList moves(board);

        // apply move ordering to sort the moves from best to worst
        moveOrdering.sortMoves(board, moves, tt, ply);

        Move bestMove = NULL_MOVE;
        for (Move move: moves) {
            // skip non-capture moves
            if (!isCapture(move))
                continue;

            if (!inCheck) {
                /*
                // Static Exchange Evaluation (SEE)
                if (seeCapture(board, move) < 0) {
                    continue;
                }
                */

                // Delta Pruning
                int captureValue = DELTA_PIECE_VALUES[typeOfPiece(board.pieceAt(move.to()))];

                if (!isPromotion(move) && bestScore + DELTA_MARGIN + captureValue < alpha && board.nonPawnMat(stm))
                    continue;
            }

            // increase the searched nodes
            searchedNodes++;

            // make move and increase ply
            board.makeMove(move);
            ply++;

            int score = -quiesceSearch(-beta, -alpha);

            // undo move and decrease ply
            board.unmakeMove(move);
            ply--;

            // update best score and best move
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;

                if (score > alpha) {
                    alpha = score;

                    // store Transposition Entry
                    if (score >= beta) {
                        tt.store(hash, bestMove, bestScore, 0, LOWER_BOUND);
                        return bestScore;
                    }
                }
            }
        }

        // check for mate and stalemate or draw
        if (moves.size() == 0)
            return inCheck ? -VALUE_MATE + ply : VALUE_DRAW;
        if (board.isDraw())
            return VALUE_DRAW;

        // store Transposition Entry
        if (bestMove != NULL_MOVE) {
            Bound ttBound = pvNode ? EXACT_BOUND : UPPER_BOUND;
            tt.store(hash, bestMove, bestScore, 0, ttBound);
        }

        return bestScore;
    }

    int Search::negamax(int alpha, int beta, int depth) {
        // check if the search should be stopped
        if (timeManager.isTimeExceeded() && timePerMove != 0) {
            return 0;
        }

        bool pvNode = (beta - alpha) != 1;;
        bool inCheck = board.inCheck();
        int bestScore = -VALUE_INFINITE;

        // set local pv length to 0
        pvTable(ply).length = 0;

        // if we reached the maximum depth, do quiescence search
        if (depth <= 0)
            return quiesceSearch(alpha, beta);

        // Transposition Table Probing
        U64 hash = board.getHash();
        TTEntry entry;
        bool ttHit = tt.lookup(entry, hash, depth);

        if (ttHit && !pvNode) {
            if (entry.bound == EXACT_BOUND)
                return entry.score;

            if (entry.bound == LOWER_BOUND)
                alpha = std::max(alpha, entry.score);
            else if (entry.bound == UPPER_BOUND)
                beta = std::min(beta, entry.score);

            if (alpha >= beta)
                return alpha;
        }

        int staticEval;
        if (inCheck)
            staticEval = -VALUE_NONE;
        else
            staticEval = ttHit ? entry.score : Eval::getEval(board);

        // Internal Iterative Deepening
        if (depth >= 3 && !ttHit)
            depth--;

        if (depth <= 0)
            return quiesceSearch(alpha, beta);

        // apply pruning techniques if it is not a pv node, root node and not in check
        if (!pvNode && !inCheck) {
            int score;

            // Razoring
            if (depth < 3 && staticEval + RAZOR_MARGIN < alpha)
                return quiesceSearch(alpha, beta);

            // Null Move Pruning
            if (board.nonPawnMat(board.sideToMove()) && depth >= 3 && staticEval >= beta) {
                int R = 4;

                board.makeNullMove();
                score = -negamax(-beta, -beta + 1, depth - R);
                board.unmakeNullMove();

                if (score >= beta) {
                    // don't return mate scores
                    if (score >= VALUE_MATE - MAX_PLY)
                        score = beta;

                    return score;
                }
            }

            // Mate Distance Pruning

            // check for beta cutoff from a theoretical mate position
            int matingValue = VALUE_MATE - ply;
            if (matingValue < beta) {
                beta = matingValue;
                if (alpha >= matingValue)
                    return matingValue; // Beta Cut-Off
            }

            // check for alpha cutoff from a theoretical mate position
            matingValue = ply - VALUE_MATE;
            if (matingValue > alpha) {
                alpha = matingValue;
                if (beta <= matingValue)
                    return matingValue; // Alpha Cut-Off
            }
        }

        // generate all legal moves
        MoveList moves(board);

        // apply move ordering to sort the moves from best to worst
        moveOrdering.sortMoves(board, moves, tt, ply);

        Move bestMove = NULL_MOVE;

        int quietMoveCount = 0;
        int moveCount = 0;
        for (Move move: moves) {
            bool moveIsCapture = isCapture(move);
            bool moveIsPromotion = isPromotion(move);

            // increase quiet move count if the move is not a capture
            if (!moveIsCapture)
                quietMoveCount++;

            // Apply Pruning Techniques
            if (!moveIsCapture && !moveIsPromotion && !inCheck) {
                // Futility Pruning
                if (depth <= 4 && staticEval + FUTILITY_MARGIN * depth < alpha)
                    continue;

                // Late Move Pruning
                if (depth <= 5 && quietMoveCount > 4 * depth * depth)
                    continue;
            }

            // One Reply Extension
            if (inCheck && moves.size() == 1)
                depth++;

            // increase the searched nodes
            searchedNodes++;

            // make move and increase ply
            board.makeMove(move);
            ply++;

            // score of the current move
            int score;

            // full-depth search for the first move
            if (moveCount == 0) {
                score = -negamax(-beta, -alpha, depth - 1);
            } else {
                // Late Move Reduction (LMR)
                if (!pvNode && moveCount >= 4 && depth >= 3 && !inCheck)
                    score = -negamax(-alpha - 1, -alpha, depth - 2);
                else
                    score = alpha + 1;

                // Principal Variation Search (PVS)
                if (score > alpha) {
                    score = -negamax(-alpha - 1, -alpha, depth - 1);

                    if (score > alpha && score < beta)
                        score = -negamax(-beta, -alpha, depth - 1);
                }
            }

            // undo move and decrease ply
            board.unmakeMove(move);
            ply--;

            // check if the search should be stopped
            if (timeManager.isTimeExceeded() && timePerMove != 0)
                return 0;

            // update best score and best move
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;

                if (score > alpha) {
                    alpha = bestScore;
                    pvTable.updatePV(ply, move);

                    // Beta Cut-off
                    if (score >= beta) {
                        // store Transposition Entry as lower bound
                        tt.store(hash, bestMove, score, std::max(depth, 0), LOWER_BOUND);

                        // update History and Killer Moves (if not a capture)
                        if (!moveIsCapture) {
                            moveOrdering.updateHistory(board, move, depth * depth);
                            moveOrdering.updateKiller(move, ply);
                        }

                        return score;
                    }
                }
            }

            // increase move count
            moveCount++;
        }

        // check for mate and stalemate or draw
        if (moves.size() == 0)
            return board.inCheck() ? -VALUE_MATE + ply : VALUE_DRAW;
        if (board.isDraw())
            return VALUE_DRAW;

        // store Transposition Entry
        if (bestMove != NULL_MOVE) {
            Bound ttBound = pvNode ? EXACT_BOUND : UPPER_BOUND;
            tt.store(hash, bestMove, bestScore, std::max(depth, 0), ttBound);
        }

        // return the best score
        return bestScore;
    }

    int Search::aspirationSearch(int depth, int prevEval) {
        int alpha = -VALUE_INFINITE;
        int beta = VALUE_INFINITE;
        int aspWindow = 30;

        // only do aspiration search if the depth is greater or equal to 9
        if (depth >= 9) {
            alpha = prevEval - aspWindow;
            beta = prevEval + aspWindow;
        }

        int value;
        while (true) {
            if (alpha < -3500)
                alpha = -VALUE_INFINITE;
            if (beta > 3500)
                beta = VALUE_INFINITE;

            value = negamax(alpha, beta, depth);

            // adjust alpha, beta and window
            if (value <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(alpha - aspWindow, (int) -VALUE_INFINITE);
                aspWindow += aspWindow / 2;
            } else if (value >= beta) {
                beta = std::min(beta + aspWindow, (int) VALUE_INFINITE);
                aspWindow += aspWindow / 2;
            } else {
                break;
            }
        }

        return value;
    }

    // time per move in ms
    Move Search::findBestMove(unsigned int timePerMove) {
        this->timePerMove = timePerMove;

        // set total time allowed for a game (in ms)
        timeManager.setTimePerMove(timePerMove);

        int prevEval = 0;

        // Iterative Deepening:
        for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
            timeManager.start();

            // reset the pv table
            pvTable.reset();

            // do search
            //int score = aspirationSearch(depth, prevEval);
            int score = aspirationSearch(depth, prevEval);

            // DEBUG: print search info
            std::cout << "info depth " << depth
                    << " nodes " << searchedNodes
                    << " score cp " << score
                    << " pv " << pvTable(0)(0) << std::endl;

            // check if the search should be stopped
            if (timeManager.isTimeExceeded() && timePerMove != 0) {
                if (pvTable(0)(0) == NULL_MOVE) {
                    std::cerr << "info: No move found!" << std::endl;
                    exit(1);
                }

                break;
            }

            prevEval = score;
        }

        std::cout << std::endl;
        // return the best move
        return pvTable(0)(0);
    }

    void Search::printPv(int depth) {
        std::cout << "PV: ";

        // print the PVLine at the given depth
        for (int i = 0; i < depth; ++i)
            std::cout << pvTable(ply)(i) << " ";

        std::cout << std::endl;
    }
} // namespace Astra
