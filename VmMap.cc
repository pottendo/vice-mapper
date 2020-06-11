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
 *
 *  $Id$
 * 
 *  File:		map-window.cc
 *  Date:		Tue May  5 22:43:21 2020
 *  Author:		pottendo (pottendo)
 *  
 *  Abstract:
 *       map-window
 * 
 *  Modifications:
 *  	$Log$
 */

#include <gtkmm/button.h>
#include <gtkmm/widget.h>
#include <gtkmm/frame.h>
#include <gtkmm/filechooserdialog.h>
#include <glibmm/fileutils.h>
#include <iostream>
#include "VmMap.h"
#include "VmMapControls.h"
#include "dialogs.h"

using namespace::std;
Glib::RefPtr<Gdk::Pixbuf> VmMap::empty_image;
double VmMap::scale_factor_x = def_zoom;
double VmMap::scale_factor_y = def_zoom;
int VmMap::nr_tiles = 0;
string VmMap::current_path="";
string VmMap::current_ext="";
string VmMap::current_basename=def_basename;

VmMap::VmMap()
    : dirty(false)
{
    //set_title("Map");

    //map_grid.attach_next_to(ma, m_button_quit, Gtk::POS_BOTTOM, 1, 1);
    empty_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, def_resX, def_resY);
    empty_image->fill(0x0000001f);

//    ctrls = new VmMapControls(*this, "Controls");
    mw_ctrls = ctrls = new VmMapControls(*this);
        
    //set_default_size(400,400);
    set_border_width(4);

    map_grid.set_hexpand(TRUE);
    map_grid.set_vexpand(TRUE);

    scw.mc = ctrls;
    scw.set_border_width(4);
    //scw.set_size_request(100, 100);
    scw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    scw.add(map_grid);
    scw.set_hexpand(TRUE);
    scw.set_vexpand(TRUE);
    
    Gtk::Frame *map_frame = nullptr;
    builder->get_widget("VmMap", map_frame);
    
    map_frame->add(scw);
    //map_frame->set_hexpand(TRUE);
    //map_frame->set_vexpand(TRUE);

    /*
    hbox.set_homogeneous(FALSE);
    hbox.pack_start(*map_frame, TRUE, TRUE, 0);

    hbox.pack_start(*ctrls->get_widget(), FALSE, FALSE, 0);
    add(hbox);
    */

    //add_events(Gdk::SCROLL_MASK);
    show_all_children();
    memset(tiles, 0, 101*101*sizeof(VmTile*));
}

VmMap::MyScw::MyScw()
{
}

/*
 * this only gets GDK_SCROLL_SMOOTH events in ScrolledWindow
 */
bool
VmMap::MyScw::on_scroll_event(GdkEventScroll *scroll_event) 
{
    if (scroll_event->direction == GDK_SCROLL_SMOOTH) {
	GdkScrollDirection dir;
	gdouble dx, dy;
	if (gdk_event_get_scroll_direction((GdkEvent *)scroll_event, &dir)) {
	    // mw_out << __FUNCTION__ << ": direction = " << dir << endl;
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
		mw_out << __FUNCTION__ << ": unknown direction: " << scroll_event->direction << endl;
	    }
	}
	else if (gdk_event_get_scroll_deltas((GdkEvent *)scroll_event, &dx, &dy)) {
	    if (dx > 0) scale_factor_x *= 1.05;
	    if (dx < 0) scale_factor_x /= 1.05;
	    if (dy > 0) scale_factor_y *= 1.05;
	    if (dy < 0) scale_factor_y /= 1.05;
	    //mw_out << __FUNCTION__ << ": dx = " << dx << ", dy = " << dy << endl;
	}

    }
    scale_factor_x = MIN(scale_factor_x, 6.0);
    scale_factor_y = MIN(scale_factor_y, 6.0);
    mc->set_zoom(scale_factor_x, scale_factor_y);
    return TRUE;
}

bool
VmMap::MyScw::on_motion_notify_event(GdkEventMotion* motion_event) 
{
    static gdouble xp, yp;
    
    if (space_modifier /* && button_press*/) {
	gdouble dx, dy, xa, ya;
	
	dx = (motion_event->x - xp);
	dy = (motion_event->y - yp);
	/*
	mw_out << __FUNCTION__ << ": space+mouse-move: hadj = " << get_hadjustment()->get_value()
	     << "- dx: " << dx << ",dy: " << dy << ", xp: " << xp << ",yp: " << yp << endle;
	*/
	xa = get_hadjustment()->get_value() - dx;
	ya = get_vadjustment()->get_value() - dy;
	get_hadjustment()->set_value(xa);
	get_vadjustment()->set_value(ya);
    }
    xp = motion_event->x;	// track last location even if not panning
    yp = motion_event->y;
    mw_status->status();
    return Gtk::ScrolledWindow::on_motion_notify_event(motion_event);
}

