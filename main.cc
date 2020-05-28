// -*-c++-*-
// $Id$
//
// File:		main.cc
// Date:		Tue May  5 21:53:54 2020
// Author:		pottendo (pottendo)
// 
// Abstract:
//      play with gtkmm, cairo, images
//
// Modifications:
// 	$Log$
//

#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm.h>
#include "myarea.h"
#include "map-window.h"

using namespace::std;
Glib::RefPtr<Gtk::Builder> builder; // global used by all GUI elements
Glib::RefPtr<Gtk::Application> app;
MyStatus *mw_status = nullptr;
map_window *mw_map = nullptr;
Gtk::Window *mainWindow = nullptr;

int main(int argc, char** argv)
{
    int argc1 = 1;
    app = Gtk::Application::create(argc1, argv, "org.gtkmm.example");
    builder = Gtk::Builder::create_from_file("./gui.glade");
    gtk_builder_connect_signals(builder->gobj(), NULL);
    mw_status = new MyStatus();
    map_window mw;
    MyArea *tile;
    for (int i = 1; i < argc; i++) {
	try {
	    tile = new MyArea(mw, argv[i]);
	}
	catch (...) { continue;	} // ignore non-tiles
    }
    if (mw.get_nrtiles() == 0) {
	tile = new MyArea(mw, NULL, 50, 50);
	mw.add_tile(tile);
    }
    mw.fill_empties();
    mw_map = &mw;

    builder->get_widget("ViceMapper", mainWindow);

    Gtk::Box *mw_box = nullptr;
    builder->get_widget("MWBox", mw_box);
    mw_box->add(mw);
    mw_box->reorder_child(mw, 1);

    
    mw_box->show_all();
    
    return app->run(*mainWindow);    

    //return app->run(mw);
}
