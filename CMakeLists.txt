cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(kjots)

find_package(ECM 5.92.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
set(CMAKE_CXX_STANDARD 17)
include(ECMAddTests)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMAddAppIcon)
include(ECMDeprecationSettings)

set(KJOTS_VERSION "5.1.0")

set(KF5_VERSION "5.92.0")
set(AKONADINOTES_LIB_VERSION "5.22.0")
set(KDEPIMLIBS_LIB_VERSION "5.22.0")
set(KMIME_LIB_VERSION "5.22.0")
set(KPIMTEXTEDIT_LIB_VERSION "5.22.0")
set(KONTACTINTERFACE_LIB_VERSION "5.22.0")

#Qt Packages
set(QT_REQUIRED_VERSION "5.15.0")
find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus PrintSupport)

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
set(GRANTLEE_VERSION "5.3")
if (QT_MAJOR_VERSION STREQUAL "5")
    find_package(Grantlee5 ${GRANTLEE_VERSION} CONFIG REQUIRED)
    set(MAIN_LIB Grantlee5::Templates Grantlee5::TextDocument)
else()
    find_package(KF6TextTemplate CONFIG REQUIRED)
    set(MAIN_LIB KF6::TextTemplate)
endif()

ecm_set_disabled_deprecation_versions(QT 5.15.2  KF 5.102.0)

option(USE_KTEXTADDONS_LIB "Use ktextaddons lib (experimental)" OFF)
set(KTEXTADDONS_MIN_VERSION "1.0.0")
if (USE_KTEXTADDONS_LIB)
    find_package(KF5TextEditTextToSpeech ${KTEXTADDONS_MIN_VERSION} CONFIG REQUIRED)
    add_definitions(-DHAVE_TEXT_TO_SPEECH_SUPPORT)
else()
    find_package(KF5PimTextEditTextToSpeech ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
endif()

##################### Definitions #####################
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_NO_CAST_FROM_ASCII)

##################### Targets #######################

add_subdirectory(src)
add_subdirectory(icons)

##################### Translations ##################

ki18n_install(po)

##################### Install ######################

install(DIRECTORY themes
        DESTINATION ${KDE_INSTALL_DATADIR}/kjots
        PATTERN *.svn EXCLUDE
)

install(FILES data/org.kde.kjots.appdata.xml
	DESTINATION ${KDE_INSTALL_METAINFODIR}
)
install(FILES data/org.kde.kjots.desktop
        DESTINATION ${KDE_INSTALL_APPDIR}
)
