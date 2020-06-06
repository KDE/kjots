/*
    This file is part of KJots.

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2007-2009 Stephen Kelly <steveire@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

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

#ifndef KJOTSWIDGET_H
#define KJOTSWIDGET_H

#include <QWidget>
#include <QAbstractItemDelegate>
#include <QPrinter>

#include <AkonadiCore/Collection>

#include <grantlee/templateloader.h>

class QActionGroup;
class QCheckBox;
class QTextEdit;
class QTextCharFormat;
class QSplitter;
class QStackedWidget;
class QModelIndex;
class QTreeView;

class KActionMenu;
class KFindDialog;
class KJob;
class KReplaceDialog;
class KSelectionProxyModel;
class KJotsBrowser;
class KJotsBrowserWidget;
class KXMLGUIClient;

namespace Akonadi
{
class EntityMimeTypeFilterModel;
class EntityOrderProxyModel;
class EntityTreeView;
class StandardNoteActionManager;
}

namespace Grantlee
{
class Engine;
}

namespace KPIMTextEdit
{
class RichTextEditorWidget;
}

class KJotsEdit;
class KJotsModel;
class KJotsTreeView;
class NoteSortProxyModel;

class KJotsWidget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KJotsWidget")

public:
    explicit KJotsWidget(QWidget *parent, KXMLGUIClient *xmlGuiclient, Qt::WindowFlags f = Qt::WindowFlags());
    ~KJotsWidget();

    QTextEdit *activeEditor();
public Q_SLOTS:
    void configure();
    void updateCaption();
    void updateMenu();

    Q_SCRIPTABLE bool queryClose();

    void setViewMode(int mode);

Q_SIGNALS:
    void canGoNextPageChanged(bool);
    void canGoPreviousPageChanged(bool);
    void canGoNextBookChanged(bool);
    void canGoPreviousBookChanged(bool);

    void captionChanged(const QString &newCaption);

protected:
    static QModelIndex previousNextEntity(QTreeView *view, int step);

    void setupGui();
    void setupActions();

    QString renderSelectionTo(const QString &theme, const QString &templ);
    QString renderSelectionToHtml();

    std::unique_ptr<QPrinter> setupPrinter(QPrinter::PrinterMode mode = QPrinter::ScreenResolution);

    void saveUIStates() const;
    void restoreUIStates();
protected Q_SLOTS:
    /**
     * Renders contents on either KJotsEdit or KJotsBrowser based on KJotsTreeView selection
     */
    void renderSelection();
    void exportSelection(const QString &theme, const QString &templ);
    void printSelection();
    void printPreviewSelection();

    void openLink(const QUrl &url);
private Q_SLOTS:
    void delayedInitialization();

    void saveState();
    void restoreState();
    void updateConfiguration();

    void print(QPrinter *printer);
private:
    // Grantlee
    Grantlee::Engine *m_templateEngine = nullptr;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> m_loader;

    // XMLGui && Actions
    KXMLGUIClient  *m_xmlGuiClient = nullptr;
    Akonadi::StandardNoteActionManager *m_actionManager = nullptr;
    QSet<QAction *> anySelectionActions, editorActions;
    QActionGroup *m_viewModeGroup = nullptr;

    // UI
    QSplitter *m_splitter1 = nullptr;
    QSplitter *m_splitter2 = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    Akonadi::EntityTreeView *m_collectionView = nullptr;
    Akonadi::EntityTreeView *m_itemView = nullptr;
    KJotsEdit      *m_editor = nullptr;
    KPIMTextEdit::RichTextEditorWidget *m_editorWidget = nullptr;
    KJotsBrowserWidget *m_browserWidget = nullptr;

    // Models
    KJotsModel *m_kjotsModel = nullptr;
    Akonadi::EntityMimeTypeFilterModel *m_collectionModel = nullptr;
    Akonadi::EntityMimeTypeFilterModel *m_itemModel = nullptr;
    NoteSortProxyModel *m_itemSortModel = nullptr;
    KSelectionProxyModel *m_collectionSelectionProxyModel = nullptr;
    Akonadi::EntityOrderProxyModel *m_orderProxy = nullptr;

    QTimer *m_autosaveTimer = nullptr;
};

#endif
