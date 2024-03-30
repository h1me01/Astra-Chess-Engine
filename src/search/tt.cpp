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

#include "tt.h"

namespace Astra {

    TTable::TTable(int sizeMB) {
        U64 sizeBytes = sizeMB * 1024 * 1024;
        std::size_t entrySize = sizeof(TTEntry);
        ttSize = sizeBytes / entrySize;

        try {
            entries = new TTEntry[ttSize];

            for (int i = 0; i < ttSize; ++i) {
                entries[i] = TTEntry();
            }
        } catch (const std::bad_alloc &e) {
            std::cerr << "Failed to allocate transposition table" << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            exit(1);
        }
    }

    TTable::~TTable() {
        delete[] entries;
    }

    bool TTable::lookup(TTEntry& entry, U64 hash, int depth) {
        U64 index = hash % ttSize;

        if(entries[index].hash == hash && entries[index].bound != NO_BOUND && entries[index].depth >= depth) {
            entry = entries[index];
            return true;
        }

        return false;
    }

    void TTable::store(U64 hash, Move move, int score, int depth, Bound bound) {
        U64 index = hash % ttSize;

        if (entries[index].hash == hash && entries[index].depth > depth) {
            return;
        }

        entries[index] = TTEntry(hash, depth, move, score, bound);
    }

} // namespace Astra
