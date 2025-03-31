#pragma once
#include <atomic>

/** Implementation of Semaphore with atomics only. */
class Semaphore
{
public:
    Semaphore(int init);
    Semaphore operator=(const Semaphore &) = delete;  // assignment constructor
    Semaphore operator()(const Semaphore &) = delete; // copy constructor
    // Blocking
    void acq();
    void rel();
    // Non-blocking
    bool try_acq();

private:
    std::atomic_int val;
};