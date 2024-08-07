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

#include "genData.h"
#include "chess/perft.h"
#include "search/search.h"

const std::string pieceNot[] = {"P", "N", "B", "R", "Q", "K", ""};
std::vector<std::string> moveAccumulator;

void printMoves() {
    for (const auto & i : moveAccumulator) {
        std::cerr << i << " ";
    }
    std::cout << std::endl;
}

int main() {
    initLookUpTables();
    zobrist::initZobristKeys();

    // generate input for neural network
    //saveNetInput(fenToInput(loadDataset(INT_MAX)));

    // test performance and correctness of move generation
    //testPerft(5);

    Board board(DEFAULT_FEN);

    while (true) {
        Astra::Search search(board);
        Move bestMove = search.findBestMove();

        Piece pc = board.pieceAt(bestMove.from());
        moveAccumulator.push_back(pieceNot[typeOfPiece(pc)] + SQSTR[bestMove.from()] + SQSTR[bestMove.to()]);

        board.makeMove(bestMove);
        board.print(WHITE);

        MoveList moves(board);
        if (moves.size() == 0 || board.isDraw()) {
            break;
        }
    }

    printMoves();

    return 0;
}
