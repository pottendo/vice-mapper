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
#include <giomm/fileoutputstream.h>
#include "dialogs.h"
#include "VmTile.h"

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

void
VmStatus::status(void)
{
    mw_map->update_state();
    string stat1 = mw_map->is_dirty() ? "mod/" : "clean/";
    string stat2 = mw_map->is_placed() ? "placed|" : "empty|";
    
    string s = stat1 + stat2 + 	
	to_string(VmTile::xmin) + "/" + to_string(VmTile::ymin) + "x" +
	to_string(VmTile::xmax) + "/" + to_string(VmTile::ymax) + "|"
	+ to_string(mw_map->get_nrtiles()) + "/" + to_string(VmTile::alloc_count);
    show(VmStatus::STATR, s);
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

Gtk::Window *VmDebug::p_win = nullptr;
Gtk::ToggleButton *VmDebug::b = nullptr;
Gtk::TextView *VmDebug::tv = nullptr;
Glib::RefPtr<Gtk::TextBuffer> VmDebug::tb;
Gtk::TextBuffer::iterator VmDebug::ti;
VmDebug::VmDebug(debug_type_t t)
{
    if (p_win == nullptr) {
	builder->get_widget("VmDebugWindow", p_win);
	p_win->set_default_size(400, 300);
	p_win->set_title("vice-mapper - debug console");
	p_win->show_all_children();
	builder->get_widget("VmDebugButton", b);
	builder->get_widget("VMDebugTextView", tv);
	tb = Gtk::TextBuffer::create();
	tv->set_buffer(tb);
	ti = tb->get_iter_at_offset(0);
    }
    if (t == MW_ERR) {
	tag = Gtk::TextBuffer::Tag::create();
	tag->property_foreground() = "red";
    }
    else {
	tag = Gtk::TextBuffer::Tag::create();
	tag->property_foreground() = "blue";
    }
    tag->property_size() = 8 * Pango::SCALE;
    
    tb->get_tag_table()->add(tag);
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
    ti = tb->insert_with_tag(ti, s, tag);
    tv->scroll_to(ti);
}

void
VmDebug::put_pixbuf(Glib::RefPtr<Gdk::Pixbuf> p)
{
    ti = tb->insert_pixbuf(ti, p);
    ti = tb->insert_with_tag(ti, "\n", tag);
    tv->scroll_to(ti);
}

void
VmDebug::save(void)
{
    try {
	Glib::RefPtr<Gio::File> f = Gio::File::create_for_path("vice-mapper.log");
	Glib::RefPtr<Gio::FileOutputStream> lfstr = f->replace("", true);
	string text;

	mw_out << __FUNCTION__ << ": save logfile to "
	       << f->get_path() << G_DIR_SEPARATOR_S << f->get_basename() << endl;
	text = string(tb->get_text());
	
	if (lfstr->write(text) == false) {
	    mw_out << __FUNCTION__ << ": write log-file failed." << endl;
	    return;
	}
	lfstr->flush();
    }
    catch (Glib::Error &e) {
	mw_out << __FUNCTION__ << ": failed with " << e.what() << endl;
	return;
    }
}

int
VmDebug::overflow(int c)
{
    static string s = "";
    
    s += (char)c;
    if (c == '\n') {
	cout << s;
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
    mw_map->update_state();
    if (mw_map->is_dirty() &&
	VmMsg("Open new map?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    mw_map->open_map();
}

G_MODULE_EXPORT void
on_MenuExport_activate(Gtk::MenuItem *m) 
{
    mw_map->export_map();
}

G_MODULE_EXPORT void
on_MenuClose_activate(Gtk::MenuItem *m) 
{
    mw_map->update_state();
    if (mw_map->is_dirty() &&
	VmMsg("Close map?", "unsaved changes will be lost").run() != Gtk::RESPONSE_OK) {
	return;
    }
    mw_map->remove_map();
}

/* 
G_MODULE_EXPORT void
on_MenuPrint_activate(Gtk::MenuItem *m) 
{
    VmMsg("Print not yet implemented!", ":-(").run();
}
*/

G_MODULE_EXPORT void
on_MenuQuit_activate(Gtk::MenuItem *m) 
{
    mw_map->update_state();
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
on_VmDebugButton_toggled(Gtk::ToggleButton *m) 
{
    mw_debug->toggle();
}
    
G_MODULE_EXPORT void
on_VmDebugSave_clicked(Gtk::ToggleButton *m) 
{
    mw_debug->save();
}
    
} /* extern "C" */
