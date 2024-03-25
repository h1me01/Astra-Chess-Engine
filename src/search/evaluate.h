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

#ifndef ASTRA_EVALUATE_H
#define ASTRA_EVALUATE_H

#include "../chess/board.h"

using namespace Chess;

/*
 * I will be replacing this with my current Neural Network in the future.
 * This is just a placeholder for now to check if the search is working.
 * I did not use my current NN because I have to gather more data and train it.
 */
namespace Astra {

    inline U64 packedData[] = {
            0x0000000000000000, 0x2328170f2d2a1401, 0x1f1f221929211507, 0x18202a1c2d261507,
            0x252e3022373a230f, 0x585b47456d65321c, 0x8d986f66a5a85f50, 0x0002000300070005,
            0xfffdfffd00060001, 0x2b1f011d20162306, 0x221c0b171f15220d, 0x1b1b131b271c1507,
            0x232d212439321f0b, 0x5b623342826c2812, 0x8db65b45c8c01014, 0x0000000000000000,
            0x615a413e423a382e, 0x6f684f506059413c, 0x82776159705a5543, 0x8b8968657a6a6150,
            0x948c7479826c6361, 0x7e81988f73648160, 0x766f7a7e70585c4e, 0x6c7956116e100000,
            0x3a3d2d2840362f31, 0x3c372a343b3a3838, 0x403e2e343c433934, 0x373e3b2e423b2f37,
            0x383b433c45433634, 0x353d4b4943494b41, 0x46432e354640342b, 0x55560000504f0511,
            0x878f635c8f915856, 0x8a8b5959898e5345, 0x8f9054518f8e514c, 0x96985a539a974a4c,
            0x9a9c67659e9d5f59, 0x989c807a9b9c7a6a, 0xa09f898ba59c6f73, 0xa1a18386a09b7e84,
            0xbcac7774b8c9736a, 0xbab17b7caebd7976, 0xc9ce7376cac57878, 0xe4de6f70dcd87577,
            0xf4ef7175eedc7582, 0xf9fa8383dfe3908e, 0xfffe7a81f4ec707f, 0xdfe79b94e1ee836c,
            0x2027252418003d38, 0x4c42091d31193035, 0x5e560001422c180a, 0x6e6200004d320200,
            0x756c000e5f3c1001, 0x6f6c333f663e3f1d, 0x535b55395c293c1b, 0x2f1e3d5e22005300,
            0x004c0037004b001f, 0x00e000ca00be00ad, 0x02e30266018800eb, 0xffdcffeeffddfff3,
            0xfff9000700010007, 0xffe90003ffeefff4, 0x00000000fff5000d,
    };

    inline const int phaseWeightTable[] = {0, 0, 1, 1, 2, 4, 0};

    // bitshift amount is implicitly modulo 64, also used in pst part of eval function
    inline int evalWeight(int item) {
        return (int) (packedData[item >> 1] >> item * 32);
    }

    inline int eval(Board &board) {
        if(board.isInsufficientMaterial()) {
            return 0;
        }

        int eval = 0x000b000a;
        int phase = 0;
        U64 pieces = board.getOccupancy(WHITE) | board.getOccupancy(BLACK);

        while (pieces != 0) {
            int sqIndex = popLsb(&pieces);
            Piece piece = board.getPiece(Square(sqIndex));
            int pieceType = (int) getPieceType(piece) + 1;
            Color pieceIsWhite = getPieceColor(piece);

            Square kingSquare = board.getKingSquare(pieceIsWhite);
            pieceType -= (sqIndex & 0b111 ^ getSquareFile(kingSquare)) >> 1 >> pieceType;

            int packedIndex = pieceType * 64 + sqIndex >> 3 ^ (pieceIsWhite == WHITE ? 0 : 0b111);
            int material = evalWeight(112 + pieceType) + (int) (packedData[packedIndex] >> (0x01455410 >> sqIndex * 4) * 8 & 0xFF00FF);

            PieceType sliderPiece = (PieceType) std::min(4, pieceType - 1);
            if (sliderPiece <= 1) sliderPiece = NO_PIECE_TYPE;
            U64 sliderAttacks = attacks(sliderPiece, Square(sqIndex), board.getOccupancy(WHITE) | board.getOccupancy(BLACK));
            int mobility = evalWeight(11 + pieceType) * popCount(sliderAttacks);

            U64 ownPawnMask = pieceIsWhite == WHITE ? 0x0101010101010100UL << sqIndex : 0x0080808080808080UL >> 63 - sqIndex;
            U64 pawnBitboard = board.getPieceBB(pieceIsWhite, PAWN);
            int ownPawnAhead = evalWeight(118 + pieceType) * popCount(ownPawnMask & pawnBitboard);

            sqIndex = material + mobility + ownPawnAhead;
            eval += pieceIsWhite == board.getTurn() ? sqIndex : -sqIndex;
            phase += phaseWeightTable[pieceType];
        }

        eval = (short) eval * phase + ((eval + 0x8000) >> 16) * (24 - phase);
        return eval / 24;
    }

} // namespace Astra


#endif //ASTRA_EVALUATE_H
