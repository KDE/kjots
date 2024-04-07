include_directories(${kjots_SOURCE_DIR}/src)

set(kontact_kjots_PART_SRCS
    kjots_plugin.cpp
    kjots_plugin.h
)

add_library(kontact_kjotsplugin MODULE ${kontact_kjots_PART_SRCS})
target_link_libraries(kontact_kjotsplugin KF6::I18n KPim6::KontactInterface)

install(TARGETS kontact_kjotsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kontact)
