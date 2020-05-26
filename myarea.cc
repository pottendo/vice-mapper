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
#include <giomm/file.h>
#include <giomm/simpleactiongroup.h>
#include <glibmm/fileutils.h>
#include <iostream>
#include <string>
#include <regex>
#include "myarea.h"
#include "map-window.h"

/* MyArea statics */
using namespace::std;
std::vector<Gtk::TargetEntry> MyArea::listTargets;
MyArea *MyArea::dnd_tile;
int MyArea::xmin = map_max - 4, MyArea::ymin = map_max - 4 , MyArea::xmax = 5, MyArea::ymax = 5;
int MyArea::cr_up=36, MyArea::cr_do=36, MyArea::cr_le=32, MyArea::cr_ri=32;
vector<MyArea *> MyArea::all_tiles;
std::string MyArea::current_path="";

/* MyArea members */
MyArea::MyArea(map_window &m, const char *fn, int x, int y)
    : mw(m)
{
    if (fn) {
	Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(fn);
	file_name = f->get_path();
	if (current_path == "")
	    current_path = f->get_parent()->get_path();
	try
	{
	    // The fractal image has been created by the XaoS program.
	    // http://xaos.sourceforge.net
	    m_image_scaled = m_image = Gdk::Pixbuf::create_from_file(fn, resX, resY);
	}
	catch(const Gio::ResourceError& ex)
	{
	    std::cerr << "ResourceError: " << ex.what() << std::endl;
	    throw -1;
	}
	catch(const Gdk::PixbufError& ex)
	{
	    std::cerr << "PixbufError: " << ex.what() << std::endl;
	    throw -1;
	}
	std::string regstr("(.*)" + std::string(def_basename) +
			   "([-0-9][0-9])x([0-9][0-9])\\.(png|PNG|gif|GIF)");
	std::regex re(regstr); 
	std::cmatch cm;
	std::regex_match(fn, cm, re, std::regex_constants::match_default);
	/*
	std::cout << cm.size() << " matches for " << fn << " were: " << endl;
	for (unsigned i=0; i<cm.size(); ++i) {
	    std::cout << "[" << cm[i] << "] ";
	}
	cout << endl;
	*/
	
	if (cm.size() > 0) {
	    xk = std::stod(cm[cm.size()-3]); // -1 would be extension
	    yk = std::stod(cm[cm.size()-2]);
	    if ((xk == 0) || (yk == 0)) throw -1; // maps start at 01x01
	    if (xk < 0) {	// identify unplaced tiles
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
	    std::cerr << "filename not following convention " << def_basename << "XX:YY.png): "
		      << fn << std::endl;
	    throw -1;
	}
    }
    else {
	xk = x; yk = y;
	file_name = "empty";
	m_image = map_window::empty_image;
	empty = true;
    }
    set_dirty(FALSE);		// initially we're in sync with files.
    m_pMenuPopup = nullptr;	// setup popup only if needed.

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

    signal_button_press_event().connect(sigc::mem_fun(*this, &MyArea::on_button_press_event), false);
    print();
}

MyArea::~MyArea()
{
    cout << "*** Destructor called for ";
    print();
    if (m_pMenuPopup)
	delete m_pMenuPopup;
}

void
MyArea::print(void) 
{
    cout << "'" << file_name << "'@" << xk << "," << yk
	 << (is_dirty() ? ",is-dirty" : ",is-clean") << endl;
}

void
MyArea::setup_popup(void)
{
    static bool is_initialized = false;
    
    if (is_initialized) return;

    auto refActionGroup = Gio::SimpleActionGroup::create();
    if (!is_empty()) {
	refActionGroup->add_action("delete",
				   sigc::mem_fun(*this, &MyArea::on_menu_delete_tile));
    }
    
    refActionGroup->add_action("process", //TODO: How to specify "<control>P" as an accelerator.
			       sigc::mem_fun(*this, &MyArea::on_menu_popup));
    
    refActionGroup->add_action("remove",
			       sigc::mem_fun(*this, &MyArea::on_menu_popup));

    insert_action_group("MApopup", refActionGroup);

    Glib::RefPtr<Gtk::Builder> m_refBuilder;
    m_refBuilder = Gtk::Builder::create();
	
    Glib::ustring ui_info =
	"<interface>"
	"  <menu id='menu-MApopup'>"
	"    <section>"
	"      <item>"
	"        <attribute name='label' translatable='yes'>delete Tile</attribute>"
	"        <attribute name='action'>MApopup.delete</attribute>"
	"      </item>"
	"      <item>"
	"        <attribute name='label' translatable='yes'>Process</attribute>"
	"        <attribute name='action'>MApopup.process</attribute>"
	"      </item>"
	"      <item>"
	"        <attribute name='label' translatable='yes'>Remove</attribute>"
	"        <attribute name='action'>MApopup.remove</attribute>"
	"      </item>"
	"    </section>"
	"  </menu>"
	"</interface>";

    m_refBuilder->add_from_string(ui_info);

    Glib::RefPtr<Glib::Object> object = m_refBuilder->get_object("menu-MApopup");
    Glib::RefPtr<Gio::Menu> gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
    m_pMenuPopup = new Gtk::Menu(gmenu);
}

void
MyArea::on_menu_delete_tile(void) 
{
    cout << __FUNCTION__ << ": called." << endl;
}

void
MyArea::on_menu_popup(void) 
{
    cout << __FUNCTION__ << ": called." << endl;
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
    if (is_empty()) {
	if ((xk == 0) || (yk == 0) || (xk == map_max) || (yk == map_max)) {
	    /* draw a border box */
	    cr->set_source_rgb(0.6, 0.6, 0.6);
	    cr->rectangle(0, 0, get_allocated_width(), get_allocated_height());
	    cr->fill();
	}
	else {
	    /* draw a light box */
	    cr->set_source_rgb(0.9, 0.9, 0.9);
	    cr->move_to(0, 0);
	    cr->line_to(get_allocated_width(), 0);
	    cr->line_to(get_allocated_width(), get_allocated_height());
	    cr->line_to(0, get_allocated_height());
	    cr->line_to(0, 0);
	    cr->stroke();
	    /* show coordinates */
	    cr->set_source_rgb(0.8, 0.8, 0.9);
	    char t[6];
	    sprintf(t, "%02dx%02d", xk, yk);
	    auto layout = create_pango_layout(t);
	    cr->move_to(1,1);
	    layout->show_in_cairo_context(cr);
	}
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
    if (this->getX() < 0) return; // we don't drag back to unplaced tiles
    /* don't place on boundaries */
    if ((this->getX() < 1) || (this->getX() > map_max-1)) return;
    if ((this->getY() < 1) || (this->getY() > map_max-1)) return;
    
    xchange_tiles(*dnd_tile, *this);
    context->drag_finish(false, false, time);
}

bool
MyArea::on_button_press_event(GdkEventButton *e) 
{
    if( (e->type == GDK_BUTTON_PRESS) && (e->button == 3) )
    {
	if (!m_pMenuPopup) setup_popup();

	if(!m_pMenuPopup->get_attach_widget())
	    m_pMenuPopup->attach_to_widget(*this);
	
	m_pMenuPopup->popup_at_pointer((GdkEvent*)e);
	return true; //It has been handled.
    }
    return FALSE;
}

void
MyArea::set_dirty(bool d) 
{
    if (is_empty()) {
	dirty = FALSE; // empty is never dirty
	return;
    }
    dirty = d;
    mw.set_dirty(d);
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
    s.sync_tile();
    d.sync_tile();
}

void
MyArea::sync_tile(void)
{
    Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(get_fname());
    int x, y;
    char xl[3], yl[3];
    getXY(x, y);
    sprintf(xl, "%02d", x);
    sprintf(yl, "%02d", y);
    
    string p = f->get_parent()->get_path() + G_DIR_SEPARATOR_S +
	def_basename + xl + "x" + yl + ".png";
    if (p == get_fname())
	set_dirty(FALSE);
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
    string s = string("xmin=") + to_string(xmin) + ",ymin=" + to_string(ymin) + ",xmax=" + to_string(xmax) + ",ymax=" + to_string(ymax);
    
    mw_status->show(s);
    
    return changed;
}

void
MyArea::refresh_minmax(void)
{
    xmin = ymin = map_max - 4;
    xmax = ymax = 5;
    std::for_each(all_tiles.begin(), all_tiles.end(),
		  [](MyArea *t)->void { (void) t->update_minmax(); } );
    //cout << "New dimension: " << xmin << "," << ymin << "x" << xmax << "," << ymax << endl;
}

void
MyArea::delete_all_tiles(void) 
{
    // TODO cleanup on map change
    current_path = "";
}


MyArea *
MyArea::lookup_by_name(std::string name) 
{
    /*
    std::vector<MyArea *>::iterator it =
	std::find_if(all_tiles.begin(), all_tiles.end(),
		     [name](MyArea *t) {
			 if (t->get_fname() == name) return true;
			 return false;
		     });
    */
    std::vector<MyArea *>::iterator it;
    MyArea *ret = NULL;
    
    for (auto it = begin(all_tiles); it != end(all_tiles); ++it) {
	if ((*it)->get_fname() == name) {
	    if (!ret) {
		ret = *it;
	    }
	    else {
		cout << __FUNCTION__ << "***found more: "; (*it)->print();
	    }
	}
    }
    /*
    if (!ret)
	cout << __FUNCTION__ << "*** not found: " << name << endl;
    */
    return ret;
}

void
MyArea::park_tile_file(void) 
{
    cout << __FUNCTION__ << ": "; print();
    Glib::RefPtr<Gio::File> fn = Gio::File::create_for_path(get_fname());
    string tmpnam = fn->get_parent()->get_path() + G_DIR_SEPARATOR_S+ "_X_" + fn->get_basename();
    cout << "generated tmpnam: " << tmpnam << endl;
    Glib::RefPtr<Gio::File> tfile = Gio::File::create_for_path(tmpnam);
    if (tfile->query_exists())
    {
	cout << __FUNCTION__ << ": ***File exists!" << tmpnam << endl;
	return;
    }
    fn->copy(tfile);
    set_fname(tmpnam);
    fn->remove();
}

void
MyArea::commit_changes(void) 
{
    string new_fn;
    if (!is_dirty()) {
	//print();
	return;
    }
    cout << __FUNCTION__ << ": ";

    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(file_name);
    char xl[3], yl[3];
    sprintf(xl, "%02d", xk);
    sprintf(yl, "%02d", yk);

    new_fn = file->get_parent()->get_path() + G_DIR_SEPARATOR_S + def_basename +
	xl + "x" + yl + ".png";
    Glib::RefPtr<Gio::File> new_file = Gio::File::create_for_path(new_fn);
    if (new_file->query_exists()) {
	/* lookup which tile references conflicting name */
	MyArea *conflicting_tile = lookup_by_name(new_fn);
	cout << "conflict of: "; print();
	cout << "with: "; conflicting_tile->print();
	conflicting_tile->park_tile_file();
    }
    cout << "rename: " << file_name << "->" << new_fn << endl;
    file->copy(new_file);
    file->remove();
    set_fname(new_fn);
    set_dirty(FALSE);
    get_window()->invalidate(TRUE);
}
