##
## CMakeLists for stamina
## Requires C++14 or higher
## Requires STORM and boost
##

cmake_minimum_required(VERSION 3.10)

# Main STAMINA executable
include_directories("src")
include_directories("src/stamina")

set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Source files for the main STAMINA CLI
set(SOURCE_DIR src)
set(SOURCE_FILES
	# Main source files for the `stamina` namespace
	src/stamina/ANSIColors.h
	src/stamina/main.cpp
	src/stamina/Stamina.cpp
	src/stamina/Stamina.h
	src/stamina/StaminaArgParse.h
)

# STAMINA library files for linking
set(LIB_FILES
	# Files for `stamina::core` namespace
	src/stamina/core/StaminaModelChecker.h
	src/stamina/core/StaminaModelChecker.cpp
	src/stamina/core/StaminaMessages.h
	src/stamina/core/StaminaMessages.cpp
	src/stamina/core/Options.h
	src/stamina/core/Options.cpp
	src/stamina/core/StateSpaceInformation.cpp
	src/stamina/core/StateSpaceInformation.h
	# Files for `stamina::util` namespace
	src/stamina/util/ModelModify.h
	src/stamina/util/ModelModify.cpp
	src/stamina/util/StateIndexArray.h
	src/stamina/util/StateIndexArray.cpp
	src/stamina/util/StateMemoryPool.h
	src/stamina/util/StateMemoryPool.cpp
	# Files for `stamina::builder` namespace
	src/stamina/builder/StaminaModelBuilder.h
	src/stamina/builder/StaminaModelBuilder.cpp
	src/stamina/builder/StaminaIterativeModelBuilder.cpp
	src/stamina/builder/StaminaIterativeModelBuilder.h
	src/stamina/builder/StaminaThreadedIterativeModelBuilder.cpp
	src/stamina/builder/StaminaPriorityModelBuilder.cpp
	src/stamina/builder/StaminaPriorityModelBuilder.h
	src/stamina/builder/StaminaReExploringModelBuilder.cpp
	src/stamina/builder/StaminaReExploringModelBuilder.h
	src/stamina/builder/ExplicitTruncatedModelBuilder.h
	src/stamina/builder/ExplicitTruncatedModelBuilder.cpp
	# Files for `stamina::builder::threads` namespace
	src/stamina/builder/threads/BaseThread.cpp
	src/stamina/builder/threads/BaseThread.h
	src/stamina/builder/threads/ControlThread.cpp
	src/stamina/builder/threads/ControlThread.h
	src/stamina/builder/threads/ExplorationThread.cpp
	src/stamina/builder/threads/ExplorationThread.h
	src/stamina/builder/threads/IterativeExplorationThread.cpp
	src/stamina/builder/threads/IterativeExplorationThread.h
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
if (storm_FOUND)
	message("STORM found!")
else()
	message("STORM not found! STORM required to build STAMINA!")
endif()

# STAMINA shared-object libraries project
project(stamina-dev VERSION 2.5.0 DESCRIPTION "Shared objects for STAMINA usage")
include_directories("src")
include_directories("src/stamina")

add_library(stamina-dev SHARED ${LIB_FILES})
set_target_properties(stamina-dev PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})
target_link_libraries(${PROJECT_NAME} PUBLIC storm storm-parsers)

# TODO: Continue cmake for lib: https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake

# Add executable target with source files listed in SOURCE_FILES variable
project(sstamina VERSION 2.5.0 DESCRIPTION "Main STAMINA/STORM executable")
add_executable(sstamina ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})
target_link_libraries(${PROJECT_NAME} PUBLIC stamina-dev storm storm-parsers)


