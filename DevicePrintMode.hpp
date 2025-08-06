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
#ifndef _DevicePrintMode
#define _DevicePrintMode

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DevicePrintMode
{
public:
   enum {
      PRINT_MODE_UNLISTED       = -1,
      PRINT_MODE_NONE,
      PRINT_MODE_1_ANY,
      PRINT_MODE_8_K,
      PRINT_MODE_8_CMY,
      PRINT_MODE_8_CMYK,
      PRINT_MODE_8_CcMmYK,
      PRINT_MODE_8_CcMmYyK,
      PRINT_MODE_8_RGB,
      PRINT_MODE_24_K,
      PRINT_MODE_24_CMY,
      PRINT_MODE_24_CMYK,
      PRINT_MODE_24_CcMmYK,
      PRINT_MODE_24_CcMmYyK,
      PRINT_MODE_24_RGB
   };
   enum {
      COUNT = PRINT_MODE_24_RGB - PRINT_MODE_UNLISTED + 1
   };

   enum {
      COLOR_TECH_K,
      COLOR_TECH_CMY,
      COLOR_TECH_CMYK,
      COLOR_TECH_CcMmYK,
      COLOR_TECH_CcMmYyK,
      COLOR_TECH_RGB
   };

   enum {
      COLOR_PLANE_CYAN,
      COLOR_PLANE_MAGENTA,
      COLOR_PLANE_YELLOW,
      COLOR_PLANE_BLACK,
      COLOR_PLANE_LIGHT_CYAN,
      COLOR_PLANE_LIGHT_MAGENTA,
      COLOR_PLANE_LIGHT_YELLOW
   };

                               DevicePrintMode         (Device                *pDevice,
                                                        PSZRO                  pszJobProperties,
                                                        int                    iPhysicalCount,
                                                        int                    iLogicalCount,
                                                        int                    iPlanes);
   virtual                    ~DevicePrintMode         ();

   static bool                 isValid                 (PSZCRO                 pszJobProperties);
   bool                        isEqual                 (PSZCRO                 pszJobProperties);

   virtual DevicePrintMode    *create                  (Device                *pDevice,
                                                        PSZCRO                 pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DevicePrintMode            *createWithHash          (Device                *pDevice,
                                                        PSZCRO                 pszCreateHash);

   virtual bool                isSupported             (PSZCRO                 pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO                 pszKey);

   std::string                *getJobPropertyType      (PSZCRO                 pszKey);
   std::string                *getJobProperty          (PSZCRO                 pszKey);
   std::string                *translateKeyValue       (PSZCRO                 pszKey,
                                                        PSZCRO                 pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                   fInDeviceSpecific = false);

   int                         getColorTech            ();
   int                         getPhysicalCount        ();  // How many colors the printer outputs
   int                         getLogicalCount         ();  // What the printer wants for bits per pel
   int                         getNumPlanes            ();  // What the printer wants for planes per bitmap
   virtual PSZCRO              getDeviceID             ();

   virtual Enumeration        *getEnumeration          (bool                   fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&    oss);
   friend std::ostream&        operator<<              (std::ostream&          os,
                                                        const DevicePrintMode& self);

protected:

   static bool                 getComponents           (PSZCRO                 pszJobProperties,
                                                        PSZRO                 *ppszPrintMode,
                                                        int                   *pindexPrintMode,
                                                        int                   *piColorTech);

   Device  *pDevice_d;
   PSZRO    pszPrintMode_d;
   int      indexPrintMode_d;
   int      iPhysicalCount_d;
   int      iLogicalCount_d;
   int      iPlanes_d;
   int      iColorTech_d;
};

#endif
