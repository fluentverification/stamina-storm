cmake_minimum_required(VERSION 3.2)

# set the project name and version
project(bug-reproduce VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# search for Storm library
find_package(storm REQUIRED)

# specify source files
set(SOURCE_FILES test.cpp)

# set executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_FILES} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE storm storm-parsers)

