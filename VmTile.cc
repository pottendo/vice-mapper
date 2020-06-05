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
 * File:		myarea.cc
 * Date:		Tue May  5 21:23:45 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     tile class 
 * 
 * Modifications:
 * 	$Log$
 */

#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <gdkmm/pixbuf.h>
#include <giomm/file.h>
#include <giomm/error.h>
#include <giomm/simpleactiongroup.h>
#include <glibmm/fileutils.h>
#include <iostream>
#include <string>
#include <regex>
#include "VmTile.h"
#include "VmMap.h"

/* MyArea statics */
using namespace::std;
std::vector<Gtk::TargetEntry> VmTile::listTargets;
VmTile *VmTile::dnd_tile;
int VmTile::alloc_count;
int VmTile::xmin = map_max - 4, VmTile::ymin = map_max - 4 , VmTile::xmax = 5, VmTile::ymax = 5;
int VmTile::cr_up=def_cry, VmTile::cr_do=def_cry, VmTile::cr_le=def_crx, VmTile::cr_ri=def_crx;
//vector<MyArea *> MyArea::all_tiles;
set<VmTile *> VmTile::all_tiles;
std::string VmTile::current_path="";
bool VmTile::tiles_placed = false;

/* MyArea members */
VmTile::VmTile(VmMap &m, const char *fn, int x, int y)
    : mw(m)
{
    if (fn) {
	Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(fn);
	set_fname(f->get_path(), f->get_basename());
	if (current_path == "") {
	    current_path = f->get_parent()->get_path();
	    mw_status->show(VmStatus::STATM, current_path);
	}
	
	try {
	    m_image_scaled = m_image = Gdk::Pixbuf::create_from_file(fn, resX, resY);
	}
	catch(const Gio::ResourceError& ex) {
	    std::cerr << "ResourceError: " << ex.what() << std::endl;
	    throw -1;
	}
	catch(const Gdk::PixbufError& ex) {
	    std::cerr << "PixbufError: " << ex.what() << std::endl;
	    throw -1;
	}
	std::string regstr("(.*)" + std::string(def_basename) +
			   "([-0-9][0-9])x([0-9][0-9][0-9]*)\\.(png|PNG|gif|GIF)");
	std::regex re(regstr); 
	std::cmatch cm;
	std::regex_match(fn, cm, re, std::regex_constants::match_default);
	/*
	mw_out << cm.size() << " matches for " << fn << " were: " << endl;
	for (unsigned i=0; i<cm.size(); ++i) {
	    mw_out << "[" << cm[i] << "] ";
	}
	mw_out << endl;
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
		VmTile *t;
		if ((t = mw.get_tile(xk, yk)) != nullptr) {
		    if (t->is_empty()) {
			delete t;
			mw.set_tile(xk,yk, nullptr);
		    }
		    else {
			mw_out << __FUNCTION__ << ": refusing to overload tile with " << *this << endl;
			throw -1;
		    }
		}
		if (yk >= map_max) throw -1; // y can be larger to allow 100+ unplaced tiles.
		mw.add_tile(this);
		tiles_placed=true;
	    }
	    empty = false;
	    all_tiles.insert(this);
	}
	else {
	    std::cerr << "filename not following convention " << def_basename << "XX:YY.png): "
		      << fn << std::endl;
	    throw -1;
	}
    }
    else {
	xk = x; yk = y;
	file_name = file_basename = "<empty>";
	m_image = VmMap::empty_image;
	empty = true;
    }
    set_dirty(FALSE);		// initially we're in sync with files.
    m_pMenuPopup = nullptr;	// setup popup only if needed.
    alloc_count++;		// recored number of allocated tiles

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
    /*
    signal_configure_event()
	.connect(sigc::mem_fun(*this, &VmTile::on_configure_event), false);
    */
    signal_drag_data_get()
	.connect(sigc::mem_fun(*this, &VmTile::on_button_drag_data_get));
    signal_drag_data_received()
	.connect(sigc::mem_fun(*this, &VmTile::on_label_drop_drag_data_received));

    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    set_selected(false);
    
    mw_out << __FUNCTION__ << ": new tile " << *this << endl;
}

VmTile::~VmTile()
{
    mw_out << "*** Destructor called for " << *this << endl;
    if (m_pMenuPopup)
	delete m_pMenuPopup;
    alloc_count--;
}

void
VmTile::print(void) 
{
    mw_out << *this;
    /*
    mw_out << "'" << file_name << "'@" << xk << "," << yk
	 << (is_dirty() ? ",is-dirty" : ",is-clean") << endl;
    */
    mw_debug->log(file_name);
}

void
VmTile::setup_popup(void)
{
    static bool is_initialized = false;
    
    if (is_initialized) return;

    auto refActionGroup = Gio::SimpleActionGroup::create();
    if (!is_empty()) {
	refActionGroup->add_action("delete",
				   sigc::mem_fun(*this, &VmTile::on_menu_delete_tile));
    }
/* not yet implemented    
    refActionGroup->add_action("icolumn", //TODO: How to specify "<control>P" as an accelerator.
			       sigc::mem_fun(*this, &VmTile::on_menu_popup));
    
    refActionGroup->add_action("irow",
			       sigc::mem_fun(*this, &VmTile::on_menu_popup));
*/
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
	"        <attribute name='label' translatable='yes'>insert Row</attribute>"
	"        <attribute name='action'>MApopup.icolumn</attribute>"
	"      </item>"
	"      <item>"
	"        <attribute name='label' translatable='yes'>insert Column</attribute>"
	"        <attribute name='action'>MApopup.irow</attribute>"
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
VmTile::on_menu_delete_tile(void) 
{
/*    
#ifdef WIN32
    bool running_on_win32 = true;
#else
    bool running_on_win32 = false;
#endif
*/
    
    if (/* running_on_win32 ||*/	// f->trash below sometimes(!) opens the std dialog on win32
	VmMsg("delete Tile", "Are you sure?").run() == Gtk::RESPONSE_OK) {
        mw_out << __FUNCTION__ << ": delete confirmed for " << *this << endl;
	Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(get_fname());
	try {
	    f->trash();		// park tiles in trash
	}
	catch (Glib::Error &e) {
	    //mw_out << __FUNCTION__ << ": move to trash failed for "; print();
	    cerr << e.what() << endl; // user has cancelled
	    //f->remove();	// plain remove only if NOT_SUPPORTED comes back TODO
	    return;
	}
	mw.remove_tile(this);
	delete this;
    }
}

void
VmTile::on_menu_popup(void) 
{
    mw_out << __FUNCTION__ << ": called." << endl;
}

std::ostream &operator<<(std::ostream & out, VmTile &t) {
    return out << "'" << t.get_fname() << "'@" << t.xk << "," << t.yk
	       << (t.is_dirty() ? ",is-dirty" : ",is-clean");
}

bool
VmTile::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
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
    mw_out << file_name << ": scaled size: " << m_image_scaled->get_width() << "x"
	 << m_image_scaled->get_height() << cr_le << "," << cr_up << endl;
    */
    
    m_image_scaled =
	m_image_scaled->scale_simple(get_allocated_width(),
				     get_allocated_height(),
				     Gdk::INTERP_BILINEAR);
    if (is_dirty()) {
	m_image_scaled->saturate_and_pixelate(m_image_scaled, 0.7, TRUE);
    }
    if (is_selected()) {
	m_image_scaled->saturate_and_pixelate(m_image_scaled, 0.9, TRUE);
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
VmTile::on_configure_event(GdkEventConfigure *configure_event)
{
    return TRUE;
}

bool
VmTile::on_enter_notify_event(GdkEventCrossing* crossing_event)
{
    //mw_out << __FUNCTION__ << ": called." << endl;
    set_selected(true);
    mw_status->show(VmStatus::STATL, to_string(xk) + "x" + to_string(yk) + "|" + file_basename);
    queue_draw();
    return FALSE;
}

bool
VmTile::on_leave_notify_event(GdkEventCrossing* crossing_event)
{
    //mw_out << __FUNCTION__ << ": called." << endl;
    set_selected(false);
    queue_draw();
    return FALSE;
}

void
VmTile::on_button_drag_data_get(
    const Glib::RefPtr<Gdk::DragContext>&,
    Gtk::SelectionData& selection_data, guint, guint)
{
    
    selection_data.set(selection_data.get_target(), 8 /* 8 bits format */,
		       (const guchar*)"TILE", 4);

    dnd_tile = this;
}

void VmTile::on_label_drop_drag_data_received(
    const Glib::RefPtr<Gdk::DragContext>& context, int, int,
    const Gtk::SelectionData& selection_data, guint, guint time)
{
    const int length = selection_data.get_length();
    if((length >= 0) && (selection_data.get_format() == 8))
    {
	if (selection_data.get_data_as_string().compare("TILE") != 0) {
	    mw_out << "Dragdest: " << selection_data.get_data_as_string() << " l: "
		 << selection_data.get_data_as_string().length() << endl;
	    return;
	}
    }
    /*
      mw_out << "Drag start at " << *dnd_tile << endl;
      mw_out << "Drag stop at " << *this << endl;
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
VmTile::on_button_press_event(GdkEventButton *e) 
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
VmTile::set_dirty(bool d) 
{
    if (is_empty()) {
	dirty = FALSE; // empty is never dirty
	return;
    }
    dirty = d;
    mw.set_dirty(d);
}

void
VmTile::scale(float sfx, float sfy) 
{
    set_size_request(m_image->get_width()/sfx, m_image->get_height()/sfy);
}

void
VmTile::xchange_tiles(VmTile &s, VmTile &d) 
{
    // call this == destination tile
    mw_out << __FUNCTION__ << ": " << s.get_fname() << " <-> " << d.get_fname() << endl;
    // set dirty flag for later commit
    d.set_dirty(true);
    s.set_dirty(true);
    mw.xchange_tiles(&s, this);
}

void
VmTile::sync_tile(void)
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
VmTile::update_minmax(void) 
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
    
    mw_status->show(VmStatus::STATR, s);
    
    return changed;
}

void
VmTile::refresh_minmax(void)
{
    xmin = ymin = map_max - 4;
    xmax = ymax = 5;
    std::for_each(all_tiles.begin(), all_tiles.end(),
		  [](VmTile *t)->void { (void) t->update_minmax(); } );
    //mw_out << "New dimension: " << xmin << "," << ymin << "x" << xmax << "," << ymax << endl;
}

VmTile *
VmTile::lookup_by_name(std::string name) 
{
    std::vector<VmTile *>::iterator it;
    VmTile *ret = NULL;
    
    for (auto it = begin(all_tiles); it != end(all_tiles); ++it) {
	if ((*it)->get_fname() == name) {
	    if (!ret) {
		ret = *it;
	    }
	    else {
		mw_out << __FUNCTION__ << "***found more: " << *(*it) << endl;
	    }
	}
    }
    /*
    if (!ret)
	mw_out << __FUNCTION__ << "*** not found: " << name << endl;
    */
    return ret;
}

void
VmTile::park_tile_file(void) 
{
    mw_out << __FUNCTION__ << ": " << *this << endl;
    Glib::RefPtr<Gio::File> fn = Gio::File::create_for_path(get_fname());
    string tmpnam = fn->get_parent()->get_path() + G_DIR_SEPARATOR_S+ "_X_" + fn->get_basename();
    mw_out << "generated tmpnam: " << tmpnam << endl;
    Glib::RefPtr<Gio::File> tfile = Gio::File::create_for_path(tmpnam);
    if (tfile->query_exists())
    {
	mw_out << __FUNCTION__ << ": ***File exists!" << tmpnam << endl;
	return;
    }
    fn->copy(tfile);
    set_fname(tmpnam, tfile->get_basename());
    fn->remove();
}

void
VmTile::commit_changes(void) 
{
    string new_fn;
    if (!is_dirty()) {
	return;
    }
    mw_out << __FUNCTION__ << ": ";

    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(file_name);
    char xl[3], yl[3];
    sprintf(xl, "%02d", xk);
    sprintf(yl, "%02d", yk);

    new_fn = file->get_parent()->get_path() + G_DIR_SEPARATOR_S + def_basename +
	xl + "x" + yl + ".png";
    Glib::RefPtr<Gio::File> new_file = Gio::File::create_for_path(new_fn);
    if (new_file->query_exists()) {
	/* lookup which tile references conflicting name */
	VmTile *conflicting_tile = lookup_by_name(new_fn);
	mw_out << "conflict of: " << *this << endl;
	mw_out << "with: " << *conflicting_tile << endl;
	conflicting_tile->park_tile_file();
    }
    mw_out << "rename: " << file_name << "->" << new_fn << endl;
    file->copy(new_file);
    file->remove();
    set_fname(new_fn, new_file->get_basename());
    set_dirty(FALSE);
    get_window()->invalidate(TRUE);
}
