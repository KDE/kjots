set(noteshared_SRCS
    notecreatorandselector.cpp notecreatorandselector.h
    notelockattribute.cpp notelockattribute.h
    notepinattribute.cpp notepinattribute.h
    standardnoteactionmanager.cpp standardnoteactionmanager.h
)

ecm_qt_declare_logging_category(noteshared_SRCS
    HEADER noteshared_debug.h
    IDENTIFIER NOTESHARED_LOG
    CATEGORY_NAME log_noteshared
)

add_library(noteshared STATIC ${noteshared_SRCS})
set_property(TARGET noteshared PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(noteshared
    Qt::Core
    Qt::Widgets
    KF${KF_MAJOR_VERSION}::I18n
    KF5::AkonadiWidgets
    KF5::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiNotes
    KF${KF_MAJOR_VERSION}::XmlGui
)

target_include_directories(noteshared PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
