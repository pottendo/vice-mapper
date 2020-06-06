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
    adj[CZX] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmMapControlsZoomAdjX"));
    adj[CZY] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmMapControlsZoomAdjY"));
    adj[CRUP] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjUp"));
    adj[CRDO] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjDown"));
    adj[CRLE] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjLeft"));
    adj[CRRI] = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("VmControlsCropAdjRight"));
    adj[CRUP]->set_upper(resY/2);
    adj[CRDO]->set_upper(resY/2-1);
    adj[CRLE]->set_upper(resX/2);
    adj[CRRI]->set_upper(resX/2-1);
    zl = nullptr;
    builder->get_widget("VmMapControlsZoomLock", zl);
    zoom_lock = zl->get_active();
}

void
VmMapControls::add_tile(VmTile *tile) 
{
    mw_out << __FUNCTION__ << ": called." << endl;
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
    VmMsg m("Save?", "bla");
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
    
bool
on_VmMapControlsReload_clicked(GdkEventButton *) {
    mw_map->reload_unplaced_tiles();
    return TRUE;
}

bool
on_VmMapControlsSave_clicked(GdkEventButton *) 
{
    //mw_out << "Save button pressed." << endl;
    mw_ctrls->commit_changes();
    return TRUE;
}

void
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
    
void
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
    
void
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

void
on_VmMapControlsZoomLock_toggled() 
{
    mw_out << __FUNCTION__ << ": called." << endl;
    mw_ctrls->toggle_zoom_lock();
}
    
} /* extern "C" */

#if 0
VmMapControls::VmMapControls(VmMap &m, const Glib::ustring &name)
    : Gtk::Frame(name),
      button_commit("commit"),
      button_reload("reload"),
      mw(m),
      unpl_tilesbox(Gtk::VBox())
{
    set_halign(Gtk::ALIGN_END);
    
    Gtk::VBox *vb = new Gtk::VBox;
    vb->set_hexpand(FALSE);
    vb->set_vexpand(FALSE);
    vb->set_size_request(200, -1);
    
    Gtk::Frame *zoom_frame = new Gtk::Frame("Zoom");
    zoom_frame->set_halign(Gtk::ALIGN_END);
    Gtk::VBox *zvb = new Gtk::VBox;
    zvb->set_size_request(200,-1);
    zvb->set_halign(Gtk::ALIGN_END);
    zoom_frame->add(*zvb);
    
    adjx = Gtk::Adjustment::create(def_zoom, 0.1, def_zoom*2, 0.05, 0.1, 0);
    Gtk::Scale *scalex = new Gtk::Scale(adjx);
    // signal handler
    adjx->signal_value_changed()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_scale_event1));
    zvb->pack_start(*scalex, FALSE, FALSE, 0);
    
    adjy = Gtk::Adjustment::create(def_zoom, 0.1, def_zoom*2, 0.05, 0.1, 0);
    Gtk::Scale *scaley = new Gtk::Scale(adjy);
    // signal handler
    adjy->signal_value_changed()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_scale_event2));
    zvb->pack_start(*scaley, FALSE, FALSE, 0);

    vb->pack_start(*zoom_frame, FALSE, FALSE, 0);

    Gtk::Frame *crop_frame = new Gtk::Frame("Crop");
    crop_frame->set_halign(Gtk::ALIGN_END);
    Gtk::VBox *cvb = new Gtk::VBox;
    cvb->set_halign(Gtk::ALIGN_END);
    cvb->set_size_request(200, -1);
    
    crop_frame->add(*cvb);
    
    adj_crup = Gtk::Adjustment::create(def_cry, 0, resY/2, 1.0, 10.0, 0);
    Gtk::Scale *scale_crup = new Gtk::Scale(adj_crup);
    scale_crup->set_digits(0);
    // signal handler
    adj_crup->signal_value_changed()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_scale_crop));
    cvb->pack_start(*scale_crup, FALSE, FALSE, 0);
    
    adj_crdo = Gtk::Adjustment::create(def_cry, 0, resY/2-1, 1.0, 10.0, 0);
    Gtk::Scale *scale_crdo = new Gtk::Scale(adj_crdo);
    scale_crdo->set_digits(0);
    // signal handler
    adj_crdo->signal_value_changed()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_scale_crop));
    cvb->pack_start(*scale_crdo, FALSE, FALSE, 0);

    adj_crle = Gtk::Adjustment::create(def_crx, 0, resX/2, 1.0, 10.0, 0);
    Gtk::Scale *scale_crle = new Gtk::Scale(adj_crle);
    scale_crle->set_digits(0);
    // signal handler
    adj_crle->signal_value_changed()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_scale_crop));
    cvb->pack_start(*scale_crle, FALSE, FALSE, 0);

    adj_crri = Gtk::Adjustment::create(def_crx, 0, resX/2-1, 1.0, 10.0, 0);
    Gtk::Scale *scale_crri = new Gtk::Scale(adj_crri);
    scale_crri->set_digits(0);
    // signal handler
    adj_crri->signal_value_changed()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_scale_crop));
    cvb->pack_start(*scale_crri, FALSE, FALSE, 0);

    
    vb->pack_start(*crop_frame, FALSE, FALSE, 0);

    Gtk::Frame *tiles_frame = new Gtk::Frame("Unplaced tiles");
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    tiles_frame->add(m_ScrolledWindow);
    unpl_tilesbox.set_halign(Gtk::ALIGN_END);
    m_ScrolledWindow.add(unpl_tilesbox);
    
    m_ScrolledWindow.set_size_request(200, -1);
    vb->pack_start(*tiles_frame, TRUE, TRUE, 0);
    
    button_reload.signal_button_press_event()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_reload_press_event), false);
    vb->pack_start(button_reload, FALSE, FALSE, 0);
    
