/*  -*-c-*-
 * File:		map-window.h
 * Date:		Tue May  5 14:44:11 2020
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
#include "myarea.h"

class map_window : public Gtk::Window
{
public:
  map_window();
  virtual ~map_window() {};

  void add_tile(MyArea *);
  void fill_empties();
  static Glib::RefPtr<Gdk::Pixbuf> empty_image;
  
private:
  // Signal handlers:
  void on_button_quit();
  bool on_scroll_event(GdkEventScroll *scroll_event);
  void scale_all(float sf);
  
  // Child widgets:
  Gtk::Grid map_grid;
  Gtk::ScrolledWindow scw;
  Glib::RefPtr<Gdk::Pixbuf> m_empty;
  MyArea *tiles[100][100];
  float scale_factor = 3.0;
};



#endif /* __map_window_h__ */
