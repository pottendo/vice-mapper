/*  -*-c-*-
 * File:		map-controls.h
 * Date:		Fri May  8 18:25:51 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     Controls widget
 * 
 * Modifications:
 * $Log$
 */

#ifndef __map_controls_h__
#define __map_controls_h__

#include <gtkmm/frame.h>
#include <gtkmm/button.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include "myarea.h"

class map_controls : public Gtk::Frame
{
    Gtk::Button button_commit;
    Gtk::Button button_reload;
    
    bool on_commit_press_event(GdkEventButton *);
    bool on_reload_press_event(GdkEventButton *);
    void on_scale_event1();
    void on_scale_event2();
    void on_scale_crop();

    map_window &mw;
    
    // Zoom controls
    Glib::RefPtr<Gtk::Adjustment> adjx;
    Glib::RefPtr<Gtk::Adjustment> adjy;
    // Crop controls
    Glib::RefPtr<Gtk::Adjustment> adj_crup;
    Glib::RefPtr<Gtk::Adjustment> adj_crdo;
    Glib::RefPtr<Gtk::Adjustment> adj_crle;
    Glib::RefPtr<Gtk::Adjustment> adj_crri;
    // Unplaced tiles
    Gtk::VBox unpl_tilesbox;
    
  protected:
    Gtk::ScrolledWindow m_ScrolledWindow;

  public:
    map_controls(map_window &m, const Glib::ustring &);
    virtual ~map_controls() {};
    
    void set_zoom(double x, double y);
    void add_tile(MyArea *tile);
    void remove_tile(MyArea *t);
    void set_dirty(bool d);
    void commit_changes(void);
};

#endif /* __map_controls_h__ */
