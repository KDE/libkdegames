/*
    This file is part of the KDE games library
    Copyright (C) 2004 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kzoommainwindow.h"
#include "kzoommainwindow.moc"

#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kmenubar.h>
#include <kxmlguifactory.h>
#include <kdebug.h>

#include <QEvent>

KZoomMainWindow::KZoomMainWindow(uint min, uint max, uint step)
  : KMainWindow(0), _zoomStep(step), _minZoom(min), _maxZoom(max)
{
  installEventFilter(this);

  _zoomInAction = KStandardAction::zoomIn(this, SLOT(zoomIn()), this);
  actionCollection()->addAction(_zoomInAction->objectName(), _zoomInAction);
  _zoomOutAction = KStandardAction::zoomOut(this, SLOT(zoomOut()), this);
  actionCollection()->addAction(_zoomOutAction->objectName(), _zoomOutAction);
  _menu = KStandardAction::showMenubar(this, SLOT(toggleMenubar()), this);
  actionCollection()->addAction(_menu->objectName(), _menu);
}

void KZoomMainWindow::init(const char *popupName)
{
  // zoom
  setZoom(readZoomSetting());

  // menubar
  _menu->setChecked( menubarVisibleSetting() );
  toggleMenubar();

  // context popup
  if (popupName) {
    QMenu *popup =
      static_cast<QMenu *>(factory()->container(popupName, this));
    Q_ASSERT(popup);
    if (popup) {
        setContextMenuPolicy(Qt::ActionsContextMenu);
        addActions(popup->actions());
    }
  }
}

void KZoomMainWindow::addZoomable(Zoomable *z)
{
  _zoomables.append(z);
}

void KZoomMainWindow::removeZoomable(Zoomable *z)
{
  _zoomables.removeAll(z);
}

bool KZoomMainWindow::eventFilter(QObject *o, QEvent *e)
{
  if ( e->type()==QEvent::LayoutHint )
    setFixedSize(minimumSize()); // because K/QMainWindow
                                 // does not manage fixed central widget
                                 // with hidden menubar...
  return KMainWindow::eventFilter(o, e);
}

void KZoomMainWindow::setZoom(uint zoom)
{
  _zoom = zoom;
  writeZoomSetting(_zoom);

  foreach(Zoomable* wid, _zoomables)
    wid->zoomChanged();
  _zoomOutAction->setEnabled( _zoom>_minZoom );
  _zoomInAction->setEnabled( _zoom<_maxZoom );
  adjustSize();
}

void KZoomMainWindow::zoomIn()
{
  setZoom(_zoom + _zoomStep);
}

void KZoomMainWindow::zoomOut()
{
  Q_ASSERT( _zoom>=_zoomStep );
  setZoom(_zoom - _zoomStep);
}

void KZoomMainWindow::toggleMenubar()
{
  if ( _menu->isChecked() ) menuBar()->show();
  else menuBar()->hide();
}

bool KZoomMainWindow::queryExit()
{
  writeMenubarVisibleSetting(_menu->isChecked());
  return KMainWindow::queryExit();
}
