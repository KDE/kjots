include_directories(${kjots_SOURCE_DIR}/src)

# CMake 3.9+ warns about automoc on files without Q_OBJECT, and doesn't know about other macros.
# 3.10+ lets us provide more macro names that require automoc.
# KF5 >= 5.42 takes care itself of adding its macros in its cmake config files
list(APPEND CMAKE_AUTOMOC_MACRO_NAMES "EXPORT_KONTACT_PLUGIN_WITH_JSON")

set(kontact_kjots_PART_SRCS
    kjots_plugin.cpp
    kjots_plugin.h
)

add_library(kontact_kjotsplugin MODULE ${kontact_kjots_PART_SRCS})
target_link_libraries(kontact_kjotsplugin KF${KF_MAJOR_VERSION}::I18n KF5::KontactInterface)

install(TARGETS kontact_kjotsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)

#qt_add_dbus_interfaces(kontact_akregator_PART_SRCS ${akregator_SOURCE_DIR}/src/org.kde.akregator.part.xml)
