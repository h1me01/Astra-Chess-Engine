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

    Board::Board(const std::string &fen) : pieceBB{0}, board{}, stm(WHITE), gamePly(0),  hash(0), pinned(0), checkers(0) {
        for (auto &i: board) {
            i = NO_PIECE;
        }
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

        history[gamePly].entry = ALL_CASTLING_MASK;
        while (ss >> token && !isspace(token)) {
            switch (token) {
                case 'K':
                    history[gamePly].entry &= ~WHITE_OO_MASK;
                    break;
                case 'Q':
                    history[gamePly].entry &= ~WHITE_OOO_MASK;
                    break;
                case 'k':
                    history[gamePly].entry &= ~BLACK_OO_MASK;
                    break;
                case 'q':
                    history[gamePly].entry &= ~BLACK_OOO_MASK;
                    break;
                default:
                    break;
            }
        }
    }

    void Board::print(Color c) {
        for (int r = RANK_8; r >= RANK_1; --r) {
            for (int f = FILE_A; f <= FILE_H; ++f) {
                int s;
                if(c == WHITE) {
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

    // prints the game not really in pgn format, moves are in the form of "e2e4 e7e5 ..."
    void Board::pgn() {
        char pieceNotation[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

        std::cout << "PGN:" << std::endl;
        for (int i = 1; i <= gamePly; ++i) {
            std::cout << pieceNotation[getPieceType(history[i].movedPiece)];
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
                    fen << (empty == 0 ? "" : std::to_string(empty))
                        << PIECE_STR[p];
                    empty = 0;
                }
            }

            if (empty != 0) {
                fen << empty;
            }

            if (i > 0) {
                fen << '/';
            }
        }

        fen << (stm == WHITE ? " w " : " b ")
            << (history[gamePly].entry & WHITE_OO_MASK ? "" : "K")
            << (history[gamePly].entry & WHITE_OOO_MASK ? "" : "Q")
            << (history[gamePly].entry & BLACK_OO_MASK ? "" : "k")
            << (history[gamePly].entry & BLACK_OOO_MASK ? "" : "q")
            << (castleRightsNotation(fen) ? " " : "- ")
            << (history[gamePly].epSquare == NO_SQUARE ? "-" : SQSTR[history[gamePly].epSquare]);

        fen << " " << history[gamePly].halfMoveClock << " " << gamePly;
        return fen.str();
    }

    // return the bitboard of all diagonal sliders of a given color
    U64 Board::getDiagSliders(Color c) const {
        return c == WHITE ?
               pieceBB[WHITE_BISHOP] | pieceBB[WHITE_QUEEN] :
               pieceBB[BLACK_BISHOP] | pieceBB[BLACK_QUEEN];
    }

    // return the bitboard of all orthogonal sliders of a given color
    U64 Board::getOrthSliders(Color c) const {
        return c == WHITE ?
               pieceBB[WHITE_ROOK] | pieceBB[WHITE_QUEEN] :
               pieceBB[BLACK_ROOK] | pieceBB[BLACK_QUEEN];
    }

    // return the bitboard of all pieces of a given color
    U64 Board::getOccupancy(Color c) const {
        return c == WHITE ?
               pieceBB[WHITE_PAWN] | pieceBB[WHITE_KNIGHT] | pieceBB[WHITE_BISHOP] |
               pieceBB[WHITE_ROOK] | pieceBB[WHITE_QUEEN] | pieceBB[WHITE_KING] :
               pieceBB[BLACK_PAWN] | pieceBB[BLACK_KNIGHT] | pieceBB[BLACK_BISHOP] |
               pieceBB[BLACK_ROOK] | pieceBB[BLACK_QUEEN] | pieceBB[BLACK_KING];
    }

    // return the bitboard of all pieces of a given color that are attacking a given square
    U64 Board::getAttackers(Color c, Square s, U64 occ) const {
        return c == WHITE ?
               (pawnAttacks(BLACK, s) & pieceBB[WHITE_PAWN]) |
               (attacks(KNIGHT,s, occ) & pieceBB[WHITE_KNIGHT]) |
               (attacks(BISHOP, s, occ) & (pieceBB[WHITE_BISHOP] | pieceBB[WHITE_QUEEN])) |
               (attacks(ROOK, s, occ) & (pieceBB[WHITE_ROOK] | pieceBB[WHITE_QUEEN])) :
               (pawnAttacks(WHITE, s) & pieceBB[BLACK_PAWN]) |
               (attacks(KNIGHT, s, occ) & pieceBB[BLACK_KNIGHT]) |
               (attacks(BISHOP, s, occ) & (pieceBB[BLACK_BISHOP] | pieceBB[BLACK_QUEEN])) |
               (attacks(ROOK, s, occ) & (pieceBB[BLACK_ROOK] | pieceBB[BLACK_QUEEN]));
    }

    bool Board::inCheck() const {
        Square kingSq = bsf(getPieceBB(stm, KING));
        U64 pieces = getOccupancy(WHITE) | getOccupancy(BLACK);
        return getAttackers(~stm, kingSq, pieces);
    }

    void Board::makeMove(Move move) {
        gamePly++;
        history[gamePly] = StateInfo(history[gamePly - 1]);
        history[gamePly].entry |= SQUARE_BB[move.to()] | SQUARE_BB[move.from()];
        history[gamePly].move = move;
        history[gamePly].movedPiece = board[move.from()];

        MoveFlags mf = move.flags();
        if(getPieceType(history[gamePly].movedPiece) == PAWN || move.isCapture()) {
            history[gamePly].halfMoveClock = 0;
        } else {
            history[gamePly].halfMoveClock++;
        }

        switch (mf) {
            case QUIET:
                movePiece(move.from(), move.to());
                break;
            case DOUBLE_PUSH:
                movePiece(move.from(), move.to());
                history[gamePly].epSquare = move.from() + relativeDir(stm, NORTH);
                break;
            case OO:
                if (stm == WHITE) {
                    movePiece(e1, g1);
                    movePiece(h1, f1);
                } else {
                    movePiece(e8, g8);
                    movePiece(h8, f8);
                }
                break;
            case OOO:
                if (stm == WHITE) {
                    movePiece(e1, c1);
                    movePiece(a1, d1);
                } else {
                    movePiece(e8, c8);
                    movePiece(a8, d8);
                }
                break;
            case EN_PASSANT:
                movePiece(move.from(), move.to());
                removePiece(move.to() + relativeDir(stm, SOUTH));
                break;
            case PR_KNIGHT:
            case PR_BISHOP:
            case PR_ROOK:
            case PR_QUEEN:
                removePiece(move.from());
                putPiece(makePiece(stm, typeOfPromotion(move.flags())), move.to());;
                break;
            case PC_KNIGHT:
            case PC_BISHOP:
            case PC_ROOK:
            case PC_QUEEN:
                removePiece(move.from());
                history[gamePly].capturedPiece = board[move.to()];
                removePiece(move.to());
                putPiece(makePiece(stm, typeOfPromotion(move.flags())), move.to());
                break;
            case CAPTURE:
                history[gamePly].capturedPiece = board[move.to()];
                const Square from = move.from();
                const Square to = move.to();
                U64 mask = SQUARE_BB[from] | SQUARE_BB[to];
                hash ^= zobrist::zobristTable[board[from]][from] ^ zobrist::zobristTable[board[from]][to]
                        ^ zobrist::zobristTable[board[to]][to];
                pieceBB[board[from]] ^= mask;
                pieceBB[board[to]] &= ~mask;
                board[to] = board[from];
                board[from] = NO_PIECE;
                break;
        }

        history[gamePly].hash = hash;
        stm = ~stm;
    }

    void Board::undoMove() {
        stm = ~stm;

        Move move = history[gamePly].move;
        switch (move.flags()) {
            case QUIET:
            case DOUBLE_PUSH:
                movePiece(move.to(), move.from());
                break;
            case OO:
                if (stm == WHITE) {
                    movePiece(g1, e1);
                    movePiece(f1, h1);
                } else {
                    movePiece(g8, e8);
                    movePiece(f8, h8);
                }
                break;
            case OOO:
                if (stm == WHITE) {
                    movePiece(c1, e1);
                    movePiece(d1, a1);
                } else {
                    movePiece(c8, e8);
                    movePiece(d8, a8);
                }
                break;
            case EN_PASSANT:
                movePiece(move.to(), move.from());
                putPiece(makePiece(~stm, PAWN), move.to() + relativeDir(stm, SOUTH));
                break;
            case PR_KNIGHT:
            case PR_BISHOP:
            case PR_ROOK:
            case PR_QUEEN:
                removePiece(move.to());
                putPiece(makePiece(stm, PAWN), move.from());
                break;
            case PC_KNIGHT:
            case PC_BISHOP:
            case PC_ROOK:
            case PC_QUEEN:
                removePiece(move.to());
                putPiece(makePiece(stm, PAWN), move.from());
                putPiece(history[gamePly].capturedPiece, move.to());
                break;
            case CAPTURE:
                movePiece(move.to(), move.from());
                putPiece(history[gamePly].capturedPiece, move.to());
                break;
        }

        gamePly--;
    }

    void Board::makeNullMove() {
        gamePly++;
        history[gamePly] = StateInfo(history[gamePly - 1]);
        stm = ~stm;
    }

    void Board::undoNullMove() {
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

    // private member

    // puts a piece on the board and updates the hash and pieces bitboards
    void Board::putPiece(Piece pc, Square s) {
        board[s] = pc;
        pieceBB[pc] |= SQUARE_BB[s];
        hash ^= zobrist::zobristTable[pc][s];
    }

    // removes a piece from the board and updates the hash and pieces bitboards
    void Board::removePiece(Square s) {
        hash ^= zobrist::zobristTable[board[s]][s];
        pieceBB[board[s]] &= ~SQUARE_BB[s];
        board[s] = NO_PIECE;
    }

    // moves a piece on the board and updates the hash and pieces bitboards
    void Board::movePiece(Square from, Square to) {
        hash ^= zobrist::zobristTable[board[from]][from] ^ zobrist::zobristTable[board[from]][to];
        pieceBB[board[from]] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
        board[to] = board[from];
        board[from] = NO_PIECE;
    }

} // namespace Chess
