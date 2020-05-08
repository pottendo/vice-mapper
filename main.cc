// -*-c++-*-
// $Id$
//
// File:		main.cc
// Date:		Tue May  5 11:53:54 2020
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

int main(int argc, char** argv)
{
    int argc1 = 1;
    auto app = Gtk::Application::create(argc1, argv, "org.gtkmm.example");

    if (argc <= 1) {
	cerr << "usage: ..." << endl;
	exit(1);
    }

    map_window mw;
    MyArea *tile;
    for (int i = 1; i < argc; i++) {
	tile = new MyArea(argv[i]);
	mw.add_tile(tile);
    }
    tile = new MyArea(NULL, 49, 49);
    mw.add_tile(tile);
    mw.fill_empties();
    
    return app->run(mw);
}
