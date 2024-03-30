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
#include "../eval/evaluate.h"

namespace Astra {

    /*
     * Global Constants
     */
    const int RAZOR_MARGIN = 190; // used for razor pruning
    const int FUTILITY_MARGIN = 68; // used for futility pruning

    /*
     * Search
     */
    Search::Search(Board &board) : board(board), tt(16), searchedNodes(0), ply(0) {
        clearData();
    }

    void Search::clearData() {
        pvTable.reset();
        moveOrdering.clear();
    }

    int Search::quiesceSearch(int alpha, int beta) {
        // check if the search should be stopped
        if (timeManager.isTimeExceeded() && timePerMove != 0) {
            return 0;
        }

        // increase the searched nodes
        searchedNodes++;

        /*
         * Used Variables
         */
        const bool inCheck = board.inCheck();

        int originalAlpha = alpha;
        int standPat = Eval::evaluate(board);

        /*
         * Transposition Table Probing:
         * - ttHit is true if an entry is found
         * - If bound is exact, return entry score
         * - If bound is lower, return beta
         * - If bound is upper, return alpha
         */
        U64 hash = board.getHash();
        TTEntry entry;
        bool ttHit = tt.lookup(entry, hash, 0);

        if (ttHit) {
            if (entry.bound == PV_NODE) {
                return entry.score;
            } else if (entry.bound == CUT_NODE && entry.score >= beta) {
                return entry.score;
            } else if (entry.bound == ALL_NODE && entry.score <= alpha) {
                return entry.score;
            }
        }

        // Alpha-Beta Pruning
        if (standPat >= beta) {
            return beta;
        }

        if (standPat > alpha) {
            alpha = standPat;
        }

        // generate all legal capture moves
        Move moves[MAX_MOVES];
        int numMoves = board.genLegalMoves(moves);

        // check for mate and stalemate or draw
        if (numMoves == 0) {
            return board.inCheck() ? ply - VALUE_MATE : VALUE_DRAW;
        } else if (board.isDraw()) {
            return VALUE_DRAW;
        }

        // apply move ordering to sort the moves from best to worst
        moveOrdering.sortMoves<QSEARCH>(board, tt, moves, numMoves, ply);

        Move bestMove;
        for (int i = 0; i < numMoves; ++i) {
            Move move = moves[i];

            // skip the move if it is not a capture
            if (!isCapture(move)) {
                continue;
            }

            // Static Exchange Evaluation (SEE)
            if (seeCapture(board, move) < 0) {
                continue;
            }

            // make move and increase ply
            board.makeMove<true>(move);
            ply++;

            int score = -quiesceSearch(-beta, -alpha);

            // undo move and decrease ply
            board.unmakeMove();
            ply--;

            // store Transposition Entry as exact or upper bound
            if (score >= beta) {
                tt.store(hash, bestMove, score, ply, CUT_NODE);
                return beta;
            }

            // update best score and best move
            if (score > alpha) {
                alpha = score;
                bestMove = move;
            }
        }

        /*
         * Transposition Table Storage (if best move is found):
         * If alpha has not changed, the search stores the current position as an upper bound.
         * If alpha has changed, the search stores the current position as an exact bound.
         */
        if (bestMove != NULL_MOVE) {
            if (alpha > originalAlpha) {
                tt.store(hash, bestMove, alpha, 0, PV_NODE);
            } else {
                tt.store(hash, bestMove, alpha, 0, ALL_NODE);
            }
        }

        // return the best score
        return alpha;
    }

