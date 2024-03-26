#ifndef ASTRA_CHESS_ENGINE_HASHING_H
#define ASTRA_CHESS_ENGINE_HASHING_H

#include <cstdint>
#include <array>

namespace Eval {

    uint64_t get_rand();

    const std::array<uint64_t, 64> white_pawn_hash = {
            0, 0, 0, 0, 0, 0, 0, 0,
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            0, 0, 0, 0, 0, 0, 0, 0
    };

    const std::array<uint64_t, 64> black_pawn_hash = {
            0, 0, 0, 0, 0, 0, 0, 0,
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            0, 0, 0, 0, 0, 0, 0, 0
    };

    const std::array<uint64_t, 64> white_knight_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> black_knight_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> white_bishop_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> black_bishop_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> white_rook_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> black_rook_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> white_queen_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> black_queen_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> white_king_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

    const std::array<uint64_t, 64> black_king_hash = {
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(),
            get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
    };

}


#endif //ASTRA_CHESS_ENGINE_HASHING_H
