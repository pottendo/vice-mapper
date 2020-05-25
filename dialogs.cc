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

#include <dialogs.h>
#include <string>
#include <gtkmm/messagedialog.h>
#include "dialogs.h"

MyDialog::MyDialog(void)
    : dlg_ret(Gtk::RESPONSE_CANCEL)
{
}

MyMsg::MyMsg(std::string s1, std::string s2)
    : Gtk::MessageDialog(s1, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL)
{
    set_secondary_text(s2);
}


