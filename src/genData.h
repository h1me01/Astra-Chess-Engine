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

#ifndef ASTRA_GENDATA_H
#define ASTRA_GENDATA_H

#include <fstream>
#include "chess/board.h"

using namespace Chess;

const std::string DATA_PATH = "C:/Users/semio/Documents/Coding/Projects/Astra-Data/TrainingData/chessData1.csv";
const std::string NET_DATA_PATH = "C:/Users/semio/Downloads/chess_data1.bin";

struct Dataset {
    std::string fen;
    float eval;

    Dataset(std::string fen, float eval) {
        this->fen = std::move(fen);
        this->eval = eval;
    }
};

struct NetInput {
    U64 pieces[NUM_COLORS][6]{};
    float target;
    Color stm;

    NetInput() {
        target = 0;
    }
};

std::vector<Dataset> loadDataset(int dataSize) {
    std::vector<Dataset> dataset;
    std::ifstream file(DATA_PATH);

    if (!file) {
        std::cerr << "Error opening file" << std::endl;
        return dataset;
    }

    std::string header;
    std::getline(file, header);

    std::string line;
    for (int i = 0; std::getline(file, line) && dataSize > i; ++i) {
        std::istringstream iss(line);
        std::string fen, eval;

        std::getline(iss, fen, ',');
        std::getline(iss, eval, ',');

        if(fen.empty()) {
            std::cout << "Empty FEN at line " << i << std::endl;
            exit(0);
        }

        Dataset data{fen.substr(1), std::stof(eval)};
        dataset.push_back(data);
    }

    std::cout << "Loaded " << dataset.size() << " positions" << std::endl;
    return dataset;
}

std::vector<NetInput> fenToInput(const std::vector<Dataset> &_dataset) {
    std::vector<NetInput> netInput;
    netInput.reserve(_dataset.size());

    for (const auto &i: _dataset) {
        NetInput input;
        input.target = i.eval;

        Board board(i.fen);

        for (PieceType pt: {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
            input.pieces[WHITE][pt] = board.pieceBitboard(WHITE, pt);
            input.pieces[BLACK][pt] = board.pieceBitboard(BLACK, pt);
        }

        input.stm = board.sideToMove();
        netInput.push_back(input);
    }

    return netInput;
}

void saveNetInput(const std::vector<NetInput>& data) {
    std::ofstream file(NET_DATA_PATH, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error writing in file" << std::endl;
    }

    for (const NetInput &input: data) {
        file.write(reinterpret_cast<const char *>(&input), sizeof(NetInput));
    }

    file.close();

    std::cout << "Saved positions at " << NET_DATA_PATH << std::endl;

    exit(0);
}

#endif //ASTRA_GENDATA_H
