/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notepinattribute.h"

using namespace NoteShared;

NotePinAttribute::NotePinAttribute()
    : Akonadi::Attribute()
{
}

NotePinAttribute *NotePinAttribute::clone() const
{
    return new NotePinAttribute();
}

QByteArray NotePinAttribute::type() const
{
    return QByteArrayLiteral("NotePinAttribute");
}

void NotePinAttribute::deserialize(const QByteArray &/*data*/)
{
    // unused
}

QByteArray NotePinAttribute::serialized() const
{
    return QByteArrayLiteral("-");
}
