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

set (GUI_PREFIX src/stamina/gui)

#include(${GUI_PREFIX}/CMakeLists.txt)

#project (xstamina)

#cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
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

#feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(xstamina_SRCS
	${GUI_PREFIX}/main.cpp
	${GUI_PREFIX}/MainWindow.cpp
	${GUI_PREFIX}/About.cpp
	${GUI_PREFIX}/Preferences.cpp
	${GUI_PREFIX}/PropertyWizard.cpp
	# Code editing
	${GUI_PREFIX}/addons/CodeEditor.cpp
	${GUI_PREFIX}/addons/LineNumberArea.cpp
	# Syntax Highlighting
	${GUI_PREFIX}/addons/highlighter/Highlighter.cpp
	${GUI_PREFIX}/addons/highlighter/PrismHighlighter.cpp
)

set(xstamina_RECS
	${GUI_PREFIX}/resources/resources.qrc
)

add_executable(xstamina ${xstamina_SRCS})

target_link_libraries(xstamina PRIVATE
	Qt5::Widgets
	KF5::CoreAddons
	KF5::I18n
	KF5::XmlGui
	KF5::TextWidgets
)

target_link_libraries(xstamina PRIVATE KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM)
target_link_libraries(xstamina PRIVATE KF5::TextEditor)
target_link_libraries(xstamina PUBLIC stamina storm storm-parsers)

set(STAMINA_PATH TODO)
message("STAMINA_PATH is set as " ${STAMINA_PATH})
# find_package(stamina REQUIRED PATHS ${STAMINA_PATH})
target_include_directories(xstamina PUBLIC PUBLIC ${SOURCE_DIR} ${GUI_PREFIX} ${storm_INCLUDE_DIR} ${storm-parsers_INCLUDE_DIR} ${STORM_PATH} ${LIB_PATH})

