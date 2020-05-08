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

class MyArea : public Gtk::DrawingArea
{
    const char *file_name;
    int xk;
    int yk;
    static const int cr_up=72, cr_do=36+5, cr_le=32+10, cr_ri=32+10;
    static std::vector<Gtk::TargetEntry> listTargets;
    static Glib::RefPtr<Gdk::Pixbuf> dnd_image;
  
  public:
    MyArea(const char *fn, int xk = -1, int yk = -1);
    virtual ~MyArea();

    void print(void);
    inline int getX(void) { return xk; }
    inline int getY(void) { return yk; }
    static int xmin, ymin, xmax, ymax;
    void scale(float sf);

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
