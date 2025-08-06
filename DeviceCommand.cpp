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
#include "DeviceCommand.hpp"
#include "DebugOutput.hpp"

DeviceCommand::
~DeviceCommand ()
{
   for ( CommandMap::iterator next = data_d.begin () ;
         next != data_d.end () ;
         next++ )
   {
//////std::cout << "$$$$$$ Deleting " << (*next).first << " @ " << (*next).second << ", " << *(*next).second << std::endl;

      delete (*next).second;
      (*next).second = 0;
   }
}

PSZCRO DeviceCommand::
getName ()
{
   return "deviceCommand";
}

void DeviceCommand::
add (PSZCRO      pszName,
     BinaryData *data)
{
   data_d[std::string (pszName)] = data;
}

BinaryData * DeviceCommand::
getCommandData (PSZCRO pszCommand)
{
   return data_d[std::string (pszCommand)];
}

#ifndef RETAIL

void DeviceCommand::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceCommand::
toString (std::ostringstream& oss)
{
   oss << "{DeviceCommand: ";

   for ( CommandMap::iterator next = data_d.begin () ;
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
operator<< (std::ostream& os, const DeviceCommand& const_self)
{
   DeviceCommand&     self = const_cast<DeviceCommand&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
