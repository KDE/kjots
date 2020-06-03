/*
    kjots

    Copyright (C) 2008 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "kjotslinkdialog.h"
#include "ui_linkdialog.h"
#include "kjotsmodel.h"

#include <QListView>
#include <QPushButton>
#include <QCompleter>

#include <KDescendantsProxyModel>


KJotsLinkDialog::KJotsLinkDialog(QAbstractItemModel *kjotsModel, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LinkDialog)
{
    ui->setupUi(this);

    m_descendantsProxyModel = new KDescendantsProxyModel(this);
    m_descendantsProxyModel->setSourceModel(kjotsModel);
    m_descendantsProxyModel->setAncestorSeparator(QStringLiteral(" / "));
    m_descendantsProxyModel->setDisplayAncestorData(true);

    ui->hrefCombo->lineEdit()->setPlaceholderText(i18n("Enter link URL, or another note or note book..."));
    ui->hrefCombo->setModel(m_descendantsProxyModel);
    // This is required because otherwise QComboBox will catch Enter, insert a new item and clear
    ui->hrefCombo->setInsertPolicy(QComboBox::NoInsert);
    ui->hrefCombo->setCurrentIndex(-1);

    auto *completer = new QCompleter(m_descendantsProxyModel, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->hrefCombo->setCompleter(completer);
}

KJotsLinkDialog::~KJotsLinkDialog() = default;

void KJotsLinkDialog::setLinkText(const QString &linkText)
{
    ui->textEdit->setText(linkText);
    if (!linkText.isEmpty()) {
        ui->textEdit->setFocus();
    }
}

void KJotsLinkDialog::setLinkUrl(const QString &linkUrl)
{
    const QModelIndex idx = KJotsModel::modelIndexForUrl(m_descendantsProxyModel, QUrl(linkUrl));
    if (idx.isValid()) {
        ui->hrefCombo->setCurrentIndex(idx.row());
    } else {
        ui->hrefCombo->setCurrentIndex(-1);
        ui->hrefCombo->setCurrentText(linkUrl);
    }
}

QString KJotsLinkDialog::linkText() const
{
    return ui->textEdit->text().trimmed();
}

QString KJotsLinkDialog::linkUrl() const
{
    const int row = ui->hrefCombo->currentIndex();
    if (row != -1) {
        return ui->hrefCombo->model()->index(row, 0).data(KJotsModel::EntityUrlRole).toString();
    } else {
        return ui->hrefCombo->currentText().trimmed();
    }
}

