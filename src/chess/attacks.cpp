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

#include "attacks.h"

namespace Chess {

    void initRookAttacks() {
        for (Square s = a1; s <= h8; ++s) {
            const U64 edges = (MASK_RANK[FILE_A] | MASK_RANK[FILE_H]) & ~MASK_RANK[squareRank(s)] |
                              (MASK_FILE[FILE_A] | MASK_FILE[FILE_H]) & ~MASK_FILE[squareFile(s)];
            ROOK_ATTACK_MASKS[s] = (MASK_RANK[squareRank(s)] ^ MASK_FILE[squareFile(s)]) & ~edges;
            ROOK_ATTACK_SHIFTS[s] = 64 - popCount(ROOK_ATTACK_MASKS[s]);

            U64 subset = 0;
            do {
                U64 index = subset;
                index = index * ROOK_MAGICS[s];
                index = index >> ROOK_ATTACK_SHIFTS[s];
                ROOK_ATTACKS[s][index] = slidingAttacks(s, subset, MASK_FILE[squareFile(s)]) |
                                         slidingAttacks(s, subset, MASK_RANK[squareRank(s)]);
                subset = (subset - ROOK_ATTACK_MASKS[s]) & ROOK_ATTACK_MASKS[s];
            } while (subset);
        }
    }

    void initBishopAttacks() {
        for (Square s = a1; s <= h8; ++s) {
            const U64 edges = (MASK_RANK[FILE_A] | MASK_RANK[FILE_H]) & ~MASK_RANK[squareRank(s)] |
                              (MASK_FILE[FILE_A] | MASK_FILE[FILE_H]) & ~MASK_FILE[squareFile(s)];
            BISHOP_ATTACK_MASKS[s] = (MASK_DIAGONAL[squareDiag(s)] ^ MASK_ANTI_DIAGONAL[squareAntiDiag(s)]) & ~edges;
            BISHOP_ATTACK_SHIFTS[s] = 64 - popCount(BISHOP_ATTACK_MASKS[s]);

            U64 subset = 0;
            do {
                U64 index = subset;
                index = index * BISHOP_MAGICS[s];
                index = index >> BISHOP_ATTACK_SHIFTS[s];
                BISHOP_ATTACKS[s][index] = slidingAttacks(s, subset, MASK_DIAGONAL[squareDiag(s)]) |
                                           slidingAttacks(s, subset, MASK_ANTI_DIAGONAL[squareAntiDiag(s)]);
                subset = (subset - BISHOP_ATTACK_MASKS[s]) & BISHOP_ATTACK_MASKS[s];
            } while (subset);
        }
    }

    void initLookUpTables() {
        initRookAttacks();
        initBishopAttacks();

        // init pseudo legal getAttacks
        memcpy(PSEUDO_LEGAL_ATTACKS[KNIGHT], KNIGHT_ATTACKS, sizeof(KNIGHT_ATTACKS));
        memcpy(PSEUDO_LEGAL_ATTACKS[KING], KING_ATTACKS, sizeof(KING_ATTACKS));

        for (Square s = a1; s <= h8; ++s) {
            PSEUDO_LEGAL_ATTACKS[ROOK][s] = getRookAttacks(s, 0);
            PSEUDO_LEGAL_ATTACKS[BISHOP][s] = getBishopAttacks(s, 0);
            PSEUDO_LEGAL_ATTACKS[QUEEN][s] = PSEUDO_LEGAL_ATTACKS[ROOK][s] | PSEUDO_LEGAL_ATTACKS[BISHOP][s];
        }

        // init squares between and line (defined in bitboard.h)
        for (Square s1 = a1; s1 <= h8; ++s1) {
            for (Square s2 = a1; s2 <= h8; ++s2) {
                const U64 s = SQUARE_BB[s1] | SQUARE_BB[s2];

                if (squareFile(s1) == squareFile(s2) || squareRank(s1) == squareRank(s2)) {
                    U64 b1 = getRookAttacks(s1, s);
                    U64 b2 = getRookAttacks(s2, s);
                    SQUARES_BETWEEN[s1][s2] = b1 & b2;

                    b1 = getRookAttacks(s1, 0);
                    b2 = getRookAttacks(s2, 0);
                    LINE[s1][s2] = b1 & b2 | SQUARE_BB[s1] | SQUARE_BB[s2];
                } else if (squareDiag(s1) == squareDiag(s2) || squareAntiDiag(s1) == squareAntiDiag(s2)) {
                    U64 b1 = getBishopAttacks(s1, s);
                    U64 b2 = getBishopAttacks(s2, s);
                    SQUARES_BETWEEN[s1][s2] = b1 & b2;

                    b1 = getBishopAttacks(s1, 0);
                    b2 = getBishopAttacks(s2, 0);
                    LINE[s1][s2] = b1 & b2 | SQUARE_BB[s1] | SQUARE_BB[s2];
                }
            }
        }
    }

} // namespace Chess
