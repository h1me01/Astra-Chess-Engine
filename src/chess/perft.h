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

#ifndef ASTRA_PERFT_H
#define ASTRA_PERFT_H

#include <chrono>
#include "board.h"
#include "movegen.h"

namespace Chess {

    struct TestCase {
        std::string fen;
        std::vector<std::pair<int, U64>> results;
    };

    // Positions from https://www.chessprogramming.org/Perft_Results

    // Test Position 1
    TestCase test1 = {
            DEFAULT_FEN,
            {{1, 20}, {2, 400}, {3, 8902}, {4, 197281}, {5, 4865609}}
    };

    // Test Position 2
    TestCase test2 = {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
            {{1, 48}, {2, 2039}, {3, 97862}, {4, 4085603}, {5, 193690690}}
    };

    // Test Position 3
    TestCase test3 = {
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
            {{1, 14}, {2, 191}, {3, 2812}, {4, 43238}, {5, 674624}}
    };

    // Test Position 4
    TestCase test4 = {
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            {{1, 6}, {2, 264}, {3, 9467}, {4, 422333}, {5, 15833292}}
    };

    // Test Position 5
    TestCase test5 = {
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
            {{1, 44}, {2, 1486}, {3, 62379}, {4, 2103487}, {5, 89941194}}
    };

    // Test Position 6
    TestCase test6 = {
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
            {{1, 46}, {2, 2079}, {3, 89890}, {4, 3894594}, {5, 164075551}}
    };

    std::vector<TestCase> testCases = {
            test1, test2, test3, test4, test5, test6
    };

    U64 perft(Board &board, int depth) {
        U64 nodes = 0;

        Move moves[MAX_MOVES];
        int numMoves = genLegalMoves(board, moves);

        if (depth == 1) {
            return (U64) numMoves;
        }

        for (int i = 0; i < numMoves; ++i) {
            board.makeMove(moves[i]);
            nodes += perft(board, depth - 1);
            board.unmakeMove(moves[i]);
        }

        return nodes;
    }

    void testPerft(int maxDepth) {
        if (maxDepth < 1 || maxDepth > testCases.size()) {
            std::cerr << "Invalid depth for Perft!" << std::endl;
            return;
        }

        for (const auto &testCase: testCases) {
            Board board(testCase.fen);

            std::cout << "\nFen: " << testCase.fen << std::endl;

            for (int depth = 1; depth <= maxDepth; ++depth) {
                auto start = std::chrono::high_resolution_clock::now();
                auto nodes = perft(board, depth);
                auto end = std::chrono::high_resolution_clock::now();

                // check if number of nodes are correct
                if (nodes != testCase.results[depth - 1].second) {
                    std::cerr << "Test failed! Expected Nodes: " << testCase.results[depth - 1].second << std::endl;
                    std::cerr << "Actual Nodes: " << nodes << std::endl;
                    exit(1);
                } else {
                    std::chrono::duration<double, std::milli> diff = end - start;
                    std::cout << "Test passed | Depth: " << depth << " | Time: " << diff.count() << " ms\n";
                }
            }
        }

        exit(0);
    }

} // namespace Chess


#endif //ASTRA_PERFT_H
