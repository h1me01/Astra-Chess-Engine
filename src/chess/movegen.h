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
   along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ASTRA_MOVEGEN_H
#define ASTRA_MOVEGEN_H

#include "board.h"

namespace Chess {

    enum GenType {
        ALL_MOVES,
        CAPTURE_MOVES
    };

    // helper to generate quiet moves
    template<MoveFlags MF = QUIET>
    inline int make(Move *&moves, Square from, U64 to) {
        int numMoves = 0;
        while (to) {
            *moves++ = Move(from, popLsb(&to), MF);
            numMoves++;
        }
        return numMoves;
    }

    // helper to generate promotion moves
    template<>
    inline int make<PROMOTIONS>(Move *&moves, Square from, U64 to) {
        int numMoves = 0;

        while (to) {
            Square s = popLsb(&to);
            *moves++ = Move(from, s, PR_KNIGHT);
            *moves++ = Move(from, s, PR_BISHOP);
            *moves++ = Move(from, s, PR_ROOK);
            *moves++ = Move(from, s, PR_QUEEN);
            numMoves += 4;
        }

        return numMoves;
    }

    // helper to generate promotion capture moves
    template<>
    inline int make<PROMOTION_CAPTURES>(Move *&moves, Square from, U64 to) {
        int numMoves = 0;

        while (to) {
            Square s = popLsb(&to);
            *moves++ = Move(from, s, PC_KNIGHT);
            *moves++ = Move(from, s, PC_BISHOP);
            *moves++ = Move(from, s, PC_ROOK);
            *moves++ = Move(from, s, PC_QUEEN);
            numMoves += 4;
        }

        return numMoves;
    }

    template<Color Us>
    U64 diagonalPawnAttacks(U64 pawns) {
        return Us == WHITE ?
               shift(NORTH_WEST, pawns) | shift(NORTH_EAST, pawns) :
               shift(SOUTH_WEST, pawns) | shift(SOUTH_EAST, pawns);
    }

    template<Color Us>
    U64 dangerMask(const Board &board, U64 occ) {
        constexpr Color them = ~Us;
        const U64 theirPawns = board.getPieceBB(them, PAWN);

        // enemy king attacks
        U64 danger = getAttacks(KING, board.kingSquare(them), occ);
        // enemy pawns attacks
        danger |= diagonalPawnAttacks<them>(theirPawns);

        // enemy knights attacks
        U64 theirKnights = board.getPieceBB(them, KNIGHT);
        while (theirKnights) {
            Square s = popLsb(&theirKnights);
            danger |= getAttacks(KNIGHT, s, occ);
        }

        // exclude our king from the occupancy, because checks should not be included
        occ ^= SQUARE_BB[board.kingSquare(Us)];

        // enemy bishop and queen attacks
        U64 theirDiagSliders = board.getDiagSliders(them);
        while (theirDiagSliders) {
            Square s = popLsb(&theirDiagSliders);
            danger |= getAttacks(BISHOP, s, occ);
        }

        // enemy rook and queen attacks
        U64 theirOrthSliders = board.getOrthSliders(them);
        while (theirOrthSliders) {
            Square s = popLsb(&theirOrthSliders);
            danger |= getAttacks(ROOK, s, occ);
        }

        return danger;
    }

    template<Color Us>
    U64 checkerMask(const Board &board, const Square &kingSq, U64 &pinned) {
        constexpr Color them = ~Us;
        const U64 theirOcc = board.getOccupancy(them);
        const U64 ourOcc = board.getOccupancy(Us);

        // enemy pawns attacks at our king
        U64 checkers = pawnAttacks(Us, kingSq) & board.getPieceBB(them, PAWN);;
        // enemy knights attacks at our king
        checkers |= getAttacks(KNIGHT, kingSq, ourOcc | theirOcc) & board.getPieceBB(them, KNIGHT);

        // potential enemy bishop, rook and queen attacks at our king
        U64 candidates =
                getAttacks(ROOK, kingSq, theirOcc) & board.getOrthSliders(them)
                | getAttacks(BISHOP, kingSq, theirOcc) & board.getDiagSliders(them);

        pinned = 0;
        while (candidates) {
            Square s = popLsb(&candidates);
            U64 blockers = SQUARES_BETWEEN[kingSq][s] & ourOcc;

            // if between the enemy slider attack and our king is no of our pieces
            // add the enemy piece to the checkers bitboard
            if (blockers == 0) {
                checkers ^= SQUARE_BB[s];
            } else if ((blockers & blockers - 1) == 0) {
                // if there is only one of our piece between them, add our piece to the pinned
                pinned ^= blockers;
            }
        }

        return checkers;
    }

