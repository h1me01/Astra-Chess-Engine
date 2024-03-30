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

#ifndef ASTRA_MISC_H
#define ASTRA_MISC_H

#include "types.h"

namespace Chess {

    // helper to print bitboards for debugging
    inline void printBitboard(U64 b) {
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 0; file < 8; ++file) {
                int square = rank * 8 + file;
                U64 mask = 1ULL << square;
                std::cout << ((b & mask) ? "1 " : "0 ");
            }

            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    // helper to represent the castleHelper rights in the fen notation
    inline bool castleNotationHelper(std::ostringstream &fenStream) {
        std::string fen = fenStream.str();
        std::string rights = fen.substr(fen.find(' ') + 1);
        return rights.find_first_of("kqKQ") != std::string::npos;
    }

    // helper to determine the type of the promotion
    inline PieceType typeOfPromotion(MoveFlags f) {
        switch (f) {
            case PR_KNIGHT:
            case PC_KNIGHT:
                return KNIGHT;
            case PR_BISHOP:
            case PC_BISHOP:
                return BISHOP;
            case PR_ROOK:
            case PC_ROOK:
                return ROOK;
            case PR_QUEEN:
            case PC_QUEEN:
                return QUEEN;
            default:
                return NO_PIECE_TYPE;
        }
    }

    // prints the move
    inline std::ostream &operator<<(std::ostream &os, const Move &m) {
        if (SQSTR[m.from()] == "a1" && SQSTR[m.to()] == "a1") {
            os << "NULL MOVE";
        } else {
            os << SQSTR[m.from()] << SQSTR[m.to()];
        }

        return os;
    }

    inline bool isCapture(Move &move) {
        return move.flags() == CAPTURE || move.flags() == EN_PASSANT || (move.flags() >= PC_KNIGHT && move.flags() <= PC_QUEEN);
    }

    inline bool isPromotion(Move &move) {
        return move.flags() >= PR_KNIGHT && move.flags() <= PC_QUEEN;
    }

    // operator to get the opposite color
    constexpr Color operator~(Color c) {
        return Color(c ^ BLACK);
    }

    constexpr Piece makePiece(Color c, PieceType pt) {
        if (pt == NO_PIECE_TYPE) { return NO_PIECE; }
        return Piece(pt + 6 * c);
    }

    constexpr PieceType typeOfPiece(Piece pc) {
        return PIECE_TO_PIECETYPE[pc];
    }

    constexpr Color colorOfPiece(Piece pc) {
        return pc < 6 ? WHITE : BLACK;
    }

    // increases the square by one
    inline Square &operator++(Square &s) {
        return s = Square(int(s) + 1);
    }

    // adds squares
    constexpr Square operator+(Square s, Direction d) {
        return Square(int(s) + int(d));
    }

    // subtracts squares
    constexpr Square operator-(Square s, Direction d) {
        return Square(int(s) - int(d));
    }

    // adds squares
    inline Square &operator+=(Square &s, Direction d) {
        return s = s + d;
    }

    // subtracts squares
    inline Square &operator-=(Square &s, Direction d) {
        return s = s - d;
    }

    constexpr Rank squareRank(Square s) {
        return Rank(s >> 3);
    }

    constexpr File squareFile(Square s) {
        return File(s & 0b111);
    }

    // gets the diagonal (a1 to h8) of the square
    constexpr int squareDiag(Square s) {
        return 7 + squareRank(s) - squareFile(s);
    }

    // gets the anti-diagonal (h1 to a8) of the square
    constexpr int squareAntiDiag(Square s) {
        return squareRank(s) + squareFile(s);
    }

    constexpr Rank relativeRank(Color c, Rank r) {
        return c == WHITE ? r : Rank(RANK_8 - r);
    }

    constexpr Direction relativeDir(Color c, Direction d) {
        return Direction(c == WHITE ? d : -d);
    }

    constexpr Square relativeSquare(Color c, Square s) {
        return c == WHITE ? s : Square(s ^ 56);
    }

} //namespace Chess


#endif //ASTRA_MISC_H
