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
#include "movegen.h"

namespace Chess {

    struct StateInfo {
        U64 hash;
        Piece movedPiece;
        Move move;
        Piece capturedPiece;
        Square epSquare;
        /* bitboard of squares on which pieces have either moved from, or have been moved to.
         * Used for castling legality checks
         */ U64 entry;
        int halfMoveClock;

        StateInfo() : hash(0), movedPiece(NO_PIECE), move(NULL_MOVE), capturedPiece(NO_PIECE),
                      epSquare(NO_SQUARE), entry(0), halfMoveClock(0) {}

        StateInfo(const StateInfo &prev) {
            hash = prev.hash;
            movedPiece = prev.movedPiece;
            move = prev.move;
            entry = prev.entry;
            capturedPiece = NO_PIECE;
            epSquare = NO_SQUARE;
            halfMoveClock = prev.halfMoveClock;
        }
    };

    class Board {
    public:
        StateInfo history[MAX_PLY * 2];
        // bitboard of enemy pieces that are currently attacking the king, updated in genLegalMoves()
        U64 checkers;
        // bitboard of pieces that are currently pinned to the king by enemy sliders, updated in genLegalMoves()
        U64 pinned;

        Board(const std::string &fen);

        void print(Color c);
        void pgn();

        std::string getFen() const;

        U64 getPieceBB(Color c, PieceType pt) const;
        Piece getPiece(Square s) const;
        Color getTurn() const;
        int getPly() const;
        U64 getHash() const;

        Square getKingSquare(Color c) const;
        U64 getDiagSliders(Color c) const;
        U64 getOrthSliders(Color c) const;
        U64 getOccupancy(Color c) const;
        U64 getAttackers(Color c, Square s, U64 occ) const;
        bool inCheck() const;

        void makeMove(Move move);
        void undoMove();
        void makeNullMove();
        void undoNullMove();

        int genLegalMoves(Move *moves);

        bool isThreefold() const;
        bool isInsufficientMaterial() const;
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

    // returns the bitboard of a given piece
    inline U64 Board::getPieceBB(Color c, PieceType pt) const {
        return pieceBB[makePiece(c, pt)];
    }

    // returns the piece on a given square
    inline Piece Board::getPiece(Square s) const {
        return board[s];
    }

    inline Color Board::getTurn() const {
        return stm;
    }

    inline int Board::getPly() const {
        return gamePly;
    }

    inline U64 Board::getHash() const {
        return hash;
    }

    inline Square Board::getKingSquare(Color c) const {
        return bsf(getPieceBB(c, KING));
    }

} // namespace Chess


#endif //ASTRA_BOARD_H
