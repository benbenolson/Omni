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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <glib.h>
#include <gmodule.h>

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << " ( --driver printer_library_name | '-sproperties=\"...\"' | --all | printer_short_name )+"
             << std::endl;
}

void
formatOutput (std::string *pstringCol1,
              size_t       iMaxSizeCol1,
              std::string *pstringCol2,
              size_t       iMaxSizeCol2,
              PSZCRO       pszCol3      = 0,
              size_t       iMaxSizeCol3 = 0)
{
   size_t cbCol1 = 0;
   size_t cbCol2 = 0;
   size_t cbCol3 = 0;

   if (  !pstringCol1
      || !pstringCol2
      )
   {
      return;
   }

   cbCol1 = pstringCol1->length ();
   cbCol2 = pstringCol2->length ();
   if (pszCol3)
      cbCol3 = strlen (pszCol3);

   std::cout << pstringCol1;
   for (int i = iMaxSizeCol1 - cbCol1; i > 0; i--)
   {
      std::cout << " ";
   }
   std::cout << "\"" << pstringCol2 << "\"";
   for (int i = iMaxSizeCol2 - cbCol2; i > 0; i--)
   {
      std::cout << " ";
   }
   if (pszCol3)
   {
      std::cout << "\"" << pszCol3 << "\"";
      for (int i = iMaxSizeCol3 - cbCol3; i > 0; i--)
      {
         std::cout << " ";
      }
   }
}

