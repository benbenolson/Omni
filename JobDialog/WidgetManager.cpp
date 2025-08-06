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
#include "WidgetManager.hpp"

using namespace Gtk;
using namespace JobDialog;

WidgetManager::
WidgetManager ()
{
}

WidgetManager::
~WidgetManager ()
{
   // @INVESTIGATE
   delete pUniqueInstance_d;
   pUniqueInstance_d = 0;
}

// static member initialization.
WidgetManager * WidgetManager::pUniqueInstance_d = 0;

WidgetManager * WidgetManager::
instance ()
{
   if (pUniqueInstance_d == 0)
      pUniqueInstance_d = new WidgetManager;

   return pUniqueInstance_d;
}

Widget* WidgetManager::
createWidget (string          widgetName,
              DriverProperty *pDriverProperty)
{
   Widget *pWidget;

   // @INVESTIGATE
   if ("TextBox" == widgetName)
      pWidget = manage (new Entry ());
   else if ("ComboBox" == widgetName)
      pWidget = manage (new Combo ());
   else
      return 0;    // @INVESTIGATE Probably its better to throw an Exception here
                   //'UnKnownWidgetException'

   WidgetInterface *pWidgetInterface = getWidgetInterface (widgetName);

   pWidgetInterface->populateValues (pWidget, pDriverProperty);

   return pWidget;
}

string WidgetManager::
getSelectedValue (Widget *pWidget, string widgetName)
{
   WidgetInterface *pWidgetInterface = getWidgetInterface (widgetName);

   return pWidgetInterface->getSelectedValue (pWidget);
}

WidgetInterface * WidgetManager::
getWidgetInterface (string widgetName)
{
   WidgetInterface *pWidgetInterface;

   // @INVESTIGATE
   if ("TextBox" == widgetName)
      pWidgetInterface = TextBoxHandler::instance ();
   else if ("ComboBox" == widgetName)
      pWidgetInterface = ComboBoxHandler::instance ();
   else
      pWidgetInterface = 0;  // @INVESTIGATE Probably its better to throw an Exception here
                             // viz., 'UnKnownWidgetException'

   return pWidgetInterface;
}
