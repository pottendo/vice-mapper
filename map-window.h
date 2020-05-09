/*  -*-c-*-
 * File:		map-window.h
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
#include "myarea.h"

class map_window : public Gtk::Window
{
    // Signal handlers:
    bool on_scroll_event(GdkEventScroll *scroll_event) override;
    void scale_all(float sf);
  
    // Child widgets:
    Gtk::Grid map_grid;
    Gtk::ScrolledWindow scw;
    Gtk::HBox hbox;		/* main layout box */
    Glib::RefPtr<Gdk::Pixbuf> m_empty;
    MyArea *tiles[100][100];
    float scale_factor = 3.0;

  public:
    map_window();
    virtual ~map_window() {};

    void add_tile(MyArea *);
    void fill_empties();
    static Glib::RefPtr<Gdk::Pixbuf> empty_image;
};

#endif /* __map_window_h__ */
