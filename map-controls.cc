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
#include <experimental/filesystem>
#include "map-controls.h"
#include "map-window.h"

using namespace::std;

map_controls::map_controls(map_window &m, const Glib::ustring &name)
    : Gtk::Frame(name),
      button_quit("Quit"),
      mw(m)
{
    cout << __FILE__ << name << " created." << endl;

    Gtk::VBox *vb = new Gtk::VBox;
    
    Gtk::Frame *zoom_frame = new Gtk::Frame("Zoom");
    Gtk::VBox *zvb = new Gtk::VBox;
    zoom_frame->add(*zvb);
    
    adjx = Gtk::Adjustment::create(3.0, 0.1, 4.0, 0.05, 0.1, 0);
    Gtk::Scale *scalex = new Gtk::Scale(adjx);
    // signal handler
    adjx->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_event1));
    zvb->pack_start(*scalex, FALSE, FALSE, 0);
    
    adjy = Gtk::Adjustment::create(3.0, 0.1, 4.0, 0.05, 0.1, 0);
    Gtk::Scale *scaley = new Gtk::Scale(adjy);
    // signal handler
    adjy->signal_value_changed()
	.connect(sigc::mem_fun(*this, &map_controls::on_scale_event2));
    zvb->pack_start(*scaley, FALSE, FALSE, 0);

    vb->pack_start(*zoom_frame, FALSE, FALSE, 0);

    Gtk::Frame *crop_frame = new Gtk::Frame("Crop");
    Gtk::VBox *cvb = new Gtk::VBox;
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

    button_quit.add_events(Gdk::BUTTON_PRESS_MASK);
    button_quit.signal_button_press_event()
	.connect(sigc::mem_fun(*this, &map_controls::on_button_quit_press_event));

    Gtk::Frame *tiles_frame = new Gtk::Frame("Unplaced tiles");
    Gtk::VBox *tvb = new Gtk::VBox;
    tiles_frame->add(*tvb);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    tvb->add(m_ScrolledWindow);
    m_ScrolledWindow.add(m_TreeView);
    m_ScrolledWindow.set_size_request(-1, 200);
    
    //Create the Tree model:
    m_refTreeModel = Gtk::ListStore::create(m_Columns);
    m_TreeView.set_model(m_refTreeModel);
    m_TreeView.enable_model_drag_source(MyArea::listTargets);
    //m_TreeView.enable_model_drag_dest(MyArea::listTargets);
    
    m_TreeView.append_column("Tile", m_Columns.m_col_tilepixmap);
    m_TreeView.append_column("Name", m_Columns.m_col_fname);

    vb->pack_start(*tiles_frame, FALSE, FALSE, 0);
    
    vb->pack_start(button_quit, FALSE, FALSE, 0);

    add(*vb);
    show_all_children();
}

bool
map_controls::on_button_quit_press_event(GdkEventButton *) 
{
    cout << "Quit button pressed." << endl;
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
    std::experimental::filesystem::path p(tile->get_fname());
    Gtk::TreeModel::Row row = *(m_refTreeModel->append());
    row[m_Columns.m_col_fname] = p.filename().string();
    row[m_Columns.m_col_tilepixmap] = tile->get_pixmap_icon();
}
