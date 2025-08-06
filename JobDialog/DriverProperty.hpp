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
#ifndef _COM_IBM_LINUX_OMNI_JOB_DriverProperty
#define _COM_IBM_LINUX_OMNI_JOB_DriverProperty

#include <vector>
#include <map>
#include <string>
#include <sstream>

class DriverProperty;

#include "Driver.hpp"

class DriverProperty
{
public:
                            DriverProperty            (std::string              propertyName,
                                                       std::string              nameForDisplay,
                                                       std::string              widgetName,
                                                       std::vector<std::string> externalValues,
                                                       std::vector<std::string> internalValues,
                                                       bool                     fEditable = true);
                            DriverProperty            (const DriverProperty&    dp);
                            DriverProperty            ();
   virtual                 ~DriverProperty            ();

   DriverProperty           operator =                (DriverProperty        dp);

   /**********************************************************/
   /* @TBD INVESTIGATE COPY CONSTRUCTOR needed for deep copy */
   /**********************************************************/

   bool                     isEditable                ();

   std::string              getName                   ();
   std::string              getPropertyNameForDisplay ();
   std::string              getWidgetName             ();
   std::string              getDefaultExternalValue   ();
   std::string              getDefaultInternalValue   ();

   std::vector<std::string> getAvailableValues        ();

   bool                     setDefaultValue           (std::string           defaultValue);

   std::string              toString                  (std::ostringstream&   oss);

protected:
   std::string              propertyName_d;
   std::string              nameForDisplay_d;
   std::string              widgetName_d;
   std::string              externalValue_d;
   bool                     fEditable_d;
   std::vector<std::string> externalValues_d;
   std::vector<std::string> internalValues_d;
};

#endif
