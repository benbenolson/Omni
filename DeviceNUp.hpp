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
#ifndef _DeviceNUp
#define _DeviceNUp

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceNUp
{
public:
                               DeviceNUp               (Device                *pDevice,
                                                        PSZRO                  pszJobProperties,
                                                        BinaryData            *pbdData,
                                                        bool                   fSimulationRequired = false);
   virtual                    ~DeviceNUp               ();

   static bool                 isValid                 (PSZCRO                 pszJobProperties);
   bool                        isEqual                 (PSZCRO                 pszJobProperties);

   virtual DeviceNUp          *create                  (Device                *pDevice,
                                                        PSZCRO                 pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceNUp                  *createWithHash          (Device                *pDevice,
                                                        PSZCRO                 pszCreateHash);

   virtual bool                isSupported             (PSZCRO                 pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO                 pszKey);

   std::string                *getJobPropertyType      (PSZCRO                 pszKey);
   std::string                *getJobProperty          (PSZCRO                 pszKey);
   std::string                *translateKeyValue       (PSZCRO                 pszKey,
                                                        PSZCRO                 pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                   fInDeviceSpecific = false);

   BinaryData                 *getData                 ();
   bool                        needsSimulation         ();
   virtual PSZCRO              getDeviceID             ();

   int                         getXPages               ();
   int                         getYPages               ();
   std::string                *getDirection            ();

   static int                  directionIndex          (PSZCRO                 pszDirection);

   virtual Enumeration        *getEnumeration          (bool                   fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&    oss);
   friend std::ostream&        operator<<              (std::ostream&          os,
                                                        const DeviceNUp&       self);

protected:

   static bool                 getComponents           (PSZCRO                 pszJobProperties,
                                                        int                   *piX,
                                                        int                   *piY,
                                                        PSZRO                 *ppszNUpDirection,
                                                        int                   *pindexDirection);

   Device     *pDevice_d;
   int         iX_d;
   int         iY_d;
   PSZRO       pszDirection_d;
   int         indexDirection_d;
   BinaryData *pbdData_d;
   bool        fSimulationRequired_d;
   PSZRO       pszDeviceID_d;
};

class DefaultNUp : public DeviceNUp
{
public:
   enum {
      DEFAULT_X                   = 1,
      DEFAULT_Y                   = 1,
      DEFAULT_INDEX_DIRECTION     = 1,
      DEFAULT_SIMULATION_REQUIRED = false
   };

                               DefaultNUp              (Device             *pDevice,
                                                        PSZRO               pszJobProperties);

   DeviceNUp                  *create                  (Device             *pDevice,
                                                        PSZCRO              pszJobProperties);
   static DeviceNUp           *createS                 (Device             *pDevice,
                                                        PSZCRO              pszJobProperties);

   bool                        isSupported             (PSZCRO              pszJobProperties);

   Enumeration                *getEnumeration          (bool                fInDeviceSpecific = false);

   static void                 writeDefaultJP          (std::ostringstream& oss);

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream& oss);
   friend std::ostream&        operator<<              (std::ostream&       os,
                                                        const DefaultNUp&   self);
};

#endif