int
handleDevice (PSZCRO pszFullDeviceName,
              PSZCRO pszJobProperties)
{
   Device            *pDevice           = 0;
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;
   GModule           *hmodDevice        = 0;

   std::cout << "Trying to load " << pszFullDeviceName << std::endl;

   hmodDevice = g_module_open (pszFullDeviceName, (GModuleFlags)0);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "hmodDevice = " << std::hex << hmodDevice << std::dec << std::endl;
#endif

   if (!hmodDevice)
   {
      DebugOutput::getErrorStream ()
         << "Error: trying to load \""
         << pszFullDeviceName
         << "\" g_module_error returns "
         << g_module_error ()
         << std::endl;

      return 2;
   }

   g_module_symbol (hmodDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << std::hex << "pfnNewDeviceWArgs = 0x" << (int)pfnNewDeviceWArgs << std::endl;
#endif

   if (!pfnNewDeviceWArgs)
   {
      DebugOutput::getErrorStream () << "g_module_error returns " << g_module_error () << std::endl;

      return 3;
   }

   pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "pDevice = " << *pDevice << std::endl;
#endif

   DeviceForm  *pDF    = pDevice->getCurrentForm ();
   Enumeration *pEnum  = pDF->getEnumeration ();
   size_t       cbCol1 = 0,
                cbCol2 = 0,
                cbCol3 = 0;

   std::cout << "Enumerating forms:" << std::endl;
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP   = 0;
      std::string *pstringAT   = 0;
      std::string *pstringForm = 0;
      PSZRO        pszSFN      = 0;

      pDF         = (DeviceForm *)pEnum->nextElement ();
      pstringJP   = pDF->getJobProperties ();
      pstringAT   = pDF->getAllTranslation ();
      pstringForm = pDF->getJobProperty ("Form");

      if (pstringForm)
      {
         pszSFN = DeviceForm::getShortFormName (pstringForm->c_str ());
      }

      cbCol1 = omni::max (cbCol1, pstringJP->length ());
      cbCol2 = omni::max (cbCol2, pstringAT->length ());
      if (pszSFN)
      {
         cbCol3 = omni::max (cbCol3, strlen (pszSFN));
      }

      delete pstringJP;
      delete pstringAT;
      delete pstringForm;
      if (pszSFN)
      {
         free ((void *)pszSFN);
      }
      delete pDF;
   }
   delete pEnum;
   cbCol1++;
   cbCol2++;
   cbCol3++;
   pDF    = pDevice->getCurrentForm ();
   pEnum  = pDF->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      HardCopyCap *pHCC        = 0;
      std::string *pstringJP   = 0;
      std::string *pstringAT   = 0;
      std::string *pstringForm = 0;
      PSZRO        pszSFN      = 0;

      pDF         = (DeviceForm *)pEnum->nextElement ();
      pHCC        = pDF->getHardCopyCap ();
      pstringJP   = pDF->getJobProperties ();
      pstringAT   = pDF->getAllTranslation ();
      pstringForm = pDF->getJobProperty ("Form");

      if (pstringForm)
      {
         pszSFN = DeviceForm::getShortFormName (pstringForm->c_str ());
      }

      formatOutput (pstringJP,
                    cbCol1,
                    pstringAT,
                    cbCol2,
                    pszSFN,
                    cbCol3);

      std::cout << "("
                << pHCC->getCx ()
                << ", "
                << pHCC->getCy ()
                << ", "
                << pHCC->getLeftClip ()
                << ", "
                << pHCC->getTopClip ()
                << ", "
                << pHCC->getRightClip ()
                << ", "
                << pHCC->getBottomClip ()
                << ")"
                << std::endl;

      delete pstringJP;
      delete pstringAT;
      delete pstringForm;
      if (pszSFN)
      {
         free ((void *)pszSFN);
      }
      delete pDF;
   }
   std::cout << "Done!" << std::endl << std::endl;

   delete pEnum;

   DeviceTray *pDT = pDevice->getCurrentTray ();

   pEnum = pDT->getEnumeration ();

   std::cout << "Enumerating trays:" << std::endl;
   cbCol1 = 0;
   cbCol2 = 0;
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP  = 0;
      std::string *pstringAT  = 0;

      pDT       = (DeviceTray *)pEnum->nextElement ();
      pstringJP = pDT->getJobProperties ();
      pstringAT = pDT->getAllTranslation ();

      cbCol1 = omni::max (cbCol1, pstringJP->length ());
      cbCol2 = omni::max (cbCol2, pstringAT->length ());

      delete pstringJP;
      delete pstringAT;
      delete pDT;
   }
   delete pEnum;
   cbCol1++;
   cbCol2++;
   pDT   = pDevice->getCurrentTray ();
   pEnum = pDT->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP  = 0;
      std::string *pstringAT  = 0;

      pDT       = (DeviceTray *)pEnum->nextElement ();
      pstringJP = pDT->getJobProperties ();
      pstringAT = pDT->getAllTranslation ();

      formatOutput (pstringJP,
                    cbCol1,
                    pstringAT,
                    cbCol2);
      std::cout << "("
                << ")"
                << std::endl;

      delete pstringJP;
      delete pstringAT;
      delete pDT;
   }
   std::cout << "Done!" << std::endl << std::endl;

   delete pEnum;

   DeviceMedia *pDM = pDevice->getCurrentMedia ();

   pDM   = pDevice->getCurrentMedia ();
   pEnum = pDM->getEnumeration ();
   pEnum = pDM->getEnumeration ();

   std::cout << "Enumerating medias:" << std::endl;
   cbCol1 = 0;
   cbCol2 = 0;
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP  = 0;
      std::string *pstringAT  = 0;

      pDM       = (DeviceMedia *)pEnum->nextElement ();
      pstringJP = pDM->getJobProperties ();
      pstringAT = pDM->getAllTranslation ();

      cbCol1 = omni::max (cbCol1, pstringJP->length ());
      cbCol2 = omni::max (cbCol2, pstringAT->length ());

      delete pstringJP;
      delete pstringAT;
      delete pDM;
   }
   delete pEnum;
   cbCol1++;
   cbCol2++;
   pDM   = pDevice->getCurrentMedia ();
   pEnum = pDM->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP  = 0;
      std::string *pstringAT  = 0;

      pDM       = (DeviceMedia *)pEnum->nextElement ();
      pstringJP = pDM->getJobProperties ();
      pstringAT = pDM->getAllTranslation ();

      formatOutput (pstringJP,
                    cbCol1,
                    pstringAT,
                    cbCol2);
      std::cout << "("
                << pDM->getColorAdjustRequired ()
                << ", "
                << pDM->getAbsorption ()
                << ")"
                << std::endl;

      delete pstringJP;
      delete pstringAT;
      delete pDM;
   }
   std::cout << "Done!" << std::endl << std::endl;

   delete pEnum;

   DeviceResolution *pDR = pDevice->getCurrentResolution ();

   pDR   = pDevice->getCurrentResolution ();
   pEnum = pDR->getEnumeration ();
   pEnum = pDR->getEnumeration ();

   std::cout << "Enumerating resolutions:" << std::endl;
   cbCol1 = 0;
   cbCol2 = 0;
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP  = 0;
      std::string *pstringAT  = 0;

      pDR       = (DeviceResolution *)pEnum->nextElement ();
      pstringJP = pDR->getJobProperties ();
      pstringAT = pDR->getAllTranslation ();

      cbCol1 = omni::max (cbCol1, pstringJP->length ());
      cbCol2 = omni::max (cbCol2, pstringAT->length ());

      delete pstringJP;
      delete pstringAT;
      delete pDR;
   }
   delete pEnum;
   cbCol1++;
   cbCol2++;
   pDR   = pDevice->getCurrentResolution ();
   pEnum = pDR->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      std::string *pstringJP  = 0;
      std::string *pstringAT  = 0;

      pDR       = (DeviceResolution *)pEnum->nextElement ();
      pstringJP = pDR->getJobProperties ();
      pstringAT = pDR->getAllTranslation ();

      formatOutput (pstringJP,
                    cbCol1,
                    pstringAT,
                    cbCol2);
      std::cout << "("
                << pDR->getXRes ()
                << ", "
                << pDR->getYRes ()
                << ", "
                << pDR->getDstBitsPerPel ()
                << ", "
                << pDR->getScanlineMultiple ()
                << ")"
                << std::endl;

      delete pstringJP;
      delete pstringAT;
      delete pDR;
   }
   std::cout << "Done!" << std::endl << std::endl;

   delete pEnum;

   delete pDevice;

   if (hmodDevice)
   {
      int rc = 0;

      rc = g_module_close (hmodDevice);

      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "g_module_close returns " << rc << std::endl;
   }

   return 0;
}

