#include "endgame.h"
#include "evalparam.h"

namespace Eval {

    const U64 LIGHT_SQUARES = 0x55aa55aa55aa55aaull;
    const U64 DARK_SQUARES = 0xaa55aa55aa55aa55ull;

    inline bool only_has_minor(const Board& board, Color side){
        if(side == WHITE) {
            U64 whiteKnights = board.getPieceBB(WHITE, KNIGHT);
            U64 whiteBishops = board.getPieceBB(WHITE, BISHOP);
            U64 whiteRooks = board.getPieceBB(WHITE, ROOK);
            U64 whiteQueens = board.getPieceBB(WHITE, QUEEN);

            return (!whiteRooks) && (!whiteQueens) and (popCount(whiteBishops | whiteKnights) <= 1);
        } else {
            U64 blackKnights = board.getPieceBB(BLACK, KNIGHT);
            U64 blackBishops = board.getPieceBB(BLACK, BISHOP);
            U64 blackRooks = board.getPieceBB(BLACK, ROOK);
            U64 blackQueens = board.getPieceBB(BLACK, QUEEN);

            return (!blackRooks) && (!blackQueens) and (popCount(blackBishops | blackKnights) <= 1);
        }
    }

    inline bool is_opposite_color_bishops(const Board &board){
        U64 whiteKnights = board.getPieceBB(WHITE, KNIGHT);
        U64 blackKnights = board.getPieceBB(BLACK, KNIGHT);
        U64 whiteBishops = board.getPieceBB(WHITE, BISHOP);
        U64 blackBishops = board.getPieceBB(BLACK, BISHOP);
        U64 whiteRooks = board.getPieceBB(WHITE, ROOK);
        U64 blackRooks = board.getPieceBB(BLACK, ROOK);
        U64 whiteQueens = board.getPieceBB(WHITE, QUEEN);
        U64 blackQueens = board.getPieceBB(BLACK, QUEEN);

        if ((!whiteRooks) && (!blackRooks) && (!whiteKnights) && (! blackKnights) && (!whiteQueens) && (!blackQueens)) {
            int white_bishop_sign = ((whiteBishops & LIGHT_SQUARES) ? 1 : 0) - ((whiteBishops & DARK_SQUARES) ? 1 : 0);
            int black_bishop_sign = ((blackBishops & LIGHT_SQUARES) ? 1 : 0) - ((blackBishops & DARK_SQUARES) ? 1 : 0);
            return white_bishop_sign * black_bishop_sign == -1;
        }
        return false;
    }

    EVAL_PARAM(better_side_pawnless, 70)
    EVAL_PARAM(better_side_one_pawn, 156)
    EVAL_PARAM(better_side_two_pawn, 212)
    EVAL_PARAM(ocb_endgame, 203)

    int make_endgame_adjustment(int raw_eval, const Board &board){
        if (is_opposite_color_bishops(board)) {
            raw_eval = (raw_eval * ocb_endgame) / 256;
        }

        int multiplier = 256;

        if(raw_eval > 0) {
            U64 whitePawns = board.getPieceBB(WHITE, PAWN);

            if (!whitePawns){
                // Better side only has a single minor - checkmate can't be forced
                if (only_has_minor(board, WHITE)) multiplier = 0;
                    // Better side may be able to checkmate, but without pawns it's more difficult
                else multiplier = better_side_pawnless;
            }

            if (popCount(whitePawns) == 1){
                multiplier = better_side_one_pawn;
            }

            if (popCount(whitePawns) == 2){
                multiplier = better_side_two_pawn;
            }
        } else {
            U64 blackPawns = board.getPieceBB(BLACK, PAWN);

            if (!blackPawns){
                // Better side only has a single minor - checkmate can't be forced
                if (only_has_minor(board, BLACK)) multiplier = 0;
                    // Better side may be able to checkmate, but without pawns it's more difficult
                else multiplier = better_side_pawnless;
            }

            if (popCount(blackPawns) == 1){
                multiplier = better_side_one_pawn;
            }

            if (popCount(blackPawns) == 2){
                multiplier = better_side_two_pawn;
            }
        }

        return (raw_eval * multiplier) / 256;
    }

}