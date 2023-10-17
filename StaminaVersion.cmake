set(STAMINA_VERSION_MAJOR "0")
set(STAMINA_VERSION_MINOR "2")
set(STAMINA_VERSION_PATCH "5")

# STAMINA Build
# option(STAMINA_BUILD_TAG "Build tag information" "unspecified")
option(STAMINA_INCLUDE_GIT_BUILD_INFO "Includes git tag information in build info" Off)
if (STAMINA_INCLUDE_GIT_BUILD_INFO)
	execute_process(
		COMMAND git rev-parse HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		OUTPUT_VARIABLE GIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	add_compile_definitions(STAMINA_BUILD_INFO="commit-${GIT_HASH}")
elseif(STAMINA_DEBUG) 
	add_compile_definitions(STAMINA_BUILD_INFO="debug-build")
else()
	add_compile_definitions(STAMINA_BUILD_INFO="release-build")
endif()

add_compile_definitions(STAMINA_VERSION_MAJOR=${STAMINA_VERSION_MAJOR})
add_compile_definitions(STAMINA_VERSION_MINOR=${STAMINA_VERSION_MINOR})
add_compile_definitions(STAMINA_VERSION_PATCH=${STAMINA_VERSION_PATCH})
