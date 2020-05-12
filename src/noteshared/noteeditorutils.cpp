/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "noteeditorutils.h"

#include <QLocale>
#include <QMimeDatabase>
#include <QFileDialog>
#include <KLocalizedString>
#include <QBuffer>
#include <QChar>
#include <QTextCursor>
#include <QTextEdit>
#include <QDateTime>
#include <QFileInfo>

using namespace NoteShared::NoteEditorUtils;

void NoteShared::NoteEditorUtils::addCheckmark(QTextCursor &cursor)
{
    const int position = cursor.position();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.insertText(QString::fromUtf16(u"\U00002713"));
    cursor.setPosition(position + 1);
}

void NoteShared::NoteEditorUtils::insertDate(QTextEdit *editor)
{
    editor->insertPlainText(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat) + QLatin1Char(' '));
}

void NoteShared::NoteEditorUtils::insertImage(QTextDocument * /*doc*/, QTextCursor &/*cursor*/, QTextEdit *par)
{
    QString imageFileName = QFileDialog::getOpenFileName(par, i18n("Select image file"), QStringLiteral("."), QStringLiteral("Images (*.png *.bmp *.jpg *.jpeg *.jpe)"));
    if (!imageFileName.isEmpty()) {
        QFileInfo qfio = QFileInfo(imageFileName);
        QImage imgRes(imageFileName);
        if (!imgRes.isNull()) {
            QMimeDatabase mimedb;
            QByteArray imageData;
            QBuffer buffer(&imageData);
            QMimeType filemime = mimedb.mimeTypeForFile(qfio);
            QString filetype = filemime.name();
            QByteArray formatChars = filemime.preferredSuffix().toUpper().toLatin1();

            buffer.open(QIODevice::WriteOnly);
            imgRes.save(&buffer, formatChars.data());
            QString Base64Image = QString::fromLatin1(imageData.toBase64().data());//is null
            par->insertHtml(QStringLiteral("<img src=\"data:%1;base64,%2\" />").arg(filetype, Base64Image));
        }
    }
}
