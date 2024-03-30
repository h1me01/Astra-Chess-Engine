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

#include "movegen.h"
#include "board.h"

namespace Chess {

    int genCastlingMoves(Board &board, Move *&moves, U64 occ, U64 danger) {
        int numMoves = 0;
        const Color us = board.sideToMove();
        const U64 castleMask = board.history[board.getPly()].entry;

        // b1, b2 are multi-purpose bitboards
        U64 b1, b2;

        // checks if king would be in check if it moved to the castling square
        b1 = (occ | danger) & shortCastlingBlockersMask(us);
        // checks if king and the h-rook have moved
        b2 = castleMask & shortCastlingMask(us);

        if (!(b1 | b2)) {
            *moves++ = us == WHITE ? Move(e1, h1, OO) : Move(e8, h8, OO);
            numMoves++;
        }

        // ignoreLongCastlingDanger is used to get rid of the danger on the b1 or b8 square
        b1 = (occ | (danger & ~ignoreLongCastlingDanger(us))) & longCastlingBlockersMask(us);
        b2 = castleMask & longCastlingMask(us);

        if (!(b1 | b2)) {
            *moves++ = us == WHITE ? Move(e1, c1, OOO) : Move(e8, c8, OOO);
            numMoves++;
        }

        return numMoves;
    }

    int genEnPassantMoves(Board &board, Move *&moves, U64 occ, U64 pinned) {
        int numMoves = 0;
        const Color us = board.sideToMove();
        const Square ourKing = board.kingSquare(us);
        const Square epSquare = board.history[board.getPly()].epSquare;

        U64 epCaptureBB = pawnAttacks(~us, epSquare) & board.getPieceBB(us, PAWN);
        // b is a multi-purpose bitboard
        U64 b = epCaptureBB & ~pinned;

        while (b) {
            Square s = popLsb(&b);
            // pseudo-pinned e.p. case
            U64 newOccMask = occ ^ SQUARE_BB[s] ^ shift(relativeDir(us, SOUTH), SQUARE_BB[epSquare]);
            U64 slidingAttack = slidingAttacks(ourKing, newOccMask, MASK_RANK[squareRank(ourKing)]);

            if ((slidingAttack & board.getOrthSliders(~us)) == 0) {
                *moves++ = Move(s, epSquare, EN_PASSANT);
                numMoves++;
            }
        }

        // pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king
        b = epCaptureBB & pinned & LINE[epSquare][ourKing];
        if (b) {
            *moves++ = Move(bsf(b), epSquare, EN_PASSANT);
            numMoves++;
        }

        return numMoves;
    }

    int genPinnedPieceMoves(Board &board, Move *&moves, U64 occ, U64 captureMask, U64 notPinned) {
        int numMoves = 0;
        const Color us = board.sideToMove();
        const Square ourKing = board.kingSquare(us);
        const U64 quietMask = ~occ;

        // b1, b2, b3 are multi-purpose bitboards
        U64 b1, b2, b3;

        // for each pinned rook, bishop or queen
        b1 = ~(notPinned | board.getPieceBB(us, KNIGHT));
        while (b1) {
            Square s = popLsb(&b1);
            // only include getAttacks that are aligned with our king
            b2 = getAttacks(typeOfPiece(board.getPiece(s)), s, occ) & LINE[ourKing][s];

            numMoves += make<CAPTURE>(moves, s, b2 & captureMask);
            numMoves += make<QUIET>(moves, s, b2 & quietMask);
        }

        // for each pinned pawn
        b1 = ~notPinned & board.getPieceBB(us, PAWN);
        while (b1) {
            Square s = popLsb(&b1);

            if (squareRank(s) == relativeRank(us, RANK_7)) {
                // quiet promotions are impossible since the square in front of the pawn will
                // either be occupied by the king or the pinner, or doing so would leave our king in check
                b2 = pawnAttacks(us, s) & captureMask & LINE[ourKing][s];
                numMoves += make<PROMOTION_CAPTURES>(moves, s, b2);
            } else {
                b2 = pawnAttacks(us, s) & captureMask & LINE[s][ourKing];
                numMoves += make<CAPTURE>(moves, s, b2);

                // single pawn pushes
                b2 = shift(relativeDir(us, NORTH), SQUARE_BB[s]) & ~occ & LINE[ourKing][s];
                // double pawn pushes
                b3 = shift(relativeDir(us, NORTH), b2 & MASK_RANK[relativeRank(us, RANK_3)]);
                numMoves += make<QUIET>(moves, s, b2);
                numMoves += make<DOUBLE_PUSH>(moves, s, b3 & ~occ & LINE[ourKing][s]);
            }
        }

        return numMoves;
    }

