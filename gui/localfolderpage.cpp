/*
 * Cantata
 *
 * Copyright (c) 2018 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "localfolderpage.h"
#include "gui/stdactions.h"
#include "gui/customactions.h"
#include "support/configuration.h"
#include "widgets/menubutton.h"

LocalFolderBrowsePage::LocalFolderBrowsePage(bool isHome, QWidget *p)
    : SinglePageWidget(p)
{
    if (isHome) {
        cfgGroup = QLatin1String("localbrowsehome");
        model = new LocalBrowseModel(cfgGroup, tr("Home"), tr("Browse files in your home folder"), ":home.svg", this);
    } else {
        cfgGroup = QLatin1String("localbrowseroot");
        model = new LocalBrowseModel(cfgGroup, tr("Root"), tr("Browse files on your computer"), ":hdd.svg", this);
    }
    proxy = new FileSystemProxyModel(model);
    connect(view, SIGNAL(itemsSelected(bool)), this, SLOT(controlActions()));
    connect(view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(itemDoubleClicked(const QModelIndex &)));
    connect(view, SIGNAL(headerClicked(int)), SLOT(headerClicked(int)));
    Configuration config(cfgGroup);
    view->setMode(ItemView::Mode_SimpleTree);
    view->load(config);
    MenuButton *menu=new MenuButton(this);
    menu->addActions(createViewActions(QList<ItemView::Mode>() << ItemView::Mode_BasicTree << ItemView::Mode_SimpleTree));
    init(ReplacePlayQueue|AppendToPlayQueue, QList<QWidget *>() << menu);

    view->addAction(StdActions::self()->addToStoredPlaylistAction);
    view->addAction(CustomActions::self());
    view->setModel(proxy);
    view->closeSearch();
    view->alwaysShowHeader();
    view->view()->setRootIndex(proxy->mapFromSource(model->setRootPath(isHome ? QDir::homePath() : "/")));
}

LocalFolderBrowsePage::~LocalFolderBrowsePage()
{
    Configuration config(cfgGroup);
    view->save(config);
    model->deleteLater();
    model=nullptr;
}

void LocalFolderBrowsePage::headerClicked(int level)
{
    if (0==level) {
        emit close();
    }
}

#include <QDebug>

void LocalFolderBrowsePage::itemDoubleClicked(const QModelIndex &)
{
    const QModelIndexList selected = view->selectedIndexes(false); // Dont need sorted selection here...
    if (1!=selected.size()) {
        return; //doubleclick should only have one selected item
    }

    if (!static_cast<BrowseModel::Item *>(selected.at(0).internalPointer())->isFolder()) {
        addSelectionToPlaylist();
    }
}

void LocalFolderBrowsePage::addSelectionToPlaylist(const QString &name, int action, quint8 priority, bool decreasePriority)
{
    qWarning() << name << action;
}

void LocalFolderBrowsePage::controlActions()
{
    QModelIndexList selected=view->selectedIndexes(false); // Dont need sorted selection here...
    StdActions::self()->enableAddToPlayQueue(!selected.isEmpty());
}
