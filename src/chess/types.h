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

#ifndef ASTRA_TYPES_H
#define ASTRA_TYPES_H

#include <iostream>
#include <string>
#include <cstdint>
#include <sstream>
#include <cmath>
#include <vector>
#include <assert.h>

namespace Chess {

    using U64 = uint64_t;

    const std::string PIECE_STR = "PNBRQKpnbrqk.";
    const std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    enum Color : int {
        WHITE, BLACK, NUM_COLORS = 2
    };

    enum Direction : int {
        NORTH = 8,
        NORTH_EAST = 9,
        EAST = 1,
        SOUTH_EAST = -7,
        SOUTH = -8,
        SOUTH_WEST = -9,
        WEST = -1,
        NORTH_WEST = 7,
        NORTH_NORTH = 16,
        SOUTH_SOUTH = -16,
    };

    enum PieceType : int {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE_TYPE, NUM_PIECE_TYPES = 6
    };

    constexpr PieceType PIECE_TO_PIECETYPE[13] = {
            PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
            PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
            NO_PIECE_TYPE
    };

    enum Piece : int {
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
        NO_PIECE, NUM_PIECES = 12
    };

    enum Square : int {
        a1, b1, c1, d1, e1, f1, g1, h1,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a8, b8, c8, d8, e8, f8, g8, h8,
        NO_SQUARE, NUM_SQUARES = 64
    };

    inline const char *SQSTR[65] = {
            "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
            "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
            "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
            "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
            "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
            "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
            "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
            "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
            "None"
    };

    enum File : int {
        FILE_A,
        FILE_B,
        FILE_C,
        FILE_D,
        FILE_E,
        FILE_F,
        FILE_G,
        FILE_H
    };

    enum Rank : int {
        RANK_1,
        RANK_2,
        RANK_3,
        RANK_4,
        RANK_5,
        RANK_6,
        RANK_7,
        RANK_8
    };

    enum {
        VALUE_DRAW = 0,
        VALUE_MATE = 32000,
        VALUE_INFINITE = 32001,
        VALUE_NONE = 32002,
    };

    enum MoveFlags : int {
        QUIET = 0,
        DOUBLE_PUSH = 1,
        OO = 2, OOO = 3,
        CAPTURE = 4,
        EN_PASSANT = 5,
        PR_KNIGHT = 6, PR_BISHOP = 7, PR_ROOK = 8, PR_QUEEN = 9,
        PC_KNIGHT = 10, PC_BISHOP = 11, PC_ROOK = 12, PC_QUEEN = 13
    };

    // max number of ply considered for one game
    const int MAX_PLY = 256;

    // max number of possible legal moves in chess are 218
    const int MAX_MOVES = 128;

    class Move {
    public:
        // default null move (a1a1)
        Move() : move(0) {}

        explicit Move(uint16_t m) { move = m; }

        Move(Square from, Square to) : move(0) {
            move = (from << 6) | to;
        }

        Move(Square from, Square to, MoveFlags flags) : move(0) {
            move = (flags << 12) | (from << 6) | to;
        }

        explicit Move(const std::string& move) {
            Square from =  Square(Rank(move[1] - '1') << 3 | File(move[0] - 'a'));
            Square to =  Square(Rank(move[3] - '1') << 3 | File(move[2] - 'a'));
            this->move = (from << 6) | to;
        }

        Square to() const { return Square(move & 0x3f); }
        Square from() const { return Square((move >> 6) & 0x3f); }
        int to_from() const { return move & 0xffff; }

        MoveFlags flags() const { return MoveFlags(move >> 12); }

        void operator=(Move m) { move = m.move; }
        bool operator==(Move m) const { return to_from() == m.to_from(); }
        bool operator!=(Move m) const { return to_from() != m.to_from(); }

    private:
        uint16_t move;

    };

    // instead of Move() use this for clarity
    const Move NULL_MOVE = Move();

} // namespace Chess


#endif //ASTRA_TYPES_H
