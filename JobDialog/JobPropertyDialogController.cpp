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
#include "JobPropertyDialogController.hpp"
#include "JobPropertyDialogView.hpp"
#include "WidgetManager.hpp"

#include <vector>
#include <gtk--/main.h>

using namespace Gtk;
using namespace JobDialog;

JobPropertyDialogController::
JobPropertyDialogController (Driver driver)
{
   driver_d = driver;
   pView_d  = 0;
}

JobPropertyDialogController::
~JobPropertyDialogController ()
{
   delete pView_d;
   pView_d = 0;

   std::vector<PropertyWidgetPair>::iterator itr;

   for (itr = propertyWidgetList_d.begin ();
        itr != propertyWidgetList_d.end ();
        itr++)
   {
      delete itr->pDriverProperty;
   }
}

std::vector<std::string> JobPropertyDialogController::
getSelectedJobProperties ()
{
   setupDialog ();

   Kit::run ();   // After setting up the dialog this starts an event loop
                  //and waits for the vents to happen.

   std::vector<std::string>                  propertyValueList;
   std::vector<PropertyWidgetPair>::iterator itr;

   for (itr = propertyWidgetList_d.begin ();
        itr != propertyWidgetList_d.end ();
        itr++)
   {
      DriverProperty *dp = itr->pDriverProperty;

      propertyValueList.push_back (dp->getName () + "=" + dp->getDefaultInternalValue ());
   }

   return propertyValueList;
}

std::vector<std::string> JobPropertyDialogController::
getDefaultJobProperties ()
{
   std::vector<std::string>           propertyValueList;
   std::vector<std::string>           propertyNames     = driver_d.getAvailablePropertyNames ();
   std::vector<std::string>::iterator itr;

   for (itr = propertyNames.begin ();
        itr != propertyNames.end ();
        itr++)
   {
      DriverProperty dp = driver_d.getProperty (*itr);

      propertyValueList.push_back (dp.getName () + "=" + dp.getDefaultExternalValue ());
   }

   return propertyValueList;
}

void JobPropertyDialogController::
setupDialog ()
{
   std::vector<std::string> propertyNames = driver_d.getAvailablePropertyNames ();

   pView_d = new JobPropertyDialogView (this, (int)propertyNames.size ());

   /*
   ** Iterate through the vector, get the DriverProperty, and get the widget made
   ** for that with the help of WidgetManager. Then, give this populated widget
   ** to View and ask it to add that widget with a label.
   ** Finally maintain the widget in a list along with DriverProperty object.
   */
   std::vector<std::string>::iterator itr;
   PropertyWidgetPair                 pair;

   for (itr = propertyNames.begin ();
        itr != propertyNames.end ();
        itr++)
   {
      DriverProperty dp      = driver_d.getProperty (*itr);
      Widget        *pWidget = WidgetManager::instance ()->createWidget (dp.getWidgetName (), &dp);

      pView_d->addWidget (pWidget, dp.getPropertyNameForDisplay ());

      pair.pDriverProperty = new DriverProperty (dp);
      pair.pWidget         = pWidget;
      pair.defaultValue    = dp.getDefaultExternalValue ();

      propertyWidgetList_d.push_back (pair);
   }

   pView_d->showDialog ();
}

void JobPropertyDialogController::
onPrintButtonClicked ()
{
   std::vector<PropertyWidgetPair>::iterator itr;

   for (itr = propertyWidgetList_d.begin ();
        itr != propertyWidgetList_d.end ();
        itr++)
   {
      DriverProperty *dp              = itr->pDriverProperty;
      std::string     newDefaultValue = WidgetManager::instance ()->getSelectedValue (itr->pWidget, dp->getWidgetName ());

      dp->setDefaultValue (newDefaultValue);
   }

   pView_d->hide_all ();

   // stop the event loop and close the Dialog.
   Kit::quit ();
}

void JobPropertyDialogController::
onSaveButtonClicked ()
{
   std::vector<PropertyWidgetPair>::iterator itr;

   for (itr = propertyWidgetList_d.begin ();
        itr != propertyWidgetList_d.end ();
        itr++)
   {
      DriverProperty *dp              = itr->pDriverProperty;
      std::string     newDefaultValue = WidgetManager::instance ()->getSelectedValue (itr->pWidget , dp->getWidgetName ());

      dp->setDefaultValue (newDefaultValue);
      driver_d.setProperty (dp->getName (), *dp);
   }

   // Don't stop the event loop. This is eqvt. to pressing Apply button.
}

void JobPropertyDialogController::
onSaveNprintButtonClicked()
{
   // Equivalent to Save click as SaveHandler includes the work PrintHandler does.
   onSaveButtonClicked ();

   pView_d->hide_all ();

   // stop the event loop and close the Dialog.
   Kit::quit ();
}

void JobPropertyDialogController::
onCancelButtonClicked ()
{
   // Do nothing right now ...
   pView_d->hide_all ();

   // stop the event loop and close the Dialog.
   Gtk::Main::quit ();
}
