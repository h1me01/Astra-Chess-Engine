#ifndef ASTRA_CHESS_ENGINE_ACCUMULATORS_H
#define ASTRA_CHESS_ENGINE_ACCUMULATORS_H

#pragma once

#include "nnue.h"

struct Accumulators {
    Accumulators() { assert(alignof(accumulators) == 32); };

    [[nodiscard]] int size() const {
        assert(index >= 0 && index < MAX_PLY + 1);
        return index;
    }

    void clear() { index = 0; }

    void push() {
        assert(index + 1 < MAX_PLY + 1);
        index++;
        accumulators[index] = accumulators[index - 1];
    }

    void pop() {
        assert(index > 0);
        index--;
    }

    NNUE::accumulator &back() {
        assert(index >= 0 && index < MAX_PLY + 1);
        return accumulators[index];
    }

private:
    alignas(32) std::array<NNUE::accumulator, MAX_PLY + 1> accumulators = {};
    int index = 0;
};

#endif //ASTRA_CHESS_ENGINE_ACCUMULATORS_H
