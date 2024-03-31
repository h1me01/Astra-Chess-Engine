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

using namespace Chess;

namespace Astra {

    // piece values
    const int pieceValues[] = {100, 310, 325, 500, 900, 10000, 0};

    /*
     * Static Exchange Evaluation (SEE)
     */
    int seeCapture(Board &board, Move &captureMove);

    /*
     * Most Valuable Victim / Least Valuable Attacker
     */
    int mvvlva(Board &board, Move move);

    /*
     * Move Ordering
     */
    enum SearchType {
        QSEARCH, NEGAMAX
    };

    // Move Scores
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

        int getHistoryScore(Board &board, Move &move);

        void updateHistory(Board &board, Move &move, int score);
        void updateKiller(Move &move, int ply);

        template<SearchType searchType>
        void sortMoves(Board &board, Move *moves, int numMoves, Move& ttMove, int ply);

    private:
        Move killer1[MAX_PLY];
        Move killer2[MAX_PLY];

        int history[NUM_COLORS][NUM_SQUARES][NUM_SQUARES]{};
    };

    template<SearchType searchType>
    inline void MoveOrdering::sortMoves(Board &board, Move *moves, int numMoves, Move& ttMove, int ply) {
        int scores[numMoves];
        for (int i = 0; i < numMoves; ++i) {
            Move move = moves[i];
            scores[i] = 0;

            if (ttMove != NULL_MOVE && move == ttMove) {
                scores[i] = TT_SCORE;
                continue;
            }

            if constexpr (searchType == QSEARCH) {
                scores[i] = CAPTURE_SCORE + mvvlva(board, move);
                continue;
            }

            if (isCapture(move)) {
                int seeScore = seeCapture(board, move);
                scores[i] = seeScore >= 0 ? CAPTURE_SCORE + mvvlva(board, move) : mvvlva(board, move);
                continue;
            }

            if (move == killer1[ply]) {
                scores[i] = KILLER_ONE_SCORE;
                continue;
            } else if (move == killer2[ply]) {
                scores[i] = KILLER_TWO_SCORE;
                continue;
            }

            scores[i] = getHistoryScore(board, move);
        }

        // Bubble Sort
        for (int i = 0; i < numMoves - 1; ++i) {
            for (int j = 0; j < numMoves - i - 1; ++j) {
                if (scores[j] < scores[j + 1]) {
                    Move tempMove = moves[j];
                    moves[j] = moves[j + 1];
                    moves[j + 1] = tempMove;

                    int tempScore = scores[j];
                    scores[j] = scores[j + 1];
                    scores[j + 1] = tempScore;
                }
            }
        }
    }

} // namespace Astra


#endif //ASTRA_MOVEORDERING_H