// button_commit.add_events(Gdk::BUTTON_PRESS_MASK);
    button_commit.signal_button_press_event()
	.connect(sigc::mem_fun(*this, &VmMapControls::on_commit_press_event), false);
    button_commit.set_sensitive(FALSE);
    button_commit.set_label("all saved");
    
    vb->pack_start(button_commit, FALSE, FALSE, 0);

    add(*vb);
    show_all_children();
}

void
VmMapControls::commit_changes(void)
{
    VmMsg m("Save?", "bla");
    if (m.run() == Gtk::RESPONSE_OK) {
	for_each(VmTile::all_tiles.begin(), VmTile::all_tiles.end(),
		 [](VmTile *t)->void { t->commit_changes(); } );
	set_dirty(FALSE);
	mw.save_settings();
    }
}

bool
VmMapControls::on_commit_press_event(GdkEventButton *) 
{
    //mw_out << "Save button pressed." << endl;
    commit_changes();
    return TRUE;
}


bool
VmMapControls::on_reload_press_event(GdkEventButton *) {
    //mw_out << __FUNCTION__ << ": called." << endl;
    mw.reload_unplaced_tiles();
    return TRUE;
}

void
VmMapControls::on_scale_event1() 
{
    //mw_out << "Scale 1: " << adjx->get_value() << endl;
    VmMap::scale_factor_x = adjx->get_value();
    mw.scale_all();
    mw.set_dirty(true);
}

void
VmMapControls::on_scale_event2() 
{
    //mw_out << "Scale 2: " << adjy->get_value() << endl;
    VmMap::scale_factor_y = adjy->get_value();
    mw.scale_all();
    mw.set_dirty(true);
}

void
VmMapControls::on_scale_crop() 
{
    //mw_out << "Scale 2: " << adjy->get_value() << endl;
    VmTile::cr_up = adj_crup->get_value();
    VmTile::cr_do = adj_crdo->get_value();
    VmTile::cr_le = adj_crle->get_value();
    VmTile::cr_ri = adj_crri->get_value();
    std::for_each(VmTile::all_tiles.begin(), VmTile::all_tiles.end(),
		  [](VmTile *t)->void {if (t->get_window()) t->get_window()->invalidate(TRUE);} );
    mw.set_dirty(true);
}

void
VmMapControls::set_zoom(double x, double y, bool dirty) 
{
    adjx->set_value(x);
    adjy->set_value(y);
    mw.set_dirty(dirty);
}

void
VmMapControls::set_crop(int u, int d, int l, int r, bool dirty) 
{
    adj_crup->set_value(u);
    adj_crdo->set_value(d);
    adj_crle->set_value(l);
    adj_crri->set_value(r);
    mw.set_dirty(dirty);
}

void
VmMapControls::add_tile(VmTile *tile) 
{
    unpl_tilesbox.pack_end(*tile, FALSE, FALSE, 4);
    unpl_tilesbox.show_all();
}

void
VmMapControls::remove_tile(VmTile *tile) 
{
    unpl_tilesbox.remove(*tile);
}

void
VmMapControls::set_dirty(bool d) 
{
    if (d) {
	button_commit.set_label("SAVE");
	button_commit.set_sensitive(TRUE);
    } else {
	button_commit.set_label("all saved");
	button_commit.set_sensitive(FALSE);
    }
}

#endif
