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

#ifndef ASTRA_ATTACKS_H
#define ASTRA_ATTACKS_H

#include "bitboard.h"

namespace Chess {

    const U64 KING_ATTACKS[NUM_SQUARES] = {
            0x302, 0x705, 0xe0a, 0x1c14,
            0x3828, 0x7050, 0xe0a0, 0xc040,
            0x30203, 0x70507, 0xe0a0e, 0x1c141c,
            0x382838, 0x705070, 0xe0a0e0, 0xc040c0,
            0x3020300, 0x7050700, 0xe0a0e00, 0x1c141c00,
            0x38283800, 0x70507000, 0xe0a0e000, 0xc040c000,
            0x302030000, 0x705070000, 0xe0a0e0000, 0x1c141c0000,
            0x3828380000, 0x7050700000, 0xe0a0e00000, 0xc040c00000,
            0x30203000000, 0x70507000000, 0xe0a0e000000, 0x1c141c000000,
            0x382838000000, 0x705070000000, 0xe0a0e0000000, 0xc040c0000000,
            0x3020300000000, 0x7050700000000, 0xe0a0e00000000, 0x1c141c00000000,
            0x38283800000000, 0x70507000000000, 0xe0a0e000000000, 0xc040c000000000,
            0x302030000000000, 0x705070000000000, 0xe0a0e0000000000, 0x1c141c0000000000,
            0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
            0x203000000000000, 0x507000000000000, 0xa0e000000000000, 0x141c000000000000,
            0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000,
    };

    const U64 KNIGHT_ATTACKS[NUM_SQUARES] = {
            0x20400, 0x50800, 0xa1100, 0x142200,
            0x284400, 0x508800, 0xa01000, 0x402000,
            0x2040004, 0x5080008, 0xa110011, 0x14220022,
            0x28440044, 0x50880088, 0xa0100010, 0x40200020,
            0x204000402, 0x508000805, 0xa1100110a, 0x1422002214,
            0x2844004428, 0x5088008850, 0xa0100010a0, 0x4020002040,
            0x20400040200, 0x50800080500, 0xa1100110a00, 0x142200221400,
            0x284400442800, 0x508800885000, 0xa0100010a000, 0x402000204000,
            0x2040004020000, 0x5080008050000, 0xa1100110a0000, 0x14220022140000,
            0x28440044280000, 0x50880088500000, 0xa0100010a00000, 0x40200020400000,
            0x204000402000000, 0x508000805000000, 0xa1100110a000000, 0x1422002214000000,
            0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
            0x400040200000000, 0x800080500000000, 0x1100110a00000000, 0x2200221400000000,
            0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
            0x4020000000000, 0x8050000000000, 0x110a0000000000, 0x22140000000000,
            0x44280000000000, 0x0088500000000000, 0x0010a00000000000, 0x20400000000000
    };

    const U64 PAWN_ATTACKS[NUM_COLORS][NUM_SQUARES] = {
            { // white pawn getAttacks
                    0x200, 0x500, 0xa00, 0x1400,
                    0x2800, 0x5000, 0xa000, 0x4000,
                    0x20000, 0x50000, 0xa0000, 0x140000,
                    0x280000, 0x500000, 0xa00000, 0x400000,
                    0x2000000, 0x5000000, 0xa000000, 0x14000000,
                    0x28000000, 0x50000000, 0xa0000000, 0x40000000,
                    0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
                    0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
                    0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
                    0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
                    0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
                    0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
                    0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
                    0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
                    0x0, 0x0, 0x0, 0x0,
                    0x0, 0x0, 0x0, 0x0,
            },
            { // black pawn getAttacks
                    0x0, 0x0, 0x0, 0x0,
                    0x0, 0x0, 0x0, 0x0,
                    0x2, 0x5, 0xa, 0x14,
                    0x28, 0x50, 0xa0, 0x40,
                    0x200, 0x500, 0xa00, 0x1400,
                    0x2800, 0x5000, 0xa000, 0x4000,
                    0x20000, 0x50000, 0xa0000, 0x140000,
                    0x280000, 0x500000, 0xa00000, 0x400000,
                    0x2000000, 0x5000000, 0xa000000, 0x14000000,
                    0x28000000, 0x50000000, 0xa0000000, 0x40000000,
                    0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
                    0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
                    0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
                    0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
                    0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
                    0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
            }
    };

    inline U64 PSEUDO_LEGAL_ATTACKS[NUM_PIECE_TYPES][NUM_SQUARES];

    inline U64 ROOK_ATTACK_MASKS[NUM_SQUARES];
    inline int ROOK_ATTACK_SHIFTS[NUM_SQUARES];
    inline U64 ROOK_ATTACKS[NUM_SQUARES][4096];

    inline U64 BISHOP_ATTACK_MASKS[NUM_SQUARES];
    inline int BISHOP_ATTACK_SHIFTS[NUM_SQUARES];
    inline U64 BISHOP_ATTACKS[NUM_SQUARES][512];

    // calculates sliding getAttacks from a given square, on a given axis
    // this uses the Hyperbola Quintessence Algorithm
    inline U64 slidingAttacks(Square s, U64 occ, U64 mask) {
        U64 maskOcc = mask & occ;
        return ((maskOcc - SQUARE_BB[s] * 2) ^ reverse(reverse(maskOcc) - reverse(SQUARE_BB[s]) * 2)) & mask;
    }

    inline U64 getRookAttacks(Square s, U64 occ) {
        U64 maskedOcc = occ & ROOK_ATTACK_MASKS[s];
        U64 index = (maskedOcc * ROOK_MAGICS[s]) >> ROOK_ATTACK_SHIFTS[s];
        return ROOK_ATTACKS[s][index];
    }

    inline U64 getBishopAttacks(Square s, U64 occ) {
        U64 maskedOcc = occ & BISHOP_ATTACK_MASKS[s];
        U64 index = (maskedOcc * BISHOP_MAGICS[s]) >> BISHOP_ATTACK_SHIFTS[s];
        return BISHOP_ATTACKS[s][index];
    }

    void initLookUpTables();

    // attacks from a given square (doesn't include pawn getAttacks)
    constexpr U64 getAttacks(PieceType pt, Square s, U64 occ) {
        switch (pt) {
            case ROOK:
                return getRookAttacks(s, occ);
            case BISHOP:
                return getBishopAttacks(s, occ);
            case QUEEN:
                return getRookAttacks(s, occ) | getBishopAttacks(s, occ);
            case KNIGHT:
            case KING:
                return PSEUDO_LEGAL_ATTACKS[pt][s];
            default:
                return 0;
        }

        return 0;
    }

    // pawn getAttacks from a given square
    constexpr U64 pawnAttacks(Color c, Square s) {
        return PAWN_ATTACKS[c][s];
    }

} // namespace Chess


#endif //ASTRA_ATTACKS_H
