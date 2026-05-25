#include "generator.hh"

#include <cassert>
#include <random>
#include <set>

namespace radio {
namespace data {

std::vector<Parameter> ParameterGenerator::generate(int number) {
    assert(number < int(ParameterType::kTotal) - 3);

    std::random_device device;
    std::mt19937 gen(device());
    std::uniform_int_distribution dist(0, int(ParameterType::kTotal) - 1);

    std::set<int> used = {0, 4, 5};

    std::vector<Parameter> params;
    params.reserve(number);

    for (int i = 0; i < number; ++i) {
        int type = dist(gen);
        for (; used.contains(type); type = dist(gen)) {}
        used.insert(type);

        params.emplace_back(static_cast<ParameterType>(type));
        std::visit(randomizer_, params.back().data);
    }

    return params;
}

} // namespace data
} // namespace radio
