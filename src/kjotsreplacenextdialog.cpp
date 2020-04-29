//
//  kjots
//
//  Copyright (C) 2008 Stephen Kelly <steveire@gmail.com>
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

#include "kjotsreplacenextdialog.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

#include <KLocalizedString>

KJotsReplaceNextDialog::KJotsReplaceNextDialog(QWidget *parent) :
    QDialog(parent), m_answer(Close)
{
    setModal(true);
    setWindowTitle(i18n("Replace"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_mainLabel = new QLabel(this);
    layout->addWidget(m_mainLabel);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    QPushButton *button = buttonBox->addButton(i18n("&All"), QDialogButtonBox::NoRole);
    connect(button, &QPushButton::clicked, this, &KJotsReplaceNextDialog::onHandleAll);
    button = buttonBox->addButton(i18n("&Skip"), QDialogButtonBox::NoRole);
    connect(button, &QPushButton::clicked, this, &KJotsReplaceNextDialog::onHandleSkip);
    button = buttonBox->addButton(i18n("Replace"), QDialogButtonBox::NoRole);
    connect(button, &QPushButton::clicked, this, &KJotsReplaceNextDialog::onHandleReplace);
    button = buttonBox->addButton(QDialogButtonBox::Close);
    connect(button, &QPushButton::clicked, this, &KJotsReplaceNextDialog::reject);
    layout->addWidget(buttonBox);
}

void KJotsReplaceNextDialog::setLabel(const QString &pattern, const QString &replacement)
{
    m_mainLabel->setText(i18n("Replace '%1' with '%2'?", pattern, replacement));
}

void KJotsReplaceNextDialog::onHandleAll()
{
    m_answer = All;
    accept();
}

void KJotsReplaceNextDialog::onHandleSkip()
{
    m_answer = Skip;
    accept();
}

void KJotsReplaceNextDialog::onHandleReplace()
{
    m_answer = Replace;
    accept();
}

