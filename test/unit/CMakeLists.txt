## Cmake for unit tests
## This file is intended to be included in the main CMakeLists
add_compile_definitions(STAMINA_UNIT_TESTS)
set(TESTS_SOURCE_FILES test/unit/unit.cpp)
link_directories (unit ${Boost_LIBRARY_DIRS} )
add_executable(unit ${TESTS_SOURCE_FILES})
target_include_directories(unit PUBLIC ${Boost_LIBRARIES} ${SOURCE_DIR} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})
target_link_libraries(unit PUBLIC storm storm-parsers stamina)
