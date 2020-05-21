// -*-c++-*-
// $Id$
//
// File:		myarea.cc
// Date:		Tue May  5 21:23:45 2020
// Author:		pottendo (pottendo)
// 
// Abstract:
//      
//
// Modifications:
// 	$Log$
//

#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <gdkmm/pixbuf.h>
#include <glibmm/fileutils.h>
#include <iostream>
#include <string>
#include "myarea.h"
#include "map-window.h"

using namespace::std;
std::vector<Gtk::TargetEntry> MyArea::listTargets;
MyArea *MyArea::dnd_tile;
int MyArea::xmin = map_max+1, MyArea::ymin = map_max + 1, MyArea::xmax = -1, MyArea::ymax = -1;
int MyArea::cr_up=36, MyArea::cr_do=36, MyArea::cr_le=32, MyArea::cr_ri=32;
vector<MyArea *> MyArea::all_tiles;

MyArea::MyArea(map_window &m, const char *fn, int x, int y)
    : mw(m)
{
    if (fn) {
	file_name = fn;
	try
	{
	    // The fractal image has been created by the XaoS program.
	    // http://xaos.sourceforge.net
	    m_image_scaled = m_image = Gdk::Pixbuf::create_from_file(fn, resX, resY);
	}
	catch(const Gio::ResourceError& ex)
	{
	    std::cerr << "ResourceError: " << ex.what() << std::endl;
	}
	catch(const Gdk::PixbufError& ex)
	{
	    std::cerr << "PixbufError: " << ex.what() << std::endl;
	}
	std::string s(fn);
	std::string::size_type t;
	t = s.find("vice-screen-");
	if (t != std::string::npos) {
	    xk = std::stod(s.substr(t+12, 2));
	    yk = std::stod(s.substr(t+12+3, 2));
	    if (xk == -1) {	// identify unplaced tiles
		if (m_image) {
		    m_image_icon = m_image->scale_simple(m_image->get_width()/4,
							 m_image->get_height()/4,
							 Gdk::INTERP_BILINEAR);
		}
		mw.add_unplaced_tile(this);
	    }
	    else {
	      (void) update_minmax();
	      mw.add_tile(this);
	    }
	    empty = false;
	    all_tiles.push_back(this);
	}
	else {
	    std::cerr << "filename not following convention (vice-screen-XX:YY.png): "
		      << s << std::endl;
	}
    }
    else {
	xk = x; yk = y;
	file_name = "empty";
	m_image = map_window::empty_image;
	empty = true;
    }
    set_dirty(FALSE);		// initially we're in sync with files.
    
    // Show at least a quarter of the image.
    if (m_image) {
	set_size_request(m_image->get_width()/3, m_image->get_height()/3);
    }

    set_hexpand(TRUE);
    set_vexpand(TRUE);
    
    listTargets.push_back( Gtk::TargetEntry("text/plain",
					    Gtk::TARGET_SAME_APP /*| Gtk::TARGET_OTHER_WIDGET*/) );
    drag_source_set(listTargets, Gdk::BUTTON1_MASK, Gdk::ACTION_COPY|Gdk::ACTION_MOVE);
    drag_dest_set(listTargets);
    drag_source_set_icon(m_image->scale_simple(m_image->get_width()/3,
					       m_image->get_height()/3,
					       Gdk::INTERP_BILINEAR));
    
    // connect signals
    signal_configure_event().connect(sigc::mem_fun(*this, &MyArea::on_configure_event), false);
    signal_drag_data_get().connect(sigc::mem_fun(*this,
						 &MyArea::on_button_drag_data_get));
    signal_drag_data_received().connect(sigc::mem_fun(*this,
						      &MyArea::on_label_drop_drag_data_received));
}

MyArea::~MyArea()
{
    cout << "*** Destructor called for ";
    print();
}

void
MyArea::print(void) 
{
    std::cout << "'" << file_name << "'@" << xk << "," << yk << std::endl;
}