    int Search::negamax(int alpha, int beta, int depth, bool doNull) {
        // check if the search should be stopped
        if (timeManager.isTimeExceeded() && timePerMove != 0) {
            return 0;
        }

        /*
         * Used Variables
         */
        const bool inCheck = board.inCheck();
        bool pvNode = (beta - alpha) != 1;

        int originalAlpha = alpha;
        int bestScore = -VALUE_MATE;
        int extension = 0;

        // set local pv length to 0
        pvTable(ply).length = 0;

        // if we reached the maximum depth, do quiescence search
        if (depth <= 0) {
            // increase the searched nodes
            searchedNodes++;
            // do a quiescence search
            //return quiesceSearch(alpha, beta);
            return Eval::evaluate(board);
        }

        /*
         * Transposition Table Probing:
         * - ttHit is true if an entry is found
         * - If bound is exact, return entry score
         * - If bound is lower, return beta
         * - If bound is upper, return alpha
         */
        U64 hash = board.getHash();
        TTEntry entry;
        bool ttHit = tt.lookup(entry, hash, depth);

        if (ttHit) {
            if (entry.bound == PV_NODE) {
                return entry.score;
            } else if (entry.bound == CUT_NODE && entry.score >= beta) {
                return entry.score;
            } else if (entry.bound == ALL_NODE && entry.score <= alpha) {
                return entry.score;
            }
        }

        /*
         * Check Extension:
         * if in check, extend the search by 1 ply
         */
        if (inCheck) {
            depth++;
        }

        // if a transposition table entry is found, use its eval
        int staticEval = ttHit ? entry.score : Eval::evaluate(board);

        // apply pruning techniques if it is not a PV node and not in check
        if (!pvNode && !inCheck) {
            int score;

            /*
             * Null Move Pruning:
             * - null move is allowed
             * - depth is greater than or equal to 4
             * - static eval is greater than or equal to the beta value
             * - heavy pieces of White and Black are present
             */
            U64 heavyWhitePieces = board.getPieceBB(WHITE, QUEEN) | board.getPieceBB(WHITE, ROOK);
            U64 heavyBlackPieces = board.getPieceBB(BLACK, QUEEN) | board.getPieceBB(BLACK, ROOK);

            if (doNull && depth >= 4 && staticEval >= beta && heavyWhitePieces && heavyBlackPieces) {
                const int R = depth > 6 ? 4 : 3;

                board.makeNullMove();
                score = negamax(-beta, 1 - beta, depth - R - 1, false);
                board.unmakeNullMove();

                if (score >= beta) {
                    return beta;
                }
            }

            /*
             * Razoring
             */
            score = staticEval + RAZOR_MARGIN;
            int newScore;

            if (score < beta) {
                if (depth == 1) {
                    newScore = quiesceSearch(alpha, beta);
                    return std::max(newScore, score);
                }

                score += RAZOR_MARGIN;

                if (score < beta && depth <= 3) {
                    newScore = quiesceSearch(alpha, beta);
                    if (newScore < beta) {
                        return std::max(newScore, score);
                    }
                }
            }

            /*
             * Internal Iterative Deepening
             */
            if (depth >= 4) {
                depth--;
            }

            /*
             * Mate Distance Pruning
             */
            // check for beta cutoff from a theoretical mate position
            int matingValue = VALUE_MATE - ply;
            if (matingValue < beta) {
                beta = matingValue;
                if (alpha >= matingValue) {
                    return matingValue; // Beta Cut-Off
                }
            }

            // check for alpha cutoff from a theoretical mate position
            matingValue = ply - VALUE_MATE;
            if (matingValue > alpha) {
                alpha = matingValue;
                if (beta <= matingValue) {
                    return matingValue; // Alpha Cut-Off
                }
            }
        }

        // generate all legal moves
        Move moves[MAX_MOVES];
        int numMoves = board.genLegalMoves(moves);

        // apply move ordering to sort the moves from best to worst
        //moveOrdering.sortMoves<NEGAMAX>(board, tt, moves, numMoves, ply);

        int quietMoveCount = 0;

        Move bestMove;
        for (int i = 0; i < numMoves; ++i) {
            Move move = moves[i];

            bool moveIsCapture = isCapture(move);
            bool moveIsPromotion = isPromotion(move);

            // increase quiet move count if the move is not a capture
            if (!moveIsCapture) {
                quietMoveCount++;
            }

            /*
             * Pruning:
             * - node is not a PV node
             * - not in check
             * - move is not a capture move
             * - move is not a promotion move
             */
            if (!pvNode && !moveIsCapture && !moveIsPromotion && !inCheck) {
                /*
                 * Futility Pruning:
                 * - condition above
                 * - current depth is less or equal to 4
                 * - static eval plus futility margin is less than alpha
                 */
                if (depth <= 4 && (staticEval + FUTILITY_MARGIN * depth) < alpha) {
                    continue;
                }

                /*
                 * Late Move Pruning:
                 * - condition above
                 * - current depth is less or equal to 5
                 * - quiet move count is greater than 4 * depth²
                 */
                if (depth <= 5 && quietMoveCount > (4 * depth * depth)) {
                    continue;
                }
            }

            /*
             * One Reply Extensions:
             * only one move is available
             */
            if (inCheck && numMoves == 1) {
                extension = 1;
            }

            // set the new depth
            int newDepth = depth - 1 + extension;

            // make move and increase ply
            board.makeMove<true>(move);
            ply++;

            // score of the current move
            int score;

            // full-depth search for the first move
            if (i == 0) {
                score = -negamax(-beta, -alpha, newDepth, true);
            } else {
                /*
                 * Late Move Reduction (LMR):
                 * - node is not a PV node
                 * - move is not one of the first four moves
                 * - current depth (ply) is greater than 3
                 * - not in check after the move
                 * - not in check before the move
                 * - move is not a capture move
                 * - move is not a promotion move
                 * - no extensions
                 */
                if (!pvNode && i >= 4 && ply > 3 && !board.inCheck() && !inCheck && !moveIsCapture && !moveIsPromotion) {
                    score = -negamax(-beta, -alpha, depth - 2, true);

                    // if the score is greater than alpha, do a full-depth search
                    if (score > alpha) {
                        score = -negamax(-beta, -alpha, newDepth, true);
                    }
                } else {
                    // Principal Variation Search (PVS)
                    score = -negamax(-alpha - 1, -alpha, newDepth, true);

                    // if the score is greater than alpha, do a full-depth search
                    if (score > alpha && score < beta) {
                        score = -negamax(-beta, -alpha, newDepth, true);
                    }
                }
            }

            // undo move and decrease ply
            board.unmakeMove();
            ply--;

            // check if the search should be stopped
            if (timeManager.isTimeExceeded() && timePerMove != 0) {
                return 0;
            }

            // update best score and best move
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;

                // update alpha
                if (bestScore > alpha) {
                    alpha = bestScore;
                }

                // update Principal Variation table if it is a PV node
                if (pvNode) {
                    pvTable.updatePV(ply, move);
                }
            }

            // Beta Cut-off
            if (score >= beta) {
                // store Transposition Entry as lower bound
                int ttDepth = std::max(depth, 0);
                tt.store(hash, bestMove, score, ttDepth, CUT_NODE);

                // update History and Killer Moves (if not a capture)
                if (!moveIsCapture) {
                    moveOrdering.updateHistory(board, move, depth * depth);
                    moveOrdering.updateKiller(move, ply);
                }

                return score;
            }
        }

