##
## CMakeLists for stamina
## Requires C++14 or higher
## Requires STORM and boost
##

cmake_minimum_required(VERSION 3.10)  # CMake version check
project(stamina-cplusplus)               # Create project "stamina-cplusplus"
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Add main.cpp file of project root directory as source file

set(SOURCE_DIR src)
set(SOURCE_FILES
    # Main source files for the `stamina` namespace
	src/stamina/ANSIColors.h
    src/stamina/main.cpp
    src/stamina/Stamina.cpp
    src/stamina/Stamina.h
    src/stamina/StaminaArgParse.h
    src/stamina/StaminaModelBuilder.h
    src/stamina/StaminaModelBuilder.cpp
    src/stamina/StaminaModelChecker.h
    src/stamina/StaminaModelChecker.cpp
    src/stamina/StaminaMessages.h
	src/stamina/StaminaMessages.cpp
    src/stamina/Options.h
    src/stamina/Options.cpp
	src/stamina/ExplicitTruncatedModelBuilder.h
	src/stamina/ExplicitTruncatedModelBuilder.cpp
	src/stamina/StateSpaceInformation.cpp
	src/stamina/StateSpaceInformation.h
	# Files for `stamina::util` namespace
	src/stamina/util/ModelModify.h
	src/stamina/util/ModelModify.cpp
	src/stamina/util/StateIndexArray.h
	src/stamina/util/StateIndexArray.cpp
)

message("STORM_PATH is set as " ${STORM_PATH})

set(LOCAL_PATH /usr/lib)
set(LIB_PATH ${STORM_PATH}/lib)
set(CUDD_PATH ${STORM_PATH}/storm/resources/3rdparty/cudd-3.0.0/cplusplus)

# Use BOOST for STORM (and also in some cases in STAMINA)
find_package(Boost)
if (Boost_FOUND)
	message("BOOST found!")
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDES})
endif (Boost_FOUND)

find_package(storm REQUIRED PATHS ${STORM_PATH})
# if (storm_FOUND)
#	message("STORM found!")
#else
#	message("STORM not found! STORM required to build STAMINA!")
#endif

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(sstamina ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})
target_link_libraries(${PROJECT_NAME} PUBLIC storm storm-parsers)
