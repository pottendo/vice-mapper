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
 * File:		map-controls.cc
 * Date:		Fri May  8 18:30:58 2020
 * Author:		pottendo (pottendo)
 * 
 * Abstract:
 *      Control widget implementation
 *
 * Modifications:
 * 	$Log$
 */

#include <iostream>
#include <gtkmm/label.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scale.h>
#include "VmMapControls.h"
#include "VmMap.h"
#include "dialogs.h"

using namespace::std;


VmMapControls::VmMapControls(VmMap &m)
    : mw(m)
{
    cf = nullptr;
    builder->get_widget("VmMapControls", cf);
    unpl_tilesbox = nullptr;
    builder->get_widget("VmMapControlsUnplTilesBox", unpl_tilesbox);
    button_save = nullptr;
    builder->get_widget("VmMapControlsSave", button_save);
    button_save->set_image_from_icon_name("document-save-symbolic");
    Gtk::Button *p = nullptr;
    builder->get_widget("VmMapControlsReload", p);
    p->set_image_from_icon_name("document-open-symbolic");
    adj[CZX] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmMapControlsZoomAdjX"));
    adj[CZY] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmMapControlsZoomAdjY"));
    adj[CRUP] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjUp"));
    adj[CRDO] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjDown"));
    adj[CRLE] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjLeft"));
    adj[CRRI] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjRight"));
    adj[CRUP]->set_upper(VmTile::min_resY/2);
    adj[CRDO]->set_upper(VmTile::min_resY/2-1);
    adj[CRLE]->set_upper(VmTile::min_resX/2);
    adj[CRRI]->set_upper(VmTile::min_resX/2-1);
    zl = nullptr;
    builder->get_widget("VmMapControlsZoomLock", zl);
    zoom_lock = zl->get_active();
}

void
VmMapControls::add_tile(VmTile *tile) 
{
    mw_out << __FUNCTION__ << ": '" << *tile << "'" << endl;
    unpl_tilesbox->pack_end(*tile, FALSE, FALSE, 4);
    unpl_tilesbox->show_all();
}

void
VmMapControls::remove_tile(VmTile *tile) 
{
    unpl_tilesbox->remove(*tile);
}

void
VmMapControls::commit_changes(void)
{
    VmMsg m("Save?");
    if (m.run() == Gtk::RESPONSE_OK) {
	for_each(VmTile::all_tiles.begin(), VmTile::all_tiles.end(),
		 [](VmTile *t)->void { t->commit_changes(); } );
	set_dirty(FALSE);
	mw_map->save_settings();
    }
}

void
VmMapControls::set_dirty(bool d) 
{
    if (d) {
	button_save->set_label("SAVE");
	button_save->set_sensitive(TRUE);
    } else {
	button_save->set_label("all saved");
	button_save->set_sensitive(FALSE);
    }
}

void
VmMapControls::set_zoom(double x, double y, bool dirty) 
{
    get_adj(CZX)->set_value(x);
    get_adj(CZY)->set_value(y);
    mw_map->set_dirty(dirty);
}

void
VmMapControls::set_crop(int u, int d, int l, int r, bool dirty) 
{
    get_adj(CRUP)->set_value(u);
    get_adj(CRDO)->set_value(d);
    get_adj(CRLE)->set_value(l);
    get_adj(CRRI)->set_value(r);
    mw_map->set_dirty(dirty);
}

void
VmMapControls::update_ctrls(void)
{
    adj[CRUP]->set_upper(VmTile::min_resY/2);
    adj[CRDO]->set_upper(VmTile::min_resY/2-1);
    adj[CRLE]->set_upper(VmTile::min_resX/2);
    adj[CRRI]->set_upper(VmTile::min_resX/2-1);
}

void
VmMapControls::toggle_zoom_lock(void) 
{
    zoom_lock = zl->get_active();
    if (zoom_lock) {
	double nv = (adj[CZX]->get_value() + adj[CZY]->get_value()) / 2;
	adj[CZX]->set_value(nv);
	adj[CZY]->set_value(nv);
    }
}

extern "C" 
{
    
G_MODULE_EXPORT bool
on_VmMapControlsReload_clicked(GdkEventButton *) {
    mw_map->reload_unplaced_tiles();
    return TRUE;
}

G_MODULE_EXPORT bool
on_VmMapControlsSave_clicked(GdkEventButton *) 
{
    //mw_out << "Save button pressed." << endl;
    mw_ctrls->commit_changes();
    return TRUE;
}

G_MODULE_EXPORT void
on_VmMapControlsZoomAdjX_value_changed() 
{
    //mw_out << "Scale 1: " << adjx->get_value() << endl;
    VmMap::scale_factor_x = mw_ctrls->get_adj(CZX)->get_value();
    if (mw_ctrls->get_zoom_lock()) {
	VmMap::scale_factor_y = VmMap::scale_factor_x;
	mw_ctrls->get_adj(CZY)->set_value(VmMap::scale_factor_x);
    }
    mw_map->scale_all();
    mw_map->set_dirty(true);
}
    
G_MODULE_EXPORT void
on_VmMapControlsZoomAdjY_value_changed() 
{
    //mw_out << "Scale 1: " << adjx->get_value() << endl;
    VmMap::scale_factor_y = mw_ctrls->get_adj(CZY)->get_value();
    if (mw_ctrls->get_zoom_lock()) {
	VmMap::scale_factor_x = VmMap::scale_factor_y;
	mw_ctrls->get_adj(CZX)->set_value(VmMap::scale_factor_y);
    }
    mw_map->scale_all();
    mw_map->set_dirty(true);
}
    
G_MODULE_EXPORT void
on_VmControlsCropAdj_value_changed() 
{
    //mw_out << "Scale 2: " << adjy->get_value() << endl;
    VmTile::cr_up = mw_ctrls->get_adj(CRUP)->get_value();
    VmTile::cr_do = mw_ctrls->get_adj(CRDO)->get_value();
    VmTile::cr_le = mw_ctrls->get_adj(CRLE)->get_value();
    VmTile::cr_ri = mw_ctrls->get_adj(CRRI)->get_value();
    std::for_each(VmTile::all_tiles.begin(), VmTile::all_tiles.end(),
		  [](VmTile *t)->void { t->queue_draw(); } );
    mw_map->set_dirty(true);
}

G_MODULE_EXPORT void
on_VmMapControlsZoomLock_toggled() 
{
    mw_out << __FUNCTION__ << ": called." << endl;
    mw_ctrls->toggle_zoom_lock();
}
    
} /* extern "C" */

