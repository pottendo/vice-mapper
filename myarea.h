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

class MyArea : public Gtk::DrawingArea
{
    const char *file_name;
    int xk;
    int yk;
    static std::vector<Gtk::TargetEntry> listTargets;
    static Glib::RefPtr<Gdk::Pixbuf> dnd_image;
  
  public:
    MyArea(const char *fn, int xk = -1, int yk = -1);
    virtual ~MyArea();

    static std::vector<MyArea *> all_tiles;

    void print(void);
    inline int getX(void) { return xk; }
    inline int getY(void) { return yk; }
    static int xmin, ymin, xmax, ymax;
    static int cr_up, cr_do, cr_le, cr_ri;
    void scale(float sfx, float sfy);

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
};


#endif /* __myarea_h__ */
