project (xstamina)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(xstamina_SRCS
	main.cpp
	MainWindow.cpp
	About.cpp
	Preferences.cpp
	addons/CodeEditor.cpp
	addons/LineNumberArea.cpp
	# Syntax Highlighting
	addons/highlighter/Highlighter.cpp
	addons/highlighter/PrismHighlighter.cpp
)

set(xstamina_RECS
	resources/resources.qrc
)

add_executable(xstamina ${xstamina_SRCS})

target_link_libraries(xstamina
	Qt5::Widgets
	KF5::CoreAddons
	KF5::I18n
	KF5::XmlGui
	KF5::TextWidgets
)

target_link_libraries(xstamina KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM)

target_link_libraries(xstamina KF5::TextEditor)

install(TARGETS xstamina  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mainWindow.ui DESTINATION ${KXMLGUI_INSTALL_DIR}/xstamina)
