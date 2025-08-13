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
#include "OmniDevice.hpp"

#include <string>
#include <cstring>

OmniDevice::
OmniDevice (PSZCRO pszLibraryName,
            PSZCRO pszJobProperties)
{
   pszLibraryName_d   = 0;
   pszJobProperties_d = 0;

   if (  pszLibraryName
      && *pszLibraryName
      )
   {
      pszLibraryName_d = (char *)malloc (strlen (pszLibraryName) + 1);
      if (pszLibraryName_d)
      {
         strcpy (pszLibraryName_d, pszLibraryName);
      }
   }
   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      pszJobProperties_d = (char *)malloc (strlen (pszJobProperties) + 1);
      if (pszJobProperties_d)
      {
         strcpy (pszJobProperties_d, pszJobProperties);
      }
   }
}

OmniDevice::
~OmniDevice ()
{
   if (pszLibraryName_d)
   {
      free (pszLibraryName_d);
      pszLibraryName_d = 0;
   }
   if (pszJobProperties_d)
   {
      free (pszJobProperties_d);
      pszJobProperties_d = 0;
   }
}

PSZCRO OmniDevice::
getLibraryName ()
{
   return pszLibraryName_d;
}

PSZCRO OmniDevice::
getJobProperties ()
{
   return pszJobProperties_d;
}

std::string OmniDevice::
toString (std::ostringstream& oss)
{
   oss << "{"
       << "pszLibraryName_d = " << pszLibraryName_d
       << ", pszJobProperties_d = " << pszJobProperties_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const OmniDevice& const_self)
{
   OmniDevice&        self = const_cast<OmniDevice&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
