cmake_minimum_required(VERSION 3.10)  # CMake version check
project(stamina-cplusplus)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard



# Add main.cpp file of project root directory as source file
set(SOURCE_FILES src/InfCTMCNextStateGenerator.h src/InfCTMCNextStateGenerator.cpp src/InfCTMCModelGenerator.cpp src/InfCTMCModelGenerator.h src/main.cpp src/ProbState.h src/StaminaCL.cpp
        src/StaminaCL.h src/StaminaModelChecker.cpp src/StaminaModelChecker.h src/StaminaOptions.cpp src/StaminaOptions.h)
set(STORM_PATH /home/awilliams/stamina/storm)

find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDES})
endif (Boost_FOUND)

#include(ExternalProject)
#ExternalProject_Add(storm URL https://github.com/moves-rwth/storm/archive/stable.zip
#SOURCE_DIR "${STORM_PATH}")

find_package(storm REQUIRED PATHS ${STORM_PATH})

#include_directories(${STORM_PATH}/src)
#include_directories(${STORM_PATH}/src/storm)
#include_directories(${STORM_PATH}/src/storm/include)
#include_directories(${STORM_PATH}/src/storm/include/storm)

#add_library(storm STATIC IMPORTED STORM_PATH)
#include_directories(${STORM_PATH})
#include_directories(${STORM_PATH}/src)
#include_directories(${STORM_PATH}/src/storm)
#include_directories(${STORM_PATH}/build/include)
#include(${STORM_PATH}/resources/3rdParty/CMakeLists.txt)





# Add executable target with source files listed in SOURCE_FILES variable
add_executable(stamina-cplusplus ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_FILES} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE storm storm-parsers)
add_dependencies(stamina-cplusplus storm)
