set(akonadi_next_SRCS
    note.cpp
    notecreatorandselector.cpp
)

ecm_qt_declare_logging_category(akonadi_next_SRCS
    HEADER akonadinext_debug.h
    IDENTIFIER AKONADINEXT_LOG
    CATEGORY_NAME log_akonadinext
)


add_library(akonadi_next STATIC ${akonadi_next_SRCS})
target_link_libraries(akonadi_next
    Qt5::Core
    KF5::AkonadiCore
    KF5::Mime
    KF5::I18n
)