    template<Color Us>
    int genCastlingMoves(const Board &board, Move *&moves, U64 occ, U64 danger) {
        int numMoves = 0;
        const U64 castleMask = board.history[board.getPly()].entry;

        // checks if king would be in check if it moved to the castling square
        U64 possibleChecks = (occ | danger) & shortCastlingBlockersMask(Us);
        // checks if king and the h-rook have moved
        U64 isAllowed = castleMask & shortCastlingMask(Us);

        if (!(possibleChecks | isAllowed)) {
            *moves++ = Us == WHITE ?
                       Move(e1, h1, OO) :
                       Move(e8, h8, OO);
            numMoves++;
        }

        // ignoreLongCastlingDanger is used to get rid of the danger on the possibleChecks or b8 square
        possibleChecks = (occ | (danger & ~ignoreLongCastlingDanger(Us))) & longCastlingBlockersMask(Us);
        isAllowed = castleMask & longCastlingMask(Us);

        if (!(possibleChecks | isAllowed)) {
            *moves++ = Us == WHITE ?
                       Move(e1, c1, OOO) :
                       Move(e8, c8, OOO);
            numMoves++;
        }

        return numMoves;
    }

    template<Color Us, GenType type>
    int genPieceMoves(const Board &board, Move *&moves, U64 occ, U64 quietMask, U64 captureMask, int checkersCount) {
        int numMoves = 0;

        // used to store square
        Square s;
        // used to store piece attack squares;
        U64 attacks;

        // pinned pieces cannot move if king is in check
        if (checkersCount == 0) {
            const Square ourKingSq = board.kingSquare(Us);

            // for each pinned rook, bishop or queen
            U64 pinnedPieces = ~(~board.pinned | board.getPieceBB(Us, KNIGHT));
            while (pinnedPieces) {
                s = popLsb(&pinnedPieces);
                // only include getAttacks that are aligned with our king
                attacks = getAttacks(typeOfPiece(board.getPiece(s)), s, occ) & LINE[ourKingSq][s];

                numMoves += make<CAPTURE>(moves, s, attacks & captureMask);
                if (type != CAPTURE_MOVES) {
                    numMoves += make<QUIET>(moves, s, attacks & quietMask);
                }
            }
        }

        // knight moves
        U64 ourKnights = board.getPieceBB(Us, KNIGHT) & ~board.pinned;
        while (ourKnights) {
            s = popLsb(&ourKnights);
            attacks = getAttacks(KNIGHT, s, occ);

            numMoves += make<CAPTURE>(moves, s, attacks & captureMask);
            if (type != CAPTURE_MOVES) {
                numMoves += make<QUIET>(moves, s, attacks & quietMask);
            }
        }

        // bishops and queens
        U64 ourDiagSliders = board.getDiagSliders(Us) & ~board.pinned;
        while (ourDiagSliders) {
            s = popLsb(&ourDiagSliders);
            attacks = getAttacks(BISHOP, s, occ);

            numMoves += make<CAPTURE>(moves, s, attacks & captureMask);
            if (type != CAPTURE_MOVES) {
                numMoves += make<QUIET>(moves, s, attacks & quietMask);
            }
        }

        // rooks and queens
        U64 ourOrthSliders = board.getOrthSliders(Us) & ~board.pinned;
        while (ourOrthSliders) {
            s = popLsb(&ourOrthSliders);
            attacks = getAttacks(ROOK, s, occ);

            numMoves += make<CAPTURE>(moves, s, attacks & captureMask);
            if (type != CAPTURE_MOVES) {
                numMoves += make<QUIET>(moves, s, attacks & quietMask);
            }
        }

        return numMoves;
    }