int
main (int argc, char *argv[])
{
   int    iFailed           = 0;
   char  *pszJobProperties  = 0;

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "Error: This program needs glib's module routines!" << std::endl;

      return 1;
   }

   if (2 > argc)
   {
      printUsage (argv[0]);

      return 2;
   }

   Omni::initialize ();

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp (argv[i], "--all"))
      {
         Enumeration *pEnum = Omni::listDevices ();

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO pszLibName  = pOD->getLibraryName ();
               PSZCRO pszJobProps = pOD->getJobProperties (); // @TBD possibly listen to pszJobProperties

               std::cerr << "Checking: " << pszLibName << ", \"" << pszJobProps << "\"" << std::endl;

               iFailed |= handleDevice (pszLibName, pszJobProps);

               delete pOD;
            }
            else
            {
               std::cerr << "Error" << std::endl;

               iFailed = 1;
            }
         }

         delete pEnum;
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

         iFailed |= handleDevice (pszDriverLibrary, pszJobProperties);

         i++;
      }
      else if (0 == strncasecmp ("-sproperties=", argv[i], 13))
      {
         char *pszJobProp   = argv[i] + 13;
         int   iSpaceNeeded = strlen (pszJobProp) + 1;

         if (pszJobProperties)
         {
            free (pszJobProperties);
            pszJobProperties = 0;
         }

         pszJobProperties = (char *)malloc (iSpaceNeeded);
         if (pszJobProperties)
         {
            strcpy (pszJobProperties, pszJobProp);
         }
         else
         {
            std::cerr << "Error:  Out of memory" << std::endl;

            iFailed = 2;
            goto BUGOUT;
         }

#ifndef RETAIL
         DebugOutput::applyAllDebugOutput (pszJobProperties);
#endif
      }
      else
      {
         DeviceInfo *pDI = Omni::findDeviceInfoFromShortName (argv[i]);

         if (pDI)
         {
            Device     *pDevice          = 0;
            GModule    *hmodDevice       = 0;
            OmniDevice *pOD              = 0;
            PSZRO       pszJobProp       = 0;
            PSZRO       pszDriverLibrary = 0;

            pDevice    = pDI->getDevice ();
            hmodDevice = pDI->getHmodDevice ();
            pOD        = pDI->getOmniDevice ();

            if (pOD)
            {
               pszJobProp = pOD->getJobProperties ();
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

            pszDriverLibrary = pDevice->getLibraryName ();

            std::cout << " --driver "
                      << pszDriverLibrary
                      << std::endl;

            iFailed |= handleDevice (pszDriverLibrary, pszJobProp);

            delete pDI;
         }
      }
   }

BUGOUT:
   // Clean up!
   if (pszJobProperties)
   {
      free (pszJobProperties);
      pszJobProperties = 0;
   }

   Omni::terminate ();

   return iFailed;
}
