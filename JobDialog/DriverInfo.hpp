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
#ifndef _DriverInfo
#define _DriverInfo

#include "Driver.hpp"

#ifdef PDC_INTERFACE_ENABLED
#include "OmniPDCProxy.hpp"
#else
#include "Device.hpp"
#endif

#include <string>
#include <vector>

#include <glib.h>
#include <gmodule.h>

class DriverInfo
{
public:
                              DriverInfo          ();
                             ~DriverInfo          ();
   void                       generateDriverInfo  ();
   std::vector<std::string>   getDeviceList       ();
   int                        openDevice          (const char      *pszFullDeviceName,
                                                   const char      *pszJobProperties);
   int                        closeDevice         ();
   Driver                     getDriver           ();

private:
   bool                       translateKeyValue   (PSZCRO           pszKey,
                                                   PSZCRO           pszValue,
                                                   std::string&     stringInternal,
                                                   std::string&     stringExternal);

   Device                     *pDevice_d;
#ifndef PDC_INTERFACE_ENABLED
   GModule                    *hmodDevice_d;
#endif

   Driver                      driver_d;

   std::vector<DriverProperty> dps_d;
};

#endif
