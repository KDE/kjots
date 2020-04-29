//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007-2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "KJotsMain.h"

#include <kactioncollection.h>
#include <qdebug.h>
#include <kstandardaction.h>
#include <KLocalizedString>

#include "KJotsSettings.h"
#include "kjotsbookmarks.h"
#include "kjotsedit.h"
#include "kjotsbrowser.h"
#include "kjotswidget.h"

#include <QStatusBar>
#include <QApplication>

//----------------------------------------------------------------------
// KJOTSMAIN
//----------------------------------------------------------------------
KJotsMain::KJotsMain()
{

    // Main widget
    //

    KStandardAction::quit(this, &KJotsMain::onQuit, actionCollection());

    component = new KJotsWidget(this, this);

    setCentralWidget(component);

    connect(component, &KJotsWidget::activeAnchorChanged, this, &KJotsMain::activeAnchorChanged);

    setupGUI();
    connect(component, &KJotsWidget::captionChanged, this, &KJotsMain::updateCaption);

}

/*!
    Sets the window caption.
*/
void KJotsMain::updateCaption(QString caption)
{
    setCaption(caption);
}

void KJotsMain::activeAnchorChanged(const QString &anchorTarget, const QString &anchorText)
{
    if (!anchorTarget.isEmpty()) {
        statusBar()->showMessage(i18nc("@info:status Link information; %1 is displayed text, %2 is link destination", "%1 -> %2",
                                       anchorText, anchorTarget));
    } else {
        statusBar()->clearMessage();
    }
}

bool KJotsMain::queryClose()
{
    return component->queryClose();
}

void KJotsMain::onQuit()
{
//     component->queryClose();
    deleteLater();
    qApp->quit();
}

