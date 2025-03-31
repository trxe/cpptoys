#include <utility>
#include <atomic>
#include <iostream>

class DefaultRefCounter
{
public:
    explicit DefaultRefCounter(uint64_t value) : counter(new std::atomic_uint64_t(value))
    {
        debug_log();
    }
    explicit DefaultRefCounter(const DefaultRefCounter &prev) : counter(prev.counter)
    {
        // not thread safe!
        counter->fetch_add(1);
        debug_log();
    }
    DefaultRefCounter &operator=(const DefaultRefCounter &prev)
    {
        counter->fetch_add(1);
        debug_log();
        return *this;
    }
    ~DefaultRefCounter()
    {
        counter->fetch_sub(1);
        debug_log();
    }
    uint64_t val() { return counter->load(); }

private:
    void debug_log()
    {
        std::cerr << "RC = " << counter->load() << std::endl;
    }
    std::atomic_uint64_t *counter;
};

template <typename T>
class NonArrayAllocator
{
public:
    template <typename... Args>
    static T *alloc(Args &&...args)
    {
        return new T(std::forward<Args>(args)...);
    }

    static void dealloc(T *value)
    {
        delete value;
    }
};

template <typename T, template <typename> typename AllocatorTemplate = NonArrayAllocator, typename RefCounter = DefaultRefCounter, typename Allocator = AllocatorTemplate<T>>
class SharedPtr
{
public:
    // friend class PtrTransfer;

    SharedPtr() : val(nullptr) {}
    explicit SharedPtr(T *val) : val(val) {}
    SharedPtr<T, AllocatorTemplate, RefCounter> &operator=(T *val)
    {
        this->val = val;
    }

    SharedPtr(const SharedPtr<T, AllocatorTemplate, RefCounter> &other) : counter(other.counter), val(other.val) {}
    SharedPtr(SharedPtr<T, AllocatorTemplate, RefCounter> &other) : counter(other.counter), val(other.val) {}

    SharedPtr<T, AllocatorTemplate, RefCounter> &operator=(SharedPtr<T, AllocatorTemplate, RefCounter> &other)
    {
        counter = other.counter;
        val = other.val;
    }

    template <typename... Args>
    SharedPtr(Args &&...args) : counter(1), val(Allocator::alloc(std::forward<Args>(args)...)) {}

    ~SharedPtr()
    {
        if (counter.val() == 1)
            Allocator::dealloc(val);
    }

    T &operator*() const { return *val; }
    T *operator->() const { return val; }
    T *get() const { return val; }

    void swap(SharedPtr<T, AllocatorTemplate, RefCounter> &other)
    {
        auto tmp = val;
        val = other.val;
        other.val = tmp;
    }

    // SharedPtr<T> &operator=(PtrTransfer<T> &wrapped_val);

private:
    // We need an rvalue only wrapper to handle the copying of
    // the pointer across a function call.
    // struct PtrTransfer
    // {
    //     T *val;
    //     PtrTransfer(SharedPtr<T> &return_val);
    // };

    RefCounter counter = 1; // default init to 1
    T *val;

    // Lock these away for now from modification
};