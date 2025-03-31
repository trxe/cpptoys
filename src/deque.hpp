#pragma once
#include <cstring>
#include <format>
#include <exception>
#include <cstddef>
#include <iostream>

#define DEFAULT_BLKSIZE 4

template <typename T, size_t BlockSize = DEFAULT_BLKSIZE>
class Deque
{
public:
    struct Block
    {
        T arr[BlockSize];
        T *_begin;
        T *_end;
    };

    // Random Access should be O(1).

    // push_back
    // push_front
    // pop_back
    // pop_front

private:
    Block *buffer;
    size_t block_qty;
    Block *_begin;
    Block *_end;
};