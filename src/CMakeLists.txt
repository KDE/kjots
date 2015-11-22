
# noteshared contains copy of some classes from kdepim/noteshared. This
# is at least until the noteshared library is split out from kdepim
# repository and we can link against it
add_subdirectory(noteshared)
# akonadi_next is dead in kdepim, those are two utility classes salvaged from
# it that were used by KJots. We could eventually move this to Akonadi Note
add_subdirectory(akonadi_next)

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
  kjotslockjob.cpp
  kjotsedit.cpp
  kjotstreeview.cpp
  kjotsbookmarks.cpp
  kjotsreplacenextdialog.cpp
  kjotsmodel.cpp
  kjotssortproxymodel.cpp
  kjotswidget.cpp
  kjotsbrowser.cpp
  kjotslinkdialog.cpp
  localresourcecreator.cpp
  kjotsbookshelfentryvalidator.cpp
  knowitimporter.cpp
  ${kjots_config_SRCS}
)

add_library(kjots_common STATIC ${kjots_common_SRCS})
target_link_libraries(kjots_common
  akonadi_next
  noteshared
  Qt5::PrintSupport
  KF5::KCMUtils
  KF5::Bookmarks
  KF5::ConfigWidgets
  KF5::KDELibs4Support
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiWidgets
  KF5::AkonadiMime
  KF5::PimTextEdit
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
kde4_add_app_icon(kjots_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-kjots.png")
# TODO: ECM_ADD_APP_ICON?

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
install(FILES kjotspartui.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kjots
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

