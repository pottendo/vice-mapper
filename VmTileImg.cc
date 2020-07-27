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
 * File:		VmTileImg.cc
 * Date:		Tue May  5 21:23:45 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     image tile class 
 * 
 * Modifications:
 * 	$Log$
 */

#include <giomm/file.h>
#include <regex>
#include "VmTileImg.h"
#include "VmMap.h"

VmTileImg::VmTileImg(VmMap &m, const char *fn, int x, int y)
    : VmTile(m, x, y)
{
    Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(fn);
    set_fname(f->get_path(), f->get_basename());
    if (VmMap::current_path == "") {
	VmMap::current_path = f->get_parent()->get_path();
	mw_status->show(VmStatus::STATM, VmMap::current_path);
    }
	
    try {
	m_image_scaled = m_image = Gdk::Pixbuf::create_from_file(fn);//, resX, resY);
	w = m_image->get_width();
	h = m_image->get_height();
    }
    catch(const Gio::ResourceError& ex) {
	mw_err << "ResourceError: " << ex.what() << std::endl;
	throw -1;
    }
    catch(const Gdk::PixbufError& ex) {
	mw_err << "PixbufError: " << ex.what() << std::endl;
	throw -1;
    }

    set_size_request(m_image->get_width()/3, m_image->get_height()/3);
    drag_source_set_icon(m_image->scale_simple(m_image->get_width()/3,
					       m_image->get_height()/3,
					       Gdk::INTERP_BILINEAR));
    std::cmatch cm;
    std::regex re_ext("(.*)\\.(png|PNG|gif|GIF|jpg|JPG)");
    std::regex_match(file_basename.c_str(), cm, re_ext, std::regex_constants::match_default);
    if (cm.size() <= 0) {
	mw_out << __FUNCTION__ << ": couldn't find extension in name " << file_basename << endl;
	throw -1;
    }
    file_ext = cm[cm.size()-1];
    if (VmMap::current_ext == "") {
	VmMap::current_ext = file_ext;
	mw_out << __FUNCTION__ << ": extension set to '" << VmMap::current_ext << "'." << endl;
    }
    // heuristic to ignore full maps, which shouldn't be listed
    string file_base = string(cm[cm.size()-2]) + ".png"; 
    //mw_out << __FUNCTION__ << ": file base: " << file_base << endl;
	
    std::string regstr("(.*)" + std::string(def_basename) +
		       "([0-9][0-9])x([0-9][0-9]*)\\.(png|PNG|gif|GIF|jpg|JPG)");
    std::regex re(regstr); 
    std::regex_match(file_basename.c_str(), cm, re, std::regex_constants::match_default);
    /*
      mw_out << cm.size() << " matches for " << fn << " were: " << endl;
      for (unsigned i=0; i<cm.size(); ++i) {
      mw_out << "[" << cm[i] << "] ";
      }
      mw_out << endl;
    */
	
    if (cm.size() > 0) {	// OK, we've found a placed tile following the filename convention
	xk = std::stod(cm[cm.size()-3]);
	yk = std::stod(cm[cm.size()-2]);
	if ((xk == 0) || (yk == 0)) {
	    mw_out << __FUNCTION__ << "basename not following convention "
		   << def_basename << "XXxYY.<valid ext>): "
		   << file_basename << std::endl;
	    throw -1; // maps start at 01x01
	}
	(void) update_minmax();
	VmTile *t;
	if ((t = mw.get_tile(xk, yk)) != nullptr) {
	    if (t->is_empty()) {
		delete t;
		mw.set_tile(xk, yk, nullptr);
	    }
	    else {
		mw_out << __FUNCTION__ << ": refusing to overload tile with " << *this << endl;
		throw -1;
	    }
	}
	mw.add_tile(this);
    }
    else {			// some image file, but not following convention -> unplaced tile
	if (!m_image) {
	    mw_out << __FUNCTION__ << ": *** m_image == 0, but no exception..." << endl;
	    throw -1;
	}
	if (string(f->get_parent()->get_basename()) + ".png" == file_base){
	    mw_out << __FUNCTION__ << ": ignoring potential full map " << file_basename << endl;
	    throw -1;
	}
	m_image_icon = m_image->scale_simple(m_image->get_width()/4,
					     m_image->get_height()/4,
					     Gdk::INTERP_BILINEAR);
	xk = -1;		// xk == -1 is the internal convention for all unplaced tiles
	yk = -1;
	mw.add_unplaced_tile(this);
    }
    empty = false;
    all_tiles.insert(this);
    min_resX = MIN(min_resX, w);
    max_resX = MAX(max_resX, w);
    min_resY = MIN(min_resY, h);
    max_resY = MAX(max_resY, h);
    /*
      mw_out << __FUNCTION__ << ": minX/maX=" << min_resX << "/"
      << max_resX << ", minY/maxY=" << min_resY << "/" << max_resY << endl;
    */
    VmMap::nr_tiles++;
}
