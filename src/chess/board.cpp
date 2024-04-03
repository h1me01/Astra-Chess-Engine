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

    Board::Board(const std::string &fen) : pieceBB{0}, board{}, stm(WHITE), gamePly(0), hash(0), pinned(0), checkers(0) {
        for (auto &i: board) { i = NO_PIECE; }
        history[0] = StateInfo();

        int square = a8;
        for (char ch: fen.substr(0, fen.find(' '))) {
            if (isdigit(ch)) {
                square += (ch - '0');
            } else if (ch == '/') {
                square -= 16;
            } else {
                putPiece<false>(Piece(PIECE_STR.find(ch)), Square(square++));
            }
        }

        refreshNNUE(getAccumulator());

        std::istringstream ss(fen.substr(fen.find(' ')));
        char token;

        ss >> token;
        stm = token == 'w' ? WHITE : BLACK;

        history[gamePly].entry = ALL_CASTLING_MASK;
        while (ss >> token && !isspace(token)) {
            switch (token) {
                case 'K': history[gamePly].entry &= ~WHITE_OO_MASK; break;
                case 'Q': history[gamePly].entry &= ~WHITE_OOO_MASK; break;
                case 'k': history[gamePly].entry &= ~BLACK_OO_MASK; break;
                case 'q': history[gamePly].entry &= ~BLACK_OOO_MASK; break;
                default: break;
            }
        }

        accumulators->clear();
    }

    Board::Board(const Board &other) {
        if (other.accumulators) {
            accumulators = std::make_unique<Accumulators>(*other.accumulators);
        }

        for (int i = 0; i < MAX_PLY * 2; ++i) {
            history[i] = other.history[i];
        }

        for (int i = 0; i < NUM_PIECES; ++i) {
            pieceBB[i] = other.pieceBB[i];
        }

        for (int i = 0; i < NUM_SQUARES; ++i) {
            board[i] = other.board[i];
        }

        checkers = other.checkers;
        pinned = other.pinned;

        hash = other.hash;
        stm = other.stm;
        gamePly = other.gamePly;
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

        std::cout << "Fen: " << getFen() << "\n\n";
    }

    // prints the game not really in san format, moves are in the form of "e2e4 e7e5 ..."
    void Board::pgn() {
        char pieceNotation[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

        std::cout << "PGN:" << std::endl;
        for (int i = 1; i <= gamePly; ++i) {
            std::cout << pieceNotation[typeOfPiece(history[i].movedPiece)];
            std::cout << SQSTR[history[i].move.from()];
            std::cout << SQSTR[history[i].move.to()] << " ";
        }
    }

    std::string Board::getFen() const {
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
            << (history[gamePly].entry & WHITE_OO_MASK ? "" : "K")
            << (history[gamePly].entry & WHITE_OOO_MASK ? "" : "Q")
            << (history[gamePly].entry & BLACK_OO_MASK ? "" : "k")
            << (history[gamePly].entry & BLACK_OOO_MASK ? "" : "q")
            << (castleNotationHelper(fen) ? " " : "- ")
            << (history[gamePly].epSquare == NO_SQUARE ? "-" : SQSTR[history[gamePly].epSquare]);

        fen << " " << history[gamePly].halfMoveClock << " " << gamePly;
        return fen.str();
    }

    // return the bitboard of all pieces of a given color
    U64 Board::getOccupancy(Color c) const {
        return getPieceBB(c, PAWN) | getPieceBB(c, KNIGHT) | getPieceBB(c, BISHOP) |
               getPieceBB(c, ROOK) | getPieceBB(c, QUEEN) | getPieceBB(c, KING);
    }

    // return the bitboard of all pieces of a given color that are attacking a given square
    U64 Board::isAttacked(Color c, Square s, U64 occ) const {
        return (pawnAttacks(~c, s) & getPieceBB(c, PAWN)) |
               (getAttacks(KNIGHT, s, occ) & getPieceBB(c, KNIGHT)) |
               (getAttacks(BISHOP, s, occ) & (getPieceBB(c, BISHOP) | getPieceBB(c, QUEEN))) |
               (getAttacks(ROOK, s, occ) & (getPieceBB(c, ROOK) | getPieceBB(c, QUEEN)));
    }

    bool Board::inCheck() const {
        Square kingSq = bsf(getPieceBB(stm, KING));
        U64 pieces = getOccupancy(WHITE) | getOccupancy(BLACK);
        return isAttacked(~stm, kingSq, pieces);
    }

    void Board::unmakeMove() {
        stm = ~stm;

        const Move move = history[gamePly].move;
        const MoveFlags mf = move.flags();
        const Square from = move.from();
        const Square to = move.to();

        if (accumulators->size()) {
            accumulators->pop();
        }

        if (mf == QUIET || mf == DOUBLE_PUSH || mf == EN_PASSANT) {
            movePiece<false>(to, from);

            if (mf == EN_PASSANT) {
                putPiece<false>(makePiece(~stm, PAWN), Square(to ^ 8));
            }
        }

        if (mf == OO) {
            if (stm == WHITE) {
                movePiece<false>(g1, e1);
                movePiece<false>(f1, h1);
            } else {
                movePiece<false>(g8, e8);
                movePiece<false>(f8, h8);
            }
        } else if (mf == OOO) {
            if (stm == WHITE) {
                movePiece<false>(c1, e1);
                movePiece<false>(d1, a1);
            } else {
                movePiece<false>(c8, e8);
                movePiece<false>(d8, a8);
            }
        }

        if (mf >= PR_KNIGHT && mf <= PC_QUEEN) {
            removePiece<false>(to);
            putPiece<false>(makePiece(stm, PAWN), from);

            if (mf >= PC_KNIGHT) {
                putPiece<false>(history[gamePly].capturedPiece, to);
            }
        }

        if (mf == CAPTURE) {
            movePiece<false>(to, from);
            putPiece<false>(history[gamePly].capturedPiece, to);
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

    bool Board::isInsufficientMaterial() const {
        // draw when KvK, KvK+B, KvK+N, K+NvK+N, K+BvK+B
        U64 pawns = getPieceBB(WHITE, PAWN) | getPieceBB(BLACK, PAWN);
        U64 queens = getPieceBB(WHITE, QUEEN) | getPieceBB(BLACK, QUEEN);
        U64 rooks = getPieceBB(WHITE, ROOK) | getPieceBB(BLACK, ROOK);
        int numWhiteMinorPieces = popCount(getPieceBB(WHITE, KNIGHT) | getPieceBB(WHITE, BISHOP));
        int numBlackMinorPieces = popCount(getPieceBB(BLACK, KNIGHT) | getPieceBB(BLACK, BISHOP));

        return !pawns && !queens && !rooks && numWhiteMinorPieces <= 1 && numBlackMinorPieces <= 1;
    }

    bool Board::isDraw() const {
        return history[gamePly].halfMoveClock >= 100 || isThreefold() || isInsufficientMaterial();
    }

    /*
     * PRIVATE FUNCTIONS
     */
    void Board::refreshNNUE(NNUE::accumulator &acc) const {
        for (int i = 0; i < N_HIDDEN_SIZE; i++) {
            acc[WHITE][i] = HIDDEN_BIAS[i];
            acc[BLACK][i] = HIDDEN_BIAS[i];
        }

        const Square ksq_white = kingSquare(WHITE);
        const Square ksq_black = kingSquare(BLACK);

        for (Square i = a1; i < NUM_SQUARES; ++i) {
            Piece p = board[i];

            if (p == NO_PIECE) {
                continue;
            }

            NNUE::activate(acc, i, p, ksq_white, ksq_black);
        }
    }

} // namespace Chess
