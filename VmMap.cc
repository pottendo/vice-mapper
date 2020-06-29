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
#include <gtkmm/pagesetup.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <cairomm/context.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <iostream>
#include <regex>
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
    : dirty(false),
      tiles_placed(false)
{
    //set_title("Map");

    //map_grid.attach_next_to(ma, m_button_quit, Gtk::POS_BOTTOM, 1, 1);
    //empty_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, def_resX, def_resY);
    //empty_image->fill(0x0000001f);

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

    preview_win = nullptr;
    builder->get_widget("VmMapPreview", preview_win);
    file_chooser = nullptr;
    builder->get_widget("VmMapPreviewFileChooser", file_chooser);

    Gtk::Frame *prev_frame = nullptr;
    builder->get_widget("VmMapPreviewFrame", prev_frame);
    map_preview = new MapPreview();
    map_preview->show();
    prev_frame->add(*map_preview);
    prev_frame->show();
    
    Gtk::Frame *map_frame = nullptr;
    builder->get_widget("VmMap", map_frame);
    map_frame->add(scw);
    show_all_children();
    memset(tiles, 0, 101*101*sizeof(VmTile*));

    pr_settings = Gtk::PrintSettings::create();
    pr_pagesettings = Gtk::PageSetup::create(); 
}

VmMap::~VmMap() 
{
    mw_out << __FUNCTION__ << ": main map delete." << endl;
    delete map_preview;
    delete mw_ctrls;
    mw_debug->save();
    delete mw_out_stream;	// a bit ugly to delete here not in main. TODO: Fixme.
    delete mw_err_stream;
    delete mw_debug;
    delete mw_debug2;
    delete mw_status;
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
VmMap::update_state(void) 
{
    tiles_placed = dirty = false;
    for (auto it = begin(VmTile::all_tiles);
	 (!dirty || !tiles_placed) && it != end(VmTile::all_tiles);
	 ++it) {
	if ((*it)->is_empty()) continue;
	dirty |= (*it)->is_dirty();
	tiles_placed |= ((*it)->getX() > 0) ? true : false;
    }
    set_dirty(dirty);
}

void
VmMap::add_tile(VmTile *a) 
{
    map_grid.attach(*a, a->getX(), a->getY());
    tiles[a->getX()][a->getY()] = a;
    a->scale(scale_factor_x, scale_factor_y); // make sure tile adjusts to current scaling
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
	    add_tile(new_empty);
	    mw_out << __FUNCTION__ << ": attached new " << *new_empty << endl;
	    resize_map();
	}
	else
	    tiles[a->getX()][a->getY()] = nullptr;
    }
    auto e = VmTile::all_tiles.erase(a);
    if (e > 1) {
	mw_err << __FUNCTION__ << ": erased " << e << " tiles." << endl;
    }
    nr_tiles = VmTile::all_tiles.size();
    update_state();
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
    if (!empty_image ||
	((empty_image->get_width() != VmTile::max_resX) || (empty_image->get_height() != VmTile::max_resY))) {
	// create a new empty_image if not yet done or if a new max has been detected
	empty_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, VmTile::max_resX, VmTile::max_resY);
	empty_image->fill(0x0000001f);
    }
    ctrls->update_ctrls();
    VmTile::refresh_minmax();
    fill_empties();
    mw_status->show(VmStatus::STATM, current_path);
    update_state();
    if (!tiles_placed && tiles[map_max / 2][map_max / 2] == nullptr)
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
	mw_err << __FUNCTION__ << ": *** cfg file not written: " << e.what() << endl;
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
	mw_err << e.what() << endl;
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
	    mw_err << __FUNCTION__ << ": read error." << endl;
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
	mw_err << __FUNCTION__ << ": *** cfg file not read: " << e.what() << endl;
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
	mw_out << __FUNCTION__ << ": removing (unplaced) " << *s << endl;
	ctrls->remove_tile(s);
    }
    if ((t = (VmTile *) map_grid.get_child_at(tsx, tsy)) != NULL) {
	mw_out << __FUNCTION__ << ": removing (placed) " << *t << endl;
	map_grid.remove(*t);
    }
    if ((t = (VmTile *) map_grid.get_child_at(tdx, tdy)) != NULL) {
	mw_out << __FUNCTION__ << ": removing (placed) " << *t << endl;
	map_grid.remove(*t);
    }
	
    s->setXY(tdx, tdy);
    add_tile(s);
    s->sync_tile();
    tiles_placed=true;
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
    tiles_placed = false;
    VmTile::min_resX = 100000;
    VmTile::min_resY = 100000;
    VmTile::max_resX = -1;
    VmTile::max_resY = -1;
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

