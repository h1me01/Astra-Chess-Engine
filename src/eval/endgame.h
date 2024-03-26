#ifndef ASTRA_CHESS_ENGINE_ENDGAME_H
#define ASTRA_CHESS_ENGINE_ENDGAME_H

#include "../chess/board.h"

namespace Eval {

    int make_endgame_adjustment(int raw_eval, const Board &board);

}


#endif //ASTRA_CHESS_ENGINE_ENDGAME_H
