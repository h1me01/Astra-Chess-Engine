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

#include "board.h"

namespace Chess {

    Board::Board(const std::string &fen) : pieceBB{0}, board{}, stm(WHITE), gamePly(0), hash(0),
                                           checkers(0), pinned(0), danger(0), captureMask(0), quietMask(0) {
        for (auto &i: board) { i = NO_PIECE; }
        history[0] = StateInfo();

        int square = a8;
        for (char ch: fen.substr(0, fen.find(' '))) {
            if (isdigit(ch)) {
                square += (ch - '0');
            } else if (ch == '/') {
                square -= 16;
            } else {
                putPiece(Piece(PIECE_STR.find(ch)), Square(square++));
            }
        }

        std::istringstream ss(fen.substr(fen.find(' ')));
        char token;

        ss >> token;
        stm = token == 'w' ? WHITE : BLACK;

        history[gamePly].castleMask = ALL_CASTLING_MASK;
        while (ss >> token && !isspace(token)) {
            switch (token) {
                case 'K':
                    history[gamePly].castleMask &= ~WHITE_OO_MASK;
                    break;
                case 'Q':
                    history[gamePly].castleMask &= ~WHITE_OOO_MASK;
                    break;
                case 'k':
                    history[gamePly].castleMask &= ~BLACK_OO_MASK;
                    break;
                case 'q':
                    history[gamePly].castleMask &= ~BLACK_OOO_MASK;
                    break;
                default:
                    break;
            }
        }
    }

