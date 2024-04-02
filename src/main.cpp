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

#include "chess/perft.h"
#include "search/search.h"

int main() {
    initLookUpTables();
    zobrist::initZobristKeys();
    NNUE::init("");

    // test the performance of the move generation
    // and also the correctness of the move generation
    testPerft(5);

    Board board("rnbqkbnr/pp1ppppp/2p5/8/2P5/8/PP1PPPPP/RNBQKBNR w KQkq - 0 2");

    Astra::Search search(board);
    search.findBestMove(0);

    return 0;
}
