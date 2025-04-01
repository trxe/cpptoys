#include <stdexcept>
#include <format>

template <typename L, typename R>
struct Pair
{
    L first;
    R second;
};

template <typename Type, typename... Args>
class Tuple
{
public:
    Tuple(Type val, Args &&...args)
        : head(val), tail(std::forward<Args>(args)...) {}
    Type head;
    Tuple<Args &&...> tail;
};

template <typename OneType>
class Tuple<OneType, void> // specialization
{
public:
    Tuple(OneType val) : head(val) {}
    OneType head;
};

template <int index, typename... TupleArgs>
auto get(Tuple<TupleArgs &&...> tuple)
{
    if (index < 0)
        throw std::runtime_error(std::format("invalid index {} < 0", index));
    if (index == 0)
        return tuple.head;
    return get<index - 1, decltype(tuple.tail)>(tuple.tail);
}
