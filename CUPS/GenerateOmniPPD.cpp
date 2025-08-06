/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2001
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
 *
 * (cd ..; . ./setit; cd CUPS; ./GenerateOmniPPD --driver libXMLOmniDevice.so -sproperties='XMLMasterFile="/usr/share/Omni/Epson 9-pin 80 Col.xml"')
 *
 */
#include <cstdio>
#include <cstdlib>
#include <iomanip>

#include <cups/cups.h>
#include <cups/raster.h>

#include <glib.h>
#include <gmodule.h>

#include <Device.hpp>
#include <Omni.hpp>
#include <JobProperties.hpp>

#define TMM_TO_INCHES 25400.0
#define BUFSIZE1      64

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << " --driver printer_library_name"
             << " --number i"
             << " [-sproperties=\"properties\"]"
             << std::endl;
}

void
outputLine (std::ostringstream& oss)
{
//#define PPD_LINE_LENGTH 254
#define PPD_LINE_LENGTH 79
   std::string stringLine  = oss.str ();
   int         cbStart     = 0;
   int         cbSublength = 0;
   int         cbEnd       = stringLine.length ();

   do
   {
      cbSublength = omni::min (PPD_LINE_LENGTH, cbEnd - cbStart);

      std::cout << stringLine.substr (cbStart, cbSublength) << std::endl;

      cbStart += cbSublength;

   } while (cbStart < cbEnd);
}

