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
#include <gtkmm/togglebutton.h>
#include <gtkmm/adjustment.h>

#include "VmTile.h"

typedef enum { CZX, CZY, CRUP, CRDO, CRLE, CRRI } VmMapCtrlAdj;

class VmMapControls
{
    VmMap &mw;
    Gtk::Frame *cf;
    Gtk::Box *unpl_tilesbox;
    Gtk::Button *button_save;
//    Gtk::Adjustment *adjx;
    Glib::RefPtr<Gtk::Adjustment> adj[6];
    Gtk::ToggleButton *zl;

    bool zoom_lock;
public:
    VmMapControls(VmMap &m);
    ~VmMapControls() {};
    
    inline Gtk::Frame *get_widget(void) { return cf; }
    inline Glib::RefPtr<Gtk::Adjustment> get_adj(VmMapCtrlAdj a) { return adj[a]; }
    void add_tile(VmTile *tile);
    void remove_tile(VmTile *t);
    void set_dirty(bool d);
    void commit_changes(void);
    void set_zoom(double x, double y, bool dirty = true);
    void set_crop(int u, int d, int l, int r, bool dirty = true);
    void update_ctrls(void);
    void toggle_zoom_lock(void);
    inline bool get_zoom_lock(void) { return zoom_lock; }
};

#endif /* __VmMapControls_h__ */