    void Board::print(Color c) {
        int s;

        for (int r = RANK_8; r >= RANK_1; --r) {
            for (int f = FILE_A; f <= FILE_H; ++f) {
                if (c == WHITE) {
                    s = (r * 8) + f;
                } else {
                    s = ((7 - r) * 8) + f;
                }
                std::cout << PIECE_STR[board[s]] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "Fen: " << fen() << "\n\n";
    }

    std::string Board::fen() const {
        std::ostringstream fen;
        int empty;

        for (int i = 56; i >= 0; i -= 8) {
            empty = 0;

            for (int j = 0; j < 8; j++) {
                Piece p = board[i + j];

                if (p == NO_PIECE) {
                    empty++;
                } else {
                    fen << (empty == 0 ? "" : std::to_string(empty)) << PIECE_STR[p];
                    empty = 0;
                }
            }

            if (empty != 0) { fen << empty; }
            if (i > 0) { fen << '/'; }
        }

        fen << (stm == WHITE ? " w " : " b ")
            << (history[gamePly].castleMask & WHITE_OO_MASK ? "" : "K")
            << (history[gamePly].castleMask & WHITE_OOO_MASK ? "" : "Q")
            << (history[gamePly].castleMask & BLACK_OO_MASK ? "" : "k")
            << (history[gamePly].castleMask & BLACK_OOO_MASK ? "" : "q")
            << (castleNotationHelper(fen) ? " " : "- ")
            << (history[gamePly].epSquare == NO_SQUARE ? "-" : SQSTR[history[gamePly].epSquare]);

        fen << " " << history[gamePly].halfMoveClock << " " << gamePly;
        return fen.str();
    }

    bool Board::nonPawnMat(Color c) const {
        return pieceBB[makePiece(c, KNIGHT)] | pieceBB[makePiece(c, BISHOP)] |
               pieceBB[makePiece(c, ROOK)] | pieceBB[makePiece(c, QUEEN)];
    }

    U64 Board::diagSliders(Color c) const {
        return c == WHITE ?
            pieceBB[WHITE_BISHOP] | pieceBB[WHITE_QUEEN]
            :
            pieceBB[BLACK_BISHOP] | pieceBB[BLACK_QUEEN];
    }

    // return the bitboard of all orthogonal sliders of a given color
    U64 Board::orthSliders(Color c) const {
        return c == WHITE ?
            pieceBB[WHITE_ROOK] | pieceBB[WHITE_QUEEN]
            :
            pieceBB[BLACK_ROOK] | pieceBB[BLACK_QUEEN];
    }

    // return the bitboard of all pieces of a given color
    U64 Board::occupancy(Color c) const {
        return c == WHITE ?
            pieceBB[WHITE_PAWN] | pieceBB[WHITE_KNIGHT] | pieceBB[WHITE_BISHOP] |
            pieceBB[WHITE_ROOK] | pieceBB[WHITE_QUEEN] | pieceBB[WHITE_KING]
            :
            pieceBB[BLACK_PAWN] | pieceBB[BLACK_KNIGHT] | pieceBB[BLACK_BISHOP] |
            pieceBB[BLACK_ROOK] | pieceBB[BLACK_QUEEN] | pieceBB[BLACK_KING];
    }

    // return the bitboard of all pieces of a given color that are attacking a given square
    U64 Board::isAttacked(Color c, Square s, U64 occ) const {
        return c == WHITE ?
               (pawnAttacks(BLACK, s) & pieceBB[WHITE_PAWN]) |
               (getAttacks(KNIGHT, s, occ) & pieceBB[WHITE_KNIGHT]) |
               (getAttacks(BISHOP, s, occ) & (pieceBB[WHITE_BISHOP] | pieceBB[WHITE_QUEEN])) |
               (getAttacks(ROOK, s, occ) & (pieceBB[WHITE_ROOK] | pieceBB[WHITE_QUEEN]))
               :
               (pawnAttacks(WHITE, s) & pieceBB[BLACK_PAWN]) |
               (getAttacks(KNIGHT, s, occ) & pieceBB[BLACK_KNIGHT]) |
               (getAttacks(BISHOP, s, occ) & (pieceBB[BLACK_BISHOP] | pieceBB[BLACK_QUEEN])) |
               (getAttacks(ROOK, s, occ) & (pieceBB[BLACK_ROOK] | pieceBB[BLACK_QUEEN]));
    }

    bool Board::inCheck() const {
        Square kingSq = bsf(pieceBitboard(stm, KING));
        U64 pieces = occupancy(WHITE) | occupancy(BLACK);
        return isAttacked(~stm, kingSq, pieces);
    }

    void Board::makeMove(const Move &move) {
        const MoveFlags mf = move.flags();
        const Square from = move.from();
        const Square to = move.to();
        const Piece pcFrom = board[from];
        const Piece pcTo = board[to];
        const PieceType pt = typeOfPiece(pcFrom);
        const U64 mask = SQUARE_BB[from] | SQUARE_BB[to];

        gamePly++;
        history[gamePly] = StateInfo(history[gamePly - 1]);
        history[gamePly].castleMask |= mask;
        history[gamePly].halfMoveClock++;

        if (pt == PAWN || pcTo != NO_PIECE) {
            history[gamePly].halfMoveClock = 0;
        }

        if (mf == QUIET || mf == DOUBLE_PUSH || mf == EN_PASSANT) {
            movePiece(from, to);

            if (mf == DOUBLE_PUSH) {
                history[gamePly].epSquare = Square(to ^ 8);
            } else if (mf == EN_PASSANT) {
                removePiece(Square(to ^ 8));
            }
        } else if (mf == OO || mf == OOO) {
            Square rookFrom, rookTo;

            if (mf == OO) {
                rookFrom = stm == WHITE ? h1 : h8;
                rookTo = stm == WHITE ? f1 : f8;
            } else {
                rookFrom = stm == WHITE ? a1 : a8;
                rookTo = stm == WHITE ? d1 : d8;
            }

            movePiece(from, to);
            movePiece(rookFrom, rookTo);
        } else if (mf >= PR_KNIGHT && mf <= PC_QUEEN) {
            removePiece(from);

            if (mf >= PC_KNIGHT) {
                history[gamePly].captured = pcTo;
                removePiece(to);
            }

            putPiece(makePiece(stm, typeOfPromotion(mf)), to);
        } else if (mf == CAPTURE) {
            history[gamePly].captured = pcTo;
            hash ^= zobrist::zobristTable[pcFrom][from] ^ zobrist::zobristTable[pcFrom][to]
                    ^ zobrist::zobristTable[pcTo][to];
            pieceBB[pcFrom] ^= mask;
            pieceBB[pcTo] &= ~mask;
            board[to] = pcFrom;
            board[from] = NO_PIECE;
        }

        history[gamePly].hash = hash;
        stm = ~stm;
    }

    void Board::unmakeMove(const Move &move) {
        stm = ~stm;

        const MoveFlags mf = move.flags();
        const Square from = move.from();
        const Square to = move.to();

        if (mf == QUIET || mf == DOUBLE_PUSH || mf == EN_PASSANT) {
            movePiece(to, from);

            if (mf == EN_PASSANT) {
                putPiece(makePiece(~stm, PAWN), Square(to ^ 8));
            }
        } else if (mf == OO|| mf == OOO) {
            Square rookFrom, rookTo;

            if (mf == OO) {
                rookFrom = stm == WHITE ? f1 : f8;
                rookTo = stm == WHITE ? h1 : h8;
            } else {
                rookFrom = stm == WHITE ? d1 : d8;
                rookTo = stm == WHITE ? a1 : a8;
            }

            movePiece(to , from);
            movePiece(rookFrom, rookTo);
        } else if (mf >= PR_KNIGHT && mf <= PC_QUEEN) {
            removePiece(to);
            putPiece(makePiece(stm, PAWN), from);

            if (mf >= PC_KNIGHT) {
                putPiece(history[gamePly].captured, to);
            }
        } else if (mf == CAPTURE) {
            movePiece(to, from);
            putPiece(history[gamePly].captured, to);
        }

        gamePly--;
    }

    void Board::makeNullMove() {
        gamePly++;
        history[gamePly] = StateInfo(history[gamePly - 1]);
        stm = ~stm;
    }

    void Board::unmakeNullMove() {
        stm = ~stm;
        gamePly--;
    }

    bool Board::isThreefold() const {
        int count = 0;

        for (int i = 0; i < gamePly; ++i) {
            if (history[i].hash == history[gamePly].hash) {
                count++;
                if (count == 2) {
                    return true;
                }
            }
        }

        return false;
    }

    bool Board::isInsufficientMat() const {
        U64 pawns = pieceBB[WHITE_PAWN] | pieceBB[BLACK_PAWN];
        U64 queens = pieceBB[WHITE_QUEEN] | pieceBB[BLACK_QUEEN];
        U64 rooks = pieceBB[WHITE_ROOK] | pieceBB[BLACK_ROOK];
        int numWhiteMinorPieces = popCount(pieceBB[WHITE_KNIGHT] | pieceBB[WHITE_BISHOP]);
        int numBlackMinorPieces = popCount(pieceBB[BLACK_KNIGHT] | pieceBB[BLACK_BISHOP]);
        // draw when KvK, KvK+B, KvK+N, K+NvK+N, K+BvK+B
        return !pawns && !queens && !rooks && numWhiteMinorPieces <= 1 && numBlackMinorPieces <= 1;
    }

    bool Board::isDraw() const {
        return history[gamePly].halfMoveClock >= 100 || isThreefold() || isInsufficientMat();
    }

    /*
     * PRIVATE FUNCTIONS
     */
    // puts a piece on the board and updates the hash and pieces bitboards
    void Board::putPiece(Piece pc, Square s) {
        board[s] = pc;
        pieceBB[pc] |= SQUARE_BB[s];
        hash ^= zobrist::zobristTable[pc][s];
    }

    // removes a piece from the board and updates the hash and pieces bitboards
    void Board::removePiece(Square s) {
        Piece pc = board[s];

        hash ^= zobrist::zobristTable[pc][s];
        pieceBB[pc] &= ~SQUARE_BB[s];
        board[s] = NO_PIECE;
    }

    // moves a piece on the board and updates the hash and pieces bitboards
    void Board::movePiece(Square from, Square to) {
        Piece pc = board[from];

        hash ^= zobrist::zobristTable[pc][from] ^ zobrist::zobristTable[pc][to];
        pieceBB[pc] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
        board[to] = pc;
        board[from] = NO_PIECE;
    }

} // namespace Chess
