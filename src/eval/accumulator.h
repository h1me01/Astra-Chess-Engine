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

#ifndef ASTRA_CHESS_ENGINE_ACCUMULATOR_H
#define ASTRA_CHESS_ENGINE_ACCUMULATOR_H

#include "../chess/misc.h"

using namespace Chess;

namespace NNUE {
    struct Accumulator {
        void init(Piece board[NUM_SQUARES]) {
            for(Square sq = a1; sq < NUM_SQUARES; ++sq) {
                Piece p = board[sq];

                if(p != NO_PIECE) {
                    PieceType pt = typeOfPiece(p);
                    Color pc = colorOfPiece(p);
                    data[index(sq, pt, pc)] = 1.0f;
                }
            }
        }

        void move(PieceType pt, Square from, Square to, Color pc) {
            int from_idx = index(from, pt, pc);
            int to_idx = index(to, pt, pc);

            data[from_idx] = 0.0f;
            data[to_idx] = 1.0f;
        }

        float *getData() {
            return data;
        }

    private:
        float data[768]{};

        int index(Square psq, PieceType pt, Color pc) {
            return psq + 64 * pt + pc * 64 * 6;
        }
    };

} // namespace NNUE


#endif //ASTRA_CHESS_ENGINE_ACCUMULATOR_H
