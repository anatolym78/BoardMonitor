#pragma once

#include <vector>

#include "parameter.hh"
#include "randomizer.hh"

namespace radio {
namespace data {

/**
 * @brief The parameter list random generator
 */
class ParameterGenerator {
public:
	/**
	 * @brief Generate list of parameters with random data
	 * @param number Number of parameters
	 */
	static std::vector<Parameter> generate(int number);

private:
	inline static ParameterRandomizer randomizer_;
};

} // namespace data
} // namespace radio
