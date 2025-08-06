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
#ifndef _Canon_Instance
#define _Canon_Instance

#include "defines.hpp"
#include "Device.hpp"

extern "C" {
   DeviceInstance *createInstance (PrintDevice    *pDevice);
   void            deleteInstance (DeviceInstance *pInstance);
};

class Canon_Instance : public DeviceInstance
{
public:
                         Canon_Instance            (PrintDevice          *pDevice);
   virtual              ~Canon_Instance            ();

   void                  initializeInstance        (PSZCRO                pszJobProperties);

   virtual bool          beginJob                  ();
   virtual bool          beginJob                  (bool                  fJobPropertiesChanged);
   virtual bool          newFrame                  ();
   virtual bool          newFrame                  (bool                  fJobPropertiesChanged);
   virtual bool          endJob                    ();
   virtual bool          abortJob                  ();

#ifndef RETAIL
   virtual void          outputSelf                ();
#endif
   virtual std::string   toString                  (std::ostringstream&   oss);
   friend std::ostream&  operator<<                (std::ostream&         os,
                                                    const Canon_Instance& device);

   void                  setupPrinter              ();
   void                  setPrintColor             ();

   POINTL             ptlPrintHead_d;

private:
   bool               fHaveInitialized_d;
   bool               fHaveSetupPrinter_d;
};

#endif
