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

#include "moveordering.h"

namespace Astra {

    /*
     * Static exchange evaluation (SEE)
     */
    U64 smallestAttacker(Board &board, Square s) {
        Color c = board.sideToMove();

        U64 attackers;
        for (PieceType pt: {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
            if (pt == PAWN) {
                attackers = pawnAttacks(~c, s) & board.pieceBitboard(c, PAWN);
            } else {
                U64 pieces = board.occupancy(WHITE) | board.occupancy(BLACK);
                attackers = getAttacks(pt, s, pieces) & board.pieceBitboard(c, pt);
            }

            if (attackers) return attackers;
        }

        return 0;
    }

    int see(Board &board, Color c, Square s) {
        int value = 0;
        U64 attackers = smallestAttacker(board, s);

        if (attackers) {
            Square attackerFrom = popLsb(attackers);
            PieceType capturedPiece = typeOfPiece(board.pieceAt(s));
            Move move = Move(attackerFrom, s, CAPTURE);

            board.makeMove(move);
            value = pieceValues[capturedPiece] - see(board, ~c, s);
            // do not consider captures if they lose material, therefor max zero
            value = std::max(0, value);
            board.unmakeMove(move);
        }

        return value;
    }

    int seeCapture(Board &board, Move &captureMove) {
        assert(isCapture(captureMove));

        PieceType to = typeOfPiece(board.pieceAt(captureMove.to()));

        board.makeMove(captureMove);
        int score = pieceValues[to] - see(board, ~board.sideToMove(), captureMove.to());
        board.unmakeMove(captureMove);

        return score;
    }

    /*
     * Most Valuable Victim / Least Valuable Attacker
     */
    constexpr int mvvlvaTable[7][7] = {
            {205, 204, 203, 202, 201, 200, 0},
            {305, 304, 303, 302, 301, 300, 0},
            {405, 404, 403, 402, 401, 400, 0},
            {505, 504, 503, 502, 501, 500, 0},
            {605, 604, 603, 602, 601, 600, 0},
            {705, 704, 703, 702, 701, 700, 0}
    };

    int mvvlva(Board &board, Move &move) {
        const int attacker = typeOfPiece(board.pieceAt(move.from()));
        const int victim = typeOfPiece(board.pieceAt(move.to()));
        return mvvlvaTable[victim][attacker];
    }

    /*
     * Move Ordering
     */
    MoveOrdering::MoveOrdering() {
        clear();
    }

    void MoveOrdering::clear() {
        for (int i = 0; i < MAX_PLY; ++i) {
            killer1[i] = NULL_MOVE;
            killer2[i] = NULL_MOVE;
        }

        for (auto &i: history) {
            for (auto &j: i) {
                for (int &k: j) k = 0;
            }
        }
    }

    void MoveOrdering::updateHistory(Board &board, Move &move, int score) {
        // check if move is a capture, return error if it is
        assert(!isCapture(move));

        history[board.sideToMove()][move.from()][move.to()] += score;
    }

    int MoveOrdering::getHistoryScore(Board &board, Move &move) const {
        return history[board.sideToMove()][move.from()][move.to()];
    }

    void MoveOrdering::updateKiller(Move &move, int ply) {
        // check if move is a capture, return error if it is
        assert(!isCapture(move));

        killer2[ply] = killer1[ply];
        killer1[ply] = move;
    }

    void MoveOrdering::sortMoves(Board &board, MoveList &moves, TTable& tt, int ply) {
        TTEntry entry;
        bool ttHit = tt.lookup(entry, board.getHash(), 0);

        std::vector<int> scores(moves.size(), 0);

        int moveCount = 0;
        for (Move move : moves) {
            if (ttHit && move == entry.move) {
                scores[moveCount] = TT_SCORE;
            } if (isCapture(move)) {
                const int seeScore = seeCapture(board, move);
                const int mvvlvaScore = mvvlva(board, move);
                scores[moveCount] = seeScore >= 0 ? CAPTURE_SCORE + mvvlvaScore : mvvlvaScore;
            } else if (move == killer1[ply]) {
                scores[moveCount] = KILLER_ONE_SCORE;
            } else if (move == killer2[ply]) {
                scores[moveCount] = KILLER_TWO_SCORE;
            } else {
                scores[moveCount] = getHistoryScore(board, move);
            }

            moveCount++;
        }

        // Bubble sort moves based on their scores
        int n = moves.size();
        bool swapped;

        do {
            swapped = false;
            for (int i = 0; i < n - 1; ++i) {
                if (scores[i] < scores[i + 1]) {
                    int tempScore = scores[i];
                    scores[i] = scores[i + 1];
                    scores[i + 1] = tempScore;

                    Move tempMove = moves[i];
                    moves[i] = moves[i + 1];
                    moves[i + 1] = tempMove;

                    swapped = true;
                }
            }

            n--;
        } while (swapped);
    }

} // namespace Astra
