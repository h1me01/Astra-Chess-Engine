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

#ifndef ASTRA_ZOBRIST_H
#define ASTRA_ZOBRIST_H

#include "types.h"

namespace Chess {

    // psuedorandom number generator from stockfish
    class PRNG {
        U64 s;

        U64 rand64() {
            s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
            return s * 2685821657736338717LL;
        }

    public:
        explicit PRNG(U64 seed) : s(seed) {}

        // generate psuedorandom number
        template<typename T>
        T rand() { return T(rand64()); }

        // generate psuedorandom number with only a few setFen bits
        template<typename T>
        T sparse_rand() {
            return T(rand64() & rand64() & rand64());
        }
    };

    namespace zobrist {
        // zobrist keys for each piece and each square
        // used to incrementally update the hash key of a position
        inline U64 zobristTable[NUM_PIECES][NUM_SQUARES];

        // initializes the zobrist table with random 64-bit numbers
        inline void initZobristKeys() {
            PRNG rng(70026072);

            for (auto & i : zobristTable) {
                for (U64 & j : i) {
                    j = rng.rand<U64>();
                }
            }
        }
    } // namespace zobrist

} // namespace Chess

#endif //ASTRA_ZOBRIST_H
