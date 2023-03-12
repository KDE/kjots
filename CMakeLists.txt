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

set(KJOTS_VERSION "5.1.1")

set(KF_VERSION "5.92.0")
set(AKONADINOTES_LIB_VERSION "5.22.80")
set(KDEPIMLIBS_LIB_VERSION "5.22.80")
set(KMIME_LIB_VERSION "5.22.80")
set(KPIMTEXTEDIT_LIB_VERSION "5.22.80")
set(KONTACTINTERFACE_LIB_VERSION "5.22.80")

#Qt Packages
set(QT_REQUIRED_VERSION "5.15.2")
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
    set(KTEXTADDONS_MIN_VERSION "1.1.0")
else()
    set(KF_MIN_VERSION "5.100.0")
    set(KF_MAJOR_VERSION "5")
    set(KTEXTADDONS_MIN_VERSION "1.0.2")
endif()
find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus PrintSupport)

# KF5 Packages
find_package(KF${KF_MAJOR_VERSION}KCMUtils ${KF_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}KIO ${KF_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Config ${KF_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}ConfigWidgets ${KF_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Parts ${KF_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Bookmarks ${KF_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}XmlGui ${KF_VERSION} CONFIG REQUIRED)

# PIM packages
find_package(KPim${KF_MAJOR_VERSION}Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}TextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
set(GRANTLEE_VERSION "5.3")
if (QT_MAJOR_VERSION STREQUAL "5")
    find_package(Grantlee5 ${GRANTLEE_VERSION} CONFIG REQUIRED)
    set(MAIN_LIB Grantlee5::Templates Grantlee5::TextDocument)
else()
    find_package(KF6TextTemplate CONFIG REQUIRED)
    set(MAIN_LIB KF6::TextTemplate)
endif()

ecm_set_disabled_deprecation_versions(QT 6.4  KF 5.104.0)

find_package(KF${KF_MAJOR_VERSION}TextEditTextToSpeech ${KTEXTADDONS_MIN_VERSION} CONFIG)
set_package_properties(KF${KF_MAJOR_VERSION}TextEditTextToSpeech PROPERTIES DESCRIPTION
    "Add support for text to speech"
    TYPE OPTIONAL
)
if (TARGET KF${KF_MAJOR_VERSION}::TextEditTextToSpeech)
    add_definitions(-DHAVE_TEXT_TO_SPEECH_SUPPORT)
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
)

install(FILES data/org.kde.kjots.appdata.xml
	DESTINATION ${KDE_INSTALL_METAINFODIR}
)
install(FILES data/org.kde.kjots.desktop
        DESTINATION ${KDE_INSTALL_APPDIR}
)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
