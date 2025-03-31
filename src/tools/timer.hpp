#pragma once
#include <chrono>
#include <syncstream>
#include <iostream>

template <typename Clock, typename Dur = std::chrono::milliseconds>
class ScopedSpan
{
    using timestamp_t = std::chrono::time_point<Clock>;

public:
    ScopedSpan() : start(Clock::now()), timesum(nullptr) {};
    ScopedSpan(uint64_t *timesum) : start(Clock::now()), timesum(timesum) {
                                        // std::cout << "timesum " << *timesum << std::endl;
                                    };
    ~ScopedSpan()
    {
        timestamp_t end = Clock::now();
        auto duration = getms(end) - getms(start);
        std::osyncstream{std::cout} << "start " << start << "\n"
                                    << "end " << end << "\n"
                                    << "duration " << duration << "ms\n"
                                    << std::endl;
        if (timesum != nullptr)
        {
            *timesum += duration;
        }
    };

private:
    uint64_t getms(const timestamp_t &ts)
    {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count());
    }
    timestamp_t start;
    uint64_t *timesum;
};

using Timer = ScopedSpan<std::chrono::system_clock>;