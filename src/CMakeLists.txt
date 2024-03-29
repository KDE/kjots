
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

set(kjots_config_SRCS kjotsconfigdlg.cpp kjotsconfigdlg.h)
kconfig_add_kcfg_files(kjots_config_SRCS KJotsSettings.kcfgc )
ki18n_wrap_ui(kjots_config_SRCS confpagemisc.ui)

set(kjots_common_SRCS
  aboutdata.cpp
  kjotsedit.cpp
  kjotsbookmarks.cpp
  kjotsmodel.cpp
  kjotswidget.cpp
  kjotsbrowser.cpp
  kjotslinkdialog.cpp
  notesortproxymodel.cpp
  uistatesaver.cpp
  aboutdata.h
  kjotsedit.h
  kjotsbookmarks.h
  kjotsmodel.h
  kjotswidget.h
  kjotsbrowser.h
  kjotslinkdialog.h
  notesortproxymodel.h
  uistatesaver.h
  ${kjots_config_SRCS}
)
ki18n_wrap_ui(kjots_common_SRCS linkdialog.ui)

add_library(kjots_common STATIC ${kjots_common_SRCS})
set_property(TARGET kjots_common PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kjots_common
    KF${KF_MAJOR_VERSION}::KCMUtils

  noteshared
  Qt::DBus
  Qt::PrintSupport
  KF${KF_MAJOR_VERSION}::Bookmarks
  KF${KF_MAJOR_VERSION}::TextWidgets
  KPim${KF_MAJOR_VERSION}::Mime
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::AkonadiWidgets
  KPim${KF_MAJOR_VERSION}::PimTextEdit
  KF${KF_MAJOR_VERSION}::XmlGui
  KF${KF_MAJOR_VERSION}::KIOWidgets
  ${MAIN_LIB}
)
if (TARGET KF${KF_MAJOR_VERSION}::TextEditTextToSpeech)
    target_link_libraries(kjots_common KF${KF_MAJOR_VERSION}::TextEditTextToSpeech)
endif()
if (TARGET KF${KF_MAJOR_VERSION}::TextAddonsWidgets)
    target_link_libraries(kjots_common KF${KF_MAJOR_VERSION}::TextAddonsWidgets)
endif()
if (TARGET KF${KF_MAJOR_VERSION}::TextCustomEditor)
    target_link_libraries(kjots_common KF${KF_MAJOR_VERSION}::TextCustomEditor)
endif()
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(kjots_common KF6::BookmarksWidgets)
endif()

#########################################################################
# D-Bus interface
#########################################################################

qt_generate_dbus_interface( kjotswidget.h org.kde.KJotsWidget.xml OPTIONS -m )
qt_add_dbus_interfaces(kjots_common_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KJotsWidget.xml)


#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   main.cpp
   KJotsMain.cpp
   KJotsMain.h
   kjots.qrc
)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/icons/*-apps-kjots.png")
ecm_add_app_icon(kjots_SRCS ICONS ${ICONS_SRCS})

add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots
  kjots_common
  KPim${KF_MAJOR_VERSION}::KontactInterface
)

install(TARGETS kjots ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kjots.kcfg
        DESTINATION ${KDE_INSTALL_KCFGDIR}
)

############## next target ##############

set(kjotspart_PART_SRCS
  kjotspart.cpp
  kjotspart.h
)

add_library(kjotspart MODULE ${kjotspart_PART_SRCS})

target_link_libraries(kjotspart
  kjots_common
  KF${KF_MAJOR_VERSION}::Parts
)

install(TARGETS kjotspart
        DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

###########################################################################

set(kcm_kjots_PART_SRCS
    kcm_kjots.cpp
    ${kjots_config_SRCS}
)

add_library(kcm_kjots MODULE ${kcm_kjots_PART_SRCS})

target_link_libraries(kcm_kjots
    KF${KF_MAJOR_VERSION}::KCMUtils
    KF${KF_MAJOR_VERSION}::I18n
)

install(TARGETS kcm_kjots
        DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
