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

    constexpr U64 shortCastlingMask(Color c) {
        return c == WHITE ? WHITE_OO_MASK : BLACK_OO_MASK;
    }

    constexpr U64 longCastlingMask(Color c) {
        return c == WHITE ? WHITE_OOO_MASK : BLACK_OOO_MASK;
    }

    constexpr U64 shortCastlingBlockersMask(Color c) {
        return c == WHITE ? 0x60 : 0x6000000000000000;
    }

    constexpr U64 longCastlingBlockersMask(Color c) {
        return c == WHITE ? 0xe : 0xE00000000000000;
    }

    constexpr U64 ignoreLongCastlingDanger(Color c) {
        return c == WHITE ? 0x2 : 0x200000000000000;
    }

    // helper to generate quiet moves
    template<MoveFlags MF>
    inline Move *make(Move *&moves, Square from, U64 to) {
        while (to) {
            *moves++ = Move(from, popLsb(&to), MF);
        }
        return moves;
    }

    // helper to generate promotion capture moves
    template<Color Us, Direction d, MoveFlags mf>
    inline Move *makePromotions(Move *&moves, U64 to) {
        while (to) {
            Square s = popLsb(&to);
            Square from = s - relativeDir(Us, d);
            *moves++ = Move(from, s, mf);
            *moves++ = Move(from, s, MoveFlags(mf + 1));
            *moves++ = Move(from, s, MoveFlags(mf + 2));
            *moves++ = Move(from, s, MoveFlags(mf + 3));
        }

        return moves;
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
    Move *genCastlingMoves(const Board &board, Move *&moves, U64 occ) {
        const U64 castleMask = board.history[board.getPly()].castleMask;

        // checks if king would be in check if it moved to the castling square
        U64 possibleChecks = (occ | board.danger) & shortCastlingBlockersMask(Us);
        // checks if king and the h-rook have moved
        U64 isAllowed = castleMask & shortCastlingMask(Us);

        if (!(possibleChecks | isAllowed)) {
            *moves++ = Us == WHITE ? Move(e1, g1, OO) : Move(e8, g8, OO);
        }

        // ignoreLongCastlingDanger is used to get rid of the danger on the possibleChecks or b8 square
        possibleChecks = (occ | (board.danger & ~ignoreLongCastlingDanger(Us))) & longCastlingBlockersMask(Us);
        isAllowed = castleMask & longCastlingMask(Us);

        if (!(possibleChecks | isAllowed)) {
            *moves++ = Us == WHITE ? Move(e1, c1, OOO) : Move(e8, c8, OOO);
        }

        return moves;
    }

    template<Color Us>
    Move *genPieceMoves(const Board &board, Move *&moves, U64 occ, int checkersCount) {
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

                moves = make<CAPTURE>(moves, s, attacks & board.captureMask);
                moves = make<QUIET>(moves, s, attacks & board.quietMask);
            }
        }

        // knight moves
        U64 ourKnights = board.getPieceBB(Us, KNIGHT) & ~board.pinned;
        while (ourKnights) {
            s = popLsb(&ourKnights);
            attacks = getAttacks(KNIGHT, s, occ);

            moves = make<CAPTURE>(moves, s, attacks & board.captureMask);
            moves = make<QUIET>(moves, s, attacks & board.quietMask);
        }

        // bishops and queens
        U64 ourDiagSliders = board.getDiagSliders(Us) & ~board.pinned;
        while (ourDiagSliders) {
            s = popLsb(&ourDiagSliders);
            attacks = getAttacks(BISHOP, s, occ);

            moves = make<CAPTURE>(moves, s, attacks & board.captureMask);
            moves = make<QUIET>(moves, s, attacks & board.quietMask);
        }

        // rooks and queens
        U64 ourOrthSliders = board.getOrthSliders(Us) & ~board.pinned;
        while (ourOrthSliders) {
            s = popLsb(&ourOrthSliders);
            attacks = getAttacks(ROOK, s, occ);

            moves = make<CAPTURE>(moves, s, attacks & board.captureMask);
            moves = make<QUIET>(moves, s, attacks & board.quietMask);
        }

        return moves;
    }

    template<Color Us>
    Move *genPawnMoves(const Board &board, Move *&moves, U64 occ, int checkersCount) {
        constexpr Color them = ~Us;
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
                    U64 attacks = pawnAttacks(Us, s) & board.captureMask & LINE[ourKingSq][s];
                    // quiet promotions are impossible since it would leave the king in check
                    while (attacks) {
                        Square to = popLsb(&attacks);
                        *moves++ = Move(s, to, PC_KNIGHT);
                        *moves++ = Move(s, to, PC_BISHOP);
                        *moves++ = Move(s, to, PC_ROOK);
                        *moves++ = Move(s, to, PC_QUEEN);
                    }
                } else {
                    U64 attacks = pawnAttacks(Us, s) & board.captureMask & LINE[s][ourKingSq];
                    moves = make<CAPTURE>(moves, s, attacks);

                    U64 singlePush = shift(relativeDir(Us, NORTH), SQUARE_BB[s]) & ~occ & LINE[ourKingSq][s];
                    moves = make<QUIET>(moves, s, singlePush);

                    U64 doublePush = shift(relativeDir(Us, NORTH), singlePush & MASK_RANK[relativeRank(Us, RANK_3)]);
                    moves = make<DOUBLE_PUSH>(moves, s, doublePush & ~occ & LINE[ourKingSq][s]);
                }
            }

            // e.p. moves
            if (epSq != NO_SQUARE) {
                const U64 theirOrthSliders = board.getOrthSliders(them);

                U64 epCaptureBB = pawnAttacks(them, epSq) & board.getPieceBB(Us, PAWN);
                U64 canCapture = epCaptureBB & ~board.pinned;

                while (canCapture) {
                    s = popLsb(&canCapture);
                    // pseudo-pinned e.p. case
                    U64 newOccMask = occ ^ SQUARE_BB[s] ^ shift(relativeDir(Us, SOUTH), SQUARE_BB[epSq]);
                    U64 attacker = slidingAttacks(ourKingSq, newOccMask, MASK_RANK[squareRank(ourKingSq)]);

                    if ((attacker & theirOrthSliders) == 0) {
                        *moves++ = Move(s, epSq, EN_PASSANT);
                    }
                }

                // pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king
                canCapture = epCaptureBB & board.pinned & LINE[epSq][ourKingSq];
                if (canCapture) {
                    *moves++ = Move(bsf(canCapture), epSq, EN_PASSANT);
                }
            }
        }

        // contains non-pinned pawns which are not on the last rank
        U64 ourPawns = board.getPieceBB(Us, PAWN) & ~board.pinned & ~MASK_RANK[relativeRank(Us, RANK_7)];
        // single pawn pushes
        U64 singlePush = shift(relativeDir(Us, NORTH), ourPawns) & ~occ;
        // double pawn pushes (only the ones that are on rank 3/6 are considered)
        U64 doublePush = shift(relativeDir(Us, NORTH), singlePush & MASK_RANK[relativeRank(Us, RANK_3)]) & board.quietMask;
        // quiet mask is applied later, to consider the possibility of a double push blocking a check
        singlePush &= board.quietMask;

        while (singlePush) {
            s = popLsb(&singlePush);
            *moves++ = Move(s - relativeDir(Us, NORTH), s, QUIET);
        }

        while (doublePush) {
            s = popLsb(&doublePush);
            *moves++ = Move(s - relativeDir(Us, NORTH_NORTH), s, DOUBLE_PUSH);
        }

        // captures
        U64 leftCaptures = shift(relativeDir(Us, NORTH_WEST), ourPawns) & board.captureMask;
        while (leftCaptures) {
            s = popLsb(&leftCaptures);
            *moves++ = Move(s - relativeDir(Us, NORTH_WEST), s, CAPTURE);
        }

        U64 rightCaptures = shift(relativeDir(Us, NORTH_EAST), ourPawns) & board.captureMask;
        while (rightCaptures) {
            s = popLsb(&rightCaptures);
            *moves++ = Move(s - relativeDir(Us, NORTH_EAST), s, CAPTURE);
        }

        // promotions
        ourPawns = board.getPieceBB(Us, PAWN) & ~board.pinned & MASK_RANK[relativeRank(Us, RANK_7)];
        if (ourPawns) {
            // attacks contains squares that the pawns can move to
            // quiet promotions
            U64 attacks = shift(relativeDir(Us, NORTH), ourPawns) & board.quietMask;
            moves = makePromotions<Us, NORTH, PR_KNIGHT>(moves, attacks);

            // promotion captures
            attacks = shift(relativeDir(Us, NORTH_WEST), ourPawns) & board.captureMask;
            moves = makePromotions<Us, NORTH_WEST, PC_KNIGHT>(moves, attacks);

            attacks = shift(relativeDir(Us, NORTH_EAST), ourPawns) & board.captureMask;
            moves = makePromotions<Us, NORTH_EAST, PC_KNIGHT>(moves, attacks);
        }

        return moves;
    }

    template<Color Us>
    Move *genLegalMoves(Board &board, Move *moves) {
        const Color them = ~Us;
        const Square epSq = board.history[board.getPly()].epSquare;
        const Square ourKingSq = board.kingSquare(Us);
        const U64 ourOcc = board.getOccupancy(Us);
        const U64 theirOcc = board.getOccupancy(them);
        const U64 occ = ourOcc | theirOcc;

        board.danger = dangerMask<Us>(board, occ);
        board.checkers = checkerMask<Us>(board, ourKingSq, board.pinned);

        // generate king moves
        U64 attacks = getAttacks(KING, ourKingSq, occ) & ~(ourOcc | board.danger);

        moves = make<CAPTURE>(moves, ourKingSq, attacks & theirOcc);
        moves = make<QUIET>(moves, ourKingSq, attacks & ~theirOcc);

        // if double check, then only king moves are legal
        int checkersCount = sparsePopCount(board.checkers);
        if (checkersCount == 2) {
            return moves;
        }

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

                    canCapture = pawnAttacks(them, epSq) & ourPawns & ~board.pinned;
                    while (canCapture) {
                        *moves++ = Move(popLsb(&canCapture), epSq, EN_PASSANT);
                    }
                }
            }

            // if checker is either a pawn or a knight, the only legal moves are to capture it
            if (checkerPiece == makePiece(them, KNIGHT)) {
                canCapture = board.isAttacked(Us, checkerSquare, occ) & ~board.pinned;
                while (canCapture) {
                    *moves++ = Move(popLsb(&canCapture), checkerSquare, CAPTURE);
                }
                return moves;
            }

            // we must capture the checking piece
            board.captureMask = board.checkers;
            // or we block it
            board.quietMask = SQUARES_BETWEEN[ourKingSq][checkerSquare];
        } else {
            // we can capture any enemy piece
            board.captureMask = theirOcc;
            // we can move to any square which is not occupied
            board.quietMask = ~occ;

            moves = genCastlingMoves<Us>(board, moves, occ);
        }

        moves = genPieceMoves<Us>(board, moves, occ, checkersCount);
        moves = genPawnMoves<Us>(board, moves, occ, checkersCount);

        return moves;
    }

    class MoveList {
    public:
        explicit MoveList(Board &board) {
            Color stm = board.sideToMove();

            if (stm == WHITE) {
                last = genLegalMoves<WHITE>(board, list);
            } else {
                last = genLegalMoves<BLACK>(board, list);
            }
        }

        constexpr Move &operator[](int i) { return list[i]; }

        const Move *begin() const { return list; }

        const Move *end() const { return last; }

        size_t size() const { return last - list; }

    private:
        Move list[MAX_MOVES];
        Move *last;
    };

} // namespace Chess


#endif //ASTRA_MOVEGEN_H
