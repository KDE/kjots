project(kjots)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 5.16.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMAddTests)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)

set(KJOTS_VERSION "4.96.0")

set(KF5_VERSION "5.16.0")
set(KDEPIMLIBS_LIB_VERSION "5.1.40")
set(KMIME_LIB_VERSION "4.87.0")
set(PIMTEXTEDIT_LIB_VERSION "5.1.40")
set(KONTACTINTERFACE_LIB_VERSION "4.82.0")

# KF5 Packages
find_package(KF5KCMUtils ${KF5_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_LIB_VERSION} CONFIG REQUIRED)

# PIM packages
find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)

find_package(Grantlee5 "5.0" CONFIG REQUIRED)


##################### Targets #######################

add_subdirectory(src)
add_subdirectory(icons)

##################### Install ######################

install(DIRECTORY themes
        DESTINATION ${DATA_INSTALL_DIR}/kjots
        PATTERN *.svn EXCLUDE
)

install(FILES data/Kjots.appdata.xml
	DESTINATION ${CMAKE_INSTALL_METAINFODIR}
)
install(FILES data/org.kde.kjots.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR}
)
