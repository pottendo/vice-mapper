// -*-c++-*-
// $Id$
//
// File:		myarea.cc
// Date:		Tue May  5 12:23:45 2020
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
Glib::RefPtr<Gdk::Pixbuf> MyArea::dnd_image;
int MyArea::xmin = -1, MyArea::ymin = -1, MyArea::xmax = -1, MyArea::ymax = -1;

MyArea::MyArea(const char *fn, int x, int y)
{
    if (fn) {
	file_name = fn;
	try
	{
	    // The fractal image has been created by the XaoS program.
	    // http://xaos.sourceforge.net
	    m_image_scaled = m_image = Gdk::Pixbuf::create_from_file(fn, 384, 272);
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
	    if (xk == -1) {	// hack to test default
		xk = 55+yk;
		yk = 50;
	    }
	    print();
	}
	else {
	    std::cerr << "Invalid filename convention (vice-screen-XX:YY.png): " << s << std::endl;
	}
    }
    else {
	xk = x; yk = y;
	file_name = "empty";
	m_image = map_window::empty_image;
    }
    if (xmin < 0) xmin = xk;
    else xmin = MIN(xmin, xk);
    if (ymin < 0) ymin = xk;
    else ymin = MIN(ymin, yk);
    xmax = MAX(xmax, xk);
    ymax = MAX(ymax, yk);
    
    // Show at least a quarter of the image.
    if (m_image) {
	set_size_request(m_image->get_width()/3, m_image->get_height()/3);
    }

    set_hexpand(TRUE);
    set_vexpand(TRUE);
    
    listTargets.push_back( Gtk::TargetEntry("SCREEN_MAPPER", Gtk::TARGET_SAME_APP) );
    drag_source_set(listTargets);
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
}

void
MyArea::print(void) 
{
    std::cout << "'" << file_name << "'@" << xk << "," << yk << std::endl;
}

bool
MyArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if (!m_image_scaled)
	return false;

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
    /*
    cout << "configure event: " << file_name << "-" <<
	get_allocated_width() << "x" << get_allocated_height() << endl;
    */
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
    return TRUE;
}

void
MyArea::on_button_drag_data_get(
    const Glib::RefPtr<Gdk::DragContext>&,
    Gtk::SelectionData& selection_data, guint, guint)
{
    selection_data.set(selection_data.get_target(), 8 /* 8 bits format */,
		       (const guchar*)"I'm Data!",
		       9 /* the length of I'm Data! in bytes */);

    cout << "Drag start at " << file_name << endl;
    dnd_image = m_image;
}

void MyArea::on_label_drop_drag_data_received(
    const Glib::RefPtr<Gdk::DragContext>& context, int, int,
    const Gtk::SelectionData& selection_data, guint, guint time)
{
    const int length = selection_data.get_length();
    if((length >= 0) && (selection_data.get_format() == 8))
    {
	std::cout << "Received \"" << selection_data.get_data_as_string()
		  << "\" in label " << std::endl;
    }
    cout << "Drag stop at " << file_name << endl;
    m_image = dnd_image;
    context->drag_finish(false, false, time);
}

void
MyArea::scale(float sf) 
{
    set_size_request(m_image->get_width()/sf, m_image->get_height()/sf);
}
