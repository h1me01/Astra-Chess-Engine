#include "eval.h"
#include "endgame.h"
#include "evalparam.h"

namespace Eval {

    EVAL_PARAM(mg_bishop_atk, 3)
    EVAL_PARAM(mg_rook_atk, 3)
    EVAL_PARAM(mg_queen_atk, -1)
    EVAL_PARAM(mg_tempo, 8)
    EVAL_PARAM(eg_bishop_atk, 6)
    EVAL_PARAM(eg_rook_atk, 4)
    EVAL_PARAM(eg_queen_atk, 5)
    EVAL_PARAM(eg_tempo, -3)
    EVAL_PARAM(eval_divisor, 285)

    U64 getAllAttacks(Board& board, Color c, PieceType pt, U64 occ) {
        U64 attacks = 0;
        U64 pieces = board.getPieceBB(c, pt);
        U64 sameColor = board.getOccupancy(c);

        while(pieces) {
            Square sq = popLsb(&pieces);
            attacks |= getAttacks(pt, sq, occ) & ~sameColor;
        }
        return attacks;
    }

    int eval(Board &board)
    {
        Color stm = board.getTurn();
        const int gamePly = board.getPly();
        const int sign = stm ? 1 : -1;

        const auto &info = board.evalInfo;

        U64 occ = board.getOccupancy(WHITE) | board.getOccupancy(BLACK);

        U64 whiteBishopAttacks = getAllAttacks(board, WHITE, BISHOP, occ);
        U64 blackBishopAttacks = getAllAttacks(board, BLACK, BISHOP, occ);
        U64 whiteRookAttacks = getAllAttacks(board, WHITE, ROOK, occ);
        U64 blackRookAttacks = getAllAttacks(board, BLACK, ROOK, occ);
        U64 whiteQueenAttacks = getAllAttacks(board, WHITE, QUEEN, occ);
        U64 blackQueenAttacks = getAllAttacks(board, BLACK, QUEEN, occ);

        U64 whiteKing = board.getPieceBB(WHITE, KING);
        U64 blackKing = board.getPieceBB(BLACK, KING);

        const int bishop_atk_cnt = popCount(whiteBishopAttacks) - popCount(blackBishopAttacks);
        const int rook_atk_cnt = popCount(whiteRookAttacks) - popCount(blackRookAttacks);
        const int queen_atk_cnt = popCount(whiteQueenAttacks) - popCount(blackQueenAttacks);

        const auto mg_pst_eval = (whiteKing % 8 >= 4) ? ((blackKing % 8 >= 4) ? info[gamePly].mg_kk : info[gamePly].mg_kq) :
                                                                 ((blackKing % 8 >= 4) ? info[gamePly].mg_qk : info[gamePly].mg_qq);
        const int mg_eval = mg_pst_eval + mg_bishop_atk * bishop_atk_cnt + mg_rook_atk * rook_atk_cnt + mg_queen_atk * queen_atk_cnt + mg_tempo * sign;
        const int eg_eval = info[gamePly].eg + eg_bishop_atk * bishop_atk_cnt + eg_rook_atk * rook_atk_cnt + eg_queen_atk * queen_atk_cnt + eg_tempo * sign;
        const int raw_eval = eg_eval * 256 + info[gamePly].phase_count * (mg_eval - eg_eval);

        return sign * make_endgame_adjustment(raw_eval, board) / eval_divisor;
    }

}