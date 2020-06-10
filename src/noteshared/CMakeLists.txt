set(noteshared_SRCS
    notecreatorandselector.cpp
    notelockattribute.cpp
    notepinattribute.cpp
    standardnoteactionmanager.cpp
)

ecm_qt_declare_logging_category(noteshared_SRCS
    HEADER noteshared_debug.h
    IDENTIFIER NOTESHARED_LOG
    CATEGORY_NAME log_noteshared
)

add_library(noteshared STATIC ${noteshared_SRCS})
target_link_libraries(noteshared
    Qt5::Core
    Qt5::Widgets
    KF5::I18n
    KF5::AkonadiCore
    KF5::AkonadiNotes
    KF5::XmlGui
)

target_include_directories(noteshared PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
