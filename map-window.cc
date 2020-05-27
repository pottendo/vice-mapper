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
#include <glibmm/fileutils.h>
#include "map-window.h"
#include "map-controls.h"
#include <iostream>

using namespace::std;
Glib::RefPtr<Gdk::Pixbuf> map_window::empty_image;
double map_window::scale_factor_x = 3.0;
double map_window::scale_factor_y = 3.0;

map_window::map_window()
    : nr_tiles(0)
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
map_window::remove_tile(MyArea *a)
{
    if (a->getX() > 0) {
	// placed tile
	MyArea *new_empty = new MyArea(*this, NULL, a->getX(), a->getY());
	map_grid.remove(*a);
	map_grid.attach(*new_empty, a->getX(), a->getY());
	tiles[a->getX()][a->getY()] = new_empty;
	if (auto e = MyArea::all_tiles.erase(a) != 1) {
	    cerr << __FUNCTION__ << ": erased " << e << " tiles." << endl;
	}
	resize_map();
    }
    else {
	ctrls->remove_tile(a);
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
    if (tsx > 0) add_tile(d);
    else {
	if (!d->is_empty())	// pushback if we placed on occupied tile
	    ctrls->add_tile(d);
    }
    // check if we need to grow/shrink
    if (s->update_minmax())
	fill_empties(); // already placed therefore s(ource)!
    resize_map();
}
