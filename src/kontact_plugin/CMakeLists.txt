include_directories(${kjots_SOURCE_DIR}/src)

set(kontact_kjots_PART_SRCS
    kjots_plugin.cpp
    kjots_plugin.h
)

add_library(kontact_kjotsplugin MODULE ${kontact_kjots_PART_SRCS})
target_link_libraries(kontact_kjotsplugin KF${KF_MAJOR_VERSION}::I18n KPim${KF_MAJOR_VERSION}::KontactInterface)

install(TARGETS kontact_kjotsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)
