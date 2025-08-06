/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef _COM_IBM_LTC_OMNI_FileSelector_Hpp_
#define _COM_IBM_LTC_OMNI_FileSelector_Hpp_

#include <gtk--/fileselection.h>
#include <gtk--/main.h>
#include <string>

using namespace Gtk;

class FileSelector : public Gtk::FileSelection
{

public:
   bool isOKClicked() { return okClicked; }
   string getSelectedFileName() { return filename; }
   void showDialog() { show(); Gtk::Main::run(); return; }

protected:
   string filename;
   bool okClicked;

   gint delete_event_impl(GdkEventAny*)
   {
      Gtk::Main::quit();
      return 0;
   }

   void onOKButtonClicked()
   {
      filename = get_filename();
      okClicked = true;
      return;
   }

public:
   FileSelector::FileSelector(string title): FileSelection(title), okClicked(false)
   {
       get_ok_button()->clicked.connect(slot(this, &FileSelector::onOKButtonClicked) );
       get_cancel_button()->clicked.connect(hide.slot());
       get_ok_button()->clicked.connect(Gtk::Main::quit.slot());
       hide.connect(Gtk::Main::quit.slot());
       set_modal(true);
   }

   FileSelector::FileSelector(string title, string homeDir): FileSelection(title), okClicked(false)
   {
       set_filename(homeDir);
       get_ok_button()->clicked.connect(slot(this, &FileSelector::onOKButtonClicked) );
       get_cancel_button()->clicked.connect(hide.slot());
       get_ok_button()->clicked.connect(Gtk::Main::quit.slot());
       hide.connect(Gtk::Main::quit.slot());
       set_modal(true);
   }

};

#endif