        // check for mate and stalemate or draw
        if (numMoves == 0) {
            return board.inCheck() ? ply - VALUE_MATE : VALUE_DRAW;
        } else if (board.isDraw()) {
            return VALUE_DRAW;
        }

        // store Transposition Entry as exact or upper bound
        int ttDepth = std::max(depth, 0);
        if (alpha > originalAlpha) {
            tt.store(hash, bestMove, bestScore, ttDepth, PV_NODE);
        } else {
            tt.store(hash, bestMove, bestScore, ttDepth, ALL_NODE);
        }

        // return the best score
        return bestScore;
    }

    int Search::aspirationSearch(int depth, int prevEval, bool doNull) {
        int alpha = -VALUE_INFINITE;
        int beta = VALUE_INFINITE;
        int aspWindow = 30;

        // only do aspiration search if the depth is greater or equal to 9
        if (depth >= 9) {
            alpha = prevEval - aspWindow;
            beta = prevEval + aspWindow;
        }

        int result = 0;
        while (true) {
            if (alpha < -3500) {
                alpha = -VALUE_INFINITE;
            }

            if (beta > 3500) {
                beta = VALUE_INFINITE;
            }

            result = negamax(alpha, beta, depth, true);

            // adjust alpha, beta and aspiration Window
            if (result <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(alpha - aspWindow, -(static_cast<int>(VALUE_INFINITE)));
                aspWindow += aspWindow / 2;
            } else if (result >= beta) {
                beta = std::min(beta + aspWindow, static_cast<int>(VALUE_INFINITE));
                aspWindow += aspWindow / 2;
            } else {
                break;
            }
        }

        return result;
    }

    // time per move in ms
    Move Search::findBestMove(unsigned int timePerMove) {
        clearData();
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
            int score = aspirationSearch(depth, prevEval, true);

            // check if the search should be stopped
            if (timeManager.isTimeExceeded() && timePerMove != 0) {
                if (pvTable(0)(0) == NULL_MOVE) {
                    std::cerr << "info: No move found!" << std::endl;
                    exit(1);
                }

                break;
            }

            // DEBUG: print info
            std::cout << "info depth: " << depth
                      << " pv: " << pvTable(0)(0)
                      << " score: " << score
                      << " nodes: " << searchedNodes << std::endl;

            prevEval = score;
            //printPv(depth);
        }

        std::cout << std::endl;
        // return the best move
        return pvTable(0)(0);
    }

    void Search::printPv(int depth) {
        std::cout << "PV: ";

        // print the PVLine at the given depth
        for (int i = 0; i < depth; ++i) {
            std::cout << pvTable(ply)(i) << " ";
        }

        std::cout << std::endl;
    }


} // namespace Astra
