option(BUILD_CPACK "Creates/includes CPack file" Off)
if (BUILD_CPACK)
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "2")
	set(CPACK_PACKAGE_VERSION_PATCH "5")
	set(CPACK_PACKAGE_DESCRIPTION "STAMINA - the STochastic Approximate Model-checker for INfinite-state Analysis")
	set(CPACK_PACKAGE_CONTACT "Josh Jeppson <joshua.jeppson@usu.edu>")
	set(CPACK_PACKAGE_VENDOR "FLUENT Verification, Utah State University")
	set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
	set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/doc/staminaLogo.svg)
	set(CPACK_PACKAGE_HOMEPAGE_URL "https://staminachecker.org")

	# Debian packaging information
	include(InstallRequiredSystemLibraries)
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "breeze-icon-theme,libkf5xmlgui-dev,libkf5textwidgets-dev,libkf5kio-dev,libkf5texteditor-dev,qtbase5-dev,qtdeclarative5-dev,libqt5svg5-dev,libkf5i18n-dev,libkf5coreaddons-dev,extra-cmake-modules,libboost-all-dev,libcln-dev,libgmp-dev,libginac-dev,automake,libglpk-dev,libhwloc-dev,libz3-dev,libxerces-c-dev,libeigen3-dev")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})

	include(CPack)
endif()
