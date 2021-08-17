cmake_minimum_required(VERSION 3.10)  # CMake version check
project(stamina-cplusplus)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)



# Add main.cpp file of project root directory as source file
# set(SOURCE_FILES src/InfCTMCModelGenerator.cpp src/InfCTMCModelGenerator.h src/main.cpp src/ProbState.h src/StaminaCL.cpp
#         src/StaminaCL.h src/StaminaModelChecker.cpp src/StaminaModelChecker.h src/StaminaOptions.cpp src/StaminaOptions.h)
set(SOURCE_DIR src)
set(SOURCE_FILES 
    src/ANSIColors.h
    src/main.cpp
    src/Stamina.cpp
    src/Stamina.h
    src/StaminaArgParse.h
    src/StaminaModelBuilder.h
    src/StaminaModelBuilder.cpp
)
# set(SOURCE_FILES src/InfCTMCNextStateGenerator.h src/InfCTMCNextStateGenerator.cpp src/main.cpp src/ProbState.h src/StaminaCL.cpp
#         src/StaminaCL.h src/StaminaModelChecker.cpp src/StaminaModelChecker.h src/StaminaOptions.cpp src/StaminaOptions.h 
#         src/StaminaExplicitModelBuilder.h src/StaminaExplicitModelBuilder.cpp)

# set(STORM_PATH /home/awilliams/stamina/storm)
set(STORM_PATH /home/josh/Documents/Work/Research/stamina/cpp/stamina-cplusplus/storm)
set(LOCAL_PATH /usr/lib)
set(LIB_PATH /home/josh/Documents/Work/Research/stamina/cpp/stamina-cplusplus/storm/lib)
set(CUDD_PATH ${STORM_PATH}/storm/resources/3rdparty/cudd-3.0.0/cplusplus)
# storm/resources/3rdparty/cudd-3.0.0/cplusplus/cuddObj.hh  

find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDES})
endif (Boost_FOUND)

# include(ExternalProject)
# ExternalProject_Add(storm URL https://github.com/moves-rwth/storm/archive/stable.zip
# SOURCE_DIR "${STORM_PATH}")

find_package(storm REQUIRED PATHS ${STORM_PATH})
# find_package(storm REQUIRED)

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


# target_include_directories(${PROJECT_NAME} PUBLIC )


# Add executable target with source files listed in SOURCE_FILES variable
add_executable(stamina-cplusplus ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})
target_link_libraries(${PROJECT_NAME} PUBLIC storm storm-parsers)
# add_dependencies(stamina-cplusplus storm)

