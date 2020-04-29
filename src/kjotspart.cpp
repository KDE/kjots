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

#include <qdebug.h>

//#include <kcomponentdata.h>
//#include <kfiledialog.h>
#include <kstandardaction.h>
#include <KActionCollection>
#include <kpluginfactory.h>
#include <KLocalizedString>

#include <QStatusBar>
#include <QIcon>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QStandardPaths>
#include <QAction>

const KAboutData &createAboutData()
{
    static AboutData aboutData;
    return aboutData;
}

K_PLUGIN_FACTORY(KJotsPartFactory, registerPlugin<KJotsPart>();)

KJotsPart::KJotsPart(QWidget *parentWidget, QObject *parent, const QVariantList & /*args*/)
    : KParts::ReadOnlyPart(parent)
{
    // we need an instance
    //QT5 setComponentData( KJotsPartFactory::componentData() );

    // this should be your custom internal widget
    mComponent = new KJotsWidget(parentWidget, this);

    mStatusBar = new KParts::StatusBarExtension(this);
    // notify the part that this is our internal widget
    setWidget(mComponent);
    initAction();

    // set our XML-UI resource file
    setComponentName(QStringLiteral("kjots"), QStringLiteral("kjots"));
    setXMLFile(QStringLiteral("kjotspartui.rc"));

    QTimer::singleShot(0, this, &KJotsPart::delayedInitialization);
}

KJotsPart::~KJotsPart()
{
    mComponent->queryClose();
}

void KJotsPart::initAction()
{
    QAction *action = new QAction(QIcon::fromTheme(QLatin1String("configure")), i18n("&Configure KJots..."), this);
    actionCollection()->addAction(QLatin1String("kjots_configure"), action);
    connect(action, &QAction::triggered, mComponent, &KJotsWidget::configure);
}

bool KJotsPart::openFile()
{
    return false;
}

void KJotsPart::delayedInitialization()
{
    connect(mComponent, &KJotsWidget::activeAnchorChanged, this, &KJotsPart::activeAnchorChanged);
}

void KJotsPart::activeAnchorChanged(const QString &anchorTarget, const QString &anchorText)
{
    if (!anchorTarget.isEmpty()) {
        mStatusBar->statusBar()->showMessage(i18nc("@info:status Link information; %1 is displayed text, %2 is link destination", "%1 -> %2",
                                                   anchorText, anchorTarget));
    } else {
        mStatusBar->statusBar()->clearMessage();
    }
}

//
// bool KJotsPart::saveFile()
// {
//     return false;
// }
#include "kjotspart.moc"
