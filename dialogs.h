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

class MyDialog 			/* let's see if this makes sense for common stuff */
{
    int dlg_ret;
  public:
    MyDialog();
    ~MyDialog() {};
};

class MyMsg : public MyDialog, public Gtk::MessageDialog {
  public:
    MyMsg(std::string s1, std::string s2);
    virtual ~MyMsg() {};
};

#endif /* __dialogs_h__ */
