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

#ifndef KZOOMMAINWINDOW_H
#define KZOOMMAINWINDOW_H

#include <kmainwindow.h>
#include <QList>

#include <libkdegames_export.h>

class KToggleAction;
class KAction;
class Zoomable;

/**
 * KZoomMainWindow is a main window of fixed size. Its size can be
 * modified with the "zoom in"/"zoom out" actions.
 *
 * It manages one or several objects of Zoomable type: 
 * their zoomChanged() method is called whenever the zoom
 * level is changed.
 * To react to zoom events make your widget (or whatever)
 * additionally inherit from Zoomable class 
 * and reimplement zoomChanged() virtual functon.
 * The usual implementation looks like this
 * /code
 * setFixedSize(newsize);
 * /endcode
 *
 * This class also has a "show/hide menubar" action and allows the use
 * of a context popup menu (useful to restore the menubar when hidden).
 */
class KDEGAMES_EXPORT KZoomMainWindow : public KMainWindow
{
  Q_OBJECT
public:
  /** Constructor. */
  KZoomMainWindow(uint minZoom, uint maxZoom, uint zoomStep);

  /** Add a zoomable to be managed i.e. the zoomChanged() method of the
   * passed object is called whenever the zoom is changed.
   */
  void addZoomable(Zoomable *z);
  /**
   *  Remove a zoomable from a list of managed objects.
   */
  void removeZoomable(Zoomable *z);
                  
  uint zoom() const { return _zoom; }
  
public Q_SLOTS:
  void zoomIn();
  void zoomOut();
  void toggleMenubar();

protected:
  /** You need to call this after the createGUI or setupGUI method
   * is called.
   * @param popupName is the name of the context popup menu as defined in
   * the ui.rc file.
   */
  void init(const char *popupName = 0);
    
  virtual void setZoom(uint zoom);
  virtual bool eventFilter(QObject *o, QEvent *e);
  virtual bool queryExit();
  
  /** You need to implement this method since different application
   * use different setting class names and keys.
   * Use something like:
   * /code
   * Settings::setZoom(zoom);
   * Settings::writeConfig();
   * /endcode
   */
  virtual void writeZoomSetting(uint zoom) = 0;
  
  /** Youneed to implement this method since different application
   * use different setting class names and keys.
   * Use something like:
   * /code
   * return Settings::zoom();
   * /endcode
   */
  virtual uint readZoomSetting() const = 0;
  
  /** You need to implement this method since different application
   * use different setting class names and keys.
   * Use something like:
   * /code
   * Settings::setMenubarVisible(visible);
   * Settings::writeConfig();
   * /endcode
   */
  virtual void writeMenubarVisibleSetting(bool visible) = 0;
  
  /** You need to implement this method since different application
   * use different setting class names and keys.
   * Use something like: 
   * /code
   * Settings::menubarVisible();
   * /endcode
   */
  virtual bool menubarVisibleSetting() const = 0;

private:
  uint _zoom, _zoomStep, _minZoom, _maxZoom;
  QList<Zoomable*> _zoomables;
  KAction *_zoomInAction, *_zoomOutAction;
  KToggleAction *_menu;
  
  /* 
  class KZoomMainWindowPrivate;
  KZoomMainWindowPrivate *d;
   */
};

/**
 *  Interface class for widgets intended to be children
 *  of KZoomMainWindow which support zooming.
 *  Inherit from this class, reimplement zoomChanged(),
 *  call KZoomMainWindow::addZoomable() somewhere and you're done.
 */
class Zoomable
{
public:
    Zoomable() { }
    virtual ~Zoomable() { }
    /**
     *  This method will be called by KZoomMainWindow when
     *  zoom actions will be triggered
     */
    virtual void zoomChanged() { }
};

#endif
