cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(KJOTS_VERSION "6.0.0")

project(kjots VERSION ${KJOTS_VERSION})

set(QT_REQUIRED_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")
set(PIM_VERSION "6.0.0")
set(KTEXTADDONS_MIN_VERSION "1.5.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

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

set(AKONADINOTES_LIB_VERSION ${PIM_VERSION})
set(KDEPIMLIBS_LIB_VERSION ${PIM_VERSION})
set(KMIME_LIB_VERSION ${PIM_VERSION})
set(KPIMTEXTEDIT_LIB_VERSION ${PIM_VERSION})
set(KONTACTINTERFACE_LIB_VERSION ${PIM_VERSION})

# Qt Packages
find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus PrintSupport)

# KF6 Packages
find_package(KF6KCMUtils ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Config ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6I18n ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Parts ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Bookmarks ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextWidgets ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextTemplate ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextCustomEditor ${KTEXTADDONS_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextAddonsWidgets ${KTEXTADDONS_MIN_VERSION} CONFIG REQUIRED)

# PIM packages
find_package(KPim6Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6TextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)


add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.6.0 KF 5.248.0)

find_package(KF6TextEditTextToSpeech ${KTEXTADDONS_MIN_VERSION} CONFIG)
set_package_properties(KF6TextEditTextToSpeech PROPERTIES DESCRIPTION
    "Add support for text to speech"
    TYPE OPTIONAL
)
if (TARGET KF6::TextEditTextToSpeech)
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
