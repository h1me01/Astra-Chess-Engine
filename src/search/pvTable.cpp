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

#include "pvTable.h"

namespace Astra {

    /*
     * PV Line
     */
    Move &PVLine::operator()(int depth) {
        return pv[depth];
    }

    Move PVLine::operator()(int depth) const {
        return pv[depth];
    }

    /*
     * PV Table
     */
    PVLine &PVTable::operator()(int depth) {
        return pvs[depth];
    }

    PVLine PVTable::operator()(int depth) const {
        return pvs[depth];
    }

    void PVTable::reset() {
        for (auto &pvLine: pvs) {
            pvLine.length = 0;
        }
    }

    void PVTable::updatePV(int ply, Move move) {
        // Add the move to the beginning of the PVLine at the current depth
        pvs[ply](0) = move;
        // Copy the PVLine at the next depth to the current depth
        memcpy(&pvs[ply](1), &pvs[ply + 1](0), sizeof(Move) * pvs[ply + 1].length);
        // Update the length of the PVLine at the current depth
        pvs[ply].length = pvs[ply + 1].length + 1;
    }

} // namespace Astra

