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
                attackers = pawnAttacks(~c, s) & board.getPieceBB(c, PAWN);
            } else {
                U64 pieces = board.getOccupancy(WHITE) | board.getOccupancy(BLACK);
                attackers = getAttacks(pt, s, pieces) & board.getPieceBB(c, pt);
            }

            if (attackers) {
                return attackers;
            }
        }

        return 0;
    }

    int see(Board &board, Color c, Square s) {
        int value = 0;
        U64 attackers = smallestAttacker(board, s);

        if (attackers) {
            Square attackerFrom = popLsb(&attackers);
            PieceType capturedPiece = typeOfPiece(board.getPiece(s));
            Move move = Move(attackerFrom, s, CAPTURE);

            board.makeMove<false>(move);
            value = pieceValues[capturedPiece] - see(board, ~c, s);
            // do not consider captures if they lose material, therefor max zero
            value = std::max(0, value);
            board.unmakeMove(move);
        }

        return value;
    }

    int seeCapture(Board &board, Move &captureMove) {
        PieceType to = typeOfPiece(board.getPiece(captureMove.to()));

        board.makeMove<false>(captureMove);
        int score = pieceValues[to] - see(board, ~board.sideToMove(), captureMove.to());
        board.unmakeMove(captureMove);

        return score;
    }

    /*
     * Most Valuable Victim / Least Valuable Attacker
     */
    constexpr int mvvlvaArray[8][8] = {
            {0, 0,   0,   0,   0,   0,   0,   0},
            {0, 205, 204, 203, 202, 201, 200, 0},
            {0, 305, 304, 303, 302, 301, 300, 0},
            {0, 405, 404, 403, 402, 401, 400, 0},
            {0, 505, 504, 503, 502, 501, 500, 0},
            {0, 605, 604, 603, 602, 601, 600, 0},
            {0, 705, 704, 703, 702, 701, 700, 0}
    };

    int mvvlva(Board &board, Move move) {
        int attacker = board.getPiece(move.from()) + 1;
        int victim = board.getPiece(move.to()) + 1;
        return mvvlvaArray[victim][attacker];
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
                for (int &k: j) { k = 0; }
            }
        }
    }

    void MoveOrdering::updateHistory(Board &board, Move &move, int score) {
        // check if move is a capture, return error if it is
        assert(!isCapture(move));

        history[board.sideToMove()][move.from()][move.to()] += score;
    }

    int MoveOrdering::getHistoryScore(Board &board, Move &move) {
        return history[board.sideToMove()][move.from()][move.to()];
    }

    void MoveOrdering::updateKiller(Move &move, int ply) {
        // check if move is a capture, return error if it is
        assert(!isCapture(move));

        if (!Move::isSame(move, killer1[ply])) {
            killer2[ply] = killer1[ply];
            killer1[ply] = move;
        }
    }

} // namespace Astra
