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
 * File:		dialogs.h
 * Date:		Sun May 24 13:50:53 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     vice-mapper dialogs
 * 
 * Modifications:
 * $Log$
 */

#ifndef __dialogs_h__
#define __dialogs_h__

#include <string>
#include <gtkmm/messagedialog.h>
#include <gtkmm/dialog.h>
#include <gtkmm/menu.h>
#include <gtkmm/builder.h>
#include <iostream>
#include "map-window.h"

using namespace::std;

class MyStatus
{
    Gtk::Label *my_status[3];
  public:
    MyStatus();
    ~MyStatus() {};

    typedef enum { STATL, STATM, STATR } MyStatusPos;
    void show(MyStatusPos w, std::string s) { my_status[w]->set_label(s); }
    void clear(void);
};

class MyMsg : public Gtk::MessageDialog {
  public:
    MyMsg(std::string s1, std::string s2);
    virtual ~MyMsg() {};
};

class MyAbout 
{
    Gtk::Dialog* pDialog = nullptr;
    
    static void on_exit_press(Gtk::Dialog &d) { d.hide(); }
    
  public:
    MyAbout(void);
    ~MyAbout() {} ;
};

extern Glib::RefPtr<Gtk::Builder> builder;
extern MyStatus *mw_status;
extern map_window *mw_map;
extern Glib::RefPtr<Gtk::Application> app;
extern Gtk::Window *mainWindow;
#endif /* __dialogs_h__ */
