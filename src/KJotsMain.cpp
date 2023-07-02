/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2007-2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KJotsMain.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>

#include <QApplication>

#include "kjotswidget.h"


//----------------------------------------------------------------------
// KJOTSMAIN
//----------------------------------------------------------------------
KJotsMain::KJotsMain(QWidget *parent)
    : KXmlGuiWindow(parent)
    , component(new KJotsWidget(this, this))
{
    KStandardAction::quit(this, &KJotsMain::close, actionCollection());

    setCentralWidget(component);

    setupGUI();
    connect(component, &KJotsWidget::captionChanged, this, qOverload<const QString &>(&KJotsMain::setCaption));
}

bool KJotsMain::queryClose()
{
    return component->queryClose();
}

#include "moc_KJotsMain.cpp"
