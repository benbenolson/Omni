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
#ifndef _PluggableInstance
#define _PluggableInstance

#include "Device.hpp"
#include "PrinterCommand.hpp"
#include "JobProperties.hpp"

#include <map>

extern "C" {
   void            deletePluggableInstance (DeviceInstance *pInstance);
};

class PluggableInstance : public DeviceInstance
{
public:
                        PluggableInstance         (PrintDevice             *pDevice,
                                                   PSZCRO                   pszExeName,
                                                   PSZCRO                   pszData);
   virtual             ~PluggableInstance         ();

   void                 initializeInstance        (PSZCRO                   pszJobProperties);

   bool                 hasError                  ();

   std::string         *getJobProperties          (bool                     fInDeviceSpecific = false);
   bool                 setJobProperties          (PSZCRO                   pszJobProperties);

   Enumeration         *getGroupEnumeration       (bool                     fInDeviceSpecific = false);

   std::string         *getJobPropertyType        (PSZCRO                   pszKey);
   std::string         *getJobProperty            (PSZCRO                   pszKey);
   std::string         *translateKeyValue         (PSZCRO                   pszKey,
                                                   PSZCRO                   pszValue);

   bool                 setOutputStream           (FILE                    *pFile);
   bool                 setErrorStream            (FILE                    *pFile);

   bool                 setLanguage               (int                      iLanguageID);

   bool                 beginJob                  ();
   bool                 beginJob                  (bool                     fJobPropertiesChanged);
   bool                 newFrame                  ();
   bool                 newFrame                  (bool                     fJobPropertiesChanged);
   bool                 endJob                    ();
   bool                 abortJob                  ();

   bool                 rasterize                 (PBYTE                    pbBits,
                                                   PBITMAPINFO2             pbmi,
                                                   PRECTL                   prectlPageLocation,
                                                   BITBLT_TYPE              eType);

#ifndef RETAIL
   void                 outputSelf                ();
#endif
   virtual std::string  toString                  (std::ostringstream&      oss);
   friend std::ostream& operator<<                (std::ostream&            os,
                                                   const PluggableInstance& self);
private:
   void                 startPDCSession           ();
   void                 stopPDCSession            (bool                     fError);
   void                 pushDeviceObjects         ();
   bool                 setJobProperties          ();
   bool                 commonBeginJob            ();
   bool                 commonNewFrame            ();

   bool                 fHasError_d;

   char                *pszExeName_d;
   char                *pszData_d;

   int                  fdS2C_d;
   int                  fdC2S_d;
   bool                 fRemoveS2C_d;
   bool                 fRemoveC2S_d;
   char                *pszS2C_d;
   char                *pszC2S_d;

   int                  idBuffer1_d;
   int                  cbBuffer1_d;
   byte                *pbBuffer1_d;
   int                  idBuffer2_d;
   int                  cbBuffer2_d;
   byte                *pbBuffer2_d;

   PrinterCommand      *pCmd_d;

   int                  fdOutputStream_d;
   int                  fdErrorStream_d;

   JobProperties        *pjpJobProperties_d;
};

#endif
