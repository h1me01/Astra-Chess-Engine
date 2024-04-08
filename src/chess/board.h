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

#ifndef ASTRA_BOARD_H
#define ASTRA_BOARD_H

#include "zobrist.h"
#include "attacks.h"

namespace Chess {

    struct StateInfo {
        U64 hash;
        Piece captured;
        Square epSquare;
        U64 castleMask;
        int halfMoveClock;

        StateInfo() : hash(0), captured(NO_PIECE), epSquare(NO_SQUARE), castleMask(0), halfMoveClock(0) {}

        StateInfo(const StateInfo &prev) {
            hash = prev.hash;
            captured = NO_PIECE;
            epSquare = NO_SQUARE;
            castleMask = prev.castleMask;
            halfMoveClock = prev.halfMoveClock;
        }
    };

    class Board {
    public:
        StateInfo history[MAX_PLY * 2];
        // contains squares of enemy pieces that check our king
        U64 checkers;
        // contains squares of our pieces that are pinned
        U64 pinned;
        // contains potential danger squares for our king
        U64 danger;
        // contains all the possible capture squares
        U64 captureMask;
        // contains all the possible squares that are not a capture
        U64 quietMask;

        Board(const std::string &fen);

        void print(Color c);

        std::string fen() const;
        U64 pieceBitboard(Color c, PieceType pt) const { return pieceBB[makePiece(c, pt)]; }
        Piece pieceAt(Square s) const { return board[s]; }
        Color sideToMove() const { return stm; }
        int ply() const { return gamePly; }
        U64 getHash() const { return hash; }
        Square kingSquare(Color c) const;
        U64 occupancy(Color c) const;
        U64 isAttacked(Color c, Square s, U64 occ) const;

        bool inCheck() const;
        bool nonPawnMat(Color c) const;

        U64 diagSliders(Color c) const;
        U64 orthSliders(Color c) const;

        void makeMove(const Move &move);
        void unmakeMove(const Move &move);

        void makeNullMove();
        void unmakeNullMove();

        bool isThreefold() const;
        bool isInsufficientMat() const;
        bool isDraw() const;

    private:
        U64 pieceBB[NUM_PIECES];
        Piece board[NUM_SQUARES];
        Color stm;
        int gamePly;
        U64 hash;

        void putPiece(Piece pc, Square s);
        void removePiece(Square s);
        void movePiece(Square from, Square to);
    };

    inline Square Board::kingSquare(Color c) const {
        return bsf(pieceBitboard(c, KING));
    }

} // namespace Chess


#endif //ASTRA_BOARD_H
