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
    KF6::I18n
    KPim6::AkonadiWidgets
    KPim6::AkonadiCore
    KPim6::AkonadiNotes
    KF6::XmlGui
)

target_include_directories(noteshared PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
