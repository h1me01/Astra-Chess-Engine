#ifndef ASTRA_CHESS_ENGINE_PSQT_H
#define ASTRA_CHESS_ENGINE_PSQT_H

#include "../chess/bitboard.h"

using namespace Chess;

namespace Eval {

    struct PstEvalInfo {
        int mg_kk;
        int mg_qk;
        int mg_kq;
        int mg_qq;
        int eg;
        int phase_count;

        U64 hash;

        PstEvalInfo() = default;

        PstEvalInfo copy() const {
            return PstEvalInfo{mg_kk, mg_qk, mg_kq, mg_qq, eg, phase_count, hash};
        }

        void move_pawn(Color white, Square from, Square to);
        void move_knight(Color white, Square from, Square to);
        void move_bishop(Color white, Square from, Square to);
        void move_rook(Color white, Square from, Square to);
        void move_queen(Color white, Square from, Square to);
        void move_king(Color white, Square from, Square to);

        void remove_pawn(Color white, Square square);
        void remove_knight(Color white, Square square);
        void remove_bishop(Color white, Square square);
        void remove_rook(Color white, Square square);
        void remove_queen(Color white, Square square);

        void promote_pawn_to_knight(Color white, Square from,  Square to);
        void promote_pawn_to_bishop(Color white, Square from,  Square to);
        void promote_pawn_to_rook(Color white, Square from,  Square to);
        void promote_pawn_to_queen(Color white, Square from,  Square to);

        void remove_bishop_pair_bonus(Color white);
    };

}


#endif //ASTRA_CHESS_ENGINE_PSQT_H
