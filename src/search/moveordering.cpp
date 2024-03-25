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

#include "moveordering.h"

namespace Astra {
    /*
     * Static exchange evaluation (SEE)
     */

    // piece values
    const int pieceValues[] = {
            PAWN_VALUE,
            KNIGHT_VALUE,
            BISHOP_VALUE,
            ROOK_VALUE,
            QUEEN_VALUE,
            KING_VALUE,
            NO_PIECE_VALUE
    };

    U64 smallestAttacker(Board &board, Square s) {
        Color c = board.getTurn();

        U64 attackers;
        for (PieceType pt: {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
            if (pt == PAWN) {
                attackers = pawnAttacks(~c, s) & board.getPieceBB(c, PAWN);
            } else {
                U64 pieces = board.getOccupancy(WHITE) | board.getOccupancy(BLACK);
                attackers = attacks(pt, s, pieces) & board.getPieceBB(c, pt);
            }

            if (attackers) {
                return attackers;
            }
        }

        return 0;
    }

    int see(Board &board, Color c, Square s) {
        int value = 0;
        U64 attackers = smallestAttacker(board, s);

        if (attackers) {
            Square attackerFrom = popLsb(&attackers);
            PieceType capturedPiece = getPieceType(board.getPiece(s));
            Move move = Move(attackerFrom, s, CAPTURE);

            board.makeMove(move);
            value = pieceValues[capturedPiece] - see(board, ~c, s);
            // do not consider captures if they lose material, therefor max zero
            value = std::max(0, value);
            board.undoMove();
        }

        return value;
    }

    int seeCapture(Board &board, Move &captureMove) {
        PieceType to = getPieceType(board.getPiece(captureMove.to()));

        board.makeMove(captureMove);
        int score = pieceValues[to] - see(board, ~board.getTurn(), captureMove.to());
        board.undoMove();

        return score;
    }

    /*
     * Move Ordering
     */
    MoveOrdering::MoveOrdering() {
        clear();
    }

    void MoveOrdering::clear() {
        for (int i = 0; i < KILLER_SIZE; ++i) {
            killer1[WHITE][i] = NULL_MOVE;
            killer2[BLACK][i] = NULL_MOVE;
        }

        for (auto &i: history) {
            for (auto &j: i) {
                for (int &k: j) {
                    k = 0;
                }
            }
        }
    }

    void MoveOrdering::updateHistory(Board &board, Move &move, int score) {
        // check if move is a capture, return error if it is
        assert(!move.isCapture());

        Piece piece = board.getPiece(move.from());
        history[piece][move.from()][move.to()] += score;
    }

    int MoveOrdering::getHistoryScore(Board &board, Move &move) {
        Piece piece = board.getPiece(move.from());
        return history[getPieceType(piece)][move.from()][move.to()];
    }

    void MoveOrdering::updateKiller(Move &move, Color c, int ply) {
        // check if move is a capture, return error if it is
        assert(!move.isCapture());

        if (!Move::isSame(move, killer1[c][ply])) {
            killer2[c][ply] = killer1[c][ply];
            killer1[c][ply] = move;
        }
    }

    /*
     * Move Order:
     * - Hash move
     * - Captures/Promotion captures
     * - Quiet Promotions
     * - Killer1 moves
     * - Killer2 moves
     * - History moves
     * - Quiet moves
     */
    void MoveOrdering::sortMoves(Board &board, TTable &tt, Move *moves, int numMoves, int ply) {
        Color color = board.getTurn();

        TTEntry entry;
        bool ttHit = tt.lookup(entry, board.getHash(), 0);

        int scores[numMoves];
        for (int i = 0; i < numMoves; ++i) {
            Move move = moves[i];
            scores[i] = 0;

            bool isCapture = move.isCapture();
            bool isPromotion = move.isPromotion();

            Square from = move.from();
            Square to = move.to();

            PieceType movePieceType = getPieceType(board.getPiece(from));
            PieceType capturePieceType = getPieceType(board.getPiece(to));

            if (ttHit && move == entry.move) {
                scores[i] += HASH_BONUS;
                continue;
            }

            if (isCapture) {
                int seeScore = seeCapture(board, move);
                int capturedPieceValue = move.flags() == EN_PASSANT ? PAWN_VALUE : pieceValues[capturePieceType];
                int captureScore = capturedPieceValue * 32768 - pieceValues[movePieceType] - 16384 + seeScore;

                scores[i] = captureScore;
                continue;
            }

            if (isPromotion) {
                scores[i] += typeOfPromotion(move.flags()) + PROMOTION_BONUS;
                continue;
            }

            if (movePieceType != KING) {
                int psqtMgScore = mg_pesto_table[movePieceType][to] - mg_pesto_table[movePieceType][from];
                int psqtEgScore = eg_pesto_table[movePieceType][to] - eg_pesto_table[movePieceType][from];

                scores[i] += (psqtMgScore + psqtEgScore) / 2;

                U64 oppAttacks = board.getAttackers(~color, to, board.getOccupancy(WHITE) | board.getOccupancy(BLACK));

                if (oppAttacks) {
                    scores[i] -= 100;
                }
            }

            if (!isCapture) {
                if (move == killer1[color][ply]) {
                    scores[i] += KILLER1_BONUS;
                } else if (move == killer2[color][ply]) {
                    scores[i] += KILLER2_BONUS;
                }

                scores[i] += getHistoryScore(board, move);
            }
        }

        // Bubble Sort
        for (int i = 0; i < numMoves - 1; ++i) {
            for (int j = 0; j < numMoves - i - 1; ++j) {
                if (scores[j] < scores[j + 1]) {
                    Move tempMove = moves[j];
                    moves[j] = moves[j + 1];
                    moves[j + 1] = tempMove;

                    int tempScore = scores[j];
                    scores[j] = scores[j + 1];
                    scores[j + 1] = tempScore;
                }
            }
        }
    }

} // namespace Astra
