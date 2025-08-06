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
#ifndef _DeviceInfo
#define _DeviceInfo

#include "Device.hpp"
#include "OmniDevice.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include <glib.h>
#include <gmodule.h>

class DeviceInfo
{
public:
                         DeviceInfo    (Device              *pDevice,
                                        GModule             *hmodDevice,
                                        OmniDevice          *pOD);
                        ~DeviceInfo    ();

   Device               *getDevice     ();
   GModule              *getHmodDevice ();
   OmniDevice           *getOmniDevice ();

#ifndef RETAIL
   void                  outputSelf    ();
#endif
   std::string           toString      (std::ostringstream&  oss);
   friend std::ostream&  operator<<    (std::ostream&        os,
                                        const DeviceInfo&    const_self);

private:
   Device     *pDevice_d;
   GModule    *hmodDevice_d;
   OmniDevice *pOD_d;
};

#endif
