#ifndef ASTRA_CHESS_ENGINE_PHASECOUNT_H
#define ASTRA_CHESS_ENGINE_PHASECOUNT_H

#include "../evalparam.h"

namespace Eval {

    EVAL_PARAM(pc_pawn, 1)
    EVAL_PARAM(pc_knight, 7)
    EVAL_PARAM(pc_bishop, 10)
    EVAL_PARAM(pc_rook, 16)
    EVAL_PARAM(pc_queen, 40)
    EVAL_PARAM(pc_intercept, 7)

}

#endif //ASTRA_CHESS_ENGINE_PHASECOUNT_H
