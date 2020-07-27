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
 * File:		VmTileEmpty.cc
 * Date:		Tue May  5 21:23:45 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     empty tile class 
 * 
 * Modifications:
 * 	$Log$
 */

#include "VmTileEmpty.h"
#include "VmMap.h"

VmTileEmpty::VmTileEmpty(VmMap &m, int x, int y)
    : VmTile(m, x, y)
{
    file_name = file_basename = "<empty>";
    m_image = VmMap::empty_image;
    drag_source_set_icon(m_image->scale_simple(m_image->get_width()/3,
					       m_image->get_height()/3,
					       Gdk::INTERP_BILINEAR));
    empty = true;
    //mw_out << __FUNCTION__ << ": created." << endl;
}

void
VmTileEmpty::on_draw_specific(const Cairo::RefPtr<Cairo::Context>& cr)
{
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
