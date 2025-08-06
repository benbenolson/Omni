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
#include "DeviceConnection.hpp"
#include "StringResource.hpp"

DeviceConnection::
DeviceConnection (Device *pDevice,
                  int     id,
                  int     iForm,
                  int     iTray,
                  int     iMedia)
{
   pDevice_d = pDevice;
   id_d      = id;
   iForm_d   = iForm;
   iTray_d   = iTray;
   iMedia_d  = iMedia;
   pszName_d = 0;
}

DeviceConnection::
~DeviceConnection ()
{
   pDevice_d = 0;
   id_d      = 0;
   iForm_d   = 0;
   iTray_d   = 0;
   iMedia_d  = 0;
   if (pszName_d)
   {
      free (pszName_d);
      pszName_d = 0;
   }
}

PSZCRO DeviceConnection::
getName ()
{
   if (pszName_d)
      return pszName_d;

   PSZCRO pszFormName   = StringResource::getString (pDevice_d->getLanguageResource (),
                                                     StringResource::STRINGGROUP_FORMS,
                                                     iForm_d);
   PSZCRO pszTrayName   = StringResource::getString (pDevice_d->getLanguageResource (),
                                                     StringResource::STRINGGROUP_TRAYS,
                                                     iTray_d);
   PSZCRO pszMediaName  = StringResource::getString (pDevice_d->getLanguageResource (),
                                                     StringResource::STRINGGROUP_MEDIAS,
                                                     iMedia_d);
   int    iBytesToAlloc = 0;

   iBytesToAlloc = strlen (pszFormName)
                 + strlen (pszTrayName)
                 + strlen (pszMediaName)
                 + 2 * 3                  // " / " separators
                 + 1
                 ;

   pszName_d = (char *)calloc (1, iBytesToAlloc);
   if (!pszName_d)
      return 0;

   char *pszCurrent = pszName_d;

   strcpy (pszCurrent, pszFormName);  pszCurrent += strlen (pszCurrent);
   strcat (pszCurrent, " / ");        pszCurrent += strlen (pszCurrent);
   strcpy (pszCurrent, pszTrayName);  pszCurrent += strlen (pszCurrent);
   strcat (pszCurrent, " / ");        pszCurrent += strlen (pszCurrent);
   strcpy (pszCurrent, pszMediaName);

   return pszName_d;
}

bool DeviceConnection::
isID (int id)
{
   return id == id_d;
}

int DeviceConnection::
getID ()
{
   return id_d;
}

int DeviceConnection::
getForm ()
{
   return iForm_d;
}

int DeviceConnection::
getTray ()
{
   return iTray_d;
}

int DeviceConnection::
getMedia ()
{
   return iMedia_d;
}

#ifndef RETAIL

void DeviceConnection::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceConnection::
toString (std::ostringstream& oss)
{
   oss << "{DeviceConnection: 0x" << std::hex << (int)this << std::dec << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceConnection& const_self)
{
   DeviceConnection&  self = const_cast<DeviceConnection&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
