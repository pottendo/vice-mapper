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
#include <string>
#include <gtkmm/messagedialog.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/builder.h>
#include "dialogs.h"

using namespace::std;

VmStatus::VmStatus(void)
{
    my_status[STATL] = my_status[STATM] = my_status[STATR] = nullptr;
	
    builder->get_widget("VMStatusL", my_status[STATL]);
    builder->get_widget("VMStatusM", my_status[STATM]);
    builder->get_widget("VMStatusR", my_status[STATR]);
}

void
VmStatus::clear(void) 
{
    my_status[STATL]->set_label("");
    my_status[STATM]->set_label("");
    my_status[STATR]->set_label("");
}

VmMsg::VmMsg(std::string s1, std::string s2) // 
    : Gtk::MessageDialog(s1, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL)
{
    set_secondary_text(s2);
}

VmAbout::VmAbout() 
{
    Gtk::AboutDialog* pDialog = nullptr;
    builder->get_widget("VMAbout", pDialog);
    pDialog->set_version(mapper_version);
    pDialog->run();
}

VmDebug::VmDebug()
{
    p_win = nullptr;
    builder->get_widget("VMDebugWindow", p_win);
    p_win->set_default_size(400, 300);
    p_win->set_title("vice-mapper - debug console");
    p_win->show_all_children();
    b = nullptr;
    builder->get_widget("VMDebugButton", b);
    tv = nullptr;
    builder->get_widget("VMDebugTextView", tv);
    tb = Gtk::TextBuffer::create();
    tv->set_buffer(tb);
}

void
VmDebug::toggle(void) 
{
    bool s = b->get_active();
    if (s) p_win->show();
    else p_win->hide();
}

void
VmDebug::log(std::string &s) 
{
    tb->insert_at_cursor(s);
    tb->insert_at_cursor("\n");
    tb->get_bounds(ti1, ti2);
    tv->scroll_to(ti2);
}

int
VmDebug::overflow(int c)
{
    static string s = "";
    if (c != '\n') {
	s += (char)c;
    }
    else
    {
	cout << s << "'" << endl;
	log(s);
	s = "";
    }
    
    return c;
}

/* callbacks from Main Menubar */
extern "C"  {
G_MODULE_EXPORT void
on_MenuAbout_activate(Gtk::MenuItem *m) 
{
    VmAbout about;
}

G_MODULE_EXPORT void
on_MenuOpen_activate(Gtk::MenuItem *m) 
{
    
    if (mw_map->is_dirty() &&
	VmMsg("Open new map?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    mw_map->open_map();
}

G_MODULE_EXPORT void
on_MenuClose_activate(Gtk::MenuItem *m) 
{
    if (mw_map->is_dirty() &&
	VmMsg("Close map?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    mw_map->remove_map();
}

G_MODULE_EXPORT void
on_MenuPrint_activate(Gtk::MenuItem *m) 
{
    VmMsg("Print not yet implemented!", ":-(").run();
}

G_MODULE_EXPORT void
on_MenuQuit_activate(Gtk::MenuItem *m) 
{
    if (mw_map->is_dirty() &&
	VmMsg("Really Quit?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    app->quit();
}

G_MODULE_EXPORT void
on_MenuSettings_activate(Gtk::MenuItem *m) 
{
    VmMsg("Settings", "lost").run();
}
    
G_MODULE_EXPORT void
on_VMDebugButton_toggled(Gtk::ToggleButton *m) 
{
    mw_debug->toggle();
}
    
} /* extern "C" */
