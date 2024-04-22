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

#include "evaluate.h"

namespace Eval {

    const int *mg_pesto_table[6] = {
            mg_pawn_table, mg_knight_table, mg_bishop_table, mg_rook_table, mg_queen_table, mg_king_table
    };

    const int *eg_pesto_table[6] = {
            eg_pawn_table, eg_knight_table, eg_bishop_table, eg_rook_table, eg_queen_table, eg_king_table
    };

    int mg_table[12][64];
    int eg_table[12][64];

    void initEvalTables() {
        for (int pt = PAWN; pt <= KING; ++pt) {
            for (int sq = 0; sq < 64; ++sq) {
                mg_table[pt][sq] = mg_value[pt] + mg_pesto_table[pt][sq];
                eg_table[pt][sq] = eg_value[pt] + eg_pesto_table[pt][sq];

                mg_table[pt + 6][sq] = mg_value[pt] + mg_pesto_table[pt][relativeSquare(BLACK, (Square) sq)];
                eg_table[pt + 6][sq] = eg_value[pt] + eg_pesto_table[pt][relativeSquare(BLACK, (Square) sq)];
            }
        }
    }

    int evaluate(Board &board) {
        int mg[2];
        int eg[2];
        int gamePhase = 0;

        mg[WHITE] = 0;
        mg[BLACK] = 0;
        eg[WHITE] = 0;
        eg[BLACK] = 0;

        /* evaluate each piece */
        for (int sq = 0; sq < 64; ++sq) {
            Piece pc = board.pieceAt((Square) sq);

            if (pc != NO_PIECE) {
                mg[colorOfPiece(pc)] += mg_table[pc][sq];
                eg[colorOfPiece(pc)] += eg_table[pc][sq];
                gamePhase += gamephaseInc[typeOfPiece(pc)];
            }
        }

        Color stm = board.sideToMove();
        /* tapered evaluate */
        int mgScore = mg[stm] - mg[~stm];
        int egScore = eg[stm] - eg[~stm];
        int mgPhase = gamePhase;
        if (mgPhase > 24) mgPhase = 24; /* in case of early promotion */
        int egPhase = 24 - mgPhase;
        return (mgScore * mgPhase + egScore * egPhase) / 24;
    }

} // namespace Eval
