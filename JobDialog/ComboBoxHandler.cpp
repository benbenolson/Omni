/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2002
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
#include "ComboBoxHandler.hpp"

using namespace Gtk;
using namespace JobDialog;

ComboBoxHandler::
ComboBoxHandler ()
{
}

// static member initialization.
ComboBoxHandler * ComboBoxHandler::pUniqueInstance = 0;

ComboBoxHandler * ComboBoxHandler::
instance ()
{
   if (pUniqueInstance == 0)
      pUniqueInstance = new ComboBoxHandler ();
	
   return pUniqueInstance;
}

bool ComboBoxHandler::
populateValues (Widget         *pWidget,
                DriverProperty *pDriverProperty)
{
   Combo *pCombo = safeDowncast (pWidget);
	
   if (pCombo == 0)
      return false;
	
   pCombo->set_popdown_strings (pDriverProperty->getAvailableValues ());
   pCombo->get_entry ()->set_text (pDriverProperty->getDefaultExternalValue ());

   // Disable the widget if not editable.
   pCombo->get_entry ()->set_sensitive (pDriverProperty->isEditable ());
	
   return true;
}

string ComboBoxHandler::
getSelectedValue (Widget *pWidget)
{
   Combo *pCombo = safeDowncast (pWidget);
	
   if (pCombo == 0)
      return "";    // @TBD Better throw 'WidgetMismatchException'

   return pCombo->get_entry ()->get_text ();	
}
	
inline
Combo * ComboBoxHandler::
safeDowncast (Widget *pWidget)
{
   if (!Combo::isA (pWidget))
      return 0;     // @TBD Need to throw 'WidgetMismatchException' here.
	
   return (Combo *)pWidget; 	 // it is a safe downcasting now ...
}	
