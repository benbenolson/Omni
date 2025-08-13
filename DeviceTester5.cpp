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

#include <Device.hpp>
#include <Omni.hpp>
#include <JobProperties.hpp>
#include <gmodule.h>
#include <cstdint>

Device *
loadDevice (PSZCRO    pszFullDeviceName,
            PSZCRO    pszJobProperties,
            GModule **phmodDevice)
{
   Device            *pDevice           = 0;
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

   if (phmodDevice)
   {
      *phmodDevice = 0;
   }

   if (  !pszFullDeviceName
      || !phmodDevice
      )
   {
      return 0;
   }

   std::cout << "Trying to load \""
             << (pszFullDeviceName ? pszFullDeviceName : "")
             << "\" with \""
             << (pszJobProperties ? pszJobProperties : "")
             << "\""
             << std::endl;

   *phmodDevice = g_module_open (pszFullDeviceName, (GModuleFlags)0);

   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << __FUNCTION__ << ": hmodDevice = " << std::hex << *phmodDevice << std::endl;

   if (!*phmodDevice)
   {
      DebugOutput::getErrorStream () << "g_module_error returns " << g_module_error () << std::endl;
      return 0;
   }

   g_module_symbol (*phmodDevice,
                    "newDeviceW_JopProp_Advanced",
                    (gpointer *)&pfnNewDeviceWArgs);

   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << __FUNCTION__ << ": pfnNewDeviceWArgs = 0x" << std::hex << reinterpret_cast<uintptr_t>(pfnNewDeviceWArgs) << std::dec << std::endl;

   if (!pfnNewDeviceWArgs)
      return 0;

   pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

   return pDevice;
}

int
handleDevice (PSZCRO pszFullDeviceName,
              PSZCRO pszJobProperties)
{
   GModule *hmodDevice = 0;
   Device  *pDevice    = 0;
   int      iRc        = 0;

   if (strstr (pszFullDeviceName, "/lib"))
   {
//    pszFullDeviceName = strstr (pszFullDeviceName, "/lib") + 1;
   }

//#define PDC 1
#ifdef PDC
   pDevice = new OmniPDCProxy (0,                 // client exe to spawn
                               pszFullDeviceName, // device name
                               pszJobProperties,  // job properties
                               true);             // is renderer
#else
   pDevice = loadDevice (pszFullDeviceName, pszJobProperties, &hmodDevice);
#endif

   if (!pDevice)
   {
      DebugOutput::getErrorStream () << "DeviceTester5::" << __FUNCTION__ << ": the new device failed to open!" << std::endl;

      iRc = 2;
      goto DONE;
   }

   if (pDevice->hasError ())
   {
      DebugOutput::getErrorStream () << "DeviceTester5::" << __FUNCTION__ << ": the new device has an error!" << std::endl;

      iRc = 3;
      goto DONE;
   }

   if (pDevice)
   {
#if 0
      std::string *pstringJP = pDevice->getJobProperties ();

      if (pstringJP)
      {
         std::cout << "\"" << *pstringJP << "\"" << std::endl;

         delete pstringJP;
      }
#endif

#if 0
      PSZCRO pszDitherJP = "dither=DITHER_STUCKI_DIFFUSION";
      PSZRO  pszLongName = 0;

      pszLongName = DeviceDither::getName (pDevice, pszDitherJP);

      std::cout << "pszLongName = " << SAFE_PRINT_PSZ (pszLongName) << std::endl;
#endif

#if 1
      Enumeration *pEnum             = 0;
      std::string *pstringCreateHash = 0;

      pEnum = DeviceDither::getAllEnumeration ();

      while (pEnum->hasMoreElements ())
      {
         JobProperties *pJP         = (JobProperties *)pEnum->nextElement ();
         PSZRO          pszDitherJP = 0;
         PSZRO          pszLongName = 0;

         if (pJP)
         {
            pszDitherJP = pJP->getJobProperties ();
         }

         pszLongName       = DeviceDither::getName (pDevice, pszDitherJP);
         pstringCreateHash = DeviceDither::getCreateHash (pszDitherJP);

         std::cout << "pszLongName       = " << SAFE_PRINT_PSZ (pszLongName) << std::endl;
         std::cout << "pstringCreateHash = " << SAFE_PRINT_STRING (pstringCreateHash) << std::endl;

         delete pstringCreateHash;
         delete pJP;
      }

      delete pEnum;
#endif

      delete pDevice;
   }

DONE:
   // Clean up
   if (hmodDevice)
   {
      g_module_close (hmodDevice);
   }

   return iRc;
}

int
main (int argc, char *argv[])
{
   char *pszJobProperties  = 0;
   int   iFailed           = 0;

   if (!g_module_supported ())
   {
      std::cerr << "This program needs glib's module routines!" << std::endl;

      return 1;
   }

   if (2 > argc)
   {
      std::cerr << "Usage: " << argv[0] << " printer_name" << std::endl;

      return 1;
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
               PSZCRO pszLibName = pOD->getLibraryName ();

               std::cout << "Checking: " << pszLibName << std::endl;

               iFailed |= handleDevice (pszLibName, pOD->getJobProperties ());

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

            goto BUGOUT;
         }
      }
   }

BUGOUT:
   if (pszJobProperties)
   {
      free (pszJobProperties);
   }

   Omni::terminate ();

   return iFailed;
}
