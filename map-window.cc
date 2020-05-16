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
{
    set_title("Map");

    //map_grid.attach_next_to(ma, m_button_quit, Gtk::POS_BOTTOM, 1, 1);
    empty_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, resX, resY);
    empty_image->fill(0x0000001f);

    ctrls = new map_controls(*this, "Controls");

    //set_default_size(400,400);
    set_border_width(4);

    map_grid.set_hexpand(TRUE);
    map_grid.set_vexpand(TRUE);

    scw.mc = ctrls;
    scw.set_border_width(4);
    scw.set_size_request(100, 100);
    scw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    scw.add(map_grid);
    scw.set_hexpand(TRUE);
    scw.set_vexpand(TRUE);
    Gtk::Frame *map_frame = new Gtk::Frame("Map"); // never deleted, TODO
    //map_frame->set_size_request(1,-1);
    
    map_frame->add(scw);
    map_frame->set_hexpand(TRUE);
    map_frame->set_vexpand(TRUE);

    hbox.set_homogeneous(FALSE);
    hbox.pack_start(*map_frame, TRUE, TRUE, 0);

    hbox.pack_start(*ctrls, FALSE, FALSE, 0);
    add(hbox);

    //add_events(Gdk::SCROLL_MASK);
    show_all_children();
    memset(tiles, 0, 100*100*sizeof(MyArea*));
}

/*
 * this only gets GDK_SCROLL_SMOOTH events in ScrolledWindow
 */
bool
map_window::MyScw::on_scroll_event(GdkEventScroll *scroll_event) 
{
    if (scroll_event->direction == GDK_SCROLL_SMOOTH) {
	GdkScrollDirection dir;
	gdouble dx, dy;
	if (gdk_event_get_scroll_direction((GdkEvent *)scroll_event, &dir)) {
	    cout << __FUNCTION__ << ": direction = " << dir << endl;
	    switch (dir) {
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
		cout << __FUNCTION__ << ": unknown direction: " << scroll_event->direction << endl;
	    }
	}
	else if (gdk_event_get_scroll_deltas((GdkEvent *)scroll_event, &dx, &dy)) {
	    if (dx > 0) scale_factor_x *= 1.05;
	    if (dx < 0) scale_factor_x /= 1.05;
	    if (dy > 0) scale_factor_y *= 1.05;
	    if (dy < 0) scale_factor_y /= 1.05;
	    cout << __FUNCTION__ << ": dx = " << dx << ", dy = " << dy << endl;
	}

    }
    scale_factor_x = MIN(scale_factor_x, 4.0);
    scale_factor_y = MIN(scale_factor_y, 4.0);
    mc->set_zoom(scale_factor_x, scale_factor_y);
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
