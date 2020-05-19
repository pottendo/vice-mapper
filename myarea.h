/*  -*-c-*-
 * File:		myarea.h
 * Date:		Tue May  5 21:22:45 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     
 * 
 * Modifications:
 * $Log$
 */

#ifndef __myarea_h__
#define __myarea_h__

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include <vector>

class map_window;

class MyArea : public Gtk::DrawingArea
{
    const char *file_name;
    bool dirty, empty;
    int xk, yk;
    static MyArea *dnd_tile;
  
  public:
    MyArea(map_window &m, const char *fn, int xk = -1, int yk = -1);
    virtual ~MyArea();

    static std::vector<MyArea *> all_tiles;
    static std::vector<Gtk::TargetEntry> listTargets;
    static void refresh_minmax(void);

    map_window &mw;

    void print(void);
    inline const char *get_fname() { return file_name; }
    inline const Glib::RefPtr<Gdk::Pixbuf> get_pixmap_icon() { return m_image_icon; }
    inline int getX(void) { return xk; }
    inline int getY(void) { return yk; }
    inline void setXY(int x, int y) { xk = x; yk = y; set_dirty(true);}
    inline void getXY(int &x, int &y) { x = getX(); y = getY(); }
    inline bool is_dirty(void) { return dirty; }
    inline void set_dirty(bool d) { dirty = (!is_empty()) ? d: FALSE; } /* empty is never dirty */
    inline bool is_empty(void) { return empty; }
	
    static int xmin, ymin, xmax, ymax;
    static int cr_up, cr_do, cr_le, cr_ri;
    void scale(float sfx, float sfy);
    void xchange_tiles(MyArea &s, MyArea &d);
    bool update_minmax(void);

  protected:
    //Override default signal handler:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    //Other signal handlers:
    bool on_configure_event(GdkEventConfigure *configure_event);
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


#endif /* __myarea_h__ */
