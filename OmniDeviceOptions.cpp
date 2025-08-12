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
#include "Device.hpp"
#include "Omni.hpp"
#include "DeviceInfo.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <list>

#include <glib.h>
#include <gmodule.h>

typedef int (*PFNSHOWOPTIONS) (Device *pDevice,
                               PSZCRO  pszJobProperties);

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << " ( --UsePDC | --!UsePDC | --driver printer_library_name | '-sproperties=\"...\"' | --Verbose | --all | printer_short_name )+"
             << std::endl;
}

int
executeDevice (PSZRO             pszFullDeviceName,
               bool              fUsePDC,
               PFNSHOWOPTIONS    pfnShowOptions,
               PSZCRO            pszJobProperties)
{
   Device       *pDevice      = 0;
   GModule      *hmodDevice   = 0;
   int           rc           = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": pszFullDeviceName = \"" << SAFE_PRINT_PSZ (pszFullDeviceName) << "\" pszJobProperties = \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\"" << std::endl;
#endif

   if (fUsePDC)
   {
#ifdef PDC_INTERFACE_ENABLED
      char *pszSlash = const_cast<char*>(strchr (pszFullDeviceName, '/'));

      while (pszSlash)
      {
         pszFullDeviceName = pszSlash + 1;

         pszSlash = const_cast<char*>(strchr (pszFullDeviceName, '/'));
      }

      pDevice = new OmniPDCProxy (0,                 // client exe to spawn
                                  pszFullDeviceName, // device name
                                  pszJobProperties,  // job properties
                                  true);             // is renderer
#endif
   }
   else
   {
      PFNNEWDEVICE       pfnNewDevice      = 0;
      PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

      hmodDevice = g_module_open (pszFullDeviceName, (GModuleFlags)0);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": hmodDevice        = " << std::hex << hmodDevice << std::dec << std::endl;
#endif

      if (!hmodDevice)
      {
         DebugOutput::getErrorStream ()
            << "Error: trying to load \""
            << SAFE_PRINT_PSZ (pszFullDeviceName)
            << "\" g_module_error returns "
            << g_module_error ()
            << std::endl;

         return 2;
      }

      g_module_symbol (hmodDevice, "newDeviceW_Advanced",         (gpointer *)&pfnNewDevice);
      g_module_symbol (hmodDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << std::hex << ": pfnNewDevice      = 0x" << (int)pfnNewDevice << std::dec << std::endl;
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << std::hex << ": pfnNewDeviceWArgs = 0x" << (int)pfnNewDeviceWArgs << std::dec << std::endl;
#endif

      if (  !pfnNewDevice
         || !pfnNewDeviceWArgs
         )
      {
         DebugOutput::getErrorStream () << "Error: trying to load newDeviceW[_JobProp]_Advanced. g_module_error returns " << g_module_error () << std::endl;

         return 3;
      }

      if (  !pszJobProperties
         || !*pszJobProperties
         )
      {
         pDevice = pfnNewDevice (true);
      }
      else
      {
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      if (pDevice)
      {
         DebugOutput::getErrorStream () << ": pDevice = " << *pDevice << std::endl;
      }
      else
      {
         DebugOutput::getErrorStream () << ": pDevice = NULL" << std::endl;
      }
   }
#endif

   if (!pDevice)
   {
      DebugOutput::getErrorStream () << "Error: No Device was created!" << std::endl;

      return 4;
   }

   // Check for an error
   if (pDevice->hasError ())
   {
      DebugOutput::getErrorStream () << "Error: The device has an error." << std::endl;

      return 5;
   }

   rc = pfnShowOptions (pDevice, pszJobProperties);

   if (pDevice->hasError ())
   {
      DebugOutput::getErrorStream () << "Error: The device has an error." << std::endl;

      return 6;
   }

   delete pDevice;

   if (hmodDevice)
   {
#ifndef RETAIL
      int rc2 =
#endif

                g_module_close (hmodDevice);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": g_module_close returns " << rc2 << std::endl;
#endif
   }

   return rc;
}

int
showDeviceJobProperties (Device *pDevice,
                         bool    fInDeviceSpecific)
{
   Enumeration *pEnumJPGroups = 0;
   Enumeration *pEnumJPGroup  = 0;

   pEnumJPGroups = pDevice->listJobProperties (fInDeviceSpecific);

   if (!pEnumJPGroups)
   {
      std::cerr << "Error: listJobProperties returns NULL!" << std::endl;

      return 1;
   }

   while (  pEnumJPGroups
         && pEnumJPGroups->hasMoreElements ()
         )
   {
      std::string *pstringDefault = 0;
      bool         fFirst         = true;

      pEnumJPGroup = (Enumeration *)pEnumJPGroups->nextElement ();

      while (pEnumJPGroup->hasMoreElements ())
      {
         JobProperties         *pJPs     = 0;
         JobPropertyEnumerator *pEnumJPs = 0;

         pJPs = (JobProperties *)pEnumJPGroup->nextElement ();
         if (pJPs)
         {
            pEnumJPs = pJPs->getEnumeration ();

            if (1 < pJPs->getNumProperties ())
            {
               fFirst = true;
            }
         }
         else
         {
            std::cerr << "Error: no job properties returned from enumeration" << std::endl;
         }

         while (  pEnumJPs
               && pEnumJPs->hasMoreElements ()
               )
         {
            PSZRO pszKey      = pEnumJPs->getCurrentKey          ();
            PSZRO pszValue    = pEnumJPs->getCurrentValue        ();
            PSZRO pszMinValue = pEnumJPs->getCurrentMinimumValue ();
            PSZRO pszMaxValue = pEnumJPs->getCurrentMaximumValue ();

            if (fFirst)
            {
               fFirst = false;

               std::cout << pszKey;

               if (1 == pJPs->getNumProperties ())
               {
                  pstringDefault = pDevice->getJobProperty (pszKey);

                  std::cout << ":" << std::endl;
               }
               else
               {
                  std::cout << "=";
               }
            }
            else
            {
               if (1 < pJPs->getNumProperties ())
               {
                  std::cout << " "
                            << pszKey
                            << "=";
               }
            }

            if (1 == pJPs->getNumProperties ())
            {
               std::cout << "\t";
            }

            if (  pszMinValue
               && pszMaxValue
               )
            {
               int iMinResult = strcmp (pszMinValue, pszValue);
               int iMaxResult = strcmp (pszMaxValue, pszValue);

               std::cout << pszMinValue;

               if (  0 == iMinResult
                  && 0 == iMaxResult
                  )
               {
                  std::cout << " (*)";
               }
               else if (  0 == iMinResult
                       && 0 != iMaxResult
                       )
               {
                  std::cout << " (*) .. "
                            << pszMaxValue;
               }
               else if (  0 != iMinResult
                       && 0 != iMaxResult
                       )
               {
                  std::cout << " .. "
                            << pszValue
                            << " (*) .. "
                            << pszMaxValue;
               }
               else
               {
                  std::cout << " .. "
                            << pszMaxValue
                            << " (*)";
               }
            }
            else
            {
               std::cout << pszValue;

               if (  1 == pJPs->getNumProperties ()
                  && pstringDefault
                  && 0 == pstringDefault->compare (pszValue)
                  )
               {
                  std::cout << " (*)";
               }

               if (1 == pJPs->getNumProperties ())
               {
                  std::cout << std::endl;
               }
            }

            pEnumJPs->nextElement ();
         }

         if (  pJPs
            && 1 < pJPs->getNumProperties ()
            )
         {
            std::cout << std::endl;
         }

         delete pEnumJPs;
         delete pJPs;
      }

      std::cout << std::endl;

      delete pstringDefault;
      delete pEnumJPGroup;
   }

   delete pEnumJPGroups;

   return 0;
}

int
showDeviceGS (Device *pDevice,
              PSZCRO  pszJobProperties)
{
   std::cout << "For gs add the following: -sDEVICE=omni -sDeviceName="
             << pDevice->getLibraryName ()
             << " -sproperties='";
   if (pszJobProperties)
   {
      std::cout << pszJobProperties << " ";
   }
   std::cout << "...'"
             << std::endl;
   std::cout << "\twhere you replace ... with the following space separated key=value (* - default):"
             << std::endl;
   std::cout << std::endl;

   return showDeviceJobProperties (pDevice, false);
}

int
showDeviceVerbose (Device *pDevice,
                   PSZCRO  pszJobProperties)
{
   JobProperties *pJP         = 0;
   PSZRO          pszJP       = 0;
   Enumeration   *pEnum       = 0;
   std::string   *pstringRet  = 0;

   std::cout << "Driver version is : "
             << pDevice->getVersion ()
             << std::endl
             << "Driver name is : "
             << pDevice->getDriverName ()
             << std::endl
             << "Device name is : "
             << pDevice->getDeviceName ()
             << std::endl
             << "Device short name is : "
             << pDevice->getShortName ()
             << std::endl
             << "Device library name is : "
             << pDevice->getLibraryName ()
             << std::endl;

   std::cout << "Device class is : ";
   switch (pDevice->getOmniClass ())
   {
   case OMNI_CLASS_UNKNOWN:  std::cout << "*** UNKNOWN ***" << std::endl; break;
   case OMNI_CLASS_COMPILED: std::cout << "compiled" << std::endl; break;
   case OMNI_CLASS_XML:      std::cout << "XML" << std::endl; break;
   case OMNI_CLASS_UPDF:     std::cout << "UPDF" << std::endl; break;
   }

   std::cout << "The PDL level is "
             << pDevice->getPDLLevel ()
             << std::endl
             << "The PDL sublevel is "
             << pDevice->getPDLSubLevel ()
             << std::endl
             << "The PDL major revision level is "
             << pDevice->getPDLMajorRevisionLevel ()
             << std::endl
             << "The PDL minor revision level is "
             << pDevice->getPDLMinorRevisionLevel ()
             << std::endl;

   pstringRet = pDevice->getJobProperties (false);
   if (pstringRet)
   {
      std::cout << "Device's job properties are : \"" << *pstringRet << "\"" << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cout << "Error: Device did not return job properties!" << std::endl;
   }

   pstringRet = pDevice->getJobProperties (true);
   if (pstringRet)
   {
      std::cout << "Device's device-specific job properties are : \"" << *pstringRet << "\"" << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cout << "Error: Device did not return device-specific job properties!" << std::endl;
   }

   showDeviceJobProperties (pDevice, true);

#ifdef INCLUDE_JP_UPDF_BOOKLET
   // @TBD
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies *pCopyCurrent = pDevice->getCurrentCopies ();

   std::cout << "Current copy is:" << std::endl;
   pstringRet = pCopyCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

   DeviceForm *pFormCurrent = pDevice->getCurrentForm ();

   std::cout << "Current form is:" << std::endl;
   pstringRet = pFormCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   // @TBD
#endif

   DeviceMedia *pMediaCurrent = pDevice->getCurrentMedia ();

   std::cout << "Current media is:" << std::endl;
   pstringRet = pMediaCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }

#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp *pNUpCurrent = pDevice->getCurrentNUp ();

   std::cout << "Current nup is:" << std::endl;
   pstringRet = pNUpCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

   DeviceOrientation *pOrientationCurrent = pDevice->getCurrentOrientation ();

   std::cout << "Current orientation is:" << std::endl;
   pstringRet = pOrientationCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin *pOutputBinCurrent = pDevice->getCurrentOutputBin ();

   std::cout << "Current outputbin is:" << std::endl;
   pstringRet = pOutputBinCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

   DevicePrintMode *pPrintModeCurrent = pDevice->getCurrentPrintMode ();

   std::cout << "Current print mode is:" << std::endl;
   pstringRet = pPrintModeCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }

   DeviceResolution *pResolutionCurrent = pDevice->getCurrentResolution ();

   std::cout << "Current resolution is:" << std::endl;
   pstringRet = pResolutionCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling *pScalingCurrent = pDevice->getCurrentScaling ();

   std::cout << "Current scaling is:" << std::endl;
   pstringRet = pScalingCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate *pSheetCollateCurrent = pDevice->getCurrentSheetCollate ();

   std::cout << "Current sheetcollate is:" << std::endl;
   pstringRet = pSheetCollateCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide *pSideCurrent = pDevice->getCurrentSide ();

   std::cout << "Current side is:" << std::endl;
   pstringRet = pSideCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching *pStitchingCurrent = pDevice->getCurrentStitching ();

   std::cout << "Current stitching is:" << std::endl;
   pstringRet = pStitchingCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

   DeviceTray *pTrayCurrent = pDevice->getCurrentTray ();

   std::cout << "Current tray is:" << std::endl;
   pstringRet = pTrayCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming *pTrimmingCurrent = pDevice->getCurrentTrimming ();

   std::cout << "Current trimming is:" << std::endl;
   pstringRet = pTrimmingCurrent->getJobProperties ();
   if (pstringRet)
   {
      std::cout << *pstringRet << std::endl;
      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty!" << std::endl;
   }
#endif

   DeviceGamma *pGamma = pDevice->getCurrentGamma ();

   std::cout << "Current gamma is:" << std::endl;

   if (pGamma)
   {
      std::cout << "{ "
                << pGamma->getCGamma ()
                << " "
                << pGamma->getMGamma ()
                << " "
                << pGamma->getYGamma ()
                << " "
                << pGamma->getKGamma ()
                << " "
                << pGamma->getCBias ()
                << " "
                << pGamma->getMBias ()
                << " "
                << pGamma->getYBias ()
                << " "
                << pGamma->getKBias ()
                << "}"
                << std::endl;
   }
   else
   {
      std::cout << "Non existant!" << std::endl;
   }

///std::cout << "Current dither is:" << std::endl;
///std::cout << pDevice->getCurrentDither () << std::endl;

   std::cout << "Current dither id is:" << std::endl;
   std::cout << pDevice->getCurrentDitherID () << std::endl;

   std::cout << "Enumerating dithering ids:" << std::endl;
   pEnum = DeviceDither::getAllEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error: Dither Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      std::cout << pszJP << std::endl;

      delete pJP;
      free ((void *)pszJP);
   }
   std::cout << "Done!" << std::endl;

   delete pEnum;

   if (pDevice->hasCapability (Capability::COLOR))
   {
      std::cout << "This device can print in color." << std::endl;
   }

   if (pDevice->hasCapability (Capability::MONOCHROME))
   {
      std::cout << "This device can print in monochrome." << std::endl;
   }

   if (pDevice->hasCapability (Capability::MIRROR))
   {
      std::cout << "This device can print in mirror image." << std::endl;
   }

   std::cout << "Portrait is ";
   if (!pOrientationCurrent->isSupported ("Rotation=Portrait"))
   {
      std::cout << "not ";
   }
   std::cout << "supported" << std::endl;

   std::cout << "Letter is ";
   if (!pFormCurrent->isSupported ("Form=na_letter"))
   {
      std::cout << "not ";
   }
   std::cout << "supported" << std::endl;

   std::cout << "Auto is ";
   if (!pTrayCurrent->isSupported ("InputTray=AutoSelect"))
   {
      std::cout << "not ";
   }
   std::cout << "supported" << std::endl;

   std::cout << "Plain is ";
   if (!pMediaCurrent->isSupported ("media=MEDIA_PLAIN"))
   {
      std::cout << "not ";
   }
   std::cout << "supported" << std::endl;

   std::cout << "360x360 is ";
   if (!pResolutionCurrent->isSupported ("Resolution=360x360"))
   {
      std::cout << "not ";
   }
   std::cout << "supported" << std::endl;

   std::cout << "CMYK is ";
   if (!pPrintModeCurrent->isSupported ("printmode=PRINT_MODE_24_CMYK"))
   {
      std::cout << "not ";
   }
   std::cout << "supported" << std::endl;

   return 0;
}

