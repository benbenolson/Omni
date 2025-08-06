/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2001
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
#ifndef _HP_LaserJet_PCL_Instance
#define _HP_LaserJet_PCL_Instance

#include "defines.hpp"
#include "Device.hpp"

extern "C" {
   DeviceInstance *createInstance (PrintDevice    *pDevice);
   void            deleteInstance (DeviceInstance *pInstance);
};

class HP_LaserJet_PCL_Instance : public DeviceInstance
{
public:
                        HP_LaserJet_PCL_Instance  (PrintDevice                    *pDevice);
   virtual             ~HP_LaserJet_PCL_Instance  ();

   void                 initializeInstance        (PSZCRO                          pszJobProperties);

   std::string         *getJobProperties          (bool                            fInDeviceSpecific);
   bool                 setJobProperties          (PSZCRO                          pszJobProperties);

   Enumeration         *getGroupEnumeration       (bool                            fInDeviceSpecific);

   std::string         *getJobPropertyType        (PSZRO                           pszKey);
   std::string         *getJobProperty            (PSZRO                           pszKey);
   std::string         *translateKeyValue         (PSZRO                           pszKey,
                                                   PSZRO                           pszValue);

   virtual bool         deviceOptionValid         (PSZRO                           pszDeviceOption);

   virtual bool         beginJob                  ();
   virtual bool         newFrame                  ();
   virtual bool         newFrame                  (bool                            fJobPropertiesChanged);
   virtual bool         endJob                    ();
   virtual bool         abortJob                  ();

#ifndef RETAIL
   virtual void         outputSelf                ();
#endif
   virtual std::string  toString                  (std::ostringstream&             oss);
   friend std::ostream& operator<<                (std::ostream&                   os,
                                                   const HP_LaserJet_PCL_Instance& device);

   void                 setupPrinter              ();

   POINTL            ptlPrintHead_d;
   int               iUOM_d;
   int               iXScalingFactor_d;
   int               iYScalingFactor_d;
   int               iYUOMScalingFactor_d;
   int               iVerticalOffset_d;

private:
   bool              fHaveInitialized_d;
   bool              fHaveSetupPrinter_d;
   int               iHardwareScaling_d;
};

#endif
