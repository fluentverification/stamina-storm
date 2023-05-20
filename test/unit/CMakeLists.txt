## Cmake for unit tests

cmake_minimum_required(VERSION 3.10)

project(unit_sstamina)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_DIR .)
set(SOURCE_FILES unit.cpp)

set(LOCAL_PATH /usr/lib)
set(LIB_PATH ${STORM_PATH}/lib)
set(CUDD_PATH ${STORM_PATH}/storm/resources/3rdparty/cudd-3.0.0/cplusplus)

find_package(Boost)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	include_directories(${Boost_INCLUDES})
endif (Boost_FOUND)

find_package(storm REQUIRED PATHS ${STORM_PATH})
find_package(sstamina REQUIRED PATHS ${STAMINA_PATH})

add_executable(unit ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${STAMINA_PATH} ${LIB_PATH})
target_link_libraries(${PROJECT_NAME} PUBLIC storm storm-parsers sstamina)