void
VmMap::export_map(void) 
{
    //mw_out << __FUNCTION__ << ": called." << endl;
    update_state();
    if (!tiles_placed) {
	mw_out << __FUNCTION__ << ": no tiles placed, nothing to export." << endl;
	return;
    }
    
    map_preview->set_header("Map: " + current_path);
    auto t = std::chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(t);
    struct tm * p = localtime(&tt);
    char ttc[256];
    strftime(ttc, 256, "%c", p);
    map_preview->set_footer(string("mapped on ") +
			    ttc + " by " +
			    Glib::get_user_name());
    map_preview->render_preview();
    preview_win->show_all_children();
    preview_win->show();
    Glib::RefPtr<Gio::File> fnp = Gio::File::create_for_path(current_path);
    string s = string(current_path) + G_DIR_SEPARATOR_S + fnp->get_basename() + ".png";
    
    mw_out << __FUNCTION__ << ": " << s << endl;
    file_chooser->set_current_folder(current_path);
    file_chooser->set_current_name(s);
}

void
VmMap::export_map_updatefn(void) 
{
    /*
    mw_out << __FUNCTION__ << ": cf=" << file_chooser->get_current_folder() << endl;
    mw_out << __FUNCTION__ << ": fn=" << file_chooser->get_filename() << endl;
    */
    string f = string(file_chooser->get_filename());
    if (f == "") f = "vice-map.png"; // if user has deleted the entry field
    Glib::RefPtr<Gio::File> fnp = Gio::File::create_for_path(f);
    string s = string(file_chooser->get_current_folder()) + G_DIR_SEPARATOR_S + fnp->get_basename();
    mw_out << __FUNCTION__ << ": " << s << endl;
    file_chooser->set_current_name(s);
}

void
VmMap::export_map_commit(bool save) 
{
    if (!save) {
	preview_win->hide();
	return;
    }
    string fn = file_chooser->get_filename();
    if (fn == "")
	return;
    std::cmatch cm;
    std::regex re_ext("(.*)\\.(png|PNG)");
    std::regex_match(fn.c_str(), cm, re_ext, std::regex_constants::match_default);
    if (cm.size() <= 0) {
	fn += ".png";
    }

    Glib::RefPtr<Gio::File> fnp = Gio::File::create_for_path(fn);
    if (fnp->query_exists()) {
	if (VmMsg("Overwrite existing file?", fn).run() != Gtk::RESPONSE_OK)
	    return;
    }
    
    map_preview->save(fn);
    preview_win->hide();
}

void
VmMap::print(void) 
{
    po = VmMap::MapPrint::create();
    po->set_default_page_setup(pr_pagesettings);
    po->set_print_settings(pr_settings);
    
    switch(po->run()) {
    case Gtk::PRINT_OPERATION_RESULT_APPLY:
	mw_out << __FUNCTION__ << ": printing finished successfully." << endl;
	break;
    case Gtk::PRINT_OPERATION_RESULT_ERROR:
	mw_err << __FUNCTION__ << ": printing failed." << endl;
	break;
    default:
	break;
    }
    pr_settings = po->get_print_settings();
}


VmMap::MapPreview::MapPreview() 
{
    set_hexpand(TRUE);
    set_vexpand(TRUE);
    header = footer = nullptr;
    builder->get_widget("VmMapPreviewHeader", header);
    builder->get_widget("VmMapPreviewFooter", footer);
    Gtk::Button *p = nullptr;
    builder->get_widget("VmMapPreviewCancel", p);
    p->set_image_from_icon_name("window-close-symbolic");
    p = nullptr;
    builder->get_widget("VmMapPreviewSave", p);
    p->set_image_from_icon_name("document-save-symbolic");
    p = nullptr;
    builder->get_widget("VmMapPreviewPrint", p);
    p->set_image_from_icon_name("document-print-symbolic");
}

void
VmMap::MapPreview::render_preview(void) 
{
    Glib::RefPtr<Gdk::Pixbuf> tile_img;
    int x, y, dxk, dyk, out_xres, out_yres;

    out_xres = out_yres = 0;
    // calculate width of output image 
    for (x = VmTile::xmin+1; x < VmTile::xmax; x++) {
	mw_map->get_tile(x, VmTile::ymin)->get_cropped_dimensions(dxk, dyk); // dyk not used
	out_xres += dxk;
    }
    
    // calculate height of output image 
    for (y = VmTile::ymin; y <= VmTile::ymax; y++) {	    
	mw_map->get_tile(VmTile::xmin, y)->get_cropped_dimensions(dxk, dyk); // dxk not used
	out_yres += dyk;
    }
    out_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, TRUE, 8, out_xres, out_yres);
    
    dyk = 0;
    for (y = VmTile::ymin; y <= VmTile::ymax; y++) {
	dxk = 0;
	for (x = VmTile::xmin+1; x < VmTile::xmax; x++) {
	    tile_img = mw_map->get_tile(x, y)->get_cropped_image();
	    tile_img->copy_area(0, 0, tile_img->get_width(), tile_img->get_height(), out_image, dxk, dyk);
	    dxk += tile_img->get_width();
	}
	dyk += tile_img->get_height();
    }

    Cairo::RefPtr<Cairo::Surface> imgs = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, out_xres, 50);
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(imgs);
    Glib::RefPtr<Gdk::Pixbuf> text;
    
    Pango::FontDescription font;
    font.set_family("Arial Rounded Mt Bold");
    font.set_size(32 * PANGO_SCALE);
    
    // header
    cr->set_source_rgb(0.8, 0.8, 0.8);
    cr->rectangle(0, 0, out_xres, 50);
    cr->fill();

    cr->set_source_rgb(0.2, 0.2, 0.2);
    auto headerlayout = create_pango_layout(header->get_text());
    headerlayout->set_font_description(font);
    cr->move_to(1, 1);
    headerlayout->show_in_cairo_context(cr);
    
    cr->set_source_rgb(0.2, 0.2, 0.2);
    cr->rectangle(0, 0, out_xres, 50);
    cr->stroke();

    imgs = cr->get_target();
    text = Gdk::Pixbuf::create(imgs, 0, 0, out_xres, 50);
    text->copy_area(0, 0, out_xres, 50, out_image, 0, 0);

    // footer 
    font.set_size(20 * PANGO_SCALE);
    cr->set_source_rgb(0.8, 0.8, 0.8);
    cr->rectangle(0, 0, out_xres, 40);
    cr->fill();

    cr->set_source_rgb(0.2, 0.2, 0.2);
    cr->move_to(1, 1);
    auto footerlayout = create_pango_layout(footer->get_text());
    footerlayout->set_font_description(font);
    footerlayout->show_in_cairo_context(cr);

    auto vm_text = create_pango_layout(" --- powered by vice-mapper");
    vm_text->set_font_description(font);
    int w, h;
    vm_text->get_pixel_size(w, h);
    cr->move_to(out_xres - w - 1, 1);
    vm_text->show_in_cairo_context(cr);
    
    cr->set_source_rgb(0.2, 0.2, 0.2);
    cr->rectangle(0, 0, out_xres, 40);
    cr->stroke();

    imgs = cr->get_target();
    text = Gdk::Pixbuf::create(imgs, 0, 0, out_xres, 40);
    text->copy_area(0, 0, out_xres, 40, out_image, 0, out_image->get_height() - 40);
//    mw_out << __FUNCTION__ << ": out_image = " << out_image->get_width() << "x" << out_image->get_height() << endl;
    queue_draw();
}

void
VmMap::MapPreview::save(std::string name)
{
    try {
	mw_out << __FUNCTION__ << ": " << name << endl;
	out_image->save(name, "png");
    }
    catch (Gdk::PixbufError &ex) {
	mw_out << __FUNCTION__ << ": failed - " << ex.what() << endl;
    }
}
    
bool
VmMap::MapPreview::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) 
{
    //mw_out << __FUNCTION__ << ": called." << endl;
    Glib::RefPtr<Gdk::Pixbuf> scaled = 
	Gdk::Pixbuf::create(out_image->get_colorspace(),
			    out_image->get_has_alpha(),
			    out_image->get_bits_per_sample(),
			    out_image->get_width(),
			    out_image->get_height());
    out_image->copy_area(0, 0,
			 out_image->get_width(),
			 out_image->get_height(),
			 scaled, 0, 0);
    scaled = scaled->scale_simple(get_allocated_width(), get_allocated_height(),
				  Gdk::INTERP_BILINEAR);
    
    Gdk::Cairo::set_source_pixbuf(cr, scaled);
    cr->paint();
    return TRUE;
}

