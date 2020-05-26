/*  -*-c-*-
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
#include "dialogs.h"

using namespace::std;

class MyStatus
{
    Gtk::Label *my_status;
  public:
    MyStatus();
    ~MyStatus() {};

    inline void show(std::string s) { my_status->set_label(s); }
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

#endif /* __dialogs_h__ */
