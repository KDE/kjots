/*
    This file is part of KJots.

    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kjotspart.h"
#include "aboutdata.h"
#include "kjotswidget.h"

#include <QIcon>
#include <QAction>

#include <KActionCollection>
#include <KPluginFactory>
#include <KLocalizedString>


K_PLUGIN_FACTORY(KJotsPartFactory, registerPlugin<KJotsPart>();)

KJotsPart::KJotsPart(QWidget *parentWidget, QObject *parent, const QVariantList & /*args*/)
    : KParts::ReadOnlyPart(parent)
{
    // this should be your custom internal widget
    mComponent = new KJotsWidget(parentWidget, this);

    // notify the part that this is our internal widget
    setWidget(mComponent);
    initAction();

    // set our XML-UI resource file
    setComponentName(QStringLiteral("kjots"), i18n("KJots"));
    setXMLFile(QStringLiteral("kjotspartui.rc"));

    connect(mComponent, &KJotsWidget::captionChanged, this, &KJotsPart::setWindowCaption);
}

KJotsPart::~KJotsPart()
{
    mComponent->queryClose();
}

void KJotsPart::initAction()
{
    auto *action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("&Configure KJots..."), this);
    actionCollection()->addAction(QStringLiteral("kjots_configure"), action);
    connect(action, &QAction::triggered, mComponent, &KJotsWidget::configure);
}

bool KJotsPart::openFile()
{
    return false;
}

#include "kjotspart.moc"
