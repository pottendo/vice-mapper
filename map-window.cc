// -*-c++-*-
// $Id$
//
// File:		map-window.cc
// Date:		Tue May  5 22:43:21 2020
// Author:		pottendo (pottendo)
// 
// Abstract:
//      map-window
//
// Modifications:
// 	$Log$
//
#include <gtkmm/button.h>
#include <gtkmm/widget.h>
#include <gtkmm/frame.h>
#include "map-window.h"
#include "map-controls.h"
#include <iostream>

using namespace::std;
Glib::RefPtr<Gdk::Pixbuf> map_window::empty_image;
double map_window::scale_factor_x = 3.0;
double map_window::scale_factor_y = 3.0;
//map_window *map_window::mw;

map_window::map_window()
    : ctrls(*this, "Controls")
{
    set_title("Map");
    //set_default_size(400,400);
    set_border_width(4);

    map_grid.set_hexpand(TRUE);
    map_grid.set_vexpand(TRUE);
    
    scw.set_border_width(4);
    scw.set_size_request(200, 300);
    scw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    scw.add(map_grid);
    scw.set_hexpand(TRUE);
    scw.set_vexpand(TRUE);
    Gtk::Frame *map_frame = new Gtk::Frame("Map"); // never deleted, TODO
    map_frame->add(scw);
    hbox.add(*map_frame);

    //ctrl_frame->set_size_request(200, -1);
    hbox.add(ctrls);
    add(hbox);

    add_events(Gdk::SCROLL_MASK);
    //map_grid.attach_next_to(ma, m_button_quit, Gtk::POS_BOTTOM, 1, 1);
    empty_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, resX, resY);
    empty_image->fill(0x0000001f);
    show_all_children();
    memset(tiles, 0, 100*100*sizeof(MyArea*));
}

bool
map_window::on_scroll_event(GdkEventScroll *scroll_event) 
{
    switch(scroll_event->direction) {
    case GDK_SCROLL_UP:
	scale_factor_x *= 1.05;
	scale_factor_y *= 1.05;
	break;
    case GDK_SCROLL_DOWN:
	scale_factor_x /= 1.05;
	scale_factor_y /= 1.05;
	break;
    default:
	;
	//cout << scroll_event->direction;
    }
    scale_factor_x = MIN(scale_factor_x, 4.0);
    scale_factor_y = MIN(scale_factor_y, 4.0);
    ctrls.set_zoom(scale_factor_x, scale_factor_y);
    return TRUE;
}

void
map_window::add_tile(MyArea *a) 
{
    map_grid.attach(*a, a->getX(), a->getY());
    tiles[a->getX()][a->getY()] = a;
    show_all_children();
}

void
map_window::fill_empties() 
{
    int x, y;
/*    
    cout << "X:" << MyArea::xmin << "-" << MyArea::xmax
	 << "Y:" << MyArea::ymin << "-" << MyArea::ymax << endl;
*/  
    for (x = MyArea::xmin; x <= MyArea::xmax; x++) {
	for (y = MyArea::ymin; y <= MyArea::ymax; y++) {
	    if (tiles[x][y] == NULL) {
		add_tile(new MyArea(*this, NULL, x, y));
	    }
	}
    }
}

void
map_window::scale_all(void) 
{
    int x, y;
    for (x = MyArea::xmin; x <= MyArea::xmax; x++) {
	for (y = MyArea::ymin; y <= MyArea::ymax; y++) {
	    if (tiles[x][y] != NULL) {
		tiles[x][y]->scale(scale_factor_x, scale_factor_y);
	    }
	}
    }
}