int
main (int argc, char *argv[])
{
   std::ostringstream  oss;
   std::string         stringOss;
   char               *pszDriverLibrary      = 0;
   PSZ                 pszJobProperties      = 0;
   PSZRO               pszDeviceName         = 0;
   GModule            *hmodDevice            = 0;
   Device             *pDevice               = 0;
   PFNNEWDEVICEWARGS   pfnNewDeviceWArgs     = 0;
   Enumeration        *pEnum                 = 0;
   JobProperties      *pJP                   = 0;
   PSZRO               pszJP                 = 0;
   DeviceForm         *pDFCurrent            = 0;
   DeviceMedia        *pDMCurrent            = 0;
   DeviceTray         *pDTCurrent            = 0;
   DeviceResolution   *pDRCurrent            = 0;
   std::string        *pstringHash           = 0;
   std::string        *pstringDitherJPs      = 0;
   int                 iFileNumber           = 0;
   int                 rc                    = 0;

   Omni::initialize ();

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcmp (argv[i], "--driver"))
      {
         pszDriverLibrary = argv[++i];
      }
      else if (0 == strncasecmp ("-sproperties=", argv[i], 13))
      {
         char *pszJobProp   = argv[i] + 13;
         int   iSpaceNeeded = strlen (pszJobProp) + 1;

         if (pszJobProperties)
         {
            free ((void *)pszJobProperties);
            pszJobProperties = 0;
         }

         pszJobProperties = (char *)malloc (iSpaceNeeded);
         if (pszJobProperties)
         {
            strcpy (pszJobProperties, pszJobProp);
         }
         else
         {
            std::cerr << "Error@" << __LINE__ << ": Out of memory" << std::endl;

            rc = __LINE__;
            goto BUGOUT;
         }

#ifndef RETAIL
         DebugOutput::applyAllDebugOutput (pszJobProperties);
#endif
      }
      else if (0 == strcmp (argv[i], "--number"))
      {
         iFileNumber = atoi (argv[++i]);
      }
      else
      {
         printUsage (argv[0]);
      }
   }

   if (!pszDriverLibrary)
   {
      printUsage (argv[0]);

      rc = __LINE__;
      goto BUGOUT;
   }

   if (0 == strstr (pszDriverLibrary, ".so"))
   {
      oss << "lib"
          << pszDriverLibrary
          << ".so";

      stringOss        = oss.str ();
      pszDriverLibrary = (char *)stringOss.c_str ();
   }

   std::cerr << "Trying to load " << pszDriverLibrary << std::endl;

   if (!Omni::openAndTestDeviceLibrary (pszDriverLibrary, &hmodDevice))
   {
      rc = __LINE__;
      goto BUGOUT;
   }

   g_module_symbol (hmodDevice,
                    "newDeviceW_JopProp_Advanced",
                    (gpointer *)&pfnNewDeviceWArgs);

   if (pfnNewDeviceWArgs)
   {
      pDevice = pfnNewDeviceWArgs (pszJobProperties, true);
   }

   if (!pDevice)
   {
      std::cerr << "Error@" << __LINE__ << ": Could not instantiate a new device with \"" << pszJobProperties << "\"" << std::endl;

      rc = __LINE__;
      goto BUGOUT;
   }

   if (pDevice->hasError ())
   {
      std::cerr << "Error@" << __LINE__ << ": The device had an error with \"" << pszJobProperties << "\"" << std::endl;

      rc = __LINE__;
      goto BUGOUT;
   }

   pszDeviceName = pDevice->getDeviceName ();

   std::cout << "*PPD-Adobe:			\"4.3\"" << std::endl;
   std::cout << "*%PPD file for CUPS/Omni." << std::endl;
   std::cout << "*%This PPD file may be freely used and distributed under the terms of" << std::endl;
   std::cout << "*%the GNU LGPL." << std::endl;
   std::cout << "*FormatVersion:			\"4.3\"" << std::endl;
   std::cout << "*FileVersion:			\"" << pDevice->getVersion () << "\"" << std::endl;
   std::cout << "*LanguageVersion:		English" << std::endl;
   std::cout << "*LanguageEncoding:		ISOLatin1" << std::endl;
   std::cout << "*PCFileName:			\"omni" << std::hex << std::setfill ('0') << std::setw (4) << iFileNumber << std::dec << ".ppd\"" << std::endl;
   std::cout << "*Manufacturer:			\"" << pDevice->getDriverName () << "\"" << std::endl;
   std::cout << "*Product:			\"(Omni v" << pDevice->getVersion () << ")\"" << std::endl;
   std::cout << "*ModelName:			\"" << pDevice->getDeviceName () << "\"" << std::endl;
   std::cout << "*NickName:			\"" << pszDeviceName << ", CUPS + omni\"" << std::endl;
   std::cout << "*ShortNickName:			\"" << pszDeviceName << "\"" << std::endl;
   std::cout << "*OmniLibrary:			\"" << pDevice->getLibraryName () << "\"" << std::endl;
   {
      std::string *pstringJP = pDevice->getJobProperties ();

      if (pstringJP)
      {
            PSZCRO pszQuoted = Omni::quoteString (pstringJP->c_str ());

            if (pszQuoted)
            {
               std::ostringstream oss;

               oss << "*OmniJobProperties:		\""
                   << pszQuoted
                   << "\"";

               outputLine (oss);

               free ((void *)pszQuoted);
            }

         delete pstringJP;
      }
   }
   std::cout << "*PSVersion:			\"(3010.000) 550\"" << std::endl;
   std::cout << "*LanguageLevel:			\"3\"" << std::endl;

   if (pDevice->hasCapability (Capability::COLOR))
   {
      std::cout << "*ColorDevice:			True" << std::endl;
      std::cout << "*DefaultColorSpace:		RGB" << std::endl;
   }
   else
   {
      std::cout << "*ColorDevice:			False" << std::endl;
      std::cout << "*DefaultColorSpace:		Gray" << std::endl;
   }

   std::cout << "*FileSystem:			False" << std::endl;
   std::cout << "*LandscapeOrientation:		Plus90" << std::endl;
   std::cout << "*TTRasterizer:			Type42" << std::endl;

   std::cout << "*cupsVersion:			1.1" << std::endl;
