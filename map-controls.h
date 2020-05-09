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

class map_controls : public Gtk::Frame
{
    Gtk::Button button_quit;
    bool on_button_press_event(GdkEventButton *) override;
  public:
    map_controls(const Glib::ustring &);
    virtual ~map_controls() {};
};

#endif /* __map_controls_h__ */
