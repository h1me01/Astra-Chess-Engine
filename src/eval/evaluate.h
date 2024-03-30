#ifndef ASTRA_CHESS_ENGINE_EVALUATE_H
#define ASTRA_CHESS_ENGINE_EVALUATE_H

#include "../chess/board.h"

namespace Eval {

    inline int evaluate(Board &board) {
        int32_t eval = NNUE::output(board.getAccumulator(), board.sideToMove());
        int halfmoves = board.history[board.getPly()].halfMoveClock;
        return static_cast<double>(eval) * (1.0 - (halfmoves / 1000.0));
    }

}  // namespace Eval

#endif //ASTRA_CHESS_ENGINE_EVALUATE_H
