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
#include "../chess/movegen.h"
#include "../eval/evaluate.h"

namespace Astra {

    /*
     * Global Constants
     */
    const int DELTA_MARGIN = 400; // used for delta pruning
    const int RAZOR_MARGIN = 129; // used for razor pruning
    const int FUTILITY_MARGIN = 68; // used for futility pruning

    const int DELTA_PIECE_VALUES[] = {114, 281, 297, 512, 936, 0, 0};

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

        // Transposition Table Probing
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

        Color stm = board.sideToMove();
        bool inCheck = board.inCheck();

        int originalAlpha = alpha;
        int standPat = Eval::evaluate(board);

        // Alpha-Beta Pruning
        if (standPat >= beta) {
            return beta;
        }

        if (standPat > alpha) {
            alpha = standPat;
        }

        Move moves[MAX_CAPTURE_MOVES];
        int numMoves = genLegalMoves<CAPTURE_MOVES>(board, moves);

        // apply move ordering to sort the moves from best to worst
        moveOrdering.sortMoves<QSEARCH>(board, moves, numMoves, entry.move, ply);

        Move bestMove;
        for (int i = 0; i < numMoves; ++i) {
            Move move = moves[i];

            // Static Exchange Evaluation (SEE)
            if (!inCheck && seeCapture(board, move) < 0) {
                continue;
            }

            // Delta Pruning
            int captureValue = DELTA_PIECE_VALUES[typeOfPiece(board.getPiece(move.to()))];

            if (!isPromotion(move) && !inCheck && standPat + DELTA_MARGIN + captureValue < alpha && board.nonPawnMaterial(stm)) {
                continue;
            }

            // make move and increase ply
            board.makeMove<true>(move);
            ply++;

            int score = -quiesceSearch(-beta, -alpha);

            // undo move and decrease ply
            board.unmakeMove(move);
            ply--;

            // store Transposition Entry
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

        // store Transposition Entry
        if (bestMove != NULL_MOVE) {
            if (alpha > originalAlpha) {
                tt.store(hash, bestMove, alpha, 0, PV_NODE);
            } else {
                tt.store(hash, bestMove, alpha, 0, ALL_NODE);
            }
        }

        return alpha;
    }

    int Search::negamax(int alpha, int beta, int depth) {
        // check if the search should be stopped
        if (timeManager.isTimeExceeded() && timePerMove != 0) {
            return 0;
        }

        bool inCheck = board.inCheck();
        bool pvNode = (beta - alpha) != 1;

        int originalAlpha = alpha;
        int bestScore = -VALUE_MATE;

        // set local pv length to 0
        pvTable(ply).length = 0;

        // if we reached the maximum depth, do quiescence search
        if (depth <= 0) {
            // increase the searched nodes
            searchedNodes++;

            // do a quiescence search
            return Eval::evaluate(board);
        }

        // Transposition Table Probing
        U64 hash = board.getHash();
        TTEntry entry;
        bool ttHit = tt.lookup(entry, hash, depth);

        if (ttHit && !pvNode) {
            if (entry.bound == PV_NODE) {
                return entry.score;
            } else if (entry.bound == CUT_NODE) {
                alpha = std::max(alpha, entry.score);
            } else if (entry.bound == ALL_NODE) {
                beta = std::min(beta, entry.score);
            }

            if (alpha >= beta) {
                return alpha;
            }
        }

        // Check Extension
        if (inCheck) {
            depth++;
        }

        int staticEval;
        if (inCheck) {
            staticEval = -VALUE_NONE;
        } else {
            staticEval = ttHit ? entry.score : Eval::evaluate(board);
        }

        // Internal Iterative Deepening
        if (depth >= 3 && !ttHit) {
            depth--;
        }

        if (depth <= 0) {
            return quiesceSearch(alpha, beta);
        }

        // apply pruning techniques if it is not a PV node and not in check
        if (!pvNode && !inCheck) {
            int score;

            // Razoring
            if (depth < 3 && staticEval + RAZOR_MARGIN < alpha) {
                return quiesceSearch(alpha, beta);
            }

            // Null Move Pruning
            if (!board.nonPawnMaterial(board.sideToMove()) && depth >= 3 &&  staticEval >= beta) {
                int R = 5 + std::min(4, depth / 5) + std::min(3, (staticEval - beta) / 214);

                board.makeNullMove();
                score = -negamax(-beta, -beta + 1, depth - R);
                board.unmakeNullMove();

                if (score >= beta) {
                    // dont return mate scores
                    if (score >= VALUE_MATE - MAX_PLY) {
                        score = beta;
                    }

                    return score;
                }
            }

            // Mate Distance Pruning

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
        int numMoves = genLegalMoves(board, moves);

        // apply move ordering to sort the moves from best to worst
        moveOrdering.sortMoves<NEGAMAX>(board, moves, numMoves, entry.move, ply);

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

            // Apply Pruning Techniques
            if (!pvNode && !moveIsCapture && !moveIsPromotion && !inCheck) {
                // Futility Pruning
                if (depth <= 4 && (staticEval + FUTILITY_MARGIN * depth) < alpha) {
                    continue;
                }

                // Late Move Pruning
                if (depth <= 5 && quietMoveCount > (4 * depth * depth)) {
                    continue;
                }
            }

            // One Reply Extension
            if (inCheck && numMoves == 1) {
                depth++;
            }

            // make move and increase ply
            board.makeMove<true>(move);
            ply++;

            // score of the current move
            int score;

            // full-depth search for the first move
            if (i == 0) {
                score = -negamax(-beta, -alpha, depth - 1);
            } else {
                // Late Move Reduction (LMR)
                if (!pvNode && i >= 4 && ply > 3 && !board.inCheck() && !inCheck && !moveIsCapture && !moveIsPromotion) {
                    score = -negamax(-beta, -alpha, depth - 2);

                    // if the score is greater than alpha, do a full-depth search
                    if (score > alpha) {
                        score = -negamax(-beta, -alpha, depth - 1);
                    }
                } else {
                    // Principal Variation Search (PVS)
                    score = -negamax(-alpha - 1, -alpha, depth - 1);

                    // if the score is greater than alpha, do a full-depth search
                    if (score > alpha && score < beta) {
                        score = -negamax(-beta, -alpha, depth - 1);
                    }
                }
            }

            // undo move and decrease ply
            board.unmakeMove(move);
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

        // store Transposition Entry
        int ttDepth = std::max(depth, 0);
        if (alpha > originalAlpha) {
            tt.store(hash, bestMove, bestScore, ttDepth, PV_NODE);
        } else {
            tt.store(hash, bestMove, bestScore, ttDepth, ALL_NODE);
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

        int value = 0;
        while (true) {
            if (alpha < -3500) {
                alpha = -VALUE_INFINITE;
            }

            if (beta > 3500) {
                beta = VALUE_INFINITE;
            }

            value = negamax(alpha, beta, depth);

            // adjust alpha, beta and aspiration Window
            if (value <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(alpha - aspWindow, -(static_cast<int>(VALUE_INFINITE)));
                aspWindow += aspWindow / 2;
            } else if (value >= beta) {
                beta = std::min(beta + aspWindow, static_cast<int>(VALUE_INFINITE));
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
            int score = aspirationSearch(depth, prevEval);

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