    int genNonPinnedPieceMoves(Board &board, Move *&moves, U64 occ, U64 quietMask, U64 captureMask, U64 notPinned) {
        int numMoves = 0;
        const Color us = board.sideToMove();

        // b1, b2 are multi-purpose bitboards
        U64 b1, b2;

        // non-pinned knight moves
        b1 = board.getPieceBB(us, KNIGHT) & notPinned;
        while (b1) {
            Square s = popLsb(&b1);
            b2 = getAttacks(KNIGHT, s, occ);

            numMoves += make<CAPTURE>(moves, s, b2 & captureMask);
            numMoves += make<QUIET>(moves, s, b2 & quietMask);
        }

        // non-pinned bishops and queens
        b1 = board.getDiagSliders(us) & notPinned;
        while (b1) {
            Square s = popLsb(&b1);
            b2 = getAttacks(BISHOP, s, occ);

            numMoves += make<CAPTURE>(moves, s, b2 & captureMask);
            numMoves += make<QUIET>(moves, s, b2 & quietMask);
        }

        // non-pinned rooks and queens
        b1 = board.getOrthSliders(us) & notPinned;
        while (b1) {
            Square s = popLsb(&b1);
            b2 = getAttacks(ROOK, s, occ);

            numMoves += make<CAPTURE>(moves, s, b2 & captureMask);
            numMoves += make<QUIET>(moves, s, b2 & quietMask);
        }

        return numMoves;
    }

    int genNonPinnedPawnMoves(Board &board, Move *&moves, U64 occ, U64 quietMask, U64 captureMask, U64 notPinned) {
        int numMoves = 0;
        const Color us = board.sideToMove();

        Square s;
        U64 b1, b2, b3;

        // contains non-pinned pawns which are not on the last rank
        b1 = board.getPieceBB(us, PAWN) & notPinned & ~MASK_RANK[relativeRank(us, RANK_7)];
        // single pawn pushes
        b2 = shift(relativeDir(us, NORTH), b1) & ~occ;
        // double pawn pushes (only the ones that are on rank 3/6 are considered)
        b3 = shift(relativeDir(us, NORTH), b2 & MASK_RANK[relativeRank(us, RANK_3)]) & quietMask;
        // quiet mask is applied later, to consider the possibility of a double push blocking a check
        b2 &= quietMask;

        while (b2) {
            s = popLsb(&b2);
            *moves++ = Move(s - relativeDir(us, NORTH), s, QUIET);
            numMoves++;
        }

        while (b3) {
            s = popLsb(&b3);
            *moves++ = Move(s - relativeDir(us, NORTH_NORTH), s, DOUBLE_PUSH);
            numMoves++;
        }

        // pawn captures
        b2 = shift(relativeDir(us, NORTH_WEST), b1) & captureMask;
        b3 = shift(relativeDir(us, NORTH_EAST), b1) & captureMask;

        while (b2) {
            s = popLsb(&b2);
            *moves++ = Move(s - relativeDir(us, NORTH_WEST), s, CAPTURE);
            numMoves++;
        }

        while (b3) {
            s = popLsb(&b3);
            *moves++ = Move(s - relativeDir(us, NORTH_EAST), s, CAPTURE);
            numMoves++;
        }

        return numMoves;
    }

    int genPromotionMoves(Board &board, Move *moves, U64 quietMask, U64 captureMask, U64 notPinned) {
        int numMoves = 0;
        const Color us = board.sideToMove();

        // b1, b2, b3 are multi-purpose bitboards
        U64 b1, b2, b3;

        // contains non-pinned pawns which are on the last rank
        b1 = board.getPieceBB(us, PAWN) & notPinned & MASK_RANK[relativeRank(us, RANK_7)];
        if (b1) {
            // quiet promotions
            b2 = shift(relativeDir(us, NORTH), b1) & quietMask;
            while (b2) {
                Square s = popLsb(&b2);
                for (MoveFlags mf: {PR_KNIGHT, PR_BISHOP, PR_ROOK, PR_QUEEN}) {
                    *moves++ = Move(s - relativeDir(us, NORTH), s, mf);
                }
                numMoves += 4;
            }

            // promotion captures
            b2 = shift(relativeDir(us, NORTH_WEST), b1) & captureMask;
            while (b2) {
                Square s = popLsb(&b2);
                for (MoveFlags mf: {PC_KNIGHT, PC_BISHOP, PC_ROOK, PC_QUEEN}) {
                    *moves++ = Move(s - relativeDir(us, NORTH_WEST), s, mf);
                }
                numMoves += 4;
            }

            b3 = shift(relativeDir(us, NORTH_EAST), b1) & captureMask;
            while (b3) {
                Square s = popLsb(&b3);
                for (MoveFlags mf: {PC_KNIGHT, PC_BISHOP, PC_ROOK, PC_QUEEN}) {
                    *moves++ = Move(s - relativeDir(us, NORTH_EAST), s, mf);
                }
                numMoves += 4;
            }
        }

        return numMoves;
    }

