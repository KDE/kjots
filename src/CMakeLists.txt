
add_subdirectory(noteshared)

add_subdirectory(kontact_plugin)

add_definitions(-DTRANSLATION_DOMAIN=\"kjots\")

configure_file(kjots-version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/kjots-version.h
               @ONLY
)

#########################################################################
# Code common to the kjots application and the kontact plugin
# Don't make it a static library, this code needs to be compiled
# with -fPIC for the part and without -fPIC for the executable
#########################################################################

set(kjots_config_SRCS kjotsconfigdlg.cpp)
kconfig_add_kcfg_files(kjots_config_SRCS KJotsSettings.kcfgc )
ki18n_wrap_ui(kjots_config_SRCS confpagemisc.ui)

set(kjots_common_SRCS
  aboutdata.cpp
  kjotsedit.cpp
  kjotsbookmarks.cpp
  kjotsmodel.cpp
  kjotssortproxymodel.cpp
  kjotswidget.cpp
  kjotsbrowser.cpp
  kjotslinkdialog.cpp
  kjotsbookshelfentryvalidator.cpp
  ${kjots_config_SRCS}
)

add_library(kjots_common STATIC ${kjots_common_SRCS})
target_link_libraries(kjots_common
  noteshared
  Qt5::DBus
  Qt5::PrintSupport
  KF5::KCMUtils
  KF5::Bookmarks
  KF5::ConfigWidgets
  KF5::TextWidgets
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiWidgets
  KF5::PimTextEdit
  KF5::XmlGui
  KF5::KIOWidgets
  Grantlee5::Templates
  Grantlee5::TextDocument
)

#########################################################################
# D-Bus interface
#########################################################################

qt5_generate_dbus_interface( kjotswidget.h org.kde.KJotsWidget.xml OPTIONS -m )
qt5_add_dbus_interfaces(kjots_common_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KJotsWidget.xml)


#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   main.cpp
   KJotsMain.cpp
)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/icons/*-apps-kjots.png")
ecm_add_app_icon(kjots_SRCS ICONS ${ICONS_SRCS})

add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots
  kjots_common
  KF5::KontactInterface
)

install(TARGETS kjots ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kjots.kcfg
        DESTINATION ${KCFG_INSTALL_DIR}
)
install(FILES kjotsui.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kjots
)

############## next target ##############

set(kjotspart_PART_SRCS
  kjotspart.cpp
)

add_library(kjotspart MODULE ${kjotspart_PART_SRCS})

target_link_libraries(kjotspart
  kjots_common
  KF5::Parts
)

install(FILES ${CMAKE_SOURCE_DIR}/data/kjotspart.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)
install(TARGETS kjotspart
        DESTINATION ${PLUGIN_INSTALL_DIR}
)

###########################################################################

set(kcm_kjots_PART_SRCS
    kcm_kjots.cpp
    ${kjots_config_SRCS}
)

add_library(kcm_kjots MODULE ${kcm_kjots_PART_SRCS})

target_link_libraries(kcm_kjots
    KF5::KCMUtils
    KF5::I18n
)

install(TARGETS kcm_kjots
        DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES ${CMAKE_SOURCE_DIR}/data/kjots_config_misc.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)

