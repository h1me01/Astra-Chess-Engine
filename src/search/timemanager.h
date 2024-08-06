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

#ifndef ASTRA_TIMEMANAGER_H
#define ASTRA_TIMEMANAGER_H

#include <chrono>
#include "../chess/types.h"

using namespace Chess;

namespace Astra {

    class TimeManager {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;

        TimeManager() : startTime(Clock::now()) {}

        // Start the timer
        void start() {
            startTime = Clock::now();
        }

        // set total time allowed for a game (in milliseconds)
        void setTimePerMove(unsigned int milliseconds) {
            timePerMove = milliseconds;
        }

        // check if the time control has been exceeded
        bool isTimeExceeded() const {
            return elapsedTime() > timePerMove;
        }

    private:
        TimePoint startTime;
        unsigned int timePerMove{};

        // get elapsed time since start (in milliseconds)
        int elapsedTime() const {
            auto currentTime = Clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        }

    };

} // namespace Astra

#endif //ASTRA_TIMEMANAGER_H
