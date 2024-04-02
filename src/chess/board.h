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

#ifndef ASTRA_BOARD_H
#define ASTRA_BOARD_H

#include <memory>
#include "zobrist.h"
#include "attacks.h"
#include "../eval/nnue.h"
#include "../eval/accumulators.h"

namespace Chess {

    struct StateInfo {
        U64 hash;
        Piece movedPiece;
        Move move;
        Piece capturedPiece;
        Square epSquare;
        int halfMoveClock;
        /* bitboard of squares on which pieces have either moved from, or have been moved to.
         * Used for castling legality checks
         */ U64 entry;

        StateInfo() : hash(0), movedPiece(NO_PIECE), move(NULL_MOVE), capturedPiece(NO_PIECE),
                      epSquare(NO_SQUARE), entry(0), halfMoveClock(0) {}

        StateInfo(const StateInfo &prev) {
            hash = prev.hash;
            movedPiece = prev.movedPiece;
            move = prev.move;
            capturedPiece = NO_PIECE;
            epSquare = NO_SQUARE;
            entry = prev.entry;
            halfMoveClock = prev.halfMoveClock;
        }
    };

    class Board {
    public:
        StateInfo history[MAX_PLY * 2];
        // bitboard of enemy pieces that are currently attacking the king, updated in genLegalMoves()
        U64 checkers;
        // bitboard of pieces that are currently pinned to the king by enemy sliders, updated in genLegalMoves()
        U64 pinned;

        Board(const std::string &fen);

        Board(const Board &other);

        void print(Color c);

        void pgn();

        std::string getFen() const;

        U64 getPieceBB(Color c, PieceType pt) const { return pieceBB[makePiece(c, pt)]; }

        Piece getPiece(Square s) const { return board[s]; }

        Color sideToMove() const { return stm; }

        int getPly() const { return gamePly; }

        U64 getHash() const { return hash; }

        Square kingSquare(Color c) const;

        U64 getDiagSliders(Color c) const;

        U64 getOrthSliders(Color c) const;

        U64 getOccupancy(Color c) const;

        U64 isAttacked(Color c, Square s, U64 occ) const;

        bool inCheck() const;

        bool nonPawnMaterial(Color c) const;

        template<bool updateNNUE>
        void makeMove(Move move);

        void unmakeMove();

        void makeNullMove();

        void unmakeNullMove();

        bool isThreefold() const;

        bool isInsufficientMaterial() const;

        bool isDraw() const;

        NNUE::accumulator &getAccumulator() { return accumulators->back(); }

    private:
        std::unique_ptr<Accumulators> accumulators = std::make_unique<Accumulators>();

        U64 pieceBB[NUM_PIECES];
        Piece board[NUM_SQUARES];
        Color stm;
        int gamePly;
        U64 hash;

        template<bool updateNNUE>
        void castleHelper(Square from, Square kingTo, Square rookFrom, Square rookTo);

        template<bool updateNNUE>
        void putPiece(Piece pc, Square s);

        template<bool updateNNUE>
        void removePiece(Square s);

        template<bool updateNNUE>
        void movePiece(Square from, Square to);

        void refreshNNUE(NNUE::accumulator &acc) const;
    };

    inline Square Board::kingSquare(Color c) const {
        return bsf(getPieceBB(c, KING));
    }

