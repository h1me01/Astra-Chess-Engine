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

#include "nnue.h"

namespace NNUE {

   void load() {

   }

   float predict(Accumulator &acc) {
      const float *input = acc.getData();

      // hidden layer
      float hiddenOutput[HIDDEN_SIZE]{};
      for (int i = 0; i < HIDDEN_SIZE; ++i) {
         float dot = 0;
         for (int j = 0; j < INPUT_SIZE; ++j)
            dot += input[j] * hiddenWeights[i * INPUT_SIZE + j];
         dot += hiddenBiases[i];

         hiddenOutput[i] = dot > 0 ? dot : 0; // relu
      }

      // output layer
      float prediction = 0;
      for (int j = 0; j < HIDDEN_SIZE; ++j) {
         prediction += hiddenOutput[j] * outputWeights[j];
      }
      prediction += outputBiases[0];

      return 1.0f / (1.0f + expf(-SIGMOID_SCALAR * prediction)); // sigmoid
   }

} // namespace NNUE
