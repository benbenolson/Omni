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
#ifndef _DeviceForm
#define _DeviceForm

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceForm
{
public:
   enum {
      NO_CAPABILITIES             = 0x00000000,
      FORM_CAPABILITY_ROLL        = 0x00000001,
      FORM_CAPABILITY_USERDEFINED = 0x00000002
   };
   enum {
      RESERVED_BITS = 2
   };

                               DeviceForm              (Device             *pDevice,
                                                        PSZRO               pszJobProperties,
                                                        int                 iCapabilities,
                                                        BinaryData         *data,
                                                        HardCopyCap        *hcInfo);
   virtual                    ~DeviceForm              ();

   static bool                 isValid                 (PSZCRO              pszJobProperties);
   bool                        isEqual                 (PSZCRO              pszJobProperties);

   virtual DeviceForm         *create                  (Device             *pDevice,
                                                        PSZCRO              pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceForm                 *createWithHash          (Device             *pDevice,
                                                        PSZCRO              pszCreateHash);

   virtual bool                isSupported             (PSZCRO              pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO              pszKey);

   std::string                *getJobPropertyType      (PSZCRO              pszKey);
   std::string                *getJobProperty          (PSZCRO              pszKey);
   std::string                *translateKeyValue       (PSZCRO              pszKey,
                                                        PSZCRO              pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                fInDeviceSpecific = false);

   bool                        hasCapability           (int                 iFlag);
   int                         getCapabilities         ();
   BinaryData                 *getData                 ();
   HardCopyCap                *getHardCopyCap          ();
   virtual PSZCRO              getDeviceID             ();

   std::string                *getForm                 ();

   void                        associateWith           (DeviceResolution   *pResolution);

   int                         getCx                   ();
   int                         getCy                   ();
   void                        setCx                   (int                 id);
   void                        setCy                   (int                 id);
   int                         getOverrideX            ();
   int                         getOverrideY            ();

   static PSZCRO               getLongFormName         (PSZCRO              pszLongName);
   static PSZCRO               getShortFormName        (PSZCRO              pszLongName);

   static bool                 isReservedKeyword       (PSZCRO              pszId);
   static int                  getReservedValue        (PSZCRO              pszId);

   virtual Enumeration        *getEnumeration          (bool                fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream& oss);
   friend std::ostream&        operator<<              (std::ostream&       os,
                                                        const DeviceForm&   self);

protected:

   static bool                 getComponents           (PSZCRO              pszJobProperties,
                                                        PSZRO              *ppszForm,
                                                        int                *pindexForm,
                                                        int                *piFormSizeX,
                                                        int                *piFormSizeY);
   Device      *pDevice_d;
   PSZRO        pszForm_d;
   int          indexForm_d;
   int          iCapabilities_d;
   BinaryData  *data_d;
   HardCopyCap *hcInfo_d;

   int          iXOverride_d;
   int          iYOverride_d;

   int          iCX_d;
   int          iCY_d;
};

#endif
