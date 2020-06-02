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
 * File:		dialogs.cc
 * Date:		Sun May 24 13:50:37 2020
 * Author:		pottendo (pottendo)
 * 
 * Abstract:
 *      vice-mapper dialiogs
 *
 * Modifications:
 * 	$Log$
 */

#include <iostream>
#include <dialogs.h>
#include <string>
#include <gtkmm/messagedialog.h>
#include <gtkmm/builder.h>
#include "dialogs.h"

using namespace::std;

MyStatus::MyStatus(void)
{
    my_status[STATL] = my_status[STATM] = my_status[STATR] = nullptr;
	
    builder->get_widget("VMStatusL", my_status[STATL]);
    builder->get_widget("VMStatusM", my_status[STATM]);
    builder->get_widget("VMStatusR", my_status[STATR]);
}

void
MyStatus::clear(void) 
{
    my_status[STATL]->set_label("");
    my_status[STATM]->set_label("");
    my_status[STATR]->set_label("");
}

MyMsg::MyMsg(std::string s1, std::string s2) // 
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

/* callbacks from Main Menubar */
extern "C"  {
G_MODULE_EXPORT void
on_MenuAbout_activate(Gtk::MenuItem *m) 
{
    MyAbout about;
}

G_MODULE_EXPORT void
on_MenuOpen_activate(Gtk::MenuItem *m) 
{
    
    if (mw_map->is_dirty() &&
	MyMsg("Open new map?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    mw_map->open_map();
}

G_MODULE_EXPORT void
on_MenuClose_activate(Gtk::MenuItem *m) 
{
    if (mw_map->is_dirty() &&
	MyMsg("Close map?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    mw_map->remove_map();
}

G_MODULE_EXPORT void
on_MenuPrint_activate(Gtk::MenuItem *m) 
{
    MyMsg("Print not yet implemented!", ":-(").run();
}

G_MODULE_EXPORT void
on_MenuQuit_activate(Gtk::MenuItem *m) 
{
    if (mw_map->is_dirty() &&
	MyMsg("Really Quit?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    app->quit();
}

G_MODULE_EXPORT void
on_MenuSettings_activate(Gtk::MenuItem *m) 
{
    MyMsg("Settings", "lost").run();
}

    
} /* extern "C" */
