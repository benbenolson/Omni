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
#ifndef _COM_IBM_LINUX_OMNI_JOB_Driver
#define _COM_IBM_LINUX_OMNI_JOB_Driver

#include <vector>
#include <map>
#include <string>
#include <sstream>

class Driver;

#include "DriverProperty.hpp"

class Driver
{
public:
                            Driver                    (std::string                   driverName,
                                                       DriverProperty              **dpArr,
                                                       int                           count);
                            Driver                    (std::string                   driverName,
                                                       std::vector<DriverProperty>   dps);
                            Driver                    ();

   std::string              getName                   ();

   std::vector<std::string> getAvailablePropertyNames ();
   DriverProperty           getProperty               (std::string              key);
   bool                     setProperty               (std::string              key,
                                                       DriverProperty           dpValue);

   std::string              toString                  (std::ostringstream&      oss);

protected:
   std::string                           driverName_d;
   std::vector<std::string>              availablePropertyNames_d;
   std::map<std::string, DriverProperty> theMap_d;
};

#endif
