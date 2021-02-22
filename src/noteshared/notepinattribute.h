/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTE_PIN_ATTRIBUTE_H
#define NOTE_PIN_ATTRIBUTE_H

#include <AkonadiCore/Attribute>

namespace NoteShared
{

class NotePinAttribute : public Akonadi::Attribute
{
public:
    explicit NotePinAttribute();

    NotePinAttribute *clone() const override;
    QByteArray type() const override;

    void deserialize(const QByteArray &data) override;
    QByteArray serialized() const override;
};
}

#endif // NOTE_PIN_ATTRIBUTE_H
