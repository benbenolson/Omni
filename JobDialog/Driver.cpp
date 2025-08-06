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

#include "Driver.hpp"

Driver::
Driver (std::string driverName, DriverProperty **dpArr, int count)
{
   driverName_d = driverName;

   for (int i = 0; i < count; i++)
   {
      availablePropertyNames_d.push_back (dpArr[i]->getName ());
      theMap_d[dpArr[i]->getName ()] = *dpArr[i];
   }
}

Driver::
Driver (std::string driverName, std::vector<DriverProperty> dps)
{
   driverName_d = driverName;
   for (int i = 0; i < (int)dps.size (); i++)
   {
      availablePropertyNames_d.push_back (dps[i].getName ());
      theMap_d[availablePropertyNames_d[i]] = dps[i];
   }
}

// @TBD remove
Driver::
Driver ()
{
}

std::string Driver::
getName ()
{
   return driverName_d;
}

std::vector<std::string> Driver::
getAvailablePropertyNames ()
{
   return availablePropertyNames_d;
}

DriverProperty Driver::
getProperty (std::string key)
{
   return theMap_d.find (key)->second;
}

bool Driver::
setProperty (std::string key, DriverProperty dpValue)
{
   (*(theMap_d.find (key))).second = dpValue;

   return true;
}

std::string Driver::
toString (std::ostringstream& oss)
{
   for (int i = 0; i < (int)availablePropertyNames_d.size (); i++)
   {
      oss << std::endl << "*****************************************************" << std::endl;

      theMap_d[availablePropertyNames_d[i]].toString (oss);

      oss << "*****************************************************" << std::endl;
   }

   return oss.str ();
}
