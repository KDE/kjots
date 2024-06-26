/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOTS_LOCK_ATTRIBUTE_H
#define KJOTS_LOCK_ATTRIBUTE_H

#include <akonadi_version.h>
#include <Akonadi/Attribute>

namespace NoteShared
{
class NoteLockAttribute : public Akonadi::Attribute
{
public:
    explicit NoteLockAttribute();

    QByteArray type() const override;

    NoteLockAttribute *clone() const override;

    QByteArray serialized() const override;

    void deserialize(const QByteArray &data) override;
};
}

#endif
