
#ecm_install_icons( ICONS hi128-apps-kjots.png  hi16-apps-kjots.png  hi22-apps-kjots.png  hi32-apps-kjots.png  hi48-apps-kjots.png  hi64-apps-kjots.png  ox16-actions-edit-delete-page.png  ox22-actions-edit-delete-page.png  ox32-actions-edit-delete-page.png  ox48-actions-edit-delete-page.png DESTINATION ${KDE_INSTALL_ICONDIR}   )

file(GLOB app_icons *-apps-kjots.*)
ecm_install_icons(ICONS ${app_icons}
                  DESTINATION ${KDE_INSTALL_ICONDIR}
                  THEME hicolor
)
