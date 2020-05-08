set(noteshared_SRCS
    notecreatorandselector.cpp
    notelockattribute.cpp
    noteeditorutils.cpp
    localresourcecreator.cpp
)

ecm_qt_declare_logging_category(noteshared_SRCS
    HEADER noteshared_debug.h
    IDENTIFIER NOTESHARED_LOG
    CATEGORY_NAME log_noteshared
)

macro(add_resource_iface _kcfgFile _ifaceName _className)
    kcfg_generate_dbus_interface(${_kcfgFile} ${_ifaceName})
    string(TOLOWER ${_className} _codeFile)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES
                                INCLUDE "metatype.h")
    qt5_add_dbus_interface(noteshared_SRCS
        ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
    )
endmacro()

add_resource_iface(${CMAKE_CURRENT_SOURCE_DIR}/maildirresource.kcfg
                   org.kde.Akonadi.Maildir.Settings MaildirSettings )

add_custom_target(noteshared_settings_xml ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/maildirresource.kcfg
)

add_library(noteshared STATIC ${noteshared_SRCS})
add_dependencies(noteshared noteshared_settings_xml)
target_link_libraries(noteshared
    Qt5::Core
    Qt5::Widgets
    KF5::I18n
    KF5::AkonadiCore
    KF5::AkonadiNotes
)

target_include_directories(noteshared PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
