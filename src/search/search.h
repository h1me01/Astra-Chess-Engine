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

#ifndef ASTRA_SEARCH_H
#define ASTRA_SEARCH_H

#include <algorithm>
#include "evaluate.h"
#include "pvtable.h"
#include "moveordering.h"
#include "timemanager.h"

namespace Astra {

    class Search {
    public:
        explicit Search(Board board);

        void printPv(int numLines);

        // set search time per move to 1000ms
        Move findBestMove(unsigned int timePerMove = 1000);

    private:
        static const int MAX_DEPTH = 64;

        TimeManager timeManager;
        MoveOrdering moveOrdering;
        TTable tt;

        U64 searchedNodes;
        int ply;

        Board board;

        PVTable pvTable;

        void clearData();

        int quiesceSearch(int alpha, int beta);
        int negamax(int alpha, int beta, int depth, bool doNull);

    };

} // namespace Astra


#endif //ASTRA_SEARCH_H