int
main (int argc, char *argv[])
{
   int            rc                = 0;
   bool           fUsePDC           = false;
   PFNSHOWOPTIONS pfnShowOptions    = showDeviceGS;
   PSZRO          pszJobProperties  = 0;
   bool           fBuildOnly        = false;
   JobProperties  jobProp;

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "Error: This program needs glib's module routines!" << std::endl;

      return __LINE__;
   }

   if (2 > argc)
   {
      printUsage (argv[0]);

      return __LINE__;
   }

   Omni::initialize ();

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp (argv[i], "--buildOnly"))
      {
         fBuildOnly = true;
      }
      else if (0 == strcasecmp (argv[i], "--all"))
      {
         Enumeration *pEnum = Omni::listDevices (fBuildOnly);

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO        pszLibName  = pOD->getLibraryName ();
               PSZRO         pszJobProps = pOD->getJobProperties ();
               JobProperties jobPropNew  = jobProp;

               jobPropNew.setJobProperties (pszJobProps);

               pszJobProps = jobPropNew.getJobProperties ();

               std::cerr << "Checking: " << pszLibName << ", \"" << pszJobProps << "\"" << std::endl;

               rc |= executeDevice (pszLibName,
                                    fUsePDC,
                                    pfnShowOptions,
                                    pszJobProps);

               if (pszJobProps)
               {
                  free ((void*)pszJobProps);
                  pszJobProps = 0;
               }

               delete pOD;
            }
            else
            {
               std::cerr << "Error: No OmniDevice was returned!" << std::endl;

               rc = __LINE__;
            }
         }

         delete pEnum;
      }
      else if (0 == strcasecmp (argv[i], "--UsePDC"))
      {
         fUsePDC = true;
      }
      else if (0 == strcasecmp (argv[i], "--!UsePDC"))
      {
         fUsePDC = false;
      }
      else if (0 == strcasecmp (argv[i], "--Verbose"))
      {
         pfnShowOptions = showDeviceVerbose;
      }
      else if (0 == strcasecmp (argv[i], "--driver"))
      {
         std::ostringstream  oss;
         std::string         stringOss;
         char               *pszDriverLibrary = argv[i + 1];

         if (0 != strncasecmp (argv[i + 1], "lib", 3))
         {
            oss << "lib"
                << pszDriverLibrary
                << ".so";

            stringOss        = oss.str ();
            pszDriverLibrary = (char *)stringOss.c_str ();
         }

         rc |= executeDevice (pszDriverLibrary,
                              fUsePDC,
                              pfnShowOptions,
                              pszJobProperties);

         i++;
      }
      else if (0 == strncasecmp ("-sproperties=", argv[i], 13))
      {
         char *pszJobProp = argv[i] + 13;

         jobProp.setJobProperties (pszJobProp);

#ifndef RETAIL
         jobProp.applyAllDebugOutput ();
#endif

         if (pszJobProperties)
         {
            free ((void *)pszJobProperties);
            pszJobProperties = 0;
         }
         pszJobProperties = jobProp.getJobProperties ();
      }
      else
      {
         DeviceInfo *pDI = Omni::findDeviceInfoFromShortName (argv[i], fBuildOnly);

         if (pDI)
         {
            Device     *pDevice    = 0;
            GModule    *hmodDevice = 0;
            OmniDevice *pOD        = 0;
            PSZRO       pszJobProp = 0;

            pDevice    = pDI->getDevice ();
            hmodDevice = pDI->getHmodDevice ();
            pOD        = pDI->getOmniDevice ();

            if (pOD)
            {
               pszJobProp = pOD->getJobProperties ();

               jobProp.setJobProperties (pszJobProp);

               pszJobProp = jobProp.getJobProperties ();
            }

            std::cout << "Info: For faster results use:"
                      << std::endl
                      << "Info:\t"
                      << argv[0];

            if (pszJobProp)
            {
               std::cout << " '-sproperties="
                         << pszJobProp
                         << "'";
            }

            std::cout << " --driver "
                      << pDevice->getLibraryName ()
                      << std::endl;

            rc |= executeDevice (pDevice->getLibraryName (),
                                 fUsePDC,
                                 pfnShowOptions,
                                 pszJobProp);

            if (pszJobProp)
            {
               free ((void *)pszJobProp);
               pszJobProp = 0;
            }

            delete pDI;
         }
      }
   }

   // Clean up!
   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
      pszJobProperties = 0;
   }

   Omni::terminate ();

   return rc;
}
