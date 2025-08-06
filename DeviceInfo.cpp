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
#include "DeviceInfo.hpp"

DeviceInfo::
DeviceInfo (Device     *pDevice,
            GModule    *hmodDevice,
            OmniDevice *pOD)
{
   pDevice_d    = pDevice;
   hmodDevice_d = hmodDevice;
   pOD_d        = pOD;
}

DeviceInfo::
~DeviceInfo ()
{
   delete pDevice_d; pDevice_d = 0;

   if (hmodDevice_d)
   {
      g_module_close (hmodDevice_d);
      hmodDevice_d = 0;
   }
   delete pOD_d; pOD_d = 0;
}

Device * DeviceInfo::
getDevice ()
{
   return pDevice_d;
}

GModule * DeviceInfo::
getHmodDevice ()
{
   return hmodDevice_d;
}

OmniDevice * DeviceInfo::
getOmniDevice ()
{
   return pOD_d;
}

#ifndef RETAIL

void DeviceInfo::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceInfo::
toString (std::ostringstream& oss)
{
   oss << "{DeviceInfo: pDevice_d = " << std::hex << pDevice_d
       << ", hmodDevice_d = " << hmodDevice_d
       << ", pOD_d = " << pOD_d << std::dec
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceInfo& const_self)
{
   DeviceInfo&        self = const_cast<DeviceInfo&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
