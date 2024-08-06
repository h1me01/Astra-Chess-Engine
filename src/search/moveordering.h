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

#ifndef ASTRA_MOVEORDERING_H
#define ASTRA_MOVEORDERING_H

#include "../chess/board.h"
#include "../chess/movegen.h"
#include "tt.h"

using namespace Chess;

namespace Astra {

    // piece values
    const int pieceValues[] = {100, 310, 325, 500, 900, 10000, 0};

    /*
     * Static Exchange Evaluation (SEE)
     */
    int seeCapture(Board &board, Move &captureMove);

    /*
     * Move Ordering
     */
    enum SearchType {
        QSEARCH, NEGAMAX
    };

    enum MoveScores : int {
        TT_SCORE = 10'000'000,
        CAPTURE_SCORE = 7'000'000,
        KILLER_ONE_SCORE = 6'000'000,
        KILLER_TWO_SCORE = 5'000'000,
    };

    class MoveOrdering {
    public:
        MoveOrdering();

        void clear();

        int getHistoryScore(Board &board, Move &move) const;

        void updateHistory(Board &board, Move &move, int score);
        void updateKiller(Move &move, int ply);

        void sortMoves(Board &board, MoveList &moves, TTable& tt, int ply);

    private:
        Move killer1[MAX_PLY];
        Move killer2[MAX_PLY];

        int history[NUM_COLORS][NUM_SQUARES][NUM_SQUARES]{};
    };

} // namespace Astra

#endif //ASTRA_MOVEORDERING_H
