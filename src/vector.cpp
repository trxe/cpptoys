#include "vector.hpp"
#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>

struct Dummy
{
    size_t a;
    size_t b;
    friend std::ostream &operator<<(std::ostream &out, const Dummy &dummy)
    {
        return out << "{" << dummy.a << ", " << dummy.b << "}";
    }
};

int main(int argc, char *argv[])
{
    // Push/Pop
    size_t size = argc > 1 ? std::stoi(argv[1]) : 10;
    Vector<Dummy> myvec(size / 2);
    myvec.push_back(Dummy{.a = 100, .b = 100});
    myvec.push_back(Dummy{.a = 50, .b = 50});

    // To test address stability later
    auto &x = myvec[1];
    for (size_t i = 0; i < size; ++i)
    {
        Dummy d{.a = i, .b = size - i};
        std::cout << "Inserting " << d << std::endl;
        myvec.push_back(d);
    }
    for (size_t i = 0; i < size; ++i)
    {
        auto val = myvec.pop_back();
        std::cout
            << "val " << val << " "
            << "size " << myvec.size() << " "
            << "alloc " << myvec.alloc_bytes() << " "
            << std::endl;
    }
    // The following code will cause heap-reuse-after-free because
    // the buffer was reallocated. our implementation of push_back/pop_back
    // are both not address stable.
    // x.a = 19;
    // x.b = 88;
    // for (const auto &x : myvec)
    // {
    //     std::cout << x << std::endl;
    // }

    // Initializer List
    Vector<size_t>
        ayylmao{5, 0, 2, 2, 0, 6, 0, 7, 3, 2, 1, 5, 2, 6};
    std::cout << "ayylmao size: " << ayylmao.size() << " alloc " << ayylmao.alloc_bytes() << std::endl;
    std::cout << ayylmao.at(2) << " " << ayylmao.at(-4) << std::endl;
    Vector<size_t> alcopy = ayylmao;
    std::cout << "alcopy size: " << alcopy.size() << " alloc " << alcopy.alloc_bytes() << std::endl;
    std::cout << alcopy.at(2) << " " << alcopy.at(-4) << std::endl;
    alcopy = alcopy;
    std::cout << "alcopy size: " << alcopy.size() << " alloc " << alcopy.alloc_bytes() << std::endl;

    // Iterators
    auto it = std::find(alcopy.begin(), alcopy.end(), static_cast<size_t>(6));
    std::cout << "pos of it: " << it - alcopy.begin() << std::endl;
    *it.value = 42069;
    for (const auto &v : alcopy)
    {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}