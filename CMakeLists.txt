##
## CMakeLists for stamina
## Requires C++14 or higher
## Requires STORM and boost
##

cmake_minimum_required(VERSION 3.10)

# Main STAMINA executable
include_directories("src")
include_directories("src/stamina")

project(sstamina)               # Create project "sstamina"
set(LIB_NAME stamina)
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Source files for the main STAMINA CLI
set(SOURCE_DIR src)
set(SOURCE_FILES
	# Main source files for the `stamina` namespace
	src/stamina/main.cpp
)

set(LIB_FILES
	src/stamina/Stamina.cpp
	# Files for `stamina::core` namespace
	src/stamina/core/StaminaModelChecker.cpp
	src/stamina/core/StaminaMessages.cpp
	src/stamina/core/Options.cpp
	src/stamina/core/StateSpaceInformation.cpp
	# Files for `stamina::util` namespace
	src/stamina/util/ModelModify.cpp
	src/stamina/util/StateIndexArray.cpp
	src/stamina/util/StateMemoryPool.cpp
	# Files for `stamina::builder` namespace
	src/stamina/builder/StaminaModelBuilder.cpp
	src/stamina/builder/StaminaIterativeModelBuilder.cpp
	src/stamina/builder/StaminaThreadedIterativeModelBuilder.cpp
	src/stamina/builder/StaminaPriorityModelBuilder.cpp
	src/stamina/builder/StaminaReExploringModelBuilder.cpp
	src/stamina/builder/ExplicitTruncatedModelBuilder.cpp
	# Files for `stamina::builder::threads` namespace
	src/stamina/builder/threads/BaseThread.cpp
	src/stamina/builder/threads/ControlThread.cpp
	src/stamina/builder/threads/ExplorationThread.cpp
	src/stamina/builder/threads/IterativeExplorationThread.cpp
	# Files for `stamina::priority` namespace
	src/stamina/priority/EventStatePriority.cpp
	src/stamina/priority/StatePriority.cpp
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

# First, create a library with all of the source files in LIB_FILES
# Add executable target with source files listed in SOURCE_FILES variable
add_library(${LIB_NAME} SHARED ${LIB_FILES})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${LIB_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})

target_link_libraries(${LIB_NAME} PUBLIC storm storm-parsers)
target_link_libraries(${PROJECT_NAME} PUBLIC stamina storm storm-parsers)

#set (GUI_PREFIX src/stamina/gui)

#include(${GUI_PREFIX}/CMakeLists.txt)