    template<Color Us, GenType type>
    int genPawnMoves(const Board &board, Move *&moves, U64 occ, U64 quietMask, U64 captureMask, int checkersCount) {
        int numMoves = 0;
        const Square epSq = board.history[board.getPly()].epSquare;
        const Square ourKingSq = board.kingSquare(Us);

        // used to store square
        Square s;

        // pinned pawns cannot move if king is in check
        if (checkersCount == 0) {
            // for each pinned pawn
            U64 pinnedPawns = board.pinned & board.getPieceBB(Us, PAWN);
            while (pinnedPawns) {
                s = popLsb(&pinnedPawns);

                if (squareRank(s) == relativeRank(Us, RANK_7)) {
                    U64 attacks = pawnAttacks(Us, s) & captureMask & LINE[ourKingSq][s];
                    // quiet promotions are impossible since it would leave the king in check
                    numMoves += make<PROMOTION_CAPTURES>(moves, s, attacks);
                } else {
                    U64 attacks = pawnAttacks(Us, s) & captureMask & LINE[s][ourKingSq];
                    numMoves += make<CAPTURE>(moves, s, attacks);

                    if (type != CAPTURE_MOVES) {
                        U64 singlePush = shift(relativeDir(Us, NORTH), SQUARE_BB[s]) & ~occ & LINE[ourKingSq][s];
                        numMoves += make<QUIET>(moves, s, singlePush);

                        U64 doublePush = shift(relativeDir(Us, NORTH), singlePush & MASK_RANK[relativeRank(Us, RANK_3)]);
                        numMoves += make<DOUBLE_PUSH>(moves, s, doublePush & ~occ & LINE[ourKingSq][s]);
                    }
                }
            }

            // e.p. moves
            if (epSq != NO_SQUARE) {
                const U64 theirOrthSliders = board.getOrthSliders(~Us);

                U64 epCaptureBB = pawnAttacks(~Us, epSq) & board.getPieceBB(Us, PAWN);
                U64 canCapture = epCaptureBB & ~board.pinned;

                while (canCapture) {
                    s = popLsb(&canCapture);
                    // pseudo-pinned e.p. case
                    U64 newOccMask = occ ^ SQUARE_BB[s] ^ shift(relativeDir(Us, SOUTH), SQUARE_BB[epSq]);
                    U64 attacker = slidingAttacks(ourKingSq, newOccMask, MASK_RANK[squareRank(ourKingSq)]);

                    if ((attacker & theirOrthSliders) == 0) {
                        *moves++ = Move(s, epSq, EN_PASSANT);
                        numMoves++;
                    }
                }

                // pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king
                canCapture = epCaptureBB & board.pinned & LINE[epSq][ourKingSq];
                if (canCapture) {
                    *moves++ = Move(bsf(canCapture), epSq, EN_PASSANT);
                    numMoves++;
                }
            }
        }

        // contains non-pinned pawns which are not on the last rank
        U64 ourPawns = board.getPieceBB(Us, PAWN) & ~board.pinned & ~MASK_RANK[relativeRank(Us, RANK_7)];
        // single pawn pushes
        U64 singlePush = shift(relativeDir(Us, NORTH), ourPawns) & ~occ;
        // double pawn pushes (only the ones that are on rank 3/6 are considered)
        U64 doublePush = shift(relativeDir(Us, NORTH), singlePush & MASK_RANK[relativeRank(Us, RANK_3)]) & quietMask;
        // quiet mask is applied later, to consider the possibility of a double push blocking a check
        singlePush &= quietMask;

        while (singlePush && type != CAPTURE_MOVES) {
            s = popLsb(&singlePush);
            *moves++ = Move(s - relativeDir(Us, NORTH), s, QUIET);
            numMoves++;
        }

        while (doublePush && type != CAPTURE_MOVES) {
            s = popLsb(&doublePush);
            *moves++ = Move(s - relativeDir(Us, NORTH_NORTH), s, DOUBLE_PUSH);
            numMoves++;
        }

        // pawn captures
        U64 leftCaptures = shift(relativeDir(Us, NORTH_WEST), ourPawns) & captureMask;
        while (leftCaptures) {
            s = popLsb(&leftCaptures);
            *moves++ = Move(s - relativeDir(Us, NORTH_WEST), s, CAPTURE);
            numMoves++;
        }

        U64 rightCaptures = shift(relativeDir(Us, NORTH_EAST), ourPawns) & captureMask;
        while (rightCaptures) {
            s = popLsb(&rightCaptures);
            *moves++ = Move(s - relativeDir(Us, NORTH_EAST), s, CAPTURE);
            numMoves++;
        }

        return numMoves;
    }

