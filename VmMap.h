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
 * File:		VmMap.h
 * Date:		Tue May  5 22:44:11 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     map_window class    
 * 
 * Modifications:
 * $Log$
 */

#ifndef __map_window_h__
#define __map_window_h__

#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/grid.h>
#include <gtkmm/hvbox.h>
#include "VmTile.h"
#include "VmMapControls.h"
#include "dialogs.h"

const int map_max = 100;	/* 100x100 tiles should be enough */
const int resX = 384;		/* typical C64 screen resolution */
const int resY = 272;
const int def_crx = 32;
const int def_cry = 36;
const double def_zoom = 3.0;

const std::string def_basename = "vice-screen-";
const std::string def_cfg_ext = ".vsm";
const std::string mapper_version = MAPPER_VERSION;

class VmMap : public Gtk::ScrolledWindow
{
    int get_empty_area(int from_x, int from_y, int to_x, int to_y);
    bool process_line(std::string);
    bool dirty;
  protected:
    class MyScw : public Gtk::ScrolledWindow
    {
	bool space_modifier;
	bool button_press;
	
	// Signal handlers:
	bool on_scroll_event(GdkEventScroll *scroll_event) override;
	bool on_motion_notify_event(GdkEventMotion* motion_event) override;
	bool on_key_press_event(GdkEventKey *key_event) override;
	bool on_key_release_event(GdkEventKey *key_event) override;
	bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
	/*
	bool on_button_press_event(GdkEventButton *button_event) override;
	bool on_button_release_event(GdkEventButton *button_event) override;
	*/
      public:
	MyScw();
	VmMapControls *mc;
	Glib::RefPtr<Gdk::Cursor> move_cursor;
    };
    // Child widgets:
    Gtk::Grid map_grid;
    MyScw scw;
    Glib::RefPtr<Gdk::Pixbuf> m_empty;
    VmTile *tiles[map_max+1][map_max+1];
    int nr_tiles;
    VmMapControls *ctrls;
    Gtk::Frame *map_frame;

  public:
    VmMap();
    virtual ~VmMap() {};

    inline void set_dirty(bool d) { dirty=d; ctrls->set_dirty(d); }
    inline bool is_dirty(void) { return dirty; }
	
    void add_tile(VmTile *);
    void remove_tile(VmTile *, bool map_remove = false);
    void add_unplaced_tile(VmTile *t) { ctrls->add_tile(t); }
    void reload_unplaced_tiles(char *path = NULL);

    void fill_empties();
    void scale_all(void);
    void xchange_tiles(VmTile *s, VmTile *d);
    inline VmTile *get_tile(int x, int y) { return tiles[x][y]; }
    inline void set_tile(int x, int y, VmTile *t = nullptr) { tiles[x][y] = t; }
    inline int get_nrtiles(void) { return nr_tiles; }
    static Glib::RefPtr<Gdk::Pixbuf> empty_image;
    static double scale_factor_x;
    static double scale_factor_y;

    void resize_map(void);
    void remove_map(void);
    void open_map(void);
    void commit_changes(void) { if (dirty) { ctrls->commit_changes(); } }
    void save_settings(void);
    bool load_settings(void);
};

#endif /* __map_window_h__ */
