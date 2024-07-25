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

#ifndef ASTRA_CHESS_ENGINE_NNUE_H
#define ASTRA_CHESS_ENGINE_NNUE_H

#include "accumulator.h"

namespace NNUE {
    const std::string WEIGHTS_PATH = "path";

    constexpr int INPUT_SIZE = 768;
    constexpr int HIDDEN_SIZE = 64;
    constexpr int OUTPUT_SIZE = 1;
    constexpr float SIGMOID_SCALAR = 2.5 / 400.0f;

    inline float hiddenWeights[HIDDEN_SIZE * INPUT_SIZE];
    inline float hiddenBiases[HIDDEN_SIZE];

    inline float outputWeights[OUTPUT_SIZE * HIDDEN_SIZE];
    inline float outputBiases[OUTPUT_SIZE];

    void load();

    float predict(Accumulator &acc);

} // namespace NNUE


#endif //ASTRA_CHESS_ENGINE_NNUE_H
