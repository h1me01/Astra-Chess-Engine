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

#ifndef ASTRA_MOVEGEN_H
#define ASTRA_MOVEGEN_H

#include "bitboard.h"

namespace Chess {

    // helper to generate quiet moves
    template<MoveFlags MF = QUIET>
    inline int make(Move *&moves, Square from, U64 to) {
        int numMoves = 0;
        while (to) {
            *moves++ = Move(from, popLsb(&to), MF);
            numMoves++;
        }
        return numMoves;
    }

    // helper to generate promotion moves
    template<>
    inline int make<PROMOTIONS>(Move *&moves, Square from, U64 to) {
        int numMoves = 0;

        while (to) {
            Square s = popLsb(&to);
            *moves++ = Move(from, s, PR_KNIGHT);
            *moves++ = Move(from, s, PR_BISHOP);
            *moves++ = Move(from, s, PR_ROOK);
            *moves++ = Move(from, s, PR_QUEEN);
            numMoves += 4;
        }

        return numMoves;
    }

    // helper to generate promotion capture moves
    template<>
    inline int make<PROMOTION_CAPTURES>(Move *&moves, Square from, U64 to) {
        int numMoves = 0;

        while (to) {
            Square s = popLsb(&to);
            *moves++ = Move(from, s, PC_KNIGHT);
            *moves++ = Move(from, s, PC_BISHOP);
            *moves++ = Move(from, s, PC_ROOK);
            *moves++ = Move(from, s, PC_QUEEN);
            numMoves += 4;
        }

        return numMoves;
    }

} // namespace Chess


#endif //ASTRA_MOVEGEN_H