    /*
     * MAKE MOVE
     */
    template<bool updateNNUE>
    void Board::makeMove(Move move) {
        const MoveFlags mf = move.flags();
        const Square from = move.from();
        const Square to = move.to();
        const Piece pcFrom = board[from];
        const Piece pcTo = board[to];
        const U64 mask = SQUARE_BB[from] | SQUARE_BB[to];

        gamePly++;
        history[gamePly] = StateInfo(history[gamePly - 1]);
        history[gamePly].entry |= mask;
        history[gamePly].move = move;
        history[gamePly].movedPiece = pcFrom;
        history[gamePly].halfMoveClock++;

        if constexpr (updateNNUE) {
            accumulators->push();
        }

        if(typeOfPiece(pcFrom) == PAWN || pcTo != NO_PIECE) {
            history[gamePly].halfMoveClock = 0;
        }

        // handle castling
        if (mf == OO) {
            if (stm == WHITE) {
                castleHelper<updateNNUE>(e1, g1, h1, f1);
            } else {
                castleHelper<updateNNUE>(e8, g8, h8, f8);
            }
        } else if (mf == OOO) {
            if (stm == WHITE) {
                castleHelper<updateNNUE>(e1, c1, a1, d1);
            } else {
                castleHelper<updateNNUE>(e8, c8, a8, d8);
            }
        } else if (mf == QUIET || mf == DOUBLE_PUSH || mf == EN_PASSANT) {
            movePiece<updateNNUE>(from, to);

            if (mf == DOUBLE_PUSH) {
                history[gamePly].epSquare = Square(to ^ 8);
            } else if (mf == EN_PASSANT) {
                removePiece<updateNNUE>(Square(to ^ 8));
            }
        } else if (mf >= PR_KNIGHT && mf <= PC_QUEEN) {
            removePiece<updateNNUE>(from);

            if (mf >= PC_KNIGHT) {
                history[gamePly].capturedPiece = pcTo;
                removePiece<updateNNUE>(to);
            }

            putPiece<updateNNUE>(makePiece(stm, typeOfPromotion(mf)), to);
        } else if (mf == CAPTURE) {
            history[gamePly].capturedPiece = pcTo;
            hash ^= zobrist::zobristTable[pcFrom][from] ^ zobrist::zobristTable[pcFrom][to]
                    ^ zobrist::zobristTable[pcTo][to];
            pieceBB[pcFrom] ^= mask;
            pieceBB[pcTo] &= ~mask;
            board[to] = pcFrom;
            board[from] = NO_PIECE;

            if constexpr (updateNNUE) {
                const Square ksq_white = kingSquare(WHITE);
                const Square ksq_black = kingSquare(BLACK);

                NNUE::deactivate(getAccumulator(), to, history[gamePly].capturedPiece, ksq_white, ksq_black);
                NNUE::move(getAccumulator(), from, to, pcTo, ksq_white, ksq_black);
            }
        }

        history[gamePly].hash = hash;
        stm = ~stm;
    }

    /*
     * PRIVATE FUNCTIONS
     */
    template<bool updateNNUE>
    void Board::castleHelper(Square from, Square kingTo, Square rookFrom, Square rookTo) {
        if (updateNNUE && NNUE::KING_BUCKET[from] != NNUE::KING_BUCKET[kingTo]) {
            movePiece<false>(from, kingTo);
            movePiece<false>(rookFrom, rookTo);
            refreshNNUE(getAccumulator());
        } else {
            movePiece<updateNNUE>(from, kingTo);
            movePiece<updateNNUE>(rookFrom, rookTo);
        }
    }

    // puts a piece on the board and updates the hash and pieces bitboards
    template<bool updateNNUE>
    void Board::putPiece(Piece pc, Square s) {
        board[s] = pc;
        pieceBB[pc] |= SQUARE_BB[s];
        hash ^= zobrist::zobristTable[pc][s];

        if constexpr (updateNNUE) {
            const Square ksq_white = kingSquare(WHITE);
            const Square ksq_black = kingSquare(BLACK);
            NNUE::activate(getAccumulator(), s, pc, ksq_white, ksq_black);
        }
    }

    // removes a piece from the board and updates the hash and pieces bitboards
    template<bool updateNNUE>
    void Board::removePiece(Square s) {
        Piece pc = board[s];

        hash ^= zobrist::zobristTable[pc][s];
        pieceBB[pc] &= ~SQUARE_BB[s];
        board[s] = NO_PIECE;

        if constexpr (updateNNUE) {
            const Square ksq_white = kingSquare(WHITE);
            const Square ksq_black = kingSquare(BLACK);
            NNUE::deactivate(getAccumulator(), s, pc, ksq_white, ksq_black);
        }
    }

    // moves a piece on the board and updates the hash and pieces bitboards
    template<bool updateNNUE>
    void Board::movePiece(Square from, Square to) {
        Piece pc = board[from];

        hash ^= zobrist::zobristTable[pc][from] ^ zobrist::zobristTable[pc][to];
        pieceBB[pc] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
        board[to] = pc;
        board[from] = NO_PIECE;

        if constexpr (updateNNUE) {
            if (typeOfPiece(pc) == KING && NNUE::KING_BUCKET[from] != NNUE::KING_BUCKET[to]) {
                refreshNNUE(getAccumulator());
            } else {
                const Square ksq_white = kingSquare(WHITE);
                const Square ksq_black = kingSquare(BLACK);

                NNUE::move(getAccumulator(), from, to, pc, ksq_white, ksq_black);
            }
        }
    }

} // namespace Chess


#endif //ASTRA_BOARD_H
