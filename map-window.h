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
#include "map-controls.h"

const int map_max = 100;	/* 100x100 tiles should be enough */
const int resX = 384;		/* typical C64 screen resolution */
const int resY = 272;

class map_window : public Gtk::Window
{
  protected:
    class MyScw : public Gtk::ScrolledWindow
    {
	// Signal handlers:
	bool on_scroll_event(GdkEventScroll *scroll_event) override;
      public:
	MyScw() { add_events(Gdk::SCROLL_MASK); };
	map_controls *mc;
    };
    // Child widgets:
    Gtk::Grid map_grid;
    MyScw scw;
    Gtk::HBox hbox;		/* main layout box */
    Glib::RefPtr<Gdk::Pixbuf> m_empty;
    MyArea *tiles[map_max][map_max];
    map_controls *ctrls;

  public:
    map_window();
    virtual ~map_window() {};

    void add_tile(MyArea *);
    void add_unplaced_tile(MyArea *t) { ctrls->add_tile(t); }
	
    void fill_empties();
    void scale_all(void);
    static Glib::RefPtr<Gdk::Pixbuf> empty_image;
    static double scale_factor_x;
    static double scale_factor_y;
};

#endif /* __map_window_h__ */
