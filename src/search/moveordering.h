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
    int mvvlva(Board &board, Move& move);

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
        void sortMoves(Board &board, MoveList &moves, Move& ttMove, int ply);

    private:
        Move killer1[MAX_PLY];
        Move killer2[MAX_PLY];

        int history[NUM_COLORS][NUM_SQUARES][NUM_SQUARES]{};
    };

    template<SearchType searchType>
    inline void MoveOrdering::sortMoves(Board &board, MoveList &moves, Move& ttMove, int ply) {
        std::vector<int> scores(moves.size(), 0);

        int moveCount = 0;
        for (Move move : moves) {
            if (ttMove != NULL_MOVE && move == ttMove) {
                scores[moveCount] = TT_SCORE;
            } else if constexpr (searchType == QSEARCH) {
                scores[moveCount] = CAPTURE_SCORE + mvvlva(board, move);
            } else if (isCapture(move)) {
                int seeScore = seeCapture(board, move);
                scores[moveCount] = seeScore >= 0 ? CAPTURE_SCORE + mvvlva(board, move) : mvvlva(board, move);
            } else if (move == killer1[ply]) {
                scores[moveCount] = KILLER_ONE_SCORE;
            } else if (move == killer2[ply]) {
                scores[moveCount] = KILLER_TWO_SCORE;
            } else {
                scores[moveCount] = getHistoryScore(board, move);
            }

            moveCount++;
        }

        // Bubble Sort
        for (int i = 0; i < moves.size() - 1; ++i) {
            for (int j = 0; j < moves.size() - i - 1; ++j) {
                if (scores[j] < scores[j + 1]) {
                    std::swap(moves[j], moves[j + 1]);
                    std::swap(scores[j], scores[j + 1]);
                }
            }
        }
    }

} // namespace Astra


#endif //ASTRA_MOVEORDERING_H
