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
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/widget.h>
#include "map-window.h"
#include <iostream>

using namespace::std;
Glib::RefPtr<Gdk::Pixbuf> map_window::empty_image;

map_window::map_window()
{
    set_title("Map");
    set_default_size(400,400);
    set_border_width(12);

    map_grid.set_hexpand(TRUE);
    map_grid.set_vexpand(TRUE);

    scw.set_border_width(10);
    scw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    scw.add(map_grid);
    scw.set_hexpand(TRUE);
    scw.set_vexpand(TRUE);
    add(scw);

    add_events(Gdk::SCROLL_MASK);
    //map_grid.attach_next_to(ma, m_button_quit, Gtk::POS_BOTTOM, 1, 1);
    empty_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, 384, 272);
    empty_image->fill(0x0000001f);
    show_all_children();
    memset(tiles, 0, 100*100*sizeof(MyArea*));
}

bool
map_window::on_scroll_event(GdkEventScroll *scroll_event) 
{
    switch(scroll_event->direction) {
    case GDK_SCROLL_UP:
	scale_factor *= 1.05;
	scale_all(scale_factor);
	break;
    case GDK_SCROLL_DOWN:
	scale_factor /= 1.05;
	scale_all(scale_factor);
	break;
    default:
	;
	//cout << scroll_event->direction;
    }
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
    cout << "X:" << MyArea::xmin << "-" << MyArea::xmax
	 << "Y:" << MyArea::ymin << "-" << MyArea::ymax << endl;
	
    for (x = MyArea::xmin; x <= MyArea::xmax; x++) {
	for (y = MyArea::ymin; y <= MyArea::ymax; y++) {
	    if (tiles[x][y] == NULL) {
		add_tile(new MyArea(NULL, x, y));
	    }
	}
    }
}

void
map_window::scale_all(float sf) 
{
    int x, y;
    for (x = MyArea::xmin; x <= MyArea::xmax; x++) {
	for (y = MyArea::ymin; y <= MyArea::ymax; y++) {
	    if (tiles[x][y] != NULL) {
		tiles[x][y]->scale(sf);
	    }
	}
    }
}
