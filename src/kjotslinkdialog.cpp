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

#include <QLabel>
#include <QCompleter>
#include <QGridLayout>
#include <QRadioButton>
#include <QTreeView>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <kdescendantsproxymodel.h>

#include "KJotsSettings.h"
#include "kjotsbookshelfentryvalidator.h"
#include "kjotsmodel.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/EntityTreeModel>

KJotsLinkDialog::KJotsLinkDialog(QAbstractItemModel *kjotsModel, QWidget *parent)
    : QDialog(parent), m_kjotsModel(kjotsModel)
{
    setWindowTitle(i18n("Manage Link"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KJotsLinkDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KJotsLinkDialog::reject);
    okButton->setDefault(true);
    setModal(true);

    KDescendantsProxyModel *proxyModel = new KDescendantsProxyModel(this);
    proxyModel->setSourceModel(kjotsModel);
    proxyModel->setAncestorSeparator(QLatin1String(" / "));

    m_descendantsProxyModel = proxyModel;

    QWidget *entries = new QWidget(this);

    QGridLayout *layout = new QGridLayout(entries);

    textLabel = new QLabel(i18n("Link Text:"), this);
    textLineEdit = new QLineEdit(this);
    textLineEdit->setClearButtonEnabled(true);
    linkUrlLabel = new QLabel(i18n("Link URL:"), this);
    linkUrlLineEdit = new QLineEdit(this);
    hrefCombo = new QComboBox(this);
    linkUrlLineEdit->setClearButtonEnabled(true);

    tree = new QTreeView();
    tree->setModel(proxyModel);
    tree->expandAll();
    tree->setColumnHidden(1, true);
    hrefCombo->setModel(proxyModel);
    hrefCombo->setView(tree);

    hrefCombo->setEditable(true);
    QCompleter *completer = new QCompleter(proxyModel, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    hrefCombo->setCompleter(completer);
    KJotsBookshelfEntryValidator *validator = new KJotsBookshelfEntryValidator(proxyModel, this);
    hrefCombo->setValidator(validator);

    QGridLayout *linkLayout = new QGridLayout();
    linkUrlLineEditRadioButton = new QRadioButton(entries);
    hrefComboRadioButton = new QRadioButton(entries);

    connect(linkUrlLineEditRadioButton, &QRadioButton::toggled, linkUrlLineEdit, &QLineEdit::setEnabled);
    connect(hrefComboRadioButton, &QRadioButton::toggled, hrefCombo, &QComboBox::setEnabled);
    hrefCombo->setEnabled(false);
    linkUrlLineEditRadioButton->setChecked(true);

    linkLayout->addWidget(linkUrlLineEditRadioButton, 0, 0);
    linkLayout->addWidget(linkUrlLineEdit, 0, 1);
    linkLayout->addWidget(hrefComboRadioButton, 1, 0);
    linkLayout->addWidget(hrefCombo, 1, 1);

    layout->addWidget(textLabel, 0, 0);
    layout->addWidget(textLineEdit, 0, 1);
    layout->addWidget(linkUrlLabel, 1, 0);
    layout->addLayout(linkLayout, 1, 1);

    mainLayout->addWidget(entries);
    mainLayout->addWidget(buttonBox);
    textLineEdit->setFocus();

    connect(hrefCombo, &QComboBox::editTextChanged, this, &KJotsLinkDialog::trySetEntry);
}

void KJotsLinkDialog::setLinkText(const QString &linkText)
{
    textLineEdit->setText(linkText);
    if (!linkText.trimmed().isEmpty()) {
        linkUrlLineEdit->setFocus();
    }
}

void KJotsLinkDialog::setLinkUrl(const QString &linkUrl)
{
    const QUrl url(linkUrl);
    if (url.scheme() == QStringLiteral("akonadi")) {
        QModelIndex idx = KJotsModel::modelIndexForUrl(m_descendantsProxyModel, url);
        if (idx.isValid()) {
            hrefComboRadioButton->setChecked(true);
            hrefCombo->view()->setCurrentIndex(idx);
            hrefCombo->setCurrentIndex(idx.row());
        }
    } else {
        linkUrlLineEdit->setText(linkUrl);
        linkUrlLineEditRadioButton->setChecked(true);
        return;
    }
}

QString KJotsLinkDialog::linkText() const
{
    return textLineEdit->text().trimmed();
}

void KJotsLinkDialog::trySetEntry(const QString &text)
{
    QString t(text);
    int pos = hrefCombo->lineEdit()->cursorPosition();
    if (hrefCombo->validator()->validate(t, pos) == KJotsBookshelfEntryValidator::Acceptable) {
        int row = hrefCombo->findText(t, Qt::MatchFixedString);
        QModelIndex index = hrefCombo->model()->index(row, 0);
        hrefCombo->view()->setCurrentIndex(index);
        hrefCombo->setCurrentIndex(row);
    }
}

QString KJotsLinkDialog::linkUrl() const
{
    if (hrefComboRadioButton->isChecked()) {
        return hrefCombo->view()->currentIndex().data(KJotsModel::EntityUrlRole).toString();
    } else {
        return linkUrlLineEdit->text();
    }
}

