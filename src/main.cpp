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
    //testPerft(5);

    Board board(DEFAULT_FEN);

    while (true) {
        Astra::Search search(board);
        Move bestMove = search.findBestMove(0);

        board.makeMove<true>(bestMove);
        board.pgn();
        std::cout << std::endl;

        Move moves[MAX_MOVES];
        int numMoves = board.genLegalMoves(moves);

        if(board.isDraw() || numMoves == 0) {
            break;
        }
    }

    board.pgn();

    return 0;
}
