#include "tools/timer.hpp"
#include "semaphore.hpp"
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <syncstream>
#include <chrono>
#include <mutex>
#include <semaphore>

using namespace std::chrono_literals;

Semaphore::Semaphore(int init) : val(init)
{
}

void Semaphore::acq()
{
    while (!try_acq())
        ;
}

bool Semaphore::try_acq()
{
    int space_left = 0;

    // if val == 0, then continue to store 0 in val.
    // if val != 0, then store val in space_left.
    if (!val.compare_exchange_strong(space_left, 0, std::memory_order_relaxed))
    // if (!val.compare_exchange_strong(space_left, 0, std::memory_order_acq_rel))
    // if (!val.compare_exchange_strong(space_left, 0, std::memory_order_seq_cst))
    {
        // if val == space_left still (i.e. we have not switched from the thread),
        // then deduct -1 from val and return true.
        // if val != space_left, then do not do anything and return false.
        return val.compare_exchange_strong(space_left, space_left - 1, std::memory_order_relaxed);
        // return val.compare_exchange_strong(space_left, space_left - 1, std::memory_order_acquire);
        // return val.compare_exchange_strong(space_left, space_left - 1, std::memory_order_seq_cst);
    }
    return false;
}

void Semaphore::rel()
{
    int space_left = 0;

    val.fetch_add(1, std::memory_order_relaxed);
    // val.fetch_add(1, std::memory_order_release);
    // val.fetch_add(1, std::memory_order_seq_cst);
}

// Testing code

constexpr size_t SEMVAL = 5;

template <size_t max_capacity = SEMVAL>
class SemLogger
{
public:
    struct Log
    {
        bool is_join;
        int worker_num;
    };

    void push(bool is_join, int worker_num)
    {
        std::lock_guard<std::mutex> lk(mut);
        loglist.push_back({
            .is_join = is_join,
            .worker_num = worker_num,
        });
    }

    void validate() const
    {
        // assumes pushes are in chronological order
        // the number of workers joined at the same time should never exceed SEMVAL
        int capacity = SEMVAL;
        for (const auto &log : loglist)
        {
            capacity -= log.is_join ? 1 : -1;
            if (capacity < 0)
            {
                throw std::runtime_error("FAILED: semaphore capacity exceeded.");
            }
        }
    }

    const std::vector<Log> &get_loglist() const { return loglist; }

private:
    std::mutex mut;
    std::vector<Log> loglist;
};

template <size_t MaxCap>
void validate_run(const SemLogger<MaxCap> &semlogger)
{
    try
    {
        semlogger.validate();
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        for (const auto &log : semlogger.get_loglist())
        {
            std::cerr << "Worker " << log.worker_num << (log.is_join ? " joining!" : " leaving.") << std::endl;
        }
        exit(1);
    }
}

template <size_t MaxCap>
void runner(const size_t &workers, const size_t &rounds, uint64_t &timesum_ms, auto &&fn)
{
    for (auto r = 0; r < rounds; ++r)
    {
        SemLogger semlogger;
        {
            Timer timer(&timesum_ms);
            Semaphore sem(MaxCap);
            std::vector<std::thread> threads;

            for (auto w = 1; w <= workers; ++w)
            {
                threads.emplace_back(fn, std::ref(sem), std::ref(semlogger), w);
            }
            std::ranges::for_each(threads, [](std::thread &thread)
                                  { thread.join(); });
        }
        validate_run<MaxCap>(semlogger);
    }
}

template <size_t MaxCap>
void runner_std(const size_t &workers, const size_t &rounds, uint64_t &timesum_ms, auto &&fn)
{
    for (auto r = 0; r < rounds; ++r)
    {
        SemLogger semlogger;
        {
            Timer timer(&timesum_ms);
            std::counting_semaphore<MaxCap> sem{MaxCap};
            std::vector<std::thread> threads;

            for (auto w = 1; w <= workers; ++w)
            {
                threads.emplace_back(fn, std::ref(sem), std::ref(semlogger), w);
            }
            std::ranges::for_each(threads, [](std::thread &thread)
                                  { thread.join(); });
        }
        validate_run<MaxCap>(semlogger);
    }
}

int main(int argc, char *argv[])
{
    size_t workers = argc > 1 ? std::stoi(argv[1]) : 10;
    size_t rounds = argc > 2 ? std::stoi(argv[2]) : 5;

    uint64_t timesum_ms = 0;
    std::cout << "mysem" << std::endl;
    runner<SEMVAL>(workers, rounds, timesum_ms, [&](Semaphore &sem, SemLogger<SEMVAL> &logger, size_t w)
                   { 
                sem.acq(); 
                // std::osyncstream{std::cout} << "Worker " << w << " joined!" << std::endl;
                logger.push(true, w);
                std::this_thread::sleep_for(100ms);
                logger.push(false, w);
                // std::osyncstream{std::cout} << "Worker " << w << " leaving!" << std::endl;
                sem.rel(); });
    std::cout << "mysem AVERAGE TIME: " << static_cast<double>(timesum_ms) / static_cast<double>(rounds) << "ms" << std::endl;
    timesum_ms = 0;
    runner_std<SEMVAL>(workers, rounds, timesum_ms, [&](std::counting_semaphore<SEMVAL> &sem, SemLogger<SEMVAL> &logger, size_t w)
                       { 
                sem.acquire(); 
                // std::osyncstream{std::cout} << "Worker " << w << " joined!" << std::endl;
                logger.push(true, w);
                std::this_thread::sleep_for(100ms);
                logger.push(false, w);
                // std::osyncstream{std::cout} << "Worker " << w << " leaving!" << std::endl;
                sem.release(); });
    std::cout << "stdsem AVERAGE TIME: " << static_cast<double>(timesum_ms) / static_cast<double>(rounds) << "ms" << std::endl;
}