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
#ifndef _DeviceConnection
#define _DeviceConnection

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceConnection
{
public:
                             DeviceConnection (Device                 *pDevice,
                                               int                     id,
                                               int                     iForm,
                                               int                     iTray,
                                               int                     iMedia);
   virtual                  ~DeviceConnection ();

   static  DeviceConnection *create           (int                     id)    { return (DeviceConnection *)0; }
   virtual DeviceConnection *create           (PSZCRO                  pszId) { return (DeviceConnection *)0; }

   PSZCRO                    getName          ();

   bool                      isID             (int                     id);
   int                       getID            ();
   int                       getForm          ();
   int                       getTray          ();
   int                       getMedia         ();
   virtual int               connectionExists (int                     iTray,
                                               int                     iForm,
                                               int                     iMedia) = 0;

#ifndef RETAIL
   void                      outputSelf       ();
#endif
   virtual std::string       toString         (std::ostringstream&     oss);
   friend std::ostream&      operator<<       (std::ostream&           os,
                                               const DeviceConnection& self);

protected:
   Device *pDevice_d;
   int     id_d;
   int     iForm_d;
   int     iTray_d;
   int     iMedia_d;
   char   *pszName_d;
};

#endif
