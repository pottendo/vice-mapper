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

class map_controls : public Gtk::Frame
{
    Gtk::Button button_quit;
    bool on_button_quit_press_event(GdkEventButton *);
    void on_scale_event1();
    void on_scale_event2();
    void on_scale_crop();

    Glib::RefPtr<Gtk::Adjustment> adjx;
    Glib::RefPtr<Gtk::Adjustment> adjy;
    Glib::RefPtr<Gtk::Adjustment> adj_crup;
    Glib::RefPtr<Gtk::Adjustment> adj_crdo;
    Glib::RefPtr<Gtk::Adjustment> adj_crle;
    Glib::RefPtr<Gtk::Adjustment> adj_crri;

  public:
    map_controls(const Glib::ustring &);
    virtual ~map_controls() {};

    void set_zoom(double x, double y);
};

#endif /* __map_controls_h__ */
