#pragma once

#include "builder.hh"

/**
 * @brief The DLL wrapper to create a driver builder
 * @return The driver builder
 */
radio::IDriverBuilder* createBuilder();
