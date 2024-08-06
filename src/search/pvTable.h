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

#ifndef ASTRACHESS_PVTABLE_H
#define ASTRACHESS_PVTABLE_H

#include "../chess/types.h"

using namespace Chess;

namespace Astra {

    // PV Table implementation from Koivisto

    /*
     * PVLine struct represents a principal variation line.
     * This struct is used to store a sequence of moves that are expected to be the best
     * move at each ply.
     */
    struct PVLine {
        // pv is an array that stores the sequence of moves that form the principal variation
        Move pv[MAX_PLY];

        // length is the number of moves in the principal variation
        uint16_t length;

        // Overloaded operator() to access the move at a specific depth.
        // This allows the struct to be accessed like an array,
        // so we can use pvLine(depth) to access the move at a specific depth.
        Move& operator()(int depth) {
            return pv[depth];
        }

        // const version of the above operator
        Move operator()(int depth) const {
            return pv[depth];
        }

    } __attribute__((aligned(128)));

    /*
     * PVTable struct represents a table of principal variations.
     * This struct is used to store multiple PVLine structs, one for each depth.
     */
    struct PVTable {
        // pvs is an array of PVLine structs, one for each depth
        PVLine pvs[MAX_PLY];

        // operator allows the struct to be accessed like an array,
        // so we can use pvTable(depth) to access the PVLine at a specific depth
        PVLine& operator()(int depth) {
            return pvs[depth];
        }

        // const version of the above operator
        PVLine operator()(int depth) const {
            return pvs[depth];
        }

        // reset all the lengths of PVLine structs to 0
        void reset() {
            for (auto &pvLine: pvs) {
                pvLine.length = 0;
            }
        }

        // updatePV updates the PVLine at a specific ply with the given move.
        // The move is added to the beginning of the PVLine and the PVLine
        // at the next depth is appended to it.
        void updatePV(int ply, Move move) {
            // Add the move to the beginning of the PVLine at the current depth
            pvs[ply](0) = move;
            // Copy the PVLine at the next depth to the current depth
            memcpy(&pvs[ply](1), &pvs[ply + 1](0), sizeof(Move) * pvs[ply + 1].length);
            // Update the length of the PVLine at the current depth
            pvs[ply].length = pvs[ply + 1].length + 1;
        }
    };

} // namespace Astra

#endif //ASTRACHESS_PVTABLE_H
