#pragma once

#include <vector>

#include "parameter.hh"
#include "series.hh"

namespace radio 
{
	namespace data
	{

		/**
		 * @brief The parameter list series generator
		 */
		class ParameterSeriesGenerator 
		{
		public:
			/**
			 * @brief Generate list of parameters with series data
			 * @param number Number of parameters
			 * @param elapsed_us Board clock value in us since start
			 */
			static std::vector<Parameter> generate(int number, int64_t elapsed_us);

		private:
			inline static Series series_;
		};

	} // namespace data
} // namespace radio
