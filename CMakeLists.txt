project(kjots)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 5.16.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
set(CMAKE_CXX_STANDARD 14)

include(ECMAddTests)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMAddAppIcon)

set(KJOTS_VERSION "5.1.0")

set(KF5_VERSION "5.71.0")
set(AKONADINOTES_LIB_VERSION "5.14.41")
set(KDEPIMLIBS_LIB_VERSION "5.1.0")
set(KMIME_LIB_VERSION "4.87.0")
set(KPIMTEXTEDIT_LIB_VERSION "5.14.44")
set(KONTACTINTERFACE_LIB_VERSION "4.82.0")

#Qt Packages
set(QT_REQUIRED_VERSION "5.14.0")
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus PrintSupport)

# KF5 Packages
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5ConfigWidgets ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Parts ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Bookmarks ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)

# PIM packages
find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)

find_package(Grantlee5 "5.0" CONFIG REQUIRED)

##################### Definitions #####################
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_NO_CAST_FROM_ASCII)

##################### Targets #######################

add_subdirectory(src)
add_subdirectory(icons)

##################### Install ######################

install(DIRECTORY themes
        DESTINATION ${DATA_INSTALL_DIR}/kjots
        PATTERN *.svn EXCLUDE
)

install(FILES data/org.kde.kjots.appdata.xml
	DESTINATION ${CMAKE_INSTALL_METAINFODIR}
)
install(FILES data/org.kde.kjots.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR}
)
