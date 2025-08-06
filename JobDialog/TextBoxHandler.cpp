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
#include "TextBoxHandler.hpp"

using namespace Gtk;
using namespace JobDialog;

// static member initialization.
TextBoxHandler * TextBoxHandler::pUniqueInstance_d = 0;

TextBoxHandler::
TextBoxHandler ()
{
}

TextBoxHandler * TextBoxHandler::
instance ()
{
   if (pUniqueInstance_d == 0)
      pUniqueInstance_d = new TextBoxHandler ();

   return pUniqueInstance_d;
}

bool TextBoxHandler::
populateValues (Widget *pWidget, DriverProperty *pDriverProperty)
{
   Entry *pEntry = safeDowncast (pWidget);

   if (pEntry == 0)
      return false;

   // There will be only one value hence the getAvailableValues
   //  and getDefaultExternalValue both should return the same value.
   pEntry->set_text (pDriverProperty->getDefaultExternalValue ());

   // Disable the Widget if not editable.
   pEntry->set_sensitive (pDriverProperty->isEditable ());

   return true;
}

string TextBoxHandler::
getSelectedValue (Widget *pWidget)
{
   Entry *pEntry = safeDowncast (pWidget);

   if (pEntry == 0)
      return "";    // @INVESTIGATE Better throw 'WidgetMismatchException'

   return pEntry->get_text ();
}

inline
Entry* TextBoxHandler::
safeDowncast (Widget *pWidget)
{
   if (!Entry::isA (pWidget))
      return 0;      // @INVESTIGATE Might need to throw 'WidgetMismatchException' here.

   return (Entry *)pWidget;   // it is a safe downcasting now ...
}
