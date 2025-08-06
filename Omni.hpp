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
#ifndef _Omni
#define _Omni

#include "Device.hpp"
#include "DeviceInfo.hpp"
#include "OmniPDCProxy.hpp"
#include "OmniDevice.hpp"

#include <glib.h>
#include <gmodule.h>

#define DEFAULT_LIBRARY_PATH HEX1(LIBRARY_PATH)
#define DEFAULT_SHARE_PATH   HEX1(SHARE_PATH)
#define DEFAULT_BIN_PATH     HEX1(BIN_PATH)

/*  NULL terminated array of library paths for omni.
 */
extern PSZCRO            vapszLibraryPaths[];
/*  NULL terminated array of share paths for omni (for XML files).
 */
extern PSZRO             vapszDataPaths[];
/*  NULL terminated array of bin paths for omni.
 */
extern PSZCRO            vapszExecutablePaths[];

class Omni
{
public:
   static void              initialize                  ();
   static void              terminate                   ();
   static int               my_system                   (PSZCRO             pszCommand);

   static void              setOutputStream             (Device            *pDevice,
                                                         FILE              *pfpNew);
   static void              setErrorStream              (Device            *pDevice,
                                                         FILE              *pfpNew);

   static bool              libraryValid                (GModule           *hDevice,
                                                         PSZCRO             pszLibraryName,
                                                         PSZCRO             pszVersion,
                                                         bool               fVerbose = false);

   static bool              openLibrary                 (PSZCRO             pszLibName,
                                                         GModule          **phLibrary);

   static bool              openAndTestDeviceLibrary    (PSZCRO             pszLibName,
                                                         GModule          **phmodDevice);

   static Device           *createDevice                (PSZCRO             pszDriver,
                                                         PSZCRO             pszDevice,
                                                         PSZCRO             pszJobProperties,
                                                         bool               fAdvanced,
                                                         GModule          **phmodDevice);

   static Device           *createDevice                (PDL               *pPDL,
                                                         PSZCRO             pszJobProperties,
                                                         bool               fAdvanced,
                                                         GModule          **phmodDevice);

   static Device           *createDevice                (OmniDevice        *pOD,
                                                         GModule          **phmodDevice);

   static DeviceInfo       *findDeviceInfoFromShortName (PSZCRO             pszShortName,
                                                         bool               fBuildOnly = false);

   static PSZCRO            openXMLFile                 (PSZCRO             pszXMLFile);

   static Enumeration      *listDevices                 (bool               fBuildOnly = false);
   static Enumeration      *listXMLDevices              (bool               fBuildOnly = false);

   static OmniDevice       *findOmniDeviceEntry         (PSZCRO             pszShortName);

   static bool              addOmniToPATH               ();

   static PSZCRO            quoteString                 (PSZRO              pszString);
   static PSZCRO            dequoteString               (PSZRO              pszString);
};

#endif
