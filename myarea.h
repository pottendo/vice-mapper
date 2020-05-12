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
    int xk, yk;
    map_window &mw;
    static Glib::RefPtr<Gdk::Pixbuf> dnd_image;
  
  public:
    MyArea(map_window &m, const char *fn, int xk = -1, int yk = -1);
    virtual ~MyArea();

    static std::vector<MyArea *> all_tiles;
    static std::vector<Gtk::TargetEntry> listTargets;

    void print(void);
    inline int getX(void) { return xk; }
    inline int getY(void) { return yk; }
    static int xmin, ymin, xmax, ymax;
    static int cr_up, cr_do, cr_le, cr_ri;
    void scale(float sfx, float sfy);
    inline const char *get_fname() { return file_name; }
    inline const Glib::RefPtr<Gdk::Pixbuf> get_pixmap_icon() { return m_image_icon; }

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
