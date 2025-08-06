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
#ifndef _Test_Instance
#define _Test_Instance

#include "defines.hpp"
#include "Device.hpp"

extern "C" {
   DeviceInstance *createInstance (PrintDevice    *pDevice);
   void            deleteInstance (DeviceInstance *pInstance);
};

class Test_Instance : public DeviceInstance
{
public:

                        Test_Instance         (PrintDevice         *pDevice);
   virtual             ~Test_Instance         ();

   void                 initializeInstance    (PSZCRO               pszJobProperties);

   virtual bool         beginJob              ();
   virtual bool         newFrame              ();
   virtual bool         newFrame              (bool                 fJobPropertiesChanged);
   virtual bool         endJob                ();
   virtual bool         abortJob              ();

   virtual std::string  toString              (std::ostringstream&  oss);
   friend std::ostream& operator<<            (std::ostream&        os,
                                               const Test_Instance& device);

   void                 setupPrinter          ();

private:
};

#endif