bool
VmMap::MyScw::on_enter_notify_event(GdkEventCrossing* crossing_event) 
{
    grab_focus();		// needed if e.g. controls took the focus
    space_modifier = false;
    return Gtk::ScrolledWindow::on_enter_notify_event(crossing_event);
}

bool
VmMap::MyScw::on_key_press_event(GdkEventKey *key_event) 
{
    if (key_event->keyval == GDK_KEY_space) {
	space_modifier = true;
	if (!move_cursor)	// cannot initialize in constructor, as window is not realized at the time
	    move_cursor = Gdk::Cursor::create(this->get_window()->get_display(),Gdk::CROSSHAIR);
	this->get_window()->set_cursor(move_cursor);
    }
    return Gtk::ScrolledWindow::on_key_press_event(key_event);
}

bool
VmMap::MyScw::on_key_release_event(GdkEventKey *key_event) 
{
    if (key_event->keyval == GDK_KEY_space) {
	space_modifier = false;
	this->get_window()->set_cursor();
    }
    return Gtk::ScrolledWindow::on_key_release_event(key_event);
}

/*
bool
map_window::MyScw::on_button_press_event(GdkEventButton* button_event) 
{
    button_press = TRUE;
    mw_out << "button press" << endl;
    Gtk::ScrolledWindow::on_button_press_event(button_event);
    return TRUE;
}

bool
map_window::MyScw::on_button_release_event(GdkEventButton* button_event) 
{
    button_press = FALSE;
    mw_out << "button rel" << endl;
    Gtk::ScrolledWindow::on_button_release_event(button_event);
    return TRUE;
}
*/

void
VmMap::add_tile(VmTile *a) 
{
    map_grid.attach(*a, a->getX(), a->getY());
    tiles[a->getX()][a->getY()] = a;
    a->scale(scale_factor_x, scale_factor_y); // make sure tile adjusts to current scaling
    //nr_tiles++;
    map_grid.show_all_children();
}

void
VmMap::remove_tile(VmTile *a, bool map_remove)
{
    // mw_out << __FUNCTION__ << ": " << map_remove << " called for " << *a << endl;
    
    if (a->getX() < 0) {
	ctrls->remove_tile(a);
    }
    else {
	map_grid.remove(*a);
	if (!map_remove) {
	    // placed tile
	    VmTile *new_empty = new VmTile(*this, NULL, a->getX(), a->getY());
	    //map_grid.attach(*new_empty, a->getX(), a->getY());
	    add_tile(new_empty);
	    mw_out << __FUNCTION__ << ": attached new " << *new_empty << endl;
	    resize_map();
	}
	else
	    tiles[a->getX()][a->getY()] = nullptr;
    }
    auto e = VmTile::all_tiles.erase(a);
    if (e > 1) {
	cerr << __FUNCTION__ << ": erased " << e << " tiles." << endl;
    }
    nr_tiles = VmTile::all_tiles.size();
    show_all_children();
}

void
VmMap::reload_unplaced_tiles(char *path)
{
    string cp = (path != NULL) ? string(path) : current_path;
    
    if (cp == "") return;
    Glib::Dir dir(cp);
    std::list<std::string> entries (dir.begin(), dir.end());

    for (auto e = entries.begin(); e != entries.end(); ++e) {
	string fp = cp + G_DIR_SEPARATOR_S + *e;
	VmTile *t = VmTile::lookup_by_name(fp);
	if (t) continue;	// skip if a tile with the filename exists
	mw_out << __FUNCTION__ << ": new tile: " << fp << endl;
	try {
	    t = new VmTile(*this, fp.c_str());
	}
	catch (...) {
	    continue;		// ignore non-tiles
	}
    }
    VmTile::refresh_minmax();
    fill_empties();
    mw_status->show(VmStatus::STATM, current_path);
    if (!VmTile::tiles_placed && tiles[map_max / 2][map_max / 2] == nullptr)
	add_tile(new VmTile(*this, NULL, map_max / 2, map_max / 2));
}