///*cupsModelNumber @TBD
   std::cout << "*cupsManualCopies:		True" << std::endl;
   std::cout << "*cupsFilter:			\"application/vnd.cups-raster 100 CUPSToOmni\"" << std::endl;

   std::cout << std::endl;

   std::cout << "*OpenUI *PageSize/Page Size: PickOne" << std::endl;
   std::cout << "*OrderDependency: 10 AnySetup *PageSize" << std::endl;

   pDFCurrent = pDevice->getCurrentForm ();
   pEnum      = pDFCurrent->getEnumeration ();

   pstringHash = pDFCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultPageSize: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   while (pEnum->hasMoreElements ())
   {
      DeviceForm *pDF = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDF = pDFCurrent->create (pDevice, pszJP);

      if (!pDF)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Form from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;
      HardCopyCap *pHCC         = pDF->getHardCopyCap ();
      int          iCx;
      int          iCy;

      iCx = (int)(72.0 * (float)pHCC->getCx () / TMM_TO_INCHES);
      iCy = (int)(72.0 * (float)pHCC->getCy () / TMM_TO_INCHES);

      pstringHash  = pDF->getCreateHash ();
      pstringXLate = pDF->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*PageSize "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\"<</PageSize["
                   << iCx
                   << " "
                   << iCy
                   << "]/ImagingBBox null>>setpagedevice\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDF;
   }

   std::cout << "*CloseUI: *PageSize" << std::endl;

   delete pEnum;

   std::cout << std::endl;

   std::cout << "*OpenUI *PageRegion: PickOne" << std::endl;
   std::cout << "*OrderDependency: 10 AnySetup *PageRegion" << std::endl;

   pEnum = pDFCurrent->getEnumeration ();

   pstringHash = pDFCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultPageRegion: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   while (pEnum->hasMoreElements ())
   {
      DeviceForm *pDF = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDF = pDFCurrent->create (pDevice, pszJP);

      if (!pDF)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Form from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;
      HardCopyCap *pHCC         = pDF->getHardCopyCap ();
      int          iCx;
      int          iCy;

      iCx = (int)(72.0 * (float)pHCC->getCx () / TMM_TO_INCHES);
      iCy = (int)(72.0 * (float)pHCC->getCy () / TMM_TO_INCHES);

      pstringHash  = pDF->getCreateHash ();
      pstringXLate = pDF->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*PageRegion "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\"<</PageRegion["
                   << iCx
                   << " "
                   << iCy
                   << "]/ImagingBBox null>>setpagedevice\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDF;
   }

   std::cout << "*CloseUI: *PageRegion" << std::endl;

   delete pEnum;

   std::cout << std::endl;

   pEnum = pDFCurrent->getEnumeration ();

   pstringHash = pDFCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultImageableArea: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   while (pEnum->hasMoreElements ())
   {
      DeviceForm *pDF = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDF = pDFCurrent->create (pDevice, pszJP);

      if (!pDF)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Form from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;
      HardCopyCap *pHCC         = pDF->getHardCopyCap ();
      float        iCx          = 72.0 * (float)pHCC->getCx         () / TMM_TO_INCHES;
      float        iCy          = 72.0 * (float)pHCC->getCy         () / TMM_TO_INCHES;
      float        iLeft        = 72.0 * (float)pHCC->getLeftClip   () / TMM_TO_INCHES;
      float        iRight       = 72.0 * (float)pHCC->getRightClip  () / TMM_TO_INCHES;
      float        iTop         = 72.0 * (float)pHCC->getTopClip    () / TMM_TO_INCHES;
      float        iBottom      = 72.0 * (float)pHCC->getBottomClip () / TMM_TO_INCHES;

      pstringHash  = pDF->getCreateHash ();
      pstringXLate = pDF->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*ImageableArea "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\""
                   << iLeft
                   << " "
                   << iBottom
                   << " "
                   << iCx - iRight
                   << " "
                   << iCy - iTop
                   << "\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDF;
   }

   delete pEnum;

   std::cout << std::endl;

   pEnum = pDFCurrent->getEnumeration ();

   pstringHash = pDFCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultPaperDimension: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   while (pEnum->hasMoreElements ())
   {
      DeviceForm *pDF = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Form Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDF = pDFCurrent->create (pDevice, pszJP);

      if (!pDF)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Form from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;
      HardCopyCap *pHCC         = pDF->getHardCopyCap ();
      int          iCx          = (int)(72.0 * (float)pHCC->getCx () / TMM_TO_INCHES);
      int          iCy          = (int)(72.0 * (float)pHCC->getCy () / TMM_TO_INCHES);

      pstringHash  = pDF->getCreateHash ();
      pstringXLate = pDF->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*PaperDimension "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\""
                   << iCx
                   << " "
                   << iCy
                   << "\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDF;
   }

   delete pEnum;

   std::cout << std::endl;

   std::cout << "*OpenUI *ColorModel: PickOne" << std::endl;
   std::cout << "*OrderDependency: 10 AnySetup *ColorModel" << std::endl;

   if (pDevice->hasCapability (Capability::COLOR))
   {
      std::cout << "*DefaultColorModel: RGB"
                << std::endl;
      std::cout << "*ColorModel Gray/Grayscale:\t\"<</cupsColorSpace "
                << (unsigned long int)CUPS_CSPACE_W
                << "/cupsColorOrder "
                << (unsigned long int)CUPS_ORDER_CHUNKED
                << "/cupsBitsPerColor 1>>setpagedevice\""
                << std::endl;
      std::cout << "*ColorModel RGB/Color:\t\"<</cupsColorSpace "
                << (unsigned long int)CUPS_CSPACE_RGB
                << "/cupsColorOrder "
                << (unsigned long int)CUPS_ORDER_CHUNKED
                << "/cupsBitsPerColor 8>>setpagedevice\"" // @TBD - shouldn't this be 24 bits per pel?
                << std::endl;
   }
   else
   {
      std::cout << "*DefaultColorModel: Gray"
                << std::endl;
      std::cout << "*ColorModel Gray/Grayscale:\t\"<</cupsColorSpace "
                << (unsigned long int)CUPS_CSPACE_W
                << "/cupsColorOrder "
                << (unsigned long int)CUPS_ORDER_CHUNKED
                << "/cupsBitsPerColor 1>>setpagedevice\""
                << std::endl;
   }

   std::cout << "*CloseUI: *ColorModel" << std::endl;

   std::cout << std::endl;

   std::cout << "*OpenUI *MediaType: PickOne" << std::endl;
   std::cout << "*OrderDependency: 10 AnySetup *MediaType" << std::endl;

   pDMCurrent = pDevice->getCurrentMedia ();

   pstringHash = pDMCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultMediaType: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   pEnum = pDMCurrent->getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      DeviceMedia *pDM = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDM = pDMCurrent->create (pDevice, pszJP);

      if (!pDM)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Media from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;

      pstringHash  = pDM->getCreateHash ();
      pstringXLate = pDM->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*MediaType "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\"<</MediaType("
                   << *pstringHash
                   << ")>>setpagedevice\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDM;
   }

   std::cout << "*CloseUI: *MediaType" << std::endl;

   delete pEnum;

   std::cout << std::endl;

   std::cout << "*OpenUI *InputSlot: PickOne" << std::endl;
   std::cout << "*OrderDependency: 10 AnySetup *InputSlot" << std::endl;

   pDTCurrent = pDevice->getCurrentTray ();

   pstringHash = pDTCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultInputSlot: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   pEnum = pDTCurrent->getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      DeviceTray *pDT = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDT = pDTCurrent->create (pDevice, pszJP);

      if (!pDT)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Tray from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;

      pstringHash  = pDT->getCreateHash ();
      pstringXLate = pDT->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*InputSlot "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\"<</MediaClass("
                   << *pstringHash
                   << ")>>setpagedevice\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDT;
   }

   std::cout << "*CloseUI: *InputSlot" << std::endl;

   delete pEnum;

   std::cout << std::endl;

   std::cout << "*OpenUI *Dither: PickOne" << std::endl;
   std::cout << "*OrderDependency: 10 AnySetup *Dither" << std::endl;

   pEnum = DeviceDither::getAllEnumeration ();

   pstringDitherJPs = DeviceDither::getDitherJobProperties (pDevice->getCurrentDitherID ());

   if (pstringDitherJPs)
   {
      pstringHash = DeviceDither::getCreateHash (pstringDitherJPs->c_str ());

      if (pstringHash)
      {
         std::cout << "*DefaultDither: " << *pstringHash << std::endl;

         delete pstringHash;
      }
      else
      {
         std::cerr << "Error@" << __LINE__ << ": getCreateHash failed!" << std::endl;
      }

      delete pstringDitherJPs;
   }
   else
   {
      std::cerr << "Error@" << __LINE__ << ": getDitherJobProperties failed!" << std::endl;
   }

   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pstringHash = DeviceDither::getCreateHash (pszJP);

      std::cout << "*Dither "
                << *pstringHash
                << "/"
                << DeviceDither::getName (pDevice, pszJP)
                << ":\t\"<</OutputType("
                << *pstringHash
                << ")>>setpagedevice\""
                << std::endl;

      delete pstringHash;
      delete pJP;
      free ((void *)pszJP);
   }

   std::cout << "*CloseUI: *Dither" << std::endl;

   delete pEnum;

   std::cout << std::endl;

   std::cout << "*OpenUI *Resolution: PickOne" << std::endl;
   std::cout << "*OrderDependency: 20 AnySetup *Resolution" << std::endl;

   pDRCurrent = pDevice->getCurrentResolution ();

   pstringHash = pDRCurrent->getCreateHash ();
   if (pstringHash)
   {
      std::cout << "*DefaultResolution: "
                << *pstringHash
                << std::endl;

      delete pstringHash;
   }

   pEnum = pDRCurrent->getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      DeviceResolution *pDR = 0;

      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error@" << __LINE__ << ": Job Properties returns NULL!" << std::endl;
         delete pJP;
         continue;
      }

      pDR = pDRCurrent->create (pDevice, pszJP);

      if (!pDR)
      {
         std::cerr << "Error@" << __LINE__ << ": Cannot create a new Resolution from \"" << pszJP << "\" !" << std::endl;
         delete pJP;
         free ((void *)pszJP);
         continue;
      }

      std::string *pstringXLate = 0;

      pstringHash  = pDR->getCreateHash ();
      pstringXLate = pDR->getAllTranslation ();

      if (  pstringHash
         && pstringXLate
         )
      {
         std::cout << "*Resolution "
                   << *pstringHash
                   << "/"
                   << *pstringXLate
                   << ":\t\"<</HWResolution["
                   << pDR->getXRes ()
                   << " "
                   << pDR->getYRes ()
                   << "]/cupsCompression "
                   << 0                        // @TBD - correct?
                   << ">>setpagedevice\""
                   << std::endl;
      }

      delete pstringHash;
      delete pstringXLate;
      delete pJP;
      free ((void *)pszJP);
      delete pDR;
   }

   std::cout << "*CloseUI: *Resolution" << std::endl;

   delete pEnum;

   std::cout << std::endl;

   std::cout << "*DefaultFont: Courier" << std::endl;
   std::cout << "*Font AvantGarde-Book: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font AvantGarde-BookOblique: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font AvantGarde-Demi: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font AvantGarde-DemiOblique: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Bookman-Demi: Standard \"(001.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Bookman-DemiItalic: Standard \"(001.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Bookman-Light: Standard \"(001.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Bookman-LightItalic: Standard \"(001.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Courier: Standard \"(002.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Courier-Bold: Standard \"(002.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Courier-BoldOblique: Standard \"(002.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Courier-Oblique: Standard \"(002.004S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-Bold: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-BoldOblique: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-Narrow: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-Narrow-Bold: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-Narrow-BoldOblique: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-Narrow-Oblique: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font Helvetica-Oblique: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font NewCenturySchlbk-Bold: Standard \"(001.009S)\" Standard ROM" << std::endl;
   std::cout << "*Font NewCenturySchlbk-BoldItalic: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font NewCenturySchlbk-Italic: Standard \"(001.006S)\" Standard ROM" << std::endl;
   std::cout << "*Font NewCenturySchlbk-Roman: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Palatino-Bold: Standard \"(001.005S)\" Standard ROM" << std::endl;
   std::cout << "*Font Palatino-BoldItalic: Standard \"(001.005S)\" Standard ROM" << std::endl;
   std::cout << "*Font Palatino-Italic: Standard \"(001.005S)\" Standard ROM" << std::endl;
   std::cout << "*Font Palatino-Roman: Standard \"(001.005S)\" Standard ROM" << std::endl;
   std::cout << "*Font Symbol: Special \"(001.007S)\" Special ROM" << std::endl;
   std::cout << "*Font Times-Bold: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Times-BoldItalic: Standard \"(001.009S)\" Standard ROM" << std::endl;
   std::cout << "*Font Times-Italic: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font Times-Roman: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font ZapfChancery-MediumItalic: Standard \"(001.007S)\" Standard ROM" << std::endl;
   std::cout << "*Font ZapfDingbats: Special \"(001.004S)\" Standard ROM" << std::endl;

   std::cout << "*%%End of " << pszDeviceName << ".ppd" << std::endl;

BUGOUT:
   delete pDevice; pDevice = 0;

   if (hmodDevice)
   {
      g_module_close (hmodDevice);
      hmodDevice = 0;
   }

   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
      pszJobProperties = 0;
   }

   Omni::terminate ();

   return rc;
}
