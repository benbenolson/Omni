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
#include "DeviceData.hpp"
#include "DebugOutput.hpp"

DeviceData::
~DeviceData ()
{
   for ( DataMap::iterator next = data_d.begin () ;
         next != data_d.end () ;
         next++ )
   {
      delete (*next).second;
   }
}

char * DeviceData::
getName ()
{
   return "deviceData";
}

void DeviceData::
add (PSZCRO pszName, BinaryData *data)
{
   data_d[std::string (pszName)] = data;
}

bool DeviceData::
getBooleanData (char *pszCommand, bool *pfData)
{
   BinaryData *pData = data_d[std::string (pszCommand)];

   if (pData)
   {
      *pfData = *((bool *)pData->getData ());

      return true;
   }
   else
   {
      *pfData = false;

      return false;
   }
}

bool DeviceData::
getByteData (char *pszCommand, byte *pbData)
{
   BinaryData *pData = data_d[std::string (pszCommand)];

   if (pData)
   {
      *pbData = *((byte *)pData->getData ());

      return true;
   }
   else
   {
      *pbData = 0;

      return false;
   }
}

bool DeviceData::
getIntData (char *pszCommand, int *piData)
{
   BinaryData *pData = data_d[std::string (pszCommand)];

   if (pData)
   {
      *piData = *((int *)pData->getData ());

      return true;
   }
   else
   {
      *piData = 0;

      return false;
   }
}

bool DeviceData::
getStringData  (char *pszCommand, char **ppszData)
{
   BinaryData *pData = data_d[std::string (pszCommand)];

   if (pData)
   {
      *ppszData = (char *)pData->getData ();

      return true;
   }
   else
   {
      *ppszData = 0;

      return false;
   }
}

bool DeviceData::
getBinaryData (char *pszCommand, BinaryData **ppbdData)
{
   BinaryData *pData = data_d[std::string (pszCommand)];

   if (pData)
   {
      *ppbdData = pData;

      return true;
   }
   else
   {
      *ppbdData = 0;

      return false;
   }
}

#ifndef RETAIL

void DeviceData::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceData::
toString (std::ostringstream& oss)
{
   oss << "{DeviceData: ";

   for ( DataMap::iterator next = data_d.begin () ;
         next != data_d.end () ;
       )
   {
      oss << (*next).first << " = " << *(*next).second;

      next++;

      if (next != data_d.end ())
      {
         oss << ", ";
      }
   }

   oss << "}";

   return oss.str ();
}
/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceData& const_self)
{
   DeviceData&        self = const_cast<DeviceData&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
