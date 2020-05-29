// -*-c++-*-
// $Id$
//
// File:		map-controls.cc
// Date:		Fri May  8 18:30:58 2020
// Author:		pottendo (pottendo)
// 
// Abstract:
//      Control widget implementation
//
// Modifications:
// 	$Log$
//
#include <iostream>
#include <gtkmm/label.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scale.h>
#include "map-controls.h"
#include "map-window.h"
#include "dialogs.h"

using namespace::std;

map_controls::map_controls(map_window &m, const Glib::ustring &name)
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
    
    adjx = Gtk::Adjustment::create(3.0, 0.1, 6.0, 0.05, 0.1, 0);
    Gtk::Scale *scalex = new Gtk::Scale(adjx);
    // signal handler
    adjx->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_event1));
    zvb->pack_start(*scalex, FALSE, FALSE, 0);
    
    adjy = Gtk::Adjustment::create(3.0, 0.1, 6.0, 0.05, 0.1, 0);
    Gtk::Scale *scaley = new Gtk::Scale(adjy);
    // signal handler
    adjy->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_event2));
    zvb->pack_start(*scaley, FALSE, FALSE, 0);

    vb->pack_start(*zoom_frame, FALSE, FALSE, 0);

    Gtk::Frame *crop_frame = new Gtk::Frame("Crop");
    crop_frame->set_halign(Gtk::ALIGN_END);
    Gtk::VBox *cvb = new Gtk::VBox;
    cvb->set_halign(Gtk::ALIGN_END);
    cvb->set_size_request(200, -1);
    
    crop_frame->add(*cvb);
    
    adj_crup = Gtk::Adjustment::create(36.0, 0, resY/2, 1.0, 10.0, 0);
    Gtk::Scale *scale_crup = new Gtk::Scale(adj_crup);
    // signal handler
    adj_crup->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_crop));
    cvb->pack_start(*scale_crup, FALSE, FALSE, 0);
    
    adj_crdo = Gtk::Adjustment::create(36.0, 0, resY/2-1, 1.0, 10.0, 0);
    Gtk::Scale *scale_crdo = new Gtk::Scale(adj_crdo);
    // signal handler
    adj_crdo->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_crop));
    cvb->pack_start(*scale_crdo, FALSE, FALSE, 0);

    adj_crle = Gtk::Adjustment::create(32.0, 0, resX/2, 1.0, 10.0, 0);
    Gtk::Scale *scale_crle = new Gtk::Scale(adj_crle);
    // signal handler
    adj_crle->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_crop));
    cvb->pack_start(*scale_crle, FALSE, FALSE, 0);

    adj_crri = Gtk::Adjustment::create(32.0, 0, resX/2-1, 1.0, 10.0, 0);
    Gtk::Scale *scale_crri = new Gtk::Scale(adj_crri);
    // signal handler
    adj_crri->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_crop));
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
	.connect(sigc::mem_fun(*this, &map_controls::on_reload_press_event), false);
    vb->pack_start(button_reload, FALSE, FALSE, 0);
    
// button_commit.add_events(Gdk::BUTTON_PRESS_MASK);
    button_commit.signal_button_press_event()
	.connect(sigc::mem_fun(*this, &map_controls::on_commit_press_event), false);
    button_commit.set_sensitive(FALSE);
    button_commit.set_label("all saved");
    
    vb->pack_start(button_commit, FALSE, FALSE, 0);

    add(*vb);
    show_all_children();
}

void
map_controls::commit_changes(void)
{
    MyMsg m("Save?", "bla");
    if (m.run() == Gtk::RESPONSE_OK) {
	for_each(MyArea::all_tiles.begin(), MyArea::all_tiles.end(),
		 [](MyArea *t)->void { t->commit_changes(); } );
	set_dirty(FALSE);
    }
}

bool
map_controls::on_commit_press_event(GdkEventButton *) 
{
    //cout << "Save button pressed." << endl;
    commit_changes();
    return TRUE;
}


bool
map_controls::on_reload_press_event(GdkEventButton *) {
    //cout << __FUNCTION__ << ": called." << endl;
    mw.reload_unplaced_tiles();
    return TRUE;
}

void
map_controls::on_scale_event1() 
{
    //cout << "Scale 1: " << adjx->get_value() << endl;
    map_window::scale_factor_x = adjx->get_value();
    mw.scale_all();
}

void
map_controls::on_scale_event2() 
{
    //cout << "Scale 2: " << adjy->get_value() << endl;
    map_window::scale_factor_y = adjy->get_value();
    mw.scale_all();
}

void
map_controls::on_scale_crop() 
{
    //cout << "Scale 2: " << adjy->get_value() << endl;
    MyArea::cr_up = adj_crup->get_value();
    MyArea::cr_do = adj_crdo->get_value();
    MyArea::cr_le = adj_crle->get_value();
    MyArea::cr_ri = adj_crri->get_value();
    std::for_each(MyArea::all_tiles.begin(), MyArea::all_tiles.end(),
		  [](MyArea *t)->void {t->get_window()->invalidate(TRUE);} );
}

void
map_controls::set_zoom(double x, double y) 
{
    adjx->set_value(x);
    adjy->set_value(y);
}

void
map_controls::add_tile(MyArea *tile) 
{
    unpl_tilesbox.pack_end(*tile, FALSE, FALSE, 4);
    unpl_tilesbox.show_all();
}

void
map_controls::remove_tile(MyArea *tile) 
{
    unpl_tilesbox.remove(*tile);
}

void
map_controls::set_dirty(bool d) 
{
    if (d) {
	button_commit.set_label("SAVE");
	button_commit.set_sensitive(TRUE);
    } else {
	button_commit.set_label("all saved");
	button_commit.set_sensitive(FALSE);
    }
}
