#include "builder_wrapper.hh"

#include <boost/dll/alias.hpp>

#include "driver/builder.hh"

radio::IDriverBuilder* createBuilder() {
    return new radio::driver::DriverBuilder();
}

// Map the function to the alias string
BOOST_DLL_ALIAS(createBuilder, dllCreateBuilder);
