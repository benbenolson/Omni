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
#ifndef _DeviceResolution
#define _DeviceResolution

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceResolution
{
public:

                               DeviceResolution        (Device                 *pDevice,
                                                        PSZRO                   pszJobProperties,
                                                        int                     iXInternalRes,
                                                        int                     iYInternalRes,
                                                        BinaryData             *pbdData,
                                                        int                     iCapabilities,
                                                        int                     iDestinationBitsPerPel,
                                                        int                     iScanlineMultiple);
   virtual                    ~DeviceResolution        ();

   static bool                 isValid                 (PSZCRO                  pszJobProperties);
   bool                        isEqual                 (PSZCRO                  pszJobProperties);

   virtual DeviceResolution   *create                  (Device                 *pDevice,
                                                        PSZCRO                  pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceResolution           *createWithHash          (Device                 *pDevice,
                                                        PSZCRO                  pszCreateHash);

   virtual bool                isSupported             (PSZCRO                  pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO                  pszKey);

   std::string                *getJobPropertyType      (PSZCRO                  pszKey);
   std::string                *getJobProperty          (PSZCRO                  pszKey);
   std::string                *translateKeyValue       (PSZCRO                  pszKey,
                                                        PSZCRO                  pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                    fInDeviceSpecific = false);

   int                         getXRes                 ();
   int                         getYRes                 ();
   int                         getExternalXRes         ();
   int                         getExternalYRes         ();
   int                         getInternalXRes         ();
   int                         getInternalYRes         ();
   BinaryData                 *getData                 ();
   bool                        hasCapability           (int                     iFlag);
   int                         getCapabilities         ();
   int                         getDstBitsPerPel        ();
   int                         getScanlineMultiple     ();
   virtual PSZCRO              getDeviceID             ();

   void                        setInternalXRes         (int                     iXInternalRes);
   void                        setInternalYRes         (int                     iYInternalRes);

   static bool                 isReservedKeyword       (PSZCRO                  pszId);
   static int                  getReservedValue        (PSZCRO                  pszId);

   virtual Enumeration        *getEnumeration          (bool                    fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&     oss);
   friend std::ostream&        operator<<              (std::ostream&           os,
                                                        const DeviceResolution& self);

protected:

   static bool                 getComponents           (PSZCRO                   pszJobProperties,
                                                        PSZRO                   *ppszResolution,
                                                        int                     *piX,
                                                        int                     *piY);

   Device     *pDevice_d;
   PSZRO       pszResolution_d;
   int         iXRes_d;
   int         iYRes_d;
   int         iXInternalRes_d;
   int         iYInternalRes_d;
   BinaryData *pbdData_d;
   int         iCapabilities_d;
   int         iDestinationBitsPerPel_d;
   int         iScanlineMultiple_d;
};

#endif
