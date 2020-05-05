/*
    Copyright (c) 2009 Montel Laurent <montel@kde.org>

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
*/

#include "kjotsconfigdlg.h"
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>

KJotsConfigDlg::KJotsConfigDlg(const QString &title, QWidget *parent)
    : KCMultiDialog(parent)
{
    setWindowTitle(title);
    setFaceType(KPageDialog::List);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    button(QDialogButtonBox::Ok)->setDefault(true);

    addModule(QStringLiteral("kjots_config_misc"));
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &KJotsConfigDlg::slotOk);
}

void KJotsConfigDlg::slotOk()
{
}

KJotsConfigMisc::KJotsConfigMisc(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    miscPage = new confPageMisc(nullptr);
    lay->addWidget(miscPage);
    connect(miscPage->autoSaveInterval, qOverload<int>(&QSpinBox::valueChanged), this, &KJotsConfigMisc::modified);
    connect(miscPage->autoSave, &QCheckBox::stateChanged, this, &KJotsConfigMisc::modified);
    load();
}

void KJotsConfigMisc::modified()
{
    Q_EMIT changed(true);
}

void KJotsConfigMisc::load()
{
    KConfig config(QStringLiteral("kjotsrc"));
    KConfigGroup group = config.group("kjots");
    miscPage->autoSaveInterval->setValue(group.readEntry("AutoSaveInterval", 5));
    miscPage->autoSave->setChecked(group.readEntry("AutoSave", true));
    Q_EMIT changed(false);
}

void KJotsConfigMisc::save()
{
    KConfig config(QStringLiteral("kjotsrc"));
    KConfigGroup group = config.group("kjots");
    group.writeEntry("AutoSaveInterval", miscPage->autoSaveInterval->value());
    group.writeEntry("AutoSave", miscPage->autoSave->isChecked());
    group.sync();
    Q_EMIT changed(false);
}

#include "moc_kjotsconfigdlg.cpp"
