cmake_minimum_required(VERSION 3.22)

project(RCImitator LANGUAGES CXX)

# Enable automatic processing of Qt-specific files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set C++ project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    add_compile_options(-Werror -Wall -Wextra -pedantic)
endif()



# Boost settings
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)

add_definitions(-DBOOST_ALL_NO_LIB)

# Include tools and library directories
find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Boost 1.89.0 REQUIRED COMPONENTS filesystem json exception)

# Build shared library containing the plugin business logic
add_library(${PROJECT_NAME}_plugin SHARED)
target_sources(${PROJECT_NAME}_plugin PRIVATE
    data/data.hh
    data/generator.cc
    data/generator.hh
    data/parameter.cc
    data/parameter.hh
    data/parameters.hh
    data/randomizer.hh
    data/serializer.cc
    data/serializer.hh
    data/series.hh
    data/series.cc

    driver/builder.cc
    driver/builder.hh
    driver/imitator.cc
    driver/imitator.hh

    builder_wrapper.cc
    builder_wrapper.hh
    builder.hh
    driver.hh
)

# Include directories to the business logic lib
target_include_directories(${PROJECT_NAME}_plugin PRIVATE
    ${Boost_INCLUDE_DIRS}
)
# Link target libraries to the business logic lib
target_link_libraries(${PROJECT_NAME}_plugin PRIVATE
    Boost::filesystem
    Boost::json
    Boost::exception
    Qt5::Core
)

# Build the main executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    builder.hh
    driver.hh
    main.cc
)

# Include directories to the main executable
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Boost_INCLUDE_DIRS}
)

# Link target libraries to the main executable
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::filesystem
    Boost::exception
    Qt5::Core
)
