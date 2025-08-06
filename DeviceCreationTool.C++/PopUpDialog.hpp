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
#ifndef _COM_IBM_LTC_OMNI_PopUpDialog_hpp_
#define _COM_IBM_LTC_OMNI_PopUpDialog_hpp_

#include <gtk--.h>

using SigC::slot;
using namespace Gtk;

class PopUpDialog: public Gtk::Window
  {
   public:
      Gtk::HButtonBox *box2;
      Gtk::Button *okay_b;
      Gtk::Button *cancel_b;

  private:
      bool canceled;
      void cancel()
      {
         canceled=true;
         Gtk::Kit::quit();
      }

      void okay()
      {
          Gtk::Kit::quit();
      }


  public:

      ~PopUpDialog(){};
      bool run()
       {
          Gtk::Kit::run();
          if (canceled)
             return false;
          return true;
       }

      gint delete_event_impl(GdkEventAny*) {return true;}

      static int pop_dialog(const string &info, const string &question)
       {
           PopUpDialog dialog(info, question);
           bool retVal = dialog.run();
           return retVal;
       }

    static int pop_dialog_OK(const string &info, const string &question)
    {
       PopUpDialog dialog(info, question);
       dialog.box2->remove(*(dialog.cancel_b));
       bool retVal = dialog.run();
       return retVal;
    }

PopUpDialog(const string &info, const string &question)
{
   canceled=false;
   set_modal(true);
   okay_b=manage(new Gtk::Button("OK"));
   cancel_b=manage(new Gtk::Button("Cancel"));
   okay_b->clicked.connect(slot(*this,&PopUpDialog::okay));
   cancel_b->clicked.connect(slot(*this,&PopUpDialog::cancel));
   box2 = manage(new Gtk::HButtonBox());
   box2->set_border_width(10);
   box2->set_spacing(20);
   box2->set_child_size(60,25);
   box2->pack_start(*okay_b,false);
   box2->pack_end(*cancel_b,false);
   Gtk::Box *box=manage(new Gtk::VBox());
   box->pack_start(*manage(new Gtk::Label(info)),true,true,10);
   box->pack_start(*manage(new Gtk::Label(question)), true, true, 10);
   box->pack_start(*box2,true,true,10);
   add(*box);
   set_border_width(10);
   set_usize(300,150);
   show_all();
 }

}; // end class definition

#endif
