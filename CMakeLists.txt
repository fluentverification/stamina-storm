##
## CMakeLists for stamina
## Requires C++14 or higher
## Requires STORM and boost
##

cmake_minimum_required(VERSION 3.10)
project(sstamina)			   # Create project "sstamina"
set(LIB_NAME stamina)
set(CMAKE_CXX_STANDARD 17)			# Enable c++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(STAMINA_DEBUG "Include debug information in the executables" OFF)
if (STAMINA_DEBUG)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# STAMINA Build
# option(STAMINA_BUILD_TAG "Build tag information" "unspecified")
execute_process(
	COMMAND git rev-parse HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	OUTPUT_VARIABLE GIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(STAMINA_BUILD_INFO="Debug Build")

# Build options
option(BUILD_GUI "Build XStamina GUI" OFF)

# These build options are for the GUI only
if (BUILD_GUI)
	set(QT_MIN_VERSION "5.3.0")
	set(KF5_MIN_VERSION "5.2.0")

	find_package(ECM 1.0.0 REQUIRED NO_MODULE)
	set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

	add_compile_definitions(STAMINA_HAS_GUI)

	include(KDEInstallDirs)
	include(KDECMakeSettings)
	#include(KDECompilerSettings)
	include(FeatureSummary)

	find_package(KF5TextEditor)

	find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
		Core	# QCommandLineParser, QStringLiteral
		Widgets # QApplication
	)

	find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
		CoreAddons	  # KAboutData
		I18n			# KLocalizedString
		XmlGui		  # KXmlGuiWindow
		TextWidgets	 # KTextEdit
	)
	find_package(KF5KIO)

	feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
	set(PICO_ENABLE_CXX_EXCEPTIONS 1)
endif()

# Main STAMINA executable
include_directories("src")

# Source files for the main STAMINA CLI
set(SOURCE_DIR src)
set(STAMINA_NAMESPACE_DIR ${SOURCE_DIR}/stamina)
include_directories("${STAMINA_NAMESPACE_DIR}")
set(SOURCE_FILES
	# Main source files for the `stamina` namespace
	${STAMINA_NAMESPACE_DIR}/main.cpp
)

set(LIB_FILES
	${STAMINA_NAMESPACE_DIR}/Stamina.cpp
	# Files for `stamina::core` namespace
	${STAMINA_NAMESPACE_DIR}/core/StaminaModelChecker.cpp
	${STAMINA_NAMESPACE_DIR}/core/StaminaMessages.cpp
	${STAMINA_NAMESPACE_DIR}/core/Options.cpp
	${STAMINA_NAMESPACE_DIR}/core/StateSpaceInformation.cpp
	# Files for `stamina::util` namespace
	${STAMINA_NAMESPACE_DIR}/util/ModelModify.cpp
	${STAMINA_NAMESPACE_DIR}/util/StateIndexArray.cpp
	${STAMINA_NAMESPACE_DIR}/util/StateMemoryPool.cpp
	# Files for `stamina::builder` namespace
	${STAMINA_NAMESPACE_DIR}/builder/StaminaModelBuilder.cpp
	${STAMINA_NAMESPACE_DIR}/builder/StaminaIterativeModelBuilder.cpp
	${STAMINA_NAMESPACE_DIR}/builder/StaminaThreadedIterativeModelBuilder.cpp
	${STAMINA_NAMESPACE_DIR}/builder/StaminaPriorityModelBuilder.cpp
	${STAMINA_NAMESPACE_DIR}/builder/StaminaReExploringModelBuilder.cpp
	${STAMINA_NAMESPACE_DIR}/builder/ExplicitTruncatedModelBuilder.cpp
	# Files for `stamina::builder::threads` namespace
	${STAMINA_NAMESPACE_DIR}/builder/threads/BaseThread.cpp
	${STAMINA_NAMESPACE_DIR}/builder/threads/ControlThread.cpp
	${STAMINA_NAMESPACE_DIR}/builder/threads/ExplorationThread.cpp
	${STAMINA_NAMESPACE_DIR}/builder/threads/IterativeExplorationThread.cpp
	# Files for `stamina::priority` namespace
	${STAMINA_NAMESPACE_DIR}/priority/EventStatePriority.cpp
	${STAMINA_NAMESPACE_DIR}/priority/StatePriority.cpp
)

set(GUI_PREFIX ${STAMINA_NAMESPACE_DIR}/gui)

set(GUI_FILES
	# Main GUI file is (currently) in the STAMINA_NAMESPACE_DIR
	${STAMINA_NAMESPACE_DIR}/gui.cpp
	# The rest of these are not
	${GUI_PREFIX}/MainWindow.cpp
	${GUI_PREFIX}/About.cpp
	${GUI_PREFIX}/Preferences.cpp
	${GUI_PREFIX}/PropertyWizard.cpp
	${GUI_PREFIX}/FindReplace.cpp
	# Message Bridge
	${GUI_PREFIX}/MessageBridge.cpp
	# Code editing
	${GUI_PREFIX}/addons/CodeEditor.cpp
	${GUI_PREFIX}/addons/LineNumberArea.cpp
	# Syntax Highlighting
	${GUI_PREFIX}/addons/highlighter/Highlighter.cpp
	${GUI_PREFIX}/addons/highlighter/PrismHighlighter.cpp
)

set(GUI_RECS
	${GUI_PREFIX}/resources.qrc
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

# Stuff for the GUI
if(BUILD_GUI)

	add_compile_definitions(STAMINA_HAS_GUI)

	set(GUI_EXECUTABLE_NAME xstamina)
	add_executable(${GUI_EXECUTABLE_NAME} ${GUI_FILES} ${GUI_RECS})
	target_link_libraries(${GUI_EXECUTABLE_NAME} PUBLIC
		Qt5::Widgets
		KF5::CoreAddons
		KF5::I18n
		KF5::XmlGui
		KF5::TextWidgets
		KF5::KIOCore
		KF5::KIOFileWidgets
		KF5::KIOWidgets
		KF5::KIONTLM
		KF5::TextEditor
	)

	target_link_libraries(${GUI_EXECUTABLE_NAME} PUBLIC stamina storm storm-parsers)

endif()

# Install stuff
install(TARGETS ${GUI_EXECUTABLE_NAME}
	CONFIGURATIONS Debug
	RUNTIME DESTINATION Debug/bin)
install(TARGETS ${GUI_EXECUTABLE_NAME}
	CONFIGURATIONS Release
	RUNTIME DESTINATION Release/bin)

option(BUILD_CPACK "Creates/includes CPack file" Off)
if (BUILD_CPACK)
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "2")
	set(CPACK_PACKAGE_VERSION_PATCH "5")
	set(CPACK_PACKAGE_DESCRIPTION "STAMINA - the STochastic Approximate Model-checker for INfinite-state Analysis")
	set(CPACK_PACKAGE_CONTACT "Josh Jeppson <joshua.jeppson@usu.edu>")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libkf5xmlgui-dev,libkf5textwidgets-dev,libkf5kio-dev,libkf5texteditor-dev,qtbase5-dev,qtdeclarative5-dev,libqt5svg5-dev,libkf5i18n-dev,libkf5coreaddons-dev,extra-cmake-modules,libboost-all-dev,libcln-dev,libgmp-dev,libginac-dev,automake,libglpk-dev,libhwloc-dev,libz3-dev,libxerces-c-dev,libeigen3-dev")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})

	include(CPack)
endif()
