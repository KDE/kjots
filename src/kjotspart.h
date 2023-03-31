/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KJOTSPART_H
#define KJOTSPART_H

#include <KParts/StatusBarExtension>
#include <KParts/ReadOnlyPart>

class QWidget;
class KAboutData;
class KJotsWidget;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Stephen Kelly <steveire@gmail.com>
 * @version 0.1
 */
class KJotsPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    explicit KJotsPart(QWidget *parentWidget, QObject *parent, const QVariantList &);
#else
    explicit KJotsPart(QWidget *parentWidget, QObject *parent, const KPluginMetaData &data, const QVariantList &);
#endif

    /**
     * Destructor
     */
    ~KJotsPart() override;
protected:
    /**
     * This must be implemented by each part
     */
    bool openFile() override;

private:
    void initAction();
    KJotsWidget *mComponent;
};

#endif // KJOTSPART_H
