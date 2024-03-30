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

#ifndef ASTRA_TT_H
#define ASTRA_TT_H

#include "../chess/types.h"

using namespace Chess;

namespace Astra {

    enum Bound {
        NO_BOUND,
        ALL_NODE,
        CUT_NODE,
        PV_NODE
    };

    struct TTEntry {
        U64 hash;
        int depth;
        Move move;
        int score;
        Bound bound;

        TTEntry() : hash(0), depth(0), move(NULL_MOVE), score(0), bound(NO_BOUND) {}

        TTEntry(U64 hash, int depth, Move move, int score, Bound bound) :
                hash(hash), depth(depth), move(move), score(score), bound(bound) {}
    };

    class TTable {
    public:
        explicit TTable(int sizeMB);

        ~TTable();

        bool lookup(TTEntry& entry, U64 hash, int depth);

        void store(U64 hash, Move move, int score, int depth, Bound bound);

    private:
        U64 ttSize;
        TTEntry *entries;

    };

} // namespace Astra


#endif //ASTRA_TT_H
