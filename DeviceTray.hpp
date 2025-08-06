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
#ifndef _DeviceTray
#define _DeviceTray

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceTray
{
public:
   enum {
      TRAY_TYPE_AUTO,
      TRAY_TYPE_MANUAL,
      TRAY_TYPE_ZERO_MARGINS
   };

                               DeviceTray              (Device             *pDevice,
                                                        PSZRO               pszJobProperties,
                                                        int                 iType,
                                                        BinaryData         *pbdData);
   virtual                    ~DeviceTray              ();

   static bool                 isValid                 (PSZCRO              pszJobProperties);
   bool                        isEqual                 (PSZCRO              pszJobProperties);

   virtual DeviceTray         *create                  (Device             *pDevice,
                                                        PSZCRO              pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceTray                 *createWithHash          (Device             *pDevice,
                                                        PSZCRO              pszCreateHash);
   virtual bool                isSupported             (PSZCRO              pszJobProperties) = 0;


   bool                        handlesKey              (PSZCRO              pszKey);

   std::string                *getJobPropertyType      (PSZCRO              pszKey);
   std::string                *getJobProperty          (PSZCRO              pszKey);
   std::string                *translateKeyValue       (PSZCRO              pszKey,
                                                        PSZCRO              pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                fInDeviceSpecific = false);

   int                         getType                 ();
   BinaryData                 *getData                 ();
   virtual PSZCRO              getDeviceID             ();

   std::string                *getInputTray            ();

   static bool                 isReservedKeyword       (PSZCRO              pszId);
   static int                  getReservedValue        (PSZCRO              pszId);

   virtual Enumeration        *getEnumeration          (bool                fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream& oss);
   friend std::ostream&        operator<<              (std::ostream&       os,
                                                        const DeviceTray&   self);

protected:

   static bool                 getComponents           (PSZCRO              pszJobProperties,
                                                        PSZRO              *ppszTray,
                                                        int                *pindexTray);

   Device     *pDevice_d;
   PSZRO       pszTray_d;
   int         indexTray_d;
   int         iType_d;
   BinaryData *pbdData_d;
};

class DefaultTray : public DeviceTray
{
public:
   enum {
      DEFAULT_TRAY_INDEX           = 2,
      DEFAULT_TYPE                 = DeviceTray::TRAY_TYPE_AUTO
   };
                        DefaultTray    (Device             *pDevice,
                                        PSZRO               pszJobProperties);

   DeviceTray          *create         (Device             *pDevice,
                                        PSZCRO              pszJobProperties);
   static DeviceTray   *createS        (Device             *pDevice,
                                        PSZCRO              pszJobProperties);

   bool                 isSupported    (PSZCRO              pszJobProperties);

   Enumeration         *getEnumeration (bool                fInDeviceSpecific = false);

   static void          writeDefaultJP (std::ostringstream& oss);
};

#endif
