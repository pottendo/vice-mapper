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
#include <gtkmm/togglebutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/dialog.h>
#include <gtkmm/menu.h>
#include <gtkmm/builder.h>
#include <iostream>
#include "VmMap.h"

using namespace::std;

class VmStatus
{
    Gtk::Label *my_status[3];
  public:
    VmStatus();
    ~VmStatus() {};

    typedef enum { STATL, STATM, STATR } VmStatusPos;
    void show(VmStatusPos w, std::string s) { my_status[w]->set_label(s); }
    void clear(void);
};

class VmMsg : public Gtk::MessageDialog {
  public:
    VmMsg(std::string s1, std::string s2);
    virtual ~VmMsg() {};
};

class VmAbout 
{
    Gtk::Dialog* pDialog = nullptr;
    
    static void on_exit_press(Gtk::Dialog &d) { d.hide(); }
    
  public:
    VmAbout(void);
    ~VmAbout() {} ;
};

class VmDebug : public std::streambuf
{
    Gtk::Window *p_win;
    Gtk::ToggleButton *b;
    Gtk::TextView *tv;
    Glib::RefPtr<Gtk::TextBuffer> tb;
    Gtk::TextBuffer::iterator ti1, ti2;
public:
    VmDebug();
    virtual ~VmDebug() { };

    void toggle();
    void log(std::string &s);
    //friend std::ostream &operator<<(std::ostream &o, VmDebug &d);
protected:
    virtual int_type overflow(int_type c);
};

extern Glib::RefPtr<Gtk::Builder> builder;
extern VmStatus *mw_status;
extern VmMap *mw_map;
extern VmDebug *mw_debug;
extern std::ostream *mw_out_stream;
#define mw_out (*mw_out_stream)
extern Glib::RefPtr<Gtk::Application> app;
extern Gtk::Window *mainWindow;

#endif /* __dialogs_h__ */
