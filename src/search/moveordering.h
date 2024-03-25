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
#include "psqt.h"
#include "tt.h"

using namespace Chess;

namespace Astra {

    /*
     * Static Exchange Evaluation (SEE)
     */
    int seeCapture(Board &board, Move &captureMove);

    /*
     * Move Ordering
     */
    class MoveOrdering {
    public:
        MoveOrdering();

        void clear();

        int getHistoryScore(Board &board, Move &move);

        void updateHistory(Board &board, Move &move, int score);

        void updateKiller(Move &move, Color c, int ply);

        void sortMoves(Board &board, TTable &tt, Move *moves, int numMoves, int ply);

    private:
        static const int KILLER_SIZE = 32;

        const int HASH_BONUS = 30'000'000;
        const int PROMOTION_BONUS = 1'000'000;
        const int KILLER1_BONUS = 200'000;
        const int KILLER2_BONUS = 100'000;

        Move killer1[NUM_COLORS][KILLER_SIZE];
        Move killer2[NUM_COLORS][KILLER_SIZE];

        int history[NUM_PIECES][NUM_SQUARES][NUM_SQUARES]{};

    };

} // namespace Astra


#endif //ASTRA_MOVEORDERING_H
