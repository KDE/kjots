/*
    This file is part of KJots.

    Copyright (C) 2020 Igor Poboiko <igor.poboiko@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
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
    return {};
}