void
VmMap::save_settings(void) 
{
    string &cp = current_path;
    if (cp == "") return;
    size_t ext_pos = def_basename.find_last_of(".");
    string cfg_fname = cp + G_DIR_SEPARATOR_S + def_basename.substr(0, ext_pos) + def_cfg_ext;
    
    mw_out << __FUNCTION__ << ": cfg file = " << cfg_fname << endl;
     
    Glib::RefPtr<Gio::File> cfg_file = Gio::File::create_for_path(cfg_fname);
    try {
	cfg_file->remove();
    }
    catch (...) {};		// ignore if not exists

    Glib::RefPtr<Gio::FileOutputStream> cfg_stream;
    
    try {
	cfg_stream = cfg_file->create_file();
    }
    catch (Gio::Error &e) {
	cerr << __FUNCTION__ << ": *** cfg file not written: " << e.what() << endl;
	return;
    }

    string head = string("# vice-mapper settings for for '" + def_basename + "'\n" +
			 "MAPV=" + mapper_version + "\n");
    cfg_stream->write(head.c_str(), head.size());
    string lines=
	string("CRUP=" + to_string(VmTile::cr_up) + "\n"
	       "CRDO=" + to_string(VmTile::cr_do) + "\n"
	       "CRLE=" + to_string(VmTile::cr_le) + "\n"
	       "CRRI=" + to_string(VmTile::cr_ri) + "\n" 
	       "ZOOMX=" + to_string(scale_factor_x) + "\n"
	       "ZOOMY=" + to_string(scale_factor_y) + "\n"
	       "#END - DONT_REMOVE_THIS_LINE\n"
	    );
    
    cfg_stream->write(lines.c_str(), lines.size());
    cfg_stream->flush();
}

bool
VmMap::process_line(string l) 
{
    //mw_out << __FUNCTION__ << ": processing line '" << l << "'" << endl;
    if (l.find("#END") < l.size())
	return false;

    try {
	if (l.find("CRUP") < l.size()) VmTile::cr_up = stoi(l.substr(5));
	if (l.find("CRDO") < l.size()) VmTile::cr_do = stoi(l.substr(5));
	if (l.find("CRLE") < l.size()) VmTile::cr_le = stoi(l.substr(5));
	if (l.find("CRRI") < l.size()) VmTile::cr_ri = stoi(l.substr(5));
	if (l.find("ZOOMX") < l.size()) { scale_factor_x = stod(l.substr(6)); }
	if (l.find("ZOOMY") < l.size()) { scale_factor_y = stod(l.substr(6)); }
    }
    catch (std::invalid_argument &e) {
	cerr << e.what() << endl;
    }
    
    return true;
}

bool
VmMap::load_settings(void) 
{
    string &cp = current_path;
    if (cp == "") return false;
    size_t ext_pos = def_basename.find_last_of(".");
    string cfg_fname = cp + G_DIR_SEPARATOR_S + def_basename.substr(0, ext_pos) + def_cfg_ext;
    
    mw_out << __FUNCTION__ << ": cfg file = " << cfg_fname << endl;
     
    Glib::RefPtr<Gio::File> cfg_file = Gio::File::create_for_path(cfg_fname);
    if (!cfg_file) {
	mw_out << __FUNCTION__ << ": can't open cfg file for map." << endl;
	return false;
    }
    
    Glib::RefPtr<Gio::FileInputStream> cfg_stream;
    try {
	cfg_stream = cfg_file->read();
	char cfg_buffer[1024];
	gsize bytes_read;
	if (cfg_stream->read_all(cfg_buffer, 1024, bytes_read) == false) {
	    cerr << __FUNCTION__ << ": read error." << endl;
	    return false;
	}
	string cfg(cfg_buffer);
	int lstart = 0;
	int lend;
	
	do {
	    lend = cfg.substr(lstart).find_first_of("\n");
	    if (!process_line(cfg.substr(lstart, lend)))
		break;
	    lstart = lstart + lend + 1;
	} while(true);
	ctrls->set_zoom(scale_factor_x, scale_factor_y, false);
	ctrls->set_crop(VmTile::cr_up, VmTile::cr_do, VmTile::cr_le, VmTile::cr_ri, false);
    }
    catch (Gio::Error &e) {
	cerr << __FUNCTION__ << ": *** cfg file not read: " << e.what() << endl;
	return false;
    }
    return true;
}

void
VmMap::fill_empties() 
{
    int x, y;
/*
    mw_out << "X:" << VmTile::xmin << "-" << VmTile::xmax
	 << "Y:" << VmTile::ymin << "-" << VmTile::ymax << endl;
*/
    try {
	for (x = VmTile::xmin; x <= VmTile::xmax; x++) {
	    for (y = VmTile::ymin; y <= VmTile::ymax; y++) {
		if (tiles[x][y] == NULL) {
		    add_tile(new VmTile(*this, NULL, x, y));
		}
	    }
	}
    }
    catch (std::exception &e) {
	mw_out << __FUNCTION__ << ": failed with exception " << e.what() << endl;
	return;
    }
}

void
VmMap::scale_all(void) 
{
    int x, y;
    for (x = VmTile::xmin; x <= VmTile::xmax; x++) {
	for (y = VmTile::ymin; y <= VmTile::ymax; y++) {
	    if (tiles[x][y] != NULL) {
		tiles[x][y]->scale(scale_factor_x, scale_factor_y);
	    }
	}
    }
}

