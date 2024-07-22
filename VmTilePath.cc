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
 * File:		VmTilePath.cc
 * Date:		Tue May  5 21:23:45 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     path tile class 
 * 
 * Modifications:
 * 	$Log$
 */
#include <cairomm/context.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <gdkmm/pixbuf.h>
#include <gtkmm/icontheme.h>
#include "VmTilePath.h"
#include "VmMap.h"

VmTilePath::VmTilePath(VmMap &m, int x, int y, VmPathDir d)
    : VmTile(m, x, y),
      dir(d)
{
    mw_out << __FUNCTION__ << ": created: " << *this << endl;
}

VmTilePath::VmTilePath(VmMap &m, VmPathDir d)
    : VmTile(m, -1, -1),
      dir(d)
{
    switch (d) {
    case P_N: angle = 0;
	break;
    case P_NW: angle = -45;
	break;
    case P_NE: angle = 45;
	break;
    case P_W: angle = -90;
	break;
    case P_E: angle = 90;
	break;
    case P_SW: angle = -135;
	break;
    case P_SE: angle = -135;
	break;
    case P_S: angle = 180;
	break;
    }
    
    Glib::RefPtr<Gtk::IconTheme> it = Gtk::IconTheme::get_default();
    m_image_scaled = m_image = it->load_icon("go-up-symbolic", 24);
    w = m_image->get_width();
    h = m_image->get_height();
    
    set_size_request(m_image->get_width(), m_image->get_height());

    mw_out << "image: " << m_image->get_width() << "x" << m_image->get_height() << endl;
    mw_debug->put_pixbuf(m_image);
    mw_out << __FUNCTION__ << ": created for MapControlWidget." << endl;
}

void
VmTilePath::on_draw_specific(const Cairo::RefPtr<Cairo::Context>& cr)
{
}

bool
VmTilePath::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) 
{
    //mw_out << __FUNCTION__ << ": called." << endl;
    cr->set_source_rgb(0.9, 0.9, 0.9);
    cr->move_to(0, 0);
    cr->line_to(m_image_scaled->get_width(), 0);
    cr->line_to(m_image_scaled->get_width(), m_image_scaled->get_height());
    cr->line_to(0, m_image_scaled->get_height());
    cr->line_to(0, 0);
    cr->stroke();
    cr->rotate(angle);
    Gdk::Cairo::set_source_pixbuf(cr, m_image_scaled,
				  m_image_scaled->get_width(),
				  m_image_scaled->get_height());
    cr->paint();
    return TRUE;
}
