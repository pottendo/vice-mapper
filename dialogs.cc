// -*-c++-*-
// $Id$
//
// File:		dialogs.cc
// Date:		Sun May 24 13:50:37 2020
// Author:		pottendo (pottendo)
// 
// Abstract:
//      vice-mapper dialiogs
//
// Modifications:
// 	$Log$
//

#include <iostream>
#include <dialogs.h>
#include <string>
#include <gtkmm/messagedialog.h>
#include <gtkmm/builder.h>
#include "dialogs.h"

using namespace::std;

MyStatus::MyStatus(void)
    : my_status(nullptr)
{
    builder->get_widget("VMStatus", my_status);
    my_status->set_label("vice-mapper status bar... ");
}

MyMsg::MyMsg(std::string s1, std::string s2)
    : Gtk::MessageDialog(s1, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL)
{
    set_secondary_text(s2);
}


MyAbout::MyAbout() 
{
    Gtk::Dialog* pDialog = nullptr;
    builder->get_widget("VMAbout", pDialog);
    pDialog->run();
}

extern "C"
void
on_MenuAbout_activate(Gtk::MenuItem *m) 
{
    MyAbout about;
}
