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

    // test the performance of the move generation
    // and also the correctness of the move generation
    ///testPerft(5);

    std::string fen = "2rq1rk1/Qp1nbppp/p1b1p3/3pB3/4P3/P1NB3P/1PP2PP1/3R1RK1 w - - 1 19";
    std::string fen2 = "7R/4P1k1/8/6Bp/7P/6K1/5PP1/8 w - - 3 75";
    std::string fen3 = "8/6p1/8/1K2p3/5p1p/8/8/1kq5 b - - 11 98";

    int debugDepths = 0;
    int debugNodes = 0;

    Board board(fen3);

    while (true) {
        Astra::Search search(board);
        Move bestMove = search.findBestMove();

        debugDepths += search.debugDepth;
        debugNodes += search.debugNodes;

        board.makeMove(bestMove);
        board.print(WHITE);

        if(board.isDraw()) {
            break;
        }

        Move moves[MAX_MOVES];
        int numMoves = board.genLegalMoves(moves);

        if(numMoves == 0) {
            break;
        }

        if(board.getPly() > 200) {
            break;
        }
    }

    std::cout << "Depth: " << debugDepths / board.getPly() << std::endl;
    std::cout << "Nodes: " << debugNodes / board.getPly() << std::endl;

    board.pgn();

    return 0;
}
