#include "shared_ptr.hpp"
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

struct Sample
{
    uint64_t id;
    std::string value;
    Sample(uint64_t id, const char *value) : id(id), value(value) {}
    Sample(const Sample &T) = default;
    Sample &operator=(const Sample &T) = default;
    friend std::ostream &operator<<(std::ostream &out, Sample const &s)
    {
        return out << "{" << s.id << " " << s.value << "}";
    }
};

void manipulate(SharedPtr<Sample> sample, char c)
{
    sample->value.push_back(c);
}

int main(int argc, char *argv[])
{
    // the following simply doesn't work:
    // array values have to be handled separately
    // TODO: identify via concepts/constraints
    // SharedPtr<Sample[2]> s({});
    // auto internal = s.get();
    // auto sam = *internal;
    // auto nxt = *(internal + 1);
    // sam->id = 10;
    // sam->value = std::string("one");
    // nxt->id = 20;
    // nxt->value = std::string("dbl");
    // std::cout << *sam << " " << *nxt << std::endl;

    Sample smpl{10, "ref_"};
    std::cout << smpl << std::endl;

    using sp_t = SharedPtr<Sample>;
    sp_t base_sample(smpl);

    {
        std::vector<sp_t> samples;
        std::cout << "PUSHBACK" << std::endl;
        for (size_t i = 0; i < 10; ++i)
        {
            samples.push_back(base_sample);
        }

        char c = '1';
        for (auto &sample : samples)
        {
            manipulate(sample, c);
            ++c;
        }
    }

    std::cout << *base_sample << std::endl;

    // BUG: Does not do anything!
    manipulate(*base_sample, '_');

    // BUT THIS SHOULD
    manipulate(base_sample, '_');

    {
        std::vector<sp_t> samples;
        std::cout << "EMPLACE_BACK" << std::endl;
        for (size_t i = 0; i < 10; ++i)
        {
            samples.emplace_back(base_sample);
        }

        char c = '1';
        for (auto &sample : samples)
        {
            manipulate(sample, c);
            ++c;
        }
    }

    std::cout << *base_sample << std::endl;

    Sample raw_pair_sample[2] = {
        {1, "_"},
        {2, "@"},
    };
    // this simple SharedPtr doesn't handle array types
    // This doesn't work (cannot convert fwded args Sample(&)[2] to Sample)
    // using sp2_t = SharedPtr<Sample[2]>;
    // sp2_t pair_sample(raw_pair_sample);
    // (*pair_sample)[0] = Sample(1, "one");
    // (*pair_sample)[1] = Sample(2, "two");
    // {
    //     std::vector<sp2_t> psamples;
    //     for (size_t i = 0; i < 10; ++i)
    //     {
    //         psamples.emplace_back(pair_sample);
    //     }
    // }
}