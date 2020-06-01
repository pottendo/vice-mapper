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
#include <gtkmm/filechooserdialog.h>
#include <glibmm/fileutils.h>
#include <iostream>
#include "map-window.h"
#include "map-controls.h"
#include "dialogs.h"

using namespace::std;
Glib::RefPtr<Gdk::Pixbuf> map_window::empty_image;
double map_window::scale_factor_x = def_zoom;
double map_window::scale_factor_y = def_zoom;

map_window::map_window()
    : dirty(false),
      nr_tiles(0)
{
    //set_title("Map");

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
    memset(tiles, 0, 101*101*sizeof(MyArea*));
}

map_window::MyScw::MyScw()
{
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
	    // cout << __FUNCTION__ << ": direction = " << dir << endl;
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
	    //cout << __FUNCTION__ << ": dx = " << dx << ", dy = " << dy << endl;
	}

    }
    scale_factor_x = MIN(scale_factor_x, 6.0);
    scale_factor_y = MIN(scale_factor_y, 6.0);
    mc->set_zoom(scale_factor_x, scale_factor_y);
    return TRUE;
}

bool
map_window::MyScw::on_motion_notify_event(GdkEventMotion* motion_event) 
{
    static gdouble xp, yp;
    
    if (space_modifier /* && button_press*/) {
	gdouble dx, dy, xa, ya;
	
	dx = (motion_event->x - xp);
	dy = (motion_event->y - yp);
	/*
	cout << __FUNCTION__ << ": space+mouse-move: hadj = " << get_hadjustment()->get_value()
	     << "- dx: " << dx << ",dy: " << dy << ", xp: " << xp << ",yp: " << yp << endle;
	*/
	xa = get_hadjustment()->get_value() - dx;
	ya = get_vadjustment()->get_value() - dy;
	get_hadjustment()->set_value(xa);
	get_vadjustment()->set_value(ya);
    }
    xp = motion_event->x;	// track last location even if not panning
    yp = motion_event->y;
    return Gtk::ScrolledWindow::on_motion_notify_event(motion_event);
}

bool
map_window::MyScw::on_enter_notify_event(GdkEventCrossing* crossing_event) 
{
    grab_focus();		// needed if e.g. controls took the focus
    space_modifier = false;
    return Gtk::ScrolledWindow::on_enter_notify_event(crossing_event);
}

bool
map_window::MyScw::on_key_press_event(GdkEventKey *key_event) 
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
map_window::MyScw::on_key_release_event(GdkEventKey *key_event) 
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
    cout << "button press" << endl;
    Gtk::ScrolledWindow::on_button_press_event(button_event);
    return TRUE;
}

bool
map_window::MyScw::on_button_release_event(GdkEventButton* button_event) 
{
    button_press = FALSE;
    cout << "button rel" << endl;
    Gtk::ScrolledWindow::on_button_release_event(button_event);
    return TRUE;
}
*/

void
map_window::add_tile(MyArea *a) 
{
    map_grid.attach(*a, a->getX(), a->getY());
    tiles[a->getX()][a->getY()] = a;
    a->scale(scale_factor_x, scale_factor_y); // make sure tile adjusts to current scaling
    nr_tiles++;
    show_all_children();
}

void
map_window::remove_tile(MyArea *a, bool map_remove)
{
    cout << __FUNCTION__ << ": " << map_remove << " called for ";
    a->print();
    
    if (a->getX() < 0) {
	ctrls->remove_tile(a);
    }
    else {
	map_grid.remove(*a);
	if (!map_remove) {
	    // placed tile
	    MyArea *new_empty = new MyArea(*this, NULL, a->getX(), a->getY());
	    //map_grid.attach(*new_empty, a->getX(), a->getY());
	    add_tile(new_empty);
	    cout << __FUNCTION__ << ": attached new ";
	    new_empty->print();
	    resize_map();
	}
	else
	    tiles[a->getX()][a->getY()] = nullptr;
    }
    auto e = MyArea::all_tiles.erase(a);
    if (e > 1) {
	cerr << __FUNCTION__ << ": erased " << e << " tiles." << endl;
    }
    nr_tiles = MyArea::all_tiles.size();
    show_all_children();
}

void
map_window::reload_unplaced_tiles(void)
{
    string &cp = MyArea::current_path;
    
    if (cp == "") return;
    Glib::Dir dir(MyArea::current_path);
    std::list<std::string> entries (dir.begin(), dir.end());

    for (auto e = entries.begin(); e != entries.end(); ++e) {
	string fp = cp + G_DIR_SEPARATOR_S + *e;
	MyArea *t = MyArea::lookup_by_name(fp);
	if (t) continue;
	cout << __FUNCTION__ << ": new tile: " << fp << endl;
	try {
	    t = new MyArea(*this, fp.c_str());
	}
	catch (...) {
	    continue;		// ignore non-tiles
	}
    }
    MyArea::refresh_minmax();
    fill_empties();
    mw_status->show(MyStatus::STATM, MyArea::current_path);
    if (!MyArea::tiles_placed)
	add_tile(new MyArea(*this, NULL, 50, 50));
}

void
map_window::save_settings(void) 
{
    string &cp = MyArea::current_path;
    if (cp == "") return;
    size_t ext_pos = def_basename.find_last_of(".");
    string cfg_fname = cp + G_DIR_SEPARATOR_S + def_basename.substr(0, ext_pos) + def_cfg_ext;
    
    cout << __FUNCTION__ << ": cfg file = " << cfg_fname << endl;
     
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
	string("CRUP=" + to_string(MyArea::cr_up) + "\n"
	       "CRDO=" + to_string(MyArea::cr_do) + "\n"
	       "CRLE=" + to_string(MyArea::cr_le) + "\n"
	       "CRRI=" + to_string(MyArea::cr_ri) + "\n" 
	       "ZOOMX=" + to_string(scale_factor_x) + "\n"
	       "ZOOMY=" + to_string(scale_factor_y) + "\n"
	       "#END - DONT_REMOVE_THIS_LINE\n"
	    );
    
    cfg_stream->write(lines.c_str(), lines.size());
    cfg_stream->flush();
}

bool
map_window::process_line(string l) 
{
    //cout << __FUNCTION__ << ": processing line '" << l << "'" << endl;
    if (l.find("#END") < l.size())
	return false;

    try {
	if (l.find("CRUP") < l.size()) MyArea::cr_up = stoi(l.substr(5));
	if (l.find("CRDO") < l.size()) MyArea::cr_do = stoi(l.substr(5));
	if (l.find("CRLE") < l.size()) MyArea::cr_le = stoi(l.substr(5));
	if (l.find("CRRI") < l.size()) MyArea::cr_ri = stoi(l.substr(5));
	if (l.find("ZOOMX") < l.size()) { scale_factor_x = stod(l.substr(6)); }
	if (l.find("ZOOMY") < l.size()) { scale_factor_y = stod(l.substr(6)); }
    }
    catch (std::invalid_argument &e) {
	cerr << e.what() << endl;
    }
    
    return true;
}

bool
map_window::load_settings(void) 
{
    string &cp = MyArea::current_path;
    if (cp == "") return false;
    size_t ext_pos = def_basename.find_last_of(".");
    string cfg_fname = cp + G_DIR_SEPARATOR_S + def_basename.substr(0, ext_pos) + def_cfg_ext;
    
    cout << __FUNCTION__ << ": cfg file = " << cfg_fname << endl;
     
    Glib::RefPtr<Gio::File> cfg_file = Gio::File::create_for_path(cfg_fname);
    if (!cfg_file) {
	cout << __FUNCTION__ << ": can't open cfg file for map." << endl;
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
	ctrls->set_crop(MyArea::cr_up, MyArea::cr_do, MyArea::cr_le, MyArea::cr_ri, false);
    }
    catch (Gio::Error &e) {
	cerr << __FUNCTION__ << ": *** cfg file not read: " << e.what() << endl;
	return false;
    }
    return true;
}

void
map_window::fill_empties() 
{
    int x, y;
/*
    cout << "X:" << MyArea::xmin << "-" << MyArea::xmax
	 << "Y:" << MyArea::ymin << "-" << MyArea::ymax << endl;
*/
    try {
	for (x = MyArea::xmin; x <= MyArea::xmax; x++) {
	    for (y = MyArea::ymin; y <= MyArea::ymax; y++) {
		if (tiles[x][y] == NULL) {
		    add_tile(new MyArea(*this, NULL, x, y));
		}
	    }
	}
    }
    catch (std::exception &e) {
	cout << __FUNCTION__ << ": failed with exception " << e.what() << endl;
	return;
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

int
map_window::get_empty_area(int from_x, int from_y, int to_x, int to_y)
{
    int x, y;
    int do_scratch = 0;
    MyArea *t;
    std::vector<MyArea *> scratch;
    
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
	cout << "found " << do_scratch << " empty tiles to remove." << endl;
	std::for_each(scratch.begin(), scratch.end(),
		      [](MyArea *x)->void {
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
map_window::xchange_tiles(MyArea *s, MyArea *d)
{
    int tsx, tsy, tdx, tdy;
    MyArea *t;
    s->getXY(tsx, tsy);		// get x,y
    d->getXY(tdx, tdy);

    if ((tsx < 0) && (tsy < 0)) {
	cout << "*** shoud not happen !!! - placed from/to unplaced tiles." << endl;
	return;
    }
    
    if (tsx < 0) {
	//cout << "remove from unplaced tiles";
	s->print();
	ctrls->remove_tile(s);
    }
    if ((t = (MyArea *) map_grid.get_child_at(s->getX(), s->getY())) != NULL) {
	//cout << "removing: ";
	t->print();
	map_grid.remove(*t);
    }
    if (tdx < 0) {
	cout << "*** should not happen !!! - move to unplaced tiles";
	//special case - add to list of unplace tiles
	return;
	
    }
    if ((t = (MyArea *) map_grid.get_child_at(d->getX(), d->getY())) != NULL) {
	cout << "removing: ";
	t->print();
	map_grid.remove(*t);
    }
	
    s->setXY(tdx, tdy);
    add_tile(s);
    d->setXY(tsx, tsy);
    if (tsx > 0) {
	add_tile(d);
    }
    else {
	if (!d->is_empty()) {	// pushback if we placed on occupied tile
	    ctrls->add_tile(d);
	}
	else {
	    delete d;		// dispose unused empty tile
	}
    }
    
    // check if we need to grow/shrink
    if (s->update_minmax())
	fill_empties(); // already placed therefore s(ource)!
    resize_map();
}

/*
 * map functions
 */
void
map_window::resize_map(void)
{
    int do_scratch = 0;
    MyArea::refresh_minmax();
    do_scratch += get_empty_area(0, 0, map_max+1, MyArea::ymin);
    do_scratch += get_empty_area(0, MyArea::ymax+1, map_max+1, map_max+1);
    do_scratch += get_empty_area(0, 0, MyArea::xmin, map_max+1);
    do_scratch += get_empty_area(MyArea::xmax+1, 0, map_max+1, map_max+1);
    if (do_scratch) {
	cout << "found " << do_scratch << " empty tiles to remove." << endl;
    }
}

void
map_window::remove_map(void) 
{
    cout << __FUNCTION__ << ": called." << endl;
    std::vector<Gtk::Widget *> to_scratch = map_grid.get_children();
    std::for_each(to_scratch.begin(), to_scratch.end(),
		  [this](Gtk::Widget *t)->void {
		      remove_tile(static_cast<MyArea*>(t), true);
		      delete static_cast<MyArea*>(t);
		  });
    
    for (auto t : MyArea::all_tiles) {
	delete t;
	MyArea::all_tiles.erase(t);
    }
    nr_tiles = MyArea::all_tiles.size();
    cout << __FUNCTION__ << ": pending tiles = " << nr_tiles
	 << ", alloc_count = " << MyArea::alloc_count << endl;
    //add_tile(new MyArea(*this, NULL, 50, 50));
    mw_status->clear();
    set_dirty(false);
    show_all_children();
}

void
map_window::open_map(void) 
{ 
    Gtk::FileChooserDialog d("Select map from folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    d.set_transient_for(*mainWindow);

    d.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    d.add_button("Select", Gtk::RESPONSE_OK);
    if (d.run() != Gtk::RESPONSE_OK)
	return;
    
    cout << __FUNCTION__ << ": load map from '" <<  d.get_filename() << "'." << endl;
    remove_map();
    MyArea::current_path = d.get_filename();
    reload_unplaced_tiles();
    if (!load_settings()) {
	ctrls->set_crop(def_cry, def_cry, def_crx, def_crx);
	ctrls->set_zoom(def_zoom, def_zoom);
    }
}
