/*
    kjots

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    SPDX-FileCopyrightText: 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2003 Stanislav Kljuhhin <crz@hot.ee>
    SPDX-FileCopyrightText: 2005-2006 Jaison Lee <lee.jaison@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KJOTSMAIN_H
#define KJOTSMAIN_H

#include <KXmlGuiWindow>

class KJotsWidget;

class KJotsMain : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit KJotsMain(QWidget *parent = nullptr);

protected:
    bool queryClose() override;

private:
    KJotsWidget *component;
};

#endif // KJOTSMAIN_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
