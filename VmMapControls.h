/* -*-c++-*-
 * This file is part of vice-mapper.
 * 
 * vice-mapper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * vice-mapper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with vice-mapper.  If not, see <https://www.gnu.org/licenses/>.
 *
 * File:		VmMapControls.h
 * Date:		Fri May  8 18:25:51 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     Controls widget
 * 
 * Modifications:
 * $Log$
 */

#ifndef __VmMapControls_h__
#define __VmMapControls_h__

#include <gtkmm/frame.h>
#include <gtkmm/button.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include "VmTile.h"

class VmMapControls : public Gtk::Frame
{
    Gtk::Button button_commit;
    Gtk::Button button_reload;
    
    bool on_commit_press_event(GdkEventButton *);
    bool on_reload_press_event(GdkEventButton *);
    void on_scale_event1();
    void on_scale_event2();
    void on_scale_crop();

    VmMap &mw;
    
    // Zoom controls
    Glib::RefPtr<Gtk::Adjustment> adjx;
    Glib::RefPtr<Gtk::Adjustment> adjy;
    // Crop controls
    Glib::RefPtr<Gtk::Adjustment> adj_crup;
    Glib::RefPtr<Gtk::Adjustment> adj_crdo;
    Glib::RefPtr<Gtk::Adjustment> adj_crle;
    Glib::RefPtr<Gtk::Adjustment> adj_crri;
    // Unplaced tiles
    Gtk::VBox unpl_tilesbox;
    
  protected:
    Gtk::ScrolledWindow m_ScrolledWindow;

  public:
    VmMapControls(VmMap &m, const Glib::ustring &);
    virtual ~VmMapControls() {};
    
    void set_zoom(double x, double y, bool dirty = true);
    void set_crop(int u, int d, int l, int r, bool dirty = true);
    void add_tile(VmTile *tile);
    void remove_tile(VmTile *t);
    void set_dirty(bool d);
    void commit_changes(void);
};

#endif /* __VmMapControls_h__ */
