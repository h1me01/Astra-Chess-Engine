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

const std::string pieceNotation[] = {"P", "N", "B", "R", "Q", "K", ""};
std::vector<std::string> moveAccumulator;

void printMoves() {
    for (int i = 0; i < moveAccumulator.size(); ++i) {
        std::cout << moveAccumulator[i] << " ";
    }

    std::cout << std::endl;
}

int main() {
    initLookUpTables();
    zobrist::initZobristKeys();
    Eval::initEval();

    // test the performance of the move generation
    // and also the correctness of the move generation
    //testPerft(5);

    Board board(DEFAULT_FEN);

    while(true) {
        Astra::Search search(board);
        Move bestMove = search.findBestMove();

        Piece pc = board.getPiece(bestMove.from());
        moveAccumulator.push_back(pieceNotation[typeOfPiece(pc)] + SQSTR[bestMove.from()] + SQSTR[bestMove.to()]);

        board.makeMove(bestMove);

        Move moves[MAX_MOVES];
        int numMoves = genLegalMoves(board, moves);

        if (numMoves == 0 || board.isDraw()) {
            break;
        }
    }

    printMoves();

    return 0;
}
