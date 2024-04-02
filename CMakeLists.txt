cmake_minimum_required(VERSION 3.27)
project(Astra_Chess_Engine)

set(CMAKE_CXX_STANDARD 17)

add_executable(Astra_Chess_Engine
        src/main.cpp
        src/chess/types.h
        src/chess/misc.h
        src/chess/bitboard.h
        src/chess/attacks.h
        src/chess/attacks.cpp
        src/chess/board.cpp
        src/chess/board.h
        src/chess/movegen.h
        src/chess/perft.h
        src/search/search.cpp
        src/search/search.h
        src/search/moveordering.cpp
        src/search/moveordering.h
        src/search/tt.h
        src/search/tt.cpp
        src/search/pvTable.h
        src/search/timemanager.h
        src/genData.h
        src/eval/nnue.cpp
        src/eval/nnue.h
        src/incbin/incbin.h
        src/eval/accumulators.h
        src/eval/evaluate.h

)
