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
#ifndef _TI_PCL3_Instance
#define _TI_PCL3_Instance

#include "defines.hpp"
#include "Device.hpp"

extern "C" {
   DeviceInstance *createInstance (PrintDevice    *pDevice);
   void            deleteInstance (DeviceInstance *pInstance);
};

class TI_PCL3_Instance : public DeviceInstance
{
public:
   typedef enum _InternalRes {
      IR_UNKNOWN     = 0,
      IR_MONOCHROME,
      IR_CMY,
      IR_CMYK
   } INTERNALRES;

                         TI_PCL3_Instance    (PrintDevice                    *pDevice);
   virtual              ~TI_PCL3_Instance    ();

   void                  initializeInstance          ();

   virtual bool          beginJob                    ();
   virtual bool          beginJob                    (bool                            fJobPropertiesChanged);
   virtual bool          newFrame                    ();
   virtual bool          newFrame                    (bool                            fJobPropertiesChanged);
   virtual bool          endJob                      ();
   virtual bool          abortJob                    ();

#ifndef RETAIL
   virtual void          outputSelf                  ();
#endif
   virtual std::string   toString                    (std::ostringstream&             oss);
   friend std::ostream&  operator<<                  (std::ostream&                   os,
                                                      const TI_PCL3_Instance& device);

   void                  setupPrinter                ();

   POINTL             ptlPrintHead_d;

   INTERNALRES        eInternalRes_d;

private:
   PSZRO                 getInternalResConfiguration (INTERNALRES                     eInternalRes);
   void                  configureRasterData         (INTERNALRES                     eInternalRes);

   bool               fHaveInitialized_d;
   bool               fHaveSetupPrinter_d;
};

#endif
