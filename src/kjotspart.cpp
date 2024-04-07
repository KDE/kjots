/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kjotspart.h"
#include "aboutdata.h"
#include "kjotswidget.h"

#include <QIcon>
#include <QAction>

#include <KActionCollection>
#include <KPluginFactory>
#include <KLocalizedString>


K_PLUGIN_CLASS_WITH_JSON(KJotsPart, "kjotspart.json")

KJotsPart::KJotsPart(QWidget *parentWidget, QObject *parent, const KPluginMetaData &data, const QVariantList &)
    : KParts::ReadOnlyPart(parent, data)
{
    // this should be your custom internal widget
    mComponent = new KJotsWidget(parentWidget, this);

    // notify the part that this is our internal widget
    setWidget(mComponent);
    initAction();

    // set our XML-UI resource file
    setComponentName(QStringLiteral("kjots"), i18n("KJots"));
    setXMLFile(QStringLiteral("kjotsui.rc"));

    connect(mComponent, &KJotsWidget::captionChanged, this, &KJotsPart::setWindowCaption);
}

KJotsPart::~KJotsPart()
{
    mComponent->queryClose();
}

void KJotsPart::initAction()
{
    auto action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("&Configure KJots..."), this);
    actionCollection()->addAction(QStringLiteral("kjots_configure"), action);
    connect(action, &QAction::triggered, mComponent, &KJotsWidget::configure);
}

bool KJotsPart::openFile()
{
    return false;
}

#include "kjotspart.moc"

#include "moc_kjotspart.cpp"
