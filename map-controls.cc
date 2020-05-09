// -*-c++-*-
// $Id$
//
// File:		map-controls.cc
// Date:		Fri May  8 18:30:58 2020
// Author:		pottendo (pottendo)
// 
// Abstract:
//      Control widget implementation
//
// Modifications:
// 	$Log$
//
#include <iostream>
#include "map-controls.h"

using namespace::std;

bool map_controls::on_button_press_event(GdkEventButton *) 
{
    cout << "Quit button pressed." << endl;
    return TRUE;
}


map_controls::map_controls(const Glib::ustring &name)
    : Gtk::Frame(name),
      button_quit("Quit")
{
    cout << __FILE__ << name << " created." << endl;
    button_quit.add_events(Gdk::BUTTON_PRESS_MASK);

    add(button_quit);
    show_all_children();
}
