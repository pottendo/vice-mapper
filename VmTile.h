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
 * File:		VmTile.h
 * Date:		Tue May  5 21:22:45 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     
 * 
 * Modifications:
 * $Log$
 */

#ifndef __VmTile_h__
#define __VmTile_h__

#include <gtkmm/drawingarea.h>
#include <gtkmm/menu.h>
#include <gdkmm/pixbuf.h>
#include <vector>
#include <set>
#include <iostream>

class VmMap;

class VmTile : public Gtk::DrawingArea
{
    std::string file_name, file_basename, file_ext;
    bool dirty, empty, selected;
    int xk, yk;
    int w, h;
    static VmTile *dnd_tile;
    
    void park_tile_file(void);
    Gtk::Menu *m_pMenuPopup;	/* TODO delete in derstructor */
    void setup_popup(void);
    void on_menu_delete_tile(void);
    void on_menu_popup(void);

  public:
    VmTile(VmMap &m, const char *fn, int xk = -1, int yk = -1);
    virtual ~VmTile();

    friend std::ostream &operator<<(std::ostream &out, VmTile &t);

    static std::set<VmTile *> all_tiles;
    static int alloc_count;
    static std::vector<Gtk::TargetEntry> listTargets;
    static std::string current_path;
    static void refresh_minmax(void);
    static VmTile *lookup_by_name(std::string name);
    static bool tiles_placed;
    
    VmMap &mw;

    void print(void);
    inline std::string get_fname() { return file_name; }
    inline void set_fname(std::string f, std::string bn) { file_name = f; file_basename = bn; }
    inline const Glib::RefPtr<Gdk::Pixbuf> get_pixmap_icon() { return m_image_icon; }
    inline int getX(void) { return xk; }
    inline int getY(void) { return yk; }
    inline void setXY(int x, int y) { xk = x; yk = y; set_dirty(true);}
    inline void getXY(int &x, int &y) { x = getX(); y = getY(); }
    inline bool is_empty(void) { return empty; }
    inline bool is_dirty(void) const { return dirty; }
    inline bool is_selected(void) const { return selected; }
    inline void set_selected(bool s) { selected = s; }
    void set_dirty(bool d);
	
    static int xmin, ymin, xmax, ymax;
    static int cr_up, cr_do, cr_le, cr_ri;
    static int min_resX, min_resY, max_resX, max_resY;
    void scale(float sfx, float sfy);
    void xchange_tiles(VmTile &s, VmTile &d);
    void sync_tile(void);
    bool update_minmax(void);
    void commit_changes(void);
    Glib::RefPtr<Gdk::Pixbuf> get_cropped_image(void);
    void get_cropped_dimensions(int &w, int &h);
    
  protected:
    //Override default signal handler:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton *e) override;
    bool on_configure_event(GdkEventConfigure *configure_event) override;
    bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
    bool on_leave_notify_event(GdkEventCrossing* crossing_event) override;
    
    //Other signal handlers:
    void on_button_drag_data_get(
	const Glib::RefPtr<Gdk::DragContext>& context,
	Gtk::SelectionData& selection_data, guint info, guint time);
    void on_label_drop_drag_data_received(
	const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
	const Gtk::SelectionData& selection_data, guint info, guint time);

    Glib::RefPtr<Gdk::Pixbuf> m_image;
    Glib::RefPtr<Gdk::Pixbuf> m_image_scaled;
    Glib::RefPtr<Gdk::Pixbuf> m_image_icon;
};


#endif /* __VmTile_h__ */
