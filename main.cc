/* -*-c++-*-
 * This file is part of vice-mapper.
 * 
 * vice-mapper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * vice-mapper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with vice-mapper.  If not, see <https://www.gnu.org/licenses/>.
 */

/*  File:		main.cc
 *  Date:		Tue May  5 21:53:54 2020
 *  Author:		pottendo (pottendo)
 *  
 *  Abstract:
 *       vice-mapper
 * 
 *  Modifications:
 *  	$Log$
 */ 

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

static gboolean on_delete_event (GtkWidget *window,
                                 GdkEvent  *event,
                                 gpointer   data)
{
    mw_map->commit_changes();
    return false;
}

    
int main(int argc, char** argv)
{
    int argc1 = 1;
    app = Gtk::Application::create(argc1, argv, "org.gtkmm.example");
    builder = Gtk::Builder::create_from_file("./gui.glade");
    gtk_builder_connect_signals(builder->gobj(), NULL);
    //cout << __FUNCTION__ << ": sizeof size_t = " << sizeof(size_t) << endl;
    mw_status = new MyStatus();
    map_window mw;
    mw_map = &mw;

    if (argc > 2) {
	cerr << "Usage: " << argv[0] << " [map-directory]" << endl;
	return 1;
    }
    if (argc == 2) {
	try {
	    mw.reload_unplaced_tiles(argv[1]);
	}
	catch (...) {
	    cerr << "Usage: " << argv[0] << " [map-directory]" << endl;
	    return 1;
	}
    }
    builder->get_widget("ViceMapper", mainWindow);
    g_signal_connect(mainWindow->gobj(), "delete-event",
		     G_CALLBACK(on_delete_event), NULL);

    Gtk::Box *mw_box = nullptr;
    builder->get_widget("MWBox", mw_box);
    mw_box->add(mw);
    mw_box->reorder_child(mw, 1);
    mw_box->show_all();

    (void) mw.load_settings();
	
    return app->run(*mainWindow);    
}
