/*
    This file is a part of KJots.

    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    , m_descendantsProxyModel(new KDescendantsProxyModel(this))
{
    ui->setupUi(this);

    m_descendantsProxyModel->setSourceModel(kjotsModel);
    m_descendantsProxyModel->setAncestorSeparator(QStringLiteral(" / "));
    m_descendantsProxyModel->setDisplayAncestorData(true);

    ui->hrefCombo->lineEdit()->setPlaceholderText(i18n("Enter link URL, or another note or note book..."));
    ui->hrefCombo->setModel(m_descendantsProxyModel.get());
    // This is required because otherwise QComboBox will catch Enter, insert a new item and clear
    ui->hrefCombo->setInsertPolicy(QComboBox::NoInsert);
    ui->hrefCombo->setCurrentIndex(-1);

    auto *completer = new QCompleter(m_descendantsProxyModel.get(), this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->hrefCombo->setCompleter(completer);

    connect(ui->hrefCombo, &QComboBox::editTextChanged, this, &KJotsLinkDialog::slotTextChanged);
    connect(ui->textEdit, &QLineEdit::textChanged, this, &KJotsLinkDialog::slotTextChanged);
    slotTextChanged();
}

KJotsLinkDialog::~KJotsLinkDialog() = default;

void KJotsLinkDialog::slotTextChanged()
{
    const bool ok = !ui->hrefCombo->currentText().trimmed().isEmpty() && !ui->textEdit->text().trimmed().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

void KJotsLinkDialog::setLinkText(const QString &linkText)
{
    ui->textEdit->setText(linkText);
    if (!linkText.isEmpty()) {
        ui->textEdit->setFocus();
    }
}

void KJotsLinkDialog::setLinkUrl(const QString &linkUrl)
{
    const QModelIndex idx = KJotsModel::modelIndexForUrl(m_descendantsProxyModel.get(), QUrl(linkUrl));
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

