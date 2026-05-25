#pragma once

#include <cstddef>
#include <vector>

namespace radio {
namespace data {

struct Parameter;

using BinaryData = std::vector<std::byte>;
using FloatData = std::vector<float>;

using ParametersData = std::vector<Parameter>;

} // namespace data
} // namespace radio
