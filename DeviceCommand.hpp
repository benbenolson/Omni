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
#ifndef _DeviceCommand
#define _DeviceCommand

#include "BinaryData.hpp"

#include <map>
#include <iostream>
#include <sstream>
#include <string>

class DeviceCommand
{
public:
   virtual             ~DeviceCommand  ();

   PSZCRO               getName        ();

   BinaryData          *getCommandData (PSZCRO               pszCommand);

#ifndef RETAIL
   void                 outputSelf     ();
#endif
   virtual std::string  toString       (std::ostringstream&  oss);
   friend std::ostream& operator<<     (std::ostream&        os,
                                        const DeviceCommand& self);

   void                 add            (PSZCRO               pszName,
                                        BinaryData          *data);

private:
   typedef std::map <std::string, BinaryData *> CommandMap;

   CommandMap data_d;
};

#endif