bool
MyArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    m_image_scaled =
	Gdk::Pixbuf::create(m_image->get_colorspace(),
			    m_image->get_has_alpha(),
			    m_image->get_bits_per_sample(),
			    m_image->get_width()-cr_le-cr_ri,
			    m_image->get_height()-cr_up-cr_do);
    m_image->copy_area(cr_le, cr_up,
		       m_image->get_width()-cr_le-cr_ri,
		       m_image->get_height()-cr_up-cr_do,
		       m_image_scaled, 0, 0);
    /*
    cout << file_name << ": scaled size: " << m_image_scaled->get_width() << "x"
	 << m_image_scaled->get_height() << cr_le << "," << cr_up << endl;
    */
    
    m_image_scaled =
	m_image_scaled->scale_simple(get_allocated_width(),
				     get_allocated_height(),
				     Gdk::INTERP_BILINEAR);
    if (is_dirty()) {
	m_image_scaled->saturate_and_pixelate(m_image_scaled, 0.7, TRUE);
    }
    
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // Draw the image in the middle of the drawing area, or (if the image is
    // larger than the drawing area) draw the middle part of the image.
    Gdk::Cairo::set_source_pixbuf(cr, m_image_scaled,
				  (width - m_image_scaled->get_width())/2,
				  (height - m_image_scaled->get_height())/2);
    cr->paint();
    return true;
}

bool
MyArea::on_configure_event(GdkEventConfigure *configure_event)
{
    //cout << __FUNCTION__ << ": " << file_name << endl;
    return TRUE;
}

void
MyArea::on_button_drag_data_get(
    const Glib::RefPtr<Gdk::DragContext>&,
    Gtk::SelectionData& selection_data, guint, guint)
{
    
    selection_data.set(selection_data.get_target(), 8 /* 8 bits format */,
		       (const guchar*)"TILE", 4);

    dnd_tile = this;
}

void MyArea::on_label_drop_drag_data_received(
    const Glib::RefPtr<Gdk::DragContext>& context, int, int,
    const Gtk::SelectionData& selection_data, guint, guint time)
{
    const int length = selection_data.get_length();
    if((length >= 0) && (selection_data.get_format() == 8))
    {
	if (selection_data.get_data_as_string().compare("TILE") != 0) {
	    cout << "Dragdest: " << selection_data.get_data_as_string() << " l: "
		 << selection_data.get_data_as_string().length() << endl;
	    return;
	}
    }
    /*
    cout << "Drag start at "; dnd_tile->print();
    cout << "Drag stop at " << file_name << endl;
    */
    if (dnd_tile == this) return; // Don't do anything if we drag over ourselves
    if (this->getX() == -1) return; // we don't drag back to unplaced tiles
    xchange_tiles(*dnd_tile, *this);
    context->drag_finish(false, false, time);
}

void
MyArea::scale(float sfx, float sfy) 
{
    set_size_request(m_image->get_width()/sfx, m_image->get_height()/sfy);
}

void
MyArea::xchange_tiles(MyArea &s, MyArea &d) 
{
    // call this == destination tile
    cout << __FUNCTION__ << ": " << s.get_fname() << " <-> " << d.get_fname() << endl;
    // set dirty flag for later commit
    d.set_dirty(true);
    s.set_dirty(true);
    mw.xchange_tiles(&s, this);
}

bool
MyArea::update_minmax(void) 
{
    bool changed = false;

    if (xk < 0) return false;	// unplaced tile, no update of maxima/minima
    
    xmin = MIN(xmin, xk);
    ymin = MIN(ymin, yk);
    xmax = MAX(xmax, xk);
    ymax = MAX(ymax, yk);
    if (xmin == xk) { xmin--; changed = true; }
    if (ymin == yk) { ymin--; changed = true; }
    if (xmax == xk) { xmax++; changed = true; }
    if (ymax == yk) { ymax++; changed = true; }
    return changed;
}

void
MyArea::refresh_minmax(void)
{
    xmin = ymin = map_max + 1;
    xmax = ymax = -1;
    std::for_each(all_tiles.begin(), all_tiles.end(),
		  [](MyArea *t)->void { (void) t->update_minmax(); } );
    //cout << "New dimension: " << xmin << "," << ymin << "x" << xmax << "," << ymax << endl;
}
