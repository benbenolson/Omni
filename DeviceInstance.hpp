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
#ifndef _DeviceInstance
#define _DeviceInstance

#include "Device.hpp"
#include "StringResource.hpp"

#include <iostream>
#include <string>

class DeviceInstance
{
public:
                         DeviceInstance            (PrintDevice          *pDevice);
   virtual              ~DeviceInstance            ();

   virtual void          initializeInstance        (PSZCRO                pszJobProperties);

   virtual bool          hasError                  ();

   virtual std::string  *getJobProperties          (bool                  fInDeviceSpecific = false);
   virtual bool          setJobProperties          (PSZCRO                pszJobProperties);

   virtual Enumeration  *getGroupEnumeration       (bool                  fInDeviceSpecific = false);

   virtual std::string  *getJobPropertyType        (PSZCRO                pszKey);
   virtual std::string  *getJobProperty            (PSZCRO                pszKey);
   virtual std::string  *translateKeyValue         (PSZCRO                pszKey,
                                                    PSZCRO                pszValue);

   DeviceOrientation    *getCurrentOrientation     ();
   PSZCRO                getCurrentDitherID        ();
   DeviceForm           *getCurrentForm            ();
   DeviceTray           *getCurrentTray            ();
   DeviceMedia          *getCurrentMedia           ();
   DeviceResolution     *getCurrentResolution      ();
   DeviceCommand        *getCommands               ();
   DeviceData           *getDeviceData             ();
   DevicePrintMode      *getCurrentPrintMode       ();
   DeviceGamma          *getCurrentGamma           ();

   void                  ditherNewFrame            ();

   bool                  hasCapability             (long                  lMask);
   bool                  hasRasterCapability       (long                  lMask);
   bool                  hasDeviceOption           (PSZCRO                pszDeviceOption);

   virtual bool          deviceOptionValid         (PSZCRO                pszDeviceOption);

   virtual bool          setOutputStream           (FILE                 *pFile);
   virtual bool          setErrorStream            (FILE                 *pFile);

   virtual bool          setLanguage               (int                   iLanguageID);

   virtual bool          beginJob                  ();
   virtual bool          beginJob                  (bool                  fJobPropertiesChanged);
   virtual bool          newFrame                  ();
   virtual bool          newFrame                  (bool                  fJobPropertiesChanged);
   virtual bool          endJob                    ();
   virtual bool          abortJob                  ();

   StringResource       *getLanguageResource       ();

   bool                  sendBinaryDataToDevice    (BinaryData           *pData);
   bool                  sendBinaryDataToDevice    (DeviceForm           *pForm);
   bool                  sendBinaryDataToDevice    (DeviceTray           *pTray);
   bool                  sendBinaryDataToDevice    (DeviceMedia          *pMedia);
   bool                  sendBinaryDataToDevice    (DeviceResolution     *pResolution);
   bool                  sendBinaryDataToDevice    (PBYTE                 pbData,
                                                    int                   iLength);

   bool                  sendPrintfToDevice        (BinaryData           *pData,
                                                                          ...);
   bool                  sendVPrintfToDevice       (BinaryData           *pData,
                                                    va_list               list);

#ifndef RETAIL
   void                  outputSelf                ();
#endif
   virtual std::string   toString                  (std::ostringstream&   oss);
   friend std::ostream&  operator<<                (std::ostream&         os,
                                                    const DeviceInstance& self);

protected:
   PrintDevice       *pDevice_d;
};

#endif
