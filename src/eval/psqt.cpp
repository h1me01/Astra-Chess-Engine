# include "psqt.h"
# include "hashing.h"
# include "weights/endgame.h"
# include "weights/oscmiddlegame.h"
# include "weights/sscmiddlegame.h"
# include "weights/phasecount.h"

#define FlipIf(cond, X) ((cond) ? ((X) ^ 56) : (X))
#define RotIf(cond, X) ((cond) ? ((X) ^ 63) : (X))

namespace Eval {

    constexpr uint64_t hash_diff(std::array<uint64_t, 64> table, Square from, Square to) {
        return table[from] ^ table[to];
    }

#define DEFINE_MOVE_FUNCTION(piece) \
void PstEvalInfo::move_##piece(Color white, Square from, Square to){ \
    const int sign = white ? 1 : -1; \
    mg_kk += sign * (ssc_mg_##piece##_table[FlipIf(white, to)] - ssc_mg_##piece##_table[FlipIf(white, from)]); \
    mg_qk += sign * (osc_mg_##piece##_table[RotIf(white, to ^ 7)] - osc_mg_##piece##_table[RotIf(white, from ^ 7)]); \
    mg_kq += sign * (osc_mg_##piece##_table[RotIf(white, to)] - osc_mg_##piece##_table[RotIf(white, from)]); \
    mg_qq += sign * (ssc_mg_##piece##_table[FlipIf(white, to ^ 7)] - ssc_mg_##piece##_table[FlipIf(white, from ^ 7)]); \
    eg += sign * (eg_##piece##_table[FlipIf(white, to)] - eg_##piece##_table[FlipIf(white, from)]); \
    hash ^= hash_diff(white ? white_##piece##_hash : black_##piece##_hash, from, to); \
}

    DEFINE_MOVE_FUNCTION(pawn);
    DEFINE_MOVE_FUNCTION(knight);
    DEFINE_MOVE_FUNCTION(bishop);
    DEFINE_MOVE_FUNCTION(rook);
    DEFINE_MOVE_FUNCTION(queen);
    DEFINE_MOVE_FUNCTION(king);

#define DEFINE_REMOVE_FUNCTION(piece) \
void PstEvalInfo::remove_##piece(Color white, Square square) { \
    const int sign = white ? -1 : 1; \
    mg_kk += sign * (ssc_mg_##piece##_table[FlipIf(white, square)] + ssc_mg_##piece); \
    mg_qk += sign * (osc_mg_##piece##_table[RotIf(white, square ^ 7)] + osc_mg_##piece); \
    mg_kq += sign * (osc_mg_##piece##_table[RotIf(white, square)] + osc_mg_##piece); \
    mg_qq += sign * (ssc_mg_##piece##_table[FlipIf(white, square ^ 7)] + ssc_mg_##piece); \
    eg += sign * (eg_##piece##_table[FlipIf(white, square)] + eg_##piece); \
    phase_count -= pc_##piece; \
    hash ^= (white ? white_##piece##_hash : black_##piece##_hash)[square]; \
}

    DEFINE_REMOVE_FUNCTION(pawn);
    DEFINE_REMOVE_FUNCTION(knight);
    DEFINE_REMOVE_FUNCTION(bishop);
    DEFINE_REMOVE_FUNCTION(rook);
    DEFINE_REMOVE_FUNCTION(queen);

#define DEFINE_PROMOTE_FUNCTION(piece) \
void PstEvalInfo::promote_pawn_to_##piece(Color white, Square from,  Square to) { \
    const int sign = white ? 1 : -1; \
    mg_kk += sign * (ssc_mg_##piece##_table[FlipIf(white, to)] + ssc_mg_##piece -  \
        ssc_mg_pawn_table[FlipIf(white, from)] - ssc_mg_pawn); \
    mg_qk += sign * (osc_mg_##piece##_table[RotIf(white, to ^ 7)] + osc_mg_##piece -  \
        osc_mg_pawn_table[RotIf(white, from ^ 7)] - osc_mg_pawn); \
    mg_kq += sign * (osc_mg_##piece##_table[RotIf(white, to)] + osc_mg_##piece -  \
        osc_mg_pawn_table[RotIf(white, from)] - osc_mg_pawn); \
    mg_qq += sign * (ssc_mg_##piece##_table[FlipIf(white, to ^ 7)] + ssc_mg_##piece - \
        ssc_mg_pawn_table[FlipIf(white, from ^ 7)] - ssc_mg_pawn); \
    eg += sign * (eg_##piece##_table[FlipIf(white, to)] + eg_##piece - eg_pawn_table[FlipIf(white, from)] - eg_pawn); \
    phase_count += pc_##piece - pc_pawn; \
    hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_##piece##_hash : black_##piece##_hash)[to]; \
}

    DEFINE_PROMOTE_FUNCTION(knight);
    DEFINE_PROMOTE_FUNCTION(bishop);
    DEFINE_PROMOTE_FUNCTION(rook);
    DEFINE_PROMOTE_FUNCTION(queen);


    void PstEvalInfo::remove_bishop_pair_bonus(Color white) {
        auto sign = white ? 1 : -1;
        mg_kk -= ssc_mg_bishop_pair * sign;
        mg_qk -= osc_mg_bishop_pair * sign;
        mg_kq -= osc_mg_bishop_pair * sign;
        mg_qq -= ssc_mg_bishop_pair * sign;
        eg -= eg_bishop_pair * sign;
    }

}