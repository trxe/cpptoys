#include "tools/timer.hpp"
#include "mutex.hpp"
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include <utility>
#include <algorithm>
#include <syncstream>
#include <chrono>
#include <mutex>

Mutex::Mutex() : val(true)
{
}

Mutex::~Mutex()
{
    this->rel();
}

void Mutex::acq()
{
    bool avail;
    // compares val to avail.
    // if val == avail, replace val with false.
    // if val != avail, load val into avail.
    do
    {
        avail = true;
        // } while (!val.compare_exchange_strong(avail, false, std::memory_order_seq_cst));
    } while (!val.compare_exchange_strong(avail, false, std::memory_order_acquire));
}
void Mutex::rel()
{
    val.store(true, std::memory_order_release);
    // val.store(true, std::memory_order_seq_cst);
}

// Testing code

void runner(const size_t &workers, const size_t &rounds, const size_t &attempts, int *shared_val_no_protect, uint64_t &timesum, auto &&fn)
{
    for (size_t a = 0; a < rounds; ++a)
    {
        {
            Timer timer(&timesum);
            std::vector<std::thread> threads;
            Mutex mutex;
            for (size_t t = 1; t <= workers; ++t)
            {
                threads.emplace_back(
                    fn,
                    t, shared_val_no_protect, std::ref(attempts), std::ref(mutex));
            }

            std::ranges::for_each(threads, [](std::thread &thread)
                                  { thread.join(); });
        }
        *shared_val_no_protect = 0;
    }
}

void runner_std(const size_t &workers, const size_t &rounds, const size_t &attempts, int *shared_val_no_protect, uint64_t &timesum, auto &&fn)
{
    for (size_t a = 0; a < rounds; ++a)
    {
        {
            Timer timer(&timesum);
            std::vector<std::thread> threads;
            std::mutex mutex;
            for (size_t t = 1; t <= workers; ++t)
            {
                threads.emplace_back(
                    fn,
                    t, shared_val_no_protect, std::ref(attempts), std::ref(mutex));
            }

            std::ranges::for_each(threads, [](std::thread &thread)
                                  { thread.join(); });
        }

        *shared_val_no_protect = 0;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Mutex Test" << std::endl;
    size_t workers = argc > 1 ? std::stoi(argv[1]) : 2;
    size_t attempts = argc > 2 ? std::stoi(argv[2]) : 1'000'000;
    size_t rounds = argc > 3 ? std::stoi(argv[3]) : 10;
    int *shared_val_no_protect = new int(0);
    auto assert_critical_section = [](int *shared_val_no_protect, size_t i, size_t val)
    {
        const auto old_cached = *shared_val_no_protect;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (*shared_val_no_protect != old_cached)
        {
            std::osyncstream{std::cout} << i << ", " << val << " [interrupted] expected: "
                                        << old_cached << "; actual: "
                                        << *shared_val_no_protect << ";" << std::endl;
        }
    };

    uint64_t timesum_ms = 0;
    runner(workers, rounds, attempts, shared_val_no_protect, timesum_ms,
           [&](size_t val, int *shared_val_no_protect, const size_t &rounds, Mutex &mutex)
           {
               for (auto i = 0; i < rounds; ++i)
               {
                   mutex.acq();
                   std::this_thread::sleep_for(std::chrono::milliseconds(10));
                   assert_critical_section(shared_val_no_protect, i, val);
                   *shared_val_no_protect += val * 10;
                   //    std::osyncstream{std::cout} << i << " Protected +" << val << ": " << *shared_val_no_protect << std::endl;
                   mutex.rel();
               }
           });
    std::cout << "mymutex AVERAGE TIME (ms): " << static_cast<double>(timesum_ms) / static_cast<double>(rounds) << "ms" << std::endl;
    timesum_ms = 0;

    runner_std(workers, rounds, attempts, shared_val_no_protect, timesum_ms,
               [&](size_t val, int *shared_val_no_protect, const size_t &rounds, std::mutex &mutex)
               {
                   for (auto i = 0; i < rounds; ++i)
                   {
                       std::lock_guard<std::mutex> lk(mutex);
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));
                       assert_critical_section(shared_val_no_protect, i, val);
                       *shared_val_no_protect += val * 10;
                       //    std::osyncstream{std::cout} << i << " stdProtected +" << val << ": " << *shared_val_no_protect << std::endl;
                   }
               });
    std::cout << "stdmutex AVERAGE TIME: " << static_cast<double>(timesum_ms) / static_cast<double>(rounds) << "ms" << std::endl;
    delete shared_val_no_protect;
    return 0;
}