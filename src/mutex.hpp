#pragma once
#include <atomic>

/** Implementation of Mutex with atomics only. */
class Mutex
{
public:
    Mutex();
    ~Mutex();
    Mutex operator=(const Mutex &) = delete;  // assignment constructor
    Mutex operator()(const Mutex &) = delete; // copy constructor
    void acq();
    void rel();

private:
    std::atomic_bool val;
};