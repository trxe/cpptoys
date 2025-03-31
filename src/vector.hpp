#pragma once
#include <cstring>
#include <format>
#include <exception>
#include <cstddef>
#include <iostream>

template <typename T>
class Vector
{
public:
    struct Iter
    {
        // iterator_traits: https://en.cppreference.com/w/cpp/iterator/iterator_traits#Member_types
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = T &;
        using pointer = T *;
        using iterator_category = std::input_iterator_tag;

        T *value;

        Iter(T *value) : value(value) {}

        T &operator*() { return *value; }

        T *operator->() { return value; }

        Iter &operator++()
        {
            ++value;
            return *this;
        }
        Iter &operator++(int)
        {
            ++value;
            return Iter(this.value - 1);
        }

        bool operator==(const Iter &other) { return value == other.value; }
        bool operator!=(const Iter &other) { return value != other.value; }
        std::ptrdiff_t operator-(const Iter &other) { return value - other.value; }
        std::ptrdiff_t operator<=>(const Iter &other) { return value - other.value; }
    };

    Vector(size_t qty) : _begin(new T[qty]), _end(_begin), alloc_qty(qty)
    {
    }
    Vector(std::initializer_list<size_t> xs) : _begin(new T[xs.size()]), _end(_begin + xs.size()), alloc_qty(xs.size())
    {
        for (auto i = 0; i < xs.size(); ++i)
        {
            _begin[i] = std::data(xs)[i];
        }
    }
    ~Vector()
    {
        delete[] _begin;
    }
    Vector(const Vector &other) : _begin(new T[other.alloc_bytes() / sizeof(T)]), _end(_begin + other.size()), alloc_qty(other.alloc_bytes() / sizeof(T))
    {
        for (size_t i = 0; i < other.size(); ++i)
        {
            _begin[i] = other.at(i);
        }
    };
    Vector &operator=(const Vector &other)
    {
        if (this == &other)
            return *this;
        alloc_qty = other.alloc_bytes() / sizeof(T);
        _begin = new T[other.size()];
        _end = _begin + other.size();
        for (size_t i = 0; i < other.size(); ++i)
        {
            _begin[i] = other.at(i);
        }
        return *this;
    };
    void push_back(const T &t)
    {
        if (size() == alloc_qty)
        {
            resize(alloc_qty << 1);
        }
        // std::cout << start << " " << start + size() << " " << size() << std::endl;
        _begin[size()] = t;
        _end += 1;
    }
    void push_back(T &&t)
    {
        if (size() == alloc_qty)
        {
            resize(alloc_qty << 1);
        }
        _begin[size()] = std::move(t);
        _end += 1;
    }
    void resize(size_t target_qty, bool force = false)
    {
        auto num_elements_to_copy = std::min(target_qty, size());
        std::cout << "reallocating..." << std::endl;
        T *new_start = new T[target_qty];
        // We cannot use memcpy to copy the old buffer to new because it only performs a
        // bytewise copy, ignoring constructors, functions and operators. So the following is invalid.
        // std::memcpy(static_cast<void *>(new_start), static_cast<void *>(start), target_size);
        for (auto n = 0; n < num_elements_to_copy; ++n)
        {
            // std::cout << new_start + n << " " << start + n << std::endl;
            *(new_start + n) = _begin[n];
        }
        alloc_qty = target_qty;
        delete[] _begin;
        // std::cout << new_start + target_qty - 1 << std::endl;
        _begin = new_start;
        _end = new_start + num_elements_to_copy;
    }
    size_t size() const
    {
        return static_cast<size_t>(_end - _begin);
    }
    size_t alloc_bytes() const
    {
        return alloc_qty * sizeof(T);
    }
    T pop_back()
    {
        T val = *(_end - 1);
        // IMPT: YOU CAN'T DELETE A PORTION OF MEMORY ALLOCATED!
        // delete &start[size()];
        auto new_size = alloc_qty >> 1;
        if (size() - 1 < new_size && new_size > 0)
        {
            resize(new_size);
        }
        _end -= 1;
        return val;
    }
    T at(int idx) const
    {
        auto curr_size = size();
        if (std::abs(idx) >= curr_size)
        {
            throw std::runtime_error(std::format("Error: idx ({}) > size ({})", idx, curr_size));
        }
        else if (idx < 0)
        {
            idx = curr_size + idx;
        }

        return _begin[idx];
    }
    T &operator[](int idx) const
    {
        auto curr_size = size();
        if (std::abs(idx) >= curr_size)
        {
            throw std::runtime_error(std::format("Error: idx ({}) > size ({})", idx, curr_size));
        }
        else if (idx < 0)
        {
            idx = curr_size + idx;
        }

        return _begin[idx];
    }

    Iter begin()
    {
        return Iter(_begin);
    }

    Iter end()
    {
        return Iter(_end);
    }

private:
    T *_begin;
    T *_end;
    size_t alloc_qty;
};