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
    void status(void);
    void clear(void);
};

class VmMsg : public Gtk::MessageDialog {
  public:
    VmMsg(std::string s1, std::string s2 = "");
    virtual ~VmMsg() {};
};

class VmAbout
{
    Gtk::Dialog* pDialog = nullptr;
  public:
    VmAbout(void);
    ~VmAbout() {} ;
};

typedef enum { MW_OUT, MW_ERR } debug_type_t;
class VmDebug : public std::streambuf
{
    static Gtk::Window *p_win;
    static Gtk::ToggleButton *b;
    static Gtk::TextView *tv;
    static Glib::RefPtr<Gtk::TextBuffer> tb;
    static Gtk::TextBuffer::iterator ti;
    Glib::RefPtr<Gtk::TextBuffer::Tag> tag;

public:
    VmDebug(debug_type_t t);
    virtual ~VmDebug() { };

    void toggle();
    void log(std::string &s);
    void put_pixbuf(Glib::RefPtr<Gdk::Pixbuf> p);
    void save(void);
protected:
    virtual int_type overflow(int_type c);
};

extern Glib::RefPtr<Gtk::Builder> builder;
extern VmStatus *mw_status;
extern VmMap *mw_map;
extern VmMapControls *mw_ctrls;
extern VmDebug *mw_debug;
extern VmDebug *mw_debug2;
extern std::ostream *mw_out_stream;
extern std::ostream *mw_err_stream;
#define mw_out (*mw_out_stream)
#define mw_err (*mw_err_stream)
extern Glib::RefPtr<Gtk::Application> app;
extern Gtk::Window *mainWindow;

#endif /* __dialogs_h__ */
