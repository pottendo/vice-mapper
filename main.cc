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
#include "VmTile.h"
#include "VmMap.h"
#include "dialogs.h"

using namespace::std;
Glib::RefPtr<Gtk::Builder> builder; // global used by all GUI elements
Glib::RefPtr<Gtk::Application> app;
VmStatus *mw_status = nullptr;
VmMap *mw_map = nullptr;
VmMapControls *mw_ctrls = nullptr;
VmDebug *mw_debug = nullptr;
VmDebug *mw_debug2 = nullptr;
std::ostream *mw_out_stream, *mw_err_stream;
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
    try {
	builder = Gtk::Builder::create_from_file("./gui.glade");
    }
    catch (Gtk::BuilderError &ex) {
	std::cerr << __FUNCTION__ << ": builder failed - " << ex.what() << endl;
	return -1;
    }
    
    gtk_builder_connect_signals(builder->gobj(), NULL);
    //cout << __FUNCTION__ << ": sizeof size_t = " << sizeof(size_t) << endl;
    srand(time(NULL));		// initialize random generator for filenames
    mw_status = new VmStatus();
    mw_out_stream = new std::ostream(mw_debug = new VmDebug(MW_OUT));
    mw_err_stream = new std::ostream(mw_debug2 = new VmDebug(MW_ERR));
    VmMap mw;
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
	    mw_err << "Usage: " << argv[0] << " [map-directory]" << endl;
	    return 1;
	}
    }
    builder->get_widget("ViceMapper", mainWindow);
    g_signal_connect(mainWindow->gobj(), "delete-event",
		     G_CALLBACK(on_delete_event), NULL);

    Gtk::Box *mw_box = nullptr;
    builder->get_widget("MWBox", mw_box);
    mw_box->show_all();
    (void) mw.load_settings();
	
    return app->run(*mainWindow);    
}