VmMap::MapPrint::MapPrint() 
{
    mw_out << __FUNCTION__ << ": created." << endl; 
}

VmMap::MapPrint::~MapPrint() 
{
    // no mw_out possible, as stream has already being destroyed.
}

Glib::RefPtr<VmMap::MapPrint> VmMap::MapPrint::create()
{
    return Glib::RefPtr<VmMap::MapPrint> (new VmMap::MapPrint());
}

void
VmMap::MapPrint::on_begin_print(const Glib::RefPtr<Gtk::PrintContext>& context) 
{
    mw_out << __FUNCTION__ << ": called." << endl;
    set_n_pages(1);
}

void
VmMap::MapPrint::on_request_page_setup(const Glib::RefPtr<Gtk::PrintContext>& context,
					 int page_no, const Glib::RefPtr<Gtk::PageSetup>& psetup)
{
    mw_out << __FUNCTION__ << ": called." << endl;
    Gtk::PaperSize ps = psetup->get_paper_size();
    double w, h, to, bo, le, ri;
    /*
    w = ps.get_width(Gtk::UNIT_POINTS);
    h = ps.get_height(Gtk::UNIT_POINTS);
    */
    w = context->get_width();
    h = context->get_height();
    to = psetup->get_top_margin(Gtk::UNIT_POINTS);
    bo = psetup->get_bottom_margin(Gtk::UNIT_POINTS);
    le = psetup->get_left_margin(Gtk::UNIT_POINTS);
    ri = psetup->get_right_margin(Gtk::UNIT_POINTS);
    mw_out << __FUNCTION__ << ": paper wxh=" << w << "x" << h
	   << ", to/bo/le/ri=" << to << "/" << bo << "/" << le << "/" << ri
	   << endl;
    
    Glib::RefPtr<Gdk::Pixbuf> tmp = mw_map->get_out_image();
    print_image = Gdk::Pixbuf::create(tmp->get_colorspace(),
				      tmp->get_has_alpha(),
				      tmp->get_bits_per_sample(),
				      tmp->get_width(),
				      tmp->get_height());
    tmp->copy_area(0, 0, tmp->get_width(), tmp->get_height(), print_image, 0, 0);
    
    print_image = print_image->scale_simple(w-le-ri, h-to-bo, Gdk::INTERP_BILINEAR);
}

void
VmMap::MapPrint::on_draw_page(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr)
{
    mw_out << __FUNCTION__ << ": called." << endl;
    Cairo::RefPtr<Cairo::Context>cr = context->get_cairo_context();
    mw_out << __FUNCTION__ << ": wxh=" << print_image->get_width() << "x" << print_image->get_height() << endl;
    mw_debug->put_pixbuf(print_image);
    Gdk::Cairo::set_source_pixbuf(cr, print_image);
    cr->paint();
}

extern "C" 
{

G_MODULE_EXPORT void
on_VmMapPreviewHeader_activate(void) 
{
//    mw_out << __FUNCTION__ << ": called." << endl;
    mw_map->update_preview();
}

G_MODULE_EXPORT void
on_VmMapPreviewFooter_activate(void) 
{
//    mw_out << __FUNCTION__ << ": called." << endl;
    mw_map->update_preview();
}
    
G_MODULE_EXPORT void
on_VmMapPreviewFileChooser_current_folder_changed(void)
{
    mw_out << __FUNCTION__ << ": called." << endl;
    mw_map->export_map_updatefn();
}
    
G_MODULE_EXPORT void
on_VmMapPreviewSave_clicked(void) 
{
//    mw_out << __FUNCTION__ << ": called." << endl;
    mw_map->export_map_commit(true);
}
    
G_MODULE_EXPORT void
on_VmMapPreviewPrint_clicked(void) 
{
    mw_out << __FUNCTION__ << ": called." << endl;
    //VmMsg("Print not yet implemented!", ":-(").run();
    mw_map->print();
}
    
G_MODULE_EXPORT void
on_VmMapPreviewCancel_clicked(void) 
{
//    mw_out << __FUNCTION__ << ": called." << endl;
    mw_map->export_map_commit(false);

}
    
}
