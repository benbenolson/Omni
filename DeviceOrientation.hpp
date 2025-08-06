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
#ifndef _DeviceOrientation
#define _DeviceOrientation

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceOrientation
{
public:

                               DeviceOrientation       (Device                  *pDevice,
                                                        PSZRO                    pszJobProperties,
                                                        bool                     fSimulationRequired = false);
   virtual                    ~DeviceOrientation       ();

   static bool                 isValid                 (PSZCRO                   pszJobProperties);
   bool                        isEqual                 (PSZCRO                   pszJobProperties);

   virtual DeviceOrientation  *create                  (Device                  *pDevice,
                                                        PSZCRO                   pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceOrientation          *createWithHash          (Device                  *pDevice,
                                                        PSZCRO                   pszCreateHash);

   virtual bool                isSupported             (PSZCRO                   pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO                   pszKey);

   std::string                *getJobPropertyType      (PSZCRO                   pszKey);
   std::string                *getJobProperty          (PSZCRO                   pszKey);
   std::string                *translateKeyValue       (PSZCRO                   pszKey,
                                                        PSZCRO                   pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                     fInDeviceSpecific = false);

   bool                        needsSimulation         ();
   virtual PSZCRO              getDeviceID             ();

   std::string                *getRotation             ();

   virtual Enumeration        *getEnumeration          (bool                     fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&      oss);
   friend std::ostream&        operator<<              (std::ostream&            os,
                                                        const DeviceOrientation& self);

protected:

   static bool                 getComponents           (PSZCRO                   pszJobProperties,
                                                        PSZRO                   *ppszRotation,
                                                        int                     *pindexRotation);

   Device     *pDevice_d;
   PSZRO       pszRotation_d;
   int         indexRotation_d;
   bool        fSimulationRequired_d;
};

class DefaultOrientation : public DeviceOrientation
{
public:
   enum {
      DEFAULT_ORIENTATION_INDEX   = 1,
      DEFAULT_SIMULATION_REQUIRED = false
   };
                             DefaultOrientation (Device             *pDevice,
                                                 PSZRO               pszJobProperties);

   DeviceOrientation        *create             (Device             *pDevice,
                                                 PSZCRO              pszJobProperties);
   static DeviceOrientation *createS            (Device             *pDevice,
                                                 PSZCRO              pszJobProperties);

   bool                      isSupported        (PSZCRO              pszJobProperties);

   Enumeration              *getEnumeration     (bool                fInDeviceSpecific = false);

   static void               writeDefaultJP     (std::ostringstream& oss);
};

#endif
