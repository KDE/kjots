include_directories(${kjots_SOURCE_DIR}/src)

set(kontact_kjots_PART_SRCS
    kjots_plugin.cpp
)

add_library(kontact_kjotsplugin MODULE ${kontact_kjots_PART_SRCS})
target_link_libraries(kontact_kjotsplugin KF5::KontactInterface)

install(TARGETS kontact_kjotsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kjotsplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
install(FILES kjots.setdlg DESTINATION ${KDE_INSTALL_DATADIR}/kontact/ksettingsdialog)

#qt5_add_dbus_interfaces(kontact_akregator_PART_SRCS ${akregator_SOURCE_DIR}/src/org.kde.akregator.part.xml)
