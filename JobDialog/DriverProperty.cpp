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
#include <iostream>
#include <vector>
#include <map>

#include "DriverProperty.hpp"

DriverProperty::
DriverProperty (std::string              propertyName,
                std::string              nameForDisplay,
                std::string              widgetName,
                std::vector<std::string> externalValues,
                std::vector<std::string> internalValues,
                bool                     fEditable)
{
   propertyName_d   = propertyName;
   nameForDisplay_d = nameForDisplay;
   widgetName_d     = widgetName;
   fEditable_d      = fEditable;
   externalValues_d = externalValues;
   externalValue_d  = *externalValues_d.begin ();
   internalValues_d = internalValues;

   externalValues_d.erase (externalValues_d.begin ());
}

DriverProperty::
DriverProperty (const DriverProperty& dp)
{
   propertyName_d   = dp.propertyName_d;
   nameForDisplay_d = dp.nameForDisplay_d;
   widgetName_d     = dp.widgetName_d;
   externalValue_d  = dp.externalValue_d;
   fEditable_d      = dp.fEditable_d;
   externalValues_d = dp.externalValues_d;
   internalValues_d = dp.internalValues_d;
}

// @TBD remove
DriverProperty::
DriverProperty ()
{
   propertyName_d    = "";
   nameForDisplay_d  = "";
   widgetName_d      = "";
   externalValue_d   = "";
   fEditable_d       = false;
}

DriverProperty::
~DriverProperty ()
{
   // @TBD
}

DriverProperty DriverProperty::
operator = (DriverProperty dp)
{
   propertyName_d   = dp.propertyName_d;
   nameForDisplay_d = dp.nameForDisplay_d;
   widgetName_d     = dp.widgetName_d;
   externalValue_d  = dp.externalValue_d;
   fEditable_d      = dp.fEditable_d;
   externalValues_d = dp.externalValues_d;
   internalValues_d = dp.internalValues_d;

   return *this;
}

bool DriverProperty::
isEditable ()
{
   return fEditable_d;
}

std::string DriverProperty::
getName ()
{
   return propertyName_d;
}

std::string DriverProperty::
getPropertyNameForDisplay ()
{
   return nameForDisplay_d;
}

std::string DriverProperty::
getWidgetName ()
{
   return widgetName_d;
}

std::string DriverProperty::
getDefaultExternalValue ()
{
   return externalValue_d;
}

std::string DriverProperty::
getDefaultInternalValue ()
{
   std::vector<std::string>::iterator itrExt = externalValues_d.begin ();
   std::vector<std::string>::iterator itrInt = internalValues_d.begin ();

   for ( ; itrExt != externalValues_d.end (); itrExt++, itrInt++ )
   {
//////std::cout << "External = " << (itrExt ? *itrExt : "NULL") << std::endl;
//////std::cout << "Internal = " << (itrInt ? *itrInt : "NULL") << std::endl;
      if (*itrExt == externalValue_d)
      {
/////////std::cout << "Returning: " << (itrInt ? *itrInt : "NULL") << std::endl;
         return *itrInt;
      }
   }

   return externalValue_d;
}

std::vector<std::string> DriverProperty::
getAvailableValues ()
{
   return externalValues_d;
}

bool DriverProperty::
setDefaultValue (std::string externalValue)
{
   externalValue_d = externalValue;

   return true;
}

std::string DriverProperty::
toString (std::ostringstream& oss)
{
   oss << "Property Name --> " << propertyName_d;
   oss << "      Display Name --> " << nameForDisplay_d << std::endl;
   oss << "Widget Name --> " << widgetName_d << "       Editable -->" << fEditable_d << std::endl;
   oss << "Default Value --> " << externalValue_d << std::endl;

   std::vector<std::string>::iterator itr;

   oss << "Available Property Values are: " << std::endl;
   for (itr = externalValues_d.begin (); itr != externalValues_d.end (); itr++)
   {
      oss << "\t\t " << *itr << std::endl;
   }

   return oss.str ();
}