int
VmMap::get_empty_area(int from_x, int from_y, int to_x, int to_y)
{
    int x, y;
    int do_scratch = 0;
    VmTile *t;
    std::vector<VmTile *> scratch;
    
    for (y = from_y; y < to_y; y++) {
	for (x = from_x; x < to_x; x++) {
	    if (tiles[x][y]) {
		if (!(t = tiles[x][y])->is_empty()) {
		    break;
		}
		scratch.push_back(t);
		do_scratch++;
	    }
	}
    }
    if (do_scratch) {
	mw_out << "found " << do_scratch << " empty tiles to remove." << endl;
	std::for_each(scratch.begin(), scratch.end(),
		      [](VmTile *x)->void {
			  if (x) {
			      x->mw.map_grid.remove(*x);
			      x->mw.tiles[x->getX()][x->getY()] = NULL;
			      delete x;
			  }
		      });
    }
    return do_scratch;
}

void
VmMap::xchange_tiles(VmTile *s, VmTile *d)
{
    int tsx, tsy, tdx, tdy;
    VmTile *t;
    s->getXY(tsx, tsy);		// get x,y
    d->getXY(tdx, tdy);

    /*
    if ((tsx < 0) && (tsy < 0)) {
	mw_out << "*** shoud not happen !!! - placed from/to unplaced tiles." << endl;
	return;
    }
    */
    if (tdx < 0) {
	mw_out << "*** should not happen !!! - move to unplaced tiles";
	//special case - add to list of unplace tiles
	return;
	
    }
    
    if (tsx < 0) {
	//mw_out << "remove from unplaced tiles";
	mw_out << *s << endl;
	ctrls->remove_tile(s);
    }
    if ((t = (VmTile *) map_grid.get_child_at(s->getX(), s->getY())) != NULL) {
	//mw_out << "removing: ";
	mw_out << *t << endl;
	map_grid.remove(*t);
    }
    if ((t = (VmTile *) map_grid.get_child_at(d->getX(), d->getY())) != NULL) {
	mw_out << "removing: " << *t << endl;
	map_grid.remove(*t);
    }
	
    s->setXY(tdx, tdy);
    add_tile(s);
    s->sync_tile();
    d->setXY(tsx, tsy);
    if (tsx > 0) {
	add_tile(d);
	d->sync_tile();
    }
    else {
	if (!d->is_empty()) {	// pushback if we placed on occupied tile
	    ctrls->add_tile(d);
	    d->sync_tile();
	}
	else {
	    delete d;		// dispose unused empty tile
	}
    }
    
    // check if we need to grow/shrink
    if (s->update_minmax() || d->update_minmax())
	fill_empties(); // already placed therefore s(ource)!
    resize_map();
}

/*
 * map functions
 */
void
VmMap::resize_map(void)
{
    int do_scratch = 0;
    VmTile::refresh_minmax();
    do_scratch += get_empty_area(0, 0, map_max+1, VmTile::ymin);
    do_scratch += get_empty_area(0, VmTile::ymax+1, map_max+1, map_max+1);
    do_scratch += get_empty_area(0, 0, VmTile::xmin, map_max+1);
    do_scratch += get_empty_area(VmTile::xmax+1, 0, map_max+1, map_max+1);
    if (do_scratch) {
	mw_out << "found " << do_scratch << " empty tiles to remove." << endl;
    }
}

void
VmMap::remove_map(void) 
{
    mw_out << __FUNCTION__ << ": called." << endl;
    std::vector<Gtk::Widget *> to_scratch = map_grid.get_children();
    std::for_each(to_scratch.begin(), to_scratch.end(),
		  [this](Gtk::Widget *t)->void {
		      remove_tile(static_cast<VmTile*>(t), true);
		      delete static_cast<VmTile*>(t);
		  });
    
    for (auto t : VmTile::all_tiles) {
	delete t;
	VmTile::all_tiles.erase(t);
	nr_tiles--;
    }
    //nr_tiles = VmTile::all_tiles.size();
    mw_out << __FUNCTION__ << ": pending tiles = " << nr_tiles
	 << ", alloc_count = " << VmTile::alloc_count << endl;
    mw_status->clear();
    set_dirty(false);
    VmTile::tiles_placed = false;
    show_all_children();
}

void
VmMap::open_map(void) 
{ 
    Gtk::FileChooserDialog d("Select map from folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    d.set_transient_for(*mainWindow);

    d.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    d.add_button("Select", Gtk::RESPONSE_OK);
    if (d.run() != Gtk::RESPONSE_OK)
	return;
    
    mw_out << __FUNCTION__ << ": load map from '" <<  d.get_filename() << "'." << endl;
    remove_map();
    current_path = d.get_filename();
    reload_unplaced_tiles();
    if (!load_settings()) {
	ctrls->set_crop(def_cry, def_cry, def_crx, def_crx);
	ctrls->set_zoom(def_zoom, def_zoom);
    }
}
