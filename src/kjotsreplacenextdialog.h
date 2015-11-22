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

#ifndef KJOTSREPLACENEXTDIALOG_H
#define KJOTSREPLACENEXTDIALOG_H

#include <QDialog>

class QString;
class QLabel;

// Mostly stolen from kdelibs/kdeui/findreplace/kreplace.cpp
class KJotsReplaceNextDialog : public QDialog
{
    Q_OBJECT
public:
    enum Answer {
        All,
        Skip,
        Replace,
        Close
    };

    explicit KJotsReplaceNextDialog(QWidget *parent);
    void setLabel(const QString &pattern, const QString &replacement);
    Answer answer() const
    {
        return m_answer;
    }
protected slots:
    void onHandleAll(void);
    void onHandleSkip(void);
    void onHandleReplace(void);

private:
    QLabel *m_mainLabel;
    Answer m_answer;
};

#endif