    int Board::genLegalMoves(Move *moves) {
        int numMoves = 0;
        const Color us = stm;
        const Color them = ~us;
        const Square ourKing = kingSquare(us);
        const U64 ourOcc = getOccupancy(us);
        const U64 theirOcc = getOccupancy(them);
        const U64 occ = ourOcc | theirOcc;
        const U64 theirDiagSliders = getDiagSliders(them);
        const U64 theirOrthSliders = getOrthSliders(them);

        // b is a multi-purpose bitboard
        U64 b;

        // squares that our king cannot move to for each enemy piece, add occ of its getAttacks to the danger bitboard
        b = getPieceBB(them, PAWN);
        U64 danger = getAttacks(KING, kingSquare(them), occ);
        danger |= them == WHITE ? shift(NORTH_WEST, b) | shift(NORTH_EAST, b) :
                  shift(SOUTH_WEST, b) | shift(SOUTH_EAST, b);

        b = getPieceBB(them, KNIGHT);
        while (b) {
            danger |= getAttacks(KNIGHT, popLsb(&b), occ);
        }

        // occ ^ SQUARE_BB[ourKing] is written to prevent the king from moving to squares
        // which are 'x-rayed' by enemy bishops and queens
        b = theirDiagSliders;
        while (b) {
            danger |= getAttacks(BISHOP, popLsb(&b), occ ^ SQUARE_BB[ourKing]);
        }

        // occ ^ SQUARE_BB[ourKing] is written to prevent the king from moving to squares
        // which are 'x-rayed' by enemy rooks and queens
        b = theirOrthSliders;
        while (b) {
            danger |= getAttacks(ROOK, popLsb(&b), occ ^ SQUARE_BB[ourKing]);
        }

        // generate legal king moves
        b = getAttacks(KING, ourKing, occ) & ~(ourOcc | danger);
        numMoves += make<CAPTURE>(moves, ourKing, b & theirOcc);
        numMoves += make<QUIET>(moves, ourKing, b & ~theirOcc);

        // capture mask filters destination squares to those that contain an enemy piece
        // that is checking the king and must be capturedPiece
        U64 captureMask;

        // quiet mask filter destination squares to those where occ must be moved
        // to block an incoming attack to the king
        U64 quietMask;

        // occ that check our king
        checkers = getAttacks(KNIGHT, ourKing, occ) & getPieceBB(them, KNIGHT)
                   | pawnAttacks(us, ourKing) & getPieceBB(them, PAWN);
        // potential checkers
        U64 candidates = getAttacks(ROOK, ourKing, theirOcc) & theirOrthSliders
                         | getAttacks(BISHOP, ourKing, theirOcc) & theirDiagSliders;

        pinned = 0;
        while (candidates) {
            Square s = popLsb(&candidates);
            b = SQUARES_BETWEEN[ourKing][s] & ourOcc;

            // if squares in between the enemy slider and our king doesn't
            // contain any of our occ, add the slider to the checker bitboard
            if (b == 0) {
                checkers ^= SQUARE_BB[s];
            } else if ((b & b - 1) == 0) {
                // if there is only one of our occ between them, add our piece to the pinned bitboard
                pinned ^= b;
            }
        }

        const U64 notPinned = ~pinned;
        const Square epSquare = history[gamePly].epSquare;

        int checkersCount = sparsePopCount(checkers);
        if (checkersCount == 2) {
            // if double check, then only king moves are legal
            return numMoves;
        }

        // single check
        if (checkersCount == 1) {
            Square checkerSquare = bsf(checkers);
            Piece checkerPiece = board[checkerSquare];

            if (checkerPiece == makePiece(them, PAWN)) {
                // if the checker is a pawn, we must check for e.p. moves that can capture it
                if (checkers == shift(relativeDir(us, SOUTH), SQUARE_BB[epSquare])) {
                    // b1 contains our pawns that can capture the checker e.p.
                    b = pawnAttacks(them, epSquare) & getPieceBB(us, PAWN) & notPinned;
                    while (b) {
                        *moves++ = Move(popLsb(&b), epSquare, EN_PASSANT);
                        numMoves++;
                    }
                }
            }

            if (checkerPiece == makePiece(them, KNIGHT)) {
                // if checker is either a pawn or a knight, the only legal moves are to capture the checker
                b = isAttacked(us, checkerSquare, occ) & notPinned;
                while (b) {
                    *moves++ = Move(popLsb(&b), checkerSquare, CAPTURE);
                    numMoves++;
                }
                return numMoves;
            }

            // we must capture the checking piece
            captureMask = checkers;
            // or we can block it since it is guaranteed to be a slider
            quietMask = SQUARES_BETWEEN[ourKing][checkerSquare];
        } else {
            // we can capture any enemy piece
            captureMask = theirOcc;
            // and we can make a quiet move to any square which is not occupied
            quietMask = ~occ;

            if (epSquare != NO_SQUARE) {
                numMoves += genEnPassantMoves(*this, moves, occ, pinned);
            }

            numMoves += genPinnedPieceMoves(*this, moves, occ, captureMask, notPinned);
            numMoves += genCastlingMoves(*this, moves, occ, danger);
        }

        numMoves += genNonPinnedPieceMoves(*this, moves, occ, quietMask, captureMask, notPinned);
        numMoves += genNonPinnedPawnMoves(*this, moves, occ, quietMask, captureMask, notPinned);
        numMoves += genPromotionMoves(*this, moves, quietMask, captureMask, notPinned);

        return numMoves;
    }

} // namespace Chess
