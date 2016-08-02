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

#ifndef NOTEEDITORUTILS_H
#define NOTEEDITORUTILS_H

class QTextCursor;
class QTextEdit;
class QTextDocument;
class QWidget;

namespace NoteShared
{
class NoteEditorUtils
{
public:
    NoteEditorUtils();
    void addCheckmark(QTextCursor &cursor);
    void insertDate(QTextEdit *editor);
    void insertImage(QTextDocument *doc, QTextCursor &cursor, QTextEdit *par);
};
}
#endif // NOTEEDITORUTILS_H