    template<Color Us, GenType type>
    int genPromotionMoves(const Board &board, Move *moves, U64 quietMask, U64 captureMask) {
        int numMoves = 0;

        // contains non-pinned pawns which are on the last rank
        U64 ourPawns = board.getPieceBB(Us, PAWN) & ~board.pinned & MASK_RANK[relativeRank(Us, RANK_7)];
        if (ourPawns) {
            // attacks contains squares that the pawns can move to
            // quiet promotions
            U64 attacks = shift(relativeDir(Us, NORTH), ourPawns) & quietMask;
            while (attacks && type != CAPTURE_MOVES) {
                Square s = popLsb(&attacks);
                for (MoveFlags mf: {PR_KNIGHT, PR_BISHOP, PR_ROOK, PR_QUEEN}) {
                    *moves++ = Move(s - relativeDir(Us, NORTH), s, mf);
                }
                numMoves += 4;
            }

            // promotion captures
            attacks = shift(relativeDir(Us, NORTH_WEST), ourPawns) & captureMask;
            while (attacks) {
                Square s = popLsb(&attacks);
                for (MoveFlags mf: {PC_KNIGHT, PC_BISHOP, PC_ROOK, PC_QUEEN}) {
                    *moves++ = Move(s - relativeDir(Us, NORTH_WEST), s, mf);
                }
                numMoves += 4;
            }

            attacks = shift(relativeDir(Us, NORTH_EAST), ourPawns) & captureMask;
            while (attacks) {
                Square s = popLsb(&attacks);
                for (MoveFlags mf: {PC_KNIGHT, PC_BISHOP, PC_ROOK, PC_QUEEN}) {
                    *moves++ = Move(s - relativeDir(Us, NORTH_EAST), s, mf);
                }
                numMoves += 4;
            }
        }

        return numMoves;
    }

    template<Color Us, GenType type>
    int genLegalMoves(Board &board, Move *moves) {
        int numMoves = 0;
        const Color them = ~Us;
        const Square epSq = board.history[board.getPly()].epSquare;
        const Square ourKingSq = board.kingSquare(Us);
        const U64 ourOcc = board.getOccupancy(Us);
        const U64 theirOcc = board.getOccupancy(them);
        const U64 occ = ourOcc | theirOcc;
        const U64 danger = dangerMask<Us>(board, occ);

        board.checkers = checkerMask<Us>(board, ourKingSq, board.pinned);

        // generate king moves
        U64 b = getAttacks(KING, ourKingSq, occ) & ~(ourOcc | danger);

        numMoves += make<CAPTURE>(moves, ourKingSq, b & theirOcc);
        if (type != CAPTURE_MOVES) {
            numMoves += make<QUIET>(moves, ourKingSq, b & ~theirOcc);
        }

        // if double check, then only king moves are legal
        int checkersCount = sparsePopCount(board.checkers);
        if (checkersCount == 2) {
            return numMoves;
        }

        // hold captures squares and quiet squares
        U64 captureMask, quietMask;

        // single check
        if (checkersCount == 1) {
            Square checkerSquare = bsf(board.checkers);
            Piece checkerPiece = board.getPiece(checkerSquare);

            // holds our pieces that can capture the checking piece
            U64 canCapture;

            if (checkerPiece == makePiece(them, PAWN)) {
                // if the checker is a pawn, we must check for e.p. moves that can capture it
                if (board.checkers == shift(relativeDir(Us, SOUTH), SQUARE_BB[epSq])) {
                    const U64 ourPawns = board.getPieceBB(Us, PAWN);

                    // b1 contains our pawns that can capture the checker e.p.
                    canCapture = pawnAttacks(them, epSq) & ourPawns & ~board.pinned;
                    while (canCapture) {
                        *moves++ = Move(popLsb(&canCapture), epSq, EN_PASSANT);
                        numMoves++;
                    }
                }
            }

            // if checker is either a pawn or a knight, the only legal moves are to capture the checker
            if (checkerPiece == makePiece(them, KNIGHT)) {
                canCapture = board.isAttacked(Us, checkerSquare, occ) & ~board.pinned;
                while (canCapture) {
                    *moves++ = Move(popLsb(&canCapture), checkerSquare, CAPTURE);
                    numMoves++;
                }
                return numMoves;
            }

            // we must capture the checking piece
            captureMask = board.checkers;
            // or we can block it
            quietMask = SQUARES_BETWEEN[ourKingSq][checkerSquare];
        } else {
            // we can capture any enemy piece
            captureMask = theirOcc;
            // and we can move to any square which is not occupied
            quietMask = ~occ;

            if (type != CAPTURE_MOVES) {
                numMoves += genCastlingMoves<Us>(board, moves, occ, danger);
            }
        }

        numMoves += genPieceMoves<Us, type>(board, moves, occ, quietMask, captureMask, checkersCount);
        numMoves += genPawnMoves<Us, type>(board, moves, occ, quietMask, captureMask, checkersCount);
        numMoves += genPromotionMoves<Us, type>(board, moves, quietMask, captureMask);

        return numMoves;
    }

    template<GenType type = ALL_MOVES>
    int genLegalMoves(Board &board, Move *moves) {
        return board.sideToMove() == WHITE ?
               genLegalMoves<WHITE, type>(board, moves) :
               genLegalMoves<BLACK, type>(board, moves);
    }

} // namespace Chess


#endif //ASTRA_MOVEGEN_H
