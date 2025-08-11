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
#include "DebugOutput.hpp"
#include "XMLInterface.hpp"

#include <cstdio>
#include <cstdlib>
#include <list>
#include <set>
#include <fstream>

#include <glob.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>

PSZCRO vapszLibraryPaths[] = {
   DEFAULT_LIBRARY_PATH,
   "",                 // give a chance for $LD_LIBRARY_PATH to work
   NULL
};
PSZRO vapszDataPaths[] = {
   "",                 // allow for a fully qualified filename to work
   DEFAULT_SHARE_PATH,
   NULL
};
PSZCRO vapszExecutablePaths[] = {
   DEFAULT_BIN_PATH,
   NULL
};
PSZ *vapszAllDataPaths = (PSZ *)vapszDataPaths;

void Omni::
initialize ()
{
#if 0
   char **apszEnviron = environ;

   while (*apszEnviron)
   {
      if (0 != strstr (*apszEnviron, "OMNI"))
      {
         std::cerr << *apszEnviron << std::endl;
      }

      apszEnviron++;
   }
#endif

   DebugOutput::applyDebugEnvironment ();

   PSZRO pszDeviceListPath = getenv ("OMNI_DEVICELIST_PATH");

   if (pszDeviceListPath)
   {
      typedef std::vector<std::string> vectorList;

      vectorList vectorDirectories;
      int        cbData            = 0;
      int        cbPointers        = 0;
      PBYTE      pbData            = 0;
      PSZ        pszPathList       = 0;

      pszPathList = (PSZ)malloc (strlen (pszDeviceListPath) + 1);

      if (pszPathList)
      {
         PSZ pszPath = pszPathList;

         strcpy (pszPathList, pszDeviceListPath);

         while (  pszPath
               && *pszPath
               )
         {
            PSZ pszColon = strchr (pszPath, ':');

            if (pszColon)
            {
               *pszColon = '\0';
            }

            vectorDirectories.push_back (std::string (pszPath));

            if (pszColon)
            {
               pszPath = pszColon + 1;
            }
            else
            {
               pszPath = 0;
            }
         }

         free ((void *)pszPathList);
      }
      for (int i = 0; vapszDataPaths[i]; i++)
      {
         if (vapszDataPaths[i])
         {
            vectorDirectories.push_back (std::string (vapszDataPaths[i]));
         }
      }

      for ( vectorList::iterator next = vectorDirectories.begin ();
            next != vectorDirectories.end ();
            next++
          )
      {
         cbPointers += sizeof (PSZ);
         cbData     += sizeof (PSZ)
                     + next->length ()
                     + 1;
      }

      cbPointers += sizeof (PSZ);
      cbData     += sizeof (PSZ);

      pbData = (PBYTE)malloc (cbData);
      if (pbData)
      {
         PSZ *ppszPointers = (PSZ *)pbData;
         PSZ  pszStrings   = (PSZ)(pbData + cbPointers);

         for ( vectorList::iterator next = vectorDirectories.begin ();
               next != vectorDirectories.end ();
               next++
             )
         {
            strcpy (pszStrings, next->c_str ());

            *ppszPointers++  = pszStrings;
            pszStrings      += strlen (pszStrings) + 1;
         }

         *ppszPointers = 0;

         vapszAllDataPaths = (PSZ *)pbData;
      }
   }

   openlog ("Omni", 0, 0);
}

void Omni::
terminate ()
{
   if (vapszAllDataPaths != vapszDataPaths)
   {
      free ((void *)vapszAllDataPaths);
      vapszAllDataPaths = (PSZ *)vapszDataPaths;
   }

   closelog ();
}

int Omni::
my_system (PSZCRO pszCommand)
{
   int pid, status;

   if (pszCommand == 0)
      return 1;

   pid = fork();
   if (pid == -1)
      return -1;

   if (pid > 0)
   {
      // parent
      do
      {
         if (waitpid (pid, &status, 0) == -1)
         {
            if (errno != EINTR)
               return -1;
         }
         else
         {
            return status;
         }

      } while (1);
   }
   else
   {
      // child
      char *argv[4];

      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = (char *)pszCommand;
      argv[3] = 0;

      // Transfer control to sh
      execvp ("/bin/sh", argv);

      // Should not execute past here
      exit (1);
   }
}

void Omni::
setOutputStream (Device *pDevice,
                 FILE   *pfpNew)
{
   if (  pDevice
      && pfpNew
      )
      pDevice->setOutputStream (pfpNew);
}

void Omni::
setErrorStream (Device *pDevice,
                FILE   *pfpNew)
{
   if (!pfpNew)
   {
      return;
   }

   DebugOutput::setErrorStream (pfpNew);

   if (pDevice)
   {
      pDevice->setErrorStream (pfpNew);
   }
}

bool Omni::
libraryValid (GModule *hDevice,
              PSZCRO   pszLibraryName,
              PSZCRO   pszVersion,
              bool     fVerbose)
{
   bool     rc  = false;
   gpointer ptr = NULL;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": libraryValid (" << pszLibraryName << ", " << std::hex << hDevice << std::dec << ", " << pszVersion << ")" << std::endl;
#endif

   // The library must have the following symbols in it

   if (  g_module_symbol (hDevice, "getVersion", &ptr)
      && g_module_symbol (hDevice, "getDeviceEnumeration", &ptr)
      && g_module_symbol (hDevice, "newDeviceW_Advanced", &ptr)
      && g_module_symbol (hDevice, "newDeviceW_JopProp_Advanced", &ptr)
      && g_module_symbol (hDevice, "deleteDevice", &ptr)
      )
   {
      PFNGETVERSION pfnGetVersion = 0;

      // Should work.  Already tested above.
      g_module_symbol (hDevice, "getVersion", (gpointer *)&pfnGetVersion);

      PSZCRO pszLibraryVersion = pfnGetVersion ();

      if (  pszLibraryVersion
         && 0 == strcmp (VERSION, pszLibraryVersion)
         )
      {
         rc = true;
      }
#ifndef RETAIL
      else if (fVerbose)
      {
         if (!pszLibraryVersion)
            DebugOutput::getErrorStream () << "for " << pszLibraryName << " getVersion returns null!" << std::endl;
         if (0 != strcmp (VERSION, pszLibraryVersion))
            DebugOutput::getErrorStream () << "for " << pszLibraryName << " " << VERSION << " != " << pszLibraryVersion << std::endl;
      }
#endif
   }
#ifndef RETAIL
   else if (  fVerbose
           && DebugOutput::shouldOutputOmni ()
           )
   {
      g_module_symbol (hDevice, "getDeviceEnumeration", &ptr);
      if (!ptr)
         DebugOutput::getErrorStream () << "for " << pszLibraryName << " getDeviceEnumeration is missing." << std::endl;
      g_module_symbol (hDevice, "newDeviceW_Advanced", &ptr);
      if (!ptr)
         DebugOutput::getErrorStream () << "for " << pszLibraryName << " newDeviceW_Advanced is missing." << std::endl;
      g_module_symbol (hDevice, "newDeviceW_JopProp_Advanced", &ptr);
      if (!ptr)
         DebugOutput::getErrorStream () << "for " << pszLibraryName << " newDeviceW_JopProp_Advanced is missing." << std::endl;
      g_module_symbol (hDevice, "deleteDevice", &ptr);
      if (!ptr)
         DebugOutput::getErrorStream () << "for " << pszLibraryName << " deleteDevice is missing." << std::endl;
      g_module_symbol (hDevice, "getVersion", &ptr);
      if (!ptr)
         DebugOutput::getErrorStream () << "for " << pszLibraryName << " getVersion is missing." << std::endl;
   }

   if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": returning " << rc << std::endl;
#endif

   return rc;
}

static bool
createLibName (char   *pszLibName,
               int     cbLibName,
               PSZCRO  pszDriver,
               PSZRO   pszDevice)
{
   int cbDriver = strlen (pszDriver);

   // Is there enough space for everything?
   if ( cbLibName < (int)( 3                   // "lib"
                         + cbDriver            // driver
                         + 1                   // "_"
                         + strlen (pszDevice)  // device
                         + 3                   // ".so"
                         )
      )
      return false;

   strcpy (pszLibName, "lib");
   strcat (pszLibName, pszDriver);
   strcat (pszLibName, "_");

   char *pszStart = pszLibName + strlen (pszLibName);

   // For example, the driver "Epson" with the device "Epson LQ-2550" is
   // called libEpson_LQ_2550.so.  The "Epson" in the device is redundant.
   if (  0 == strncasecmp (pszDevice,
                           pszDriver,
                           cbDriver)
      && ' ' == pszDevice[cbDriver]
      )
   {
      pszDevice += cbDriver + 1;
   }

   strcat (pszLibName, pszDevice);

   while (*pszStart)
   {
      switch (*pszStart)
      {
      case '-':
      case '+':
      case '(':
      case ')':
         *pszStart = '_';
         break;
      }

      pszStart++;
   }

   strcat (pszLibName, ".so");

   return true;
}

bool Omni::
openLibrary (PSZCRO    pszLibName,
             GModule **phLibrary)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": g_module_open (" << pszLibName << ")" << std::endl;
#endif

   *phLibrary = 0;

   if (!pszLibName)
   {
      return false;
   }

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "This program needs glib's module routines!" << std::endl;

      return false;
   }

   char *pszDeviceLib = 0;

   for (int i = 0; vapszLibraryPaths[i] && !*phLibrary; i++)
   {
      pszDeviceLib = (char *)malloc ( strlen (pszLibName)
                                    + strlen (vapszLibraryPaths[i])
                                    + 1);
      if (pszDeviceLib)
      {
         sprintf (pszDeviceLib, "%s%s", vapszLibraryPaths[i], pszLibName);

         *phLibrary = g_module_open (pszDeviceLib, (GModuleFlags)0);
      }
      else
      {
         return false;
      }

      free (pszDeviceLib);
   }

   if (!*phLibrary)
   {
      DebugOutput::logMessage (LOG_ERR, "ERROR: Failed to load \"%s\", reason \"%s\"", pszLibName, g_module_error ());

      DebugOutput::getErrorStream () << std::endl << "<<<<<<<<<<<<<<<<<<<<<< ERROR >>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
      DebugOutput::getErrorStream () << std::endl << std::endl;
      DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": for \"" << pszLibName << "\", g_module_error returns \"" << g_module_error () << "\"" << std::endl;
      DebugOutput::getErrorStream () << std::endl;
      DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Omni device library not found in the following paths:" << std::endl;
      for (int i = 0; vapszLibraryPaths[i]; i++)
      {
         DebugOutput::getErrorStream () << "\t" << vapszLibraryPaths[i] << "." << std::endl;
      }
      DebugOutput::getErrorStream () << "\t$LD_LIBRARY_PATH (";
      char *pszPath = getenv ("LD_LIBRARY_PATH");
      if (pszPath)
      {
         DebugOutput::getErrorStream () << pszPath;
      }
      else
      {
         DebugOutput::getErrorStream () << "NULL";
      }
      DebugOutput::getErrorStream () << ")" << std::endl;

      return 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": phLibrary = " << std::hex << (int)phLibrary << std::dec << std::endl;
#endif

   if (!*phLibrary)
      return false;

   return true;
}

bool Omni::
openAndTestDeviceLibrary (PSZCRO    pszLibName,
                          GModule **phmodDevice)
{
   if (!openLibrary (pszLibName, phmodDevice))
      return false;

   return libraryValid (*phmodDevice, pszLibName, VERSION);
}

Device * Omni::
createDevice (PSZCRO    pszDriver,
              PSZCRO    pszDevice,
              PSZCRO    pszJobProperties,
              bool      fAdvanced,
              GModule **phmodDevice)
{
   char               achLibDeviceName[256]; // @TBD - need max path size
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs     = 0;
   Device            *pDevice               = 0;

   *phmodDevice = 0;

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "This program needs glib's module routines!" << std::endl;
      return 0;
   }

   if (createLibName (achLibDeviceName,
                      sizeof (achLibDeviceName),
                      pszDriver,
                      pszDevice))
   {
      if (openAndTestDeviceLibrary (achLibDeviceName, phmodDevice))
      {
         // Should work.  openAndTestDeviceLibrary tests for existance of all required entry points
         g_module_symbol (*phmodDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

         pDevice = pfnNewDeviceWArgs (pszJobProperties,
                                      fAdvanced);

         if (pDevice)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ())
            {
               DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": strcasecmp (" << pszDriver << ", " << pDevice->getDriverName () << ") = " << std::dec << strcasecmp (pszDriver, pDevice->getDriverName ()) << std::endl;
               DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": strcasecmp (" << pszDevice << ", " << pDevice->getDeviceName () << ") = " << strcasecmp (pszDevice, pDevice->getDeviceName ()) << std::endl;
            }
#endif

            if (  0 != strcasecmp (pszDriver, pDevice->getDriverName ())
               && 0 != strcasecmp (pszDevice, pDevice->getDeviceName ())
               )
            {
               // The driver and device name are not the same!
               delete pDevice;
               pDevice = 0;
            }
         }
      }

      if (  !pDevice
         && phmodDevice
         )
      {
#ifndef RETAIL
         int rc =
#endif
                  g_module_close (*phmodDevice);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": createDevice @ " << std::dec << __LINE__ << " : g_module_close (" << std::hex << *phmodDevice << ") = " << rc << std::endl;
#endif

         *phmodDevice = 0;
      }
   }

   if (!pDevice)
   {
      // Do it the hard way
      Enumeration *pEnum  = Omni::listDevices ();
      bool         fFound = false;

      while (  !fFound
            && pEnum->hasMoreElements ()
            )
      {
         OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

         if (pOD)
         {
            PSZCRO pszLibName = pOD->getLibraryName ();

#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": createDevice g_module_open (" << pszLibName << ")" << std::endl;
#endif

            *phmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

            if (  *phmodDevice
               && libraryValid (*phmodDevice, pszLibName, VERSION)
               )
            {
               // Should work.  libraryValid tests for existance of all required entry points
               g_module_symbol (*phmodDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

               pDevice = pfnNewDeviceWArgs (pszJobProperties, fAdvanced);

#ifndef RETAIL
               if (DebugOutput::shouldOutputOmni ())
               {
                  DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": strcasecmp (" << pszDriver << ", " << pDevice->getDriverName () << ") = " << std::dec << strcasecmp (pszDriver, pDevice->getDriverName ()) << std::endl;
                  DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": strcasecmp (" << pszDevice << ", " << pDevice->getDeviceName () << ") = " << strcasecmp (pszDevice, pDevice->getDeviceName ()) << std::endl;
               }
#endif

               if (  0 == strcasecmp (pszDriver, pDevice->getDriverName ())
                  && 0 == strcasecmp (pszDevice, pDevice->getDeviceName ())
                  )
                  fFound = true;
            }

            delete pOD;
         }

         if (!fFound)
         {
            delete pDevice;
            pDevice = 0;

#ifndef RETAIL
            int rc =
#endif
                     g_module_close (*phmodDevice);

#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": createDevice @ " << std::dec <<  __LINE__ << " : g_module_close (" << std::hex << *phmodDevice << ") = " << rc << std::endl;
#endif

            *phmodDevice = 0;
         }
      }

      delete pEnum;
   }

   return pDevice;
}

Device * Omni::
createDevice (PDL      *pPDL,
              PSZCRO    pszJobProperties,
              bool      fAdvanced,
              GModule **phmodDevice)
{
   // @TBD
   return 0;
}

Device * Omni::
createDevice (OmniDevice  *pOD,
              GModule    **phmodDevice)
{
   PSZCRO             pszLibName        = pOD->getLibraryName ();
   PSZCRO             pszJobProperties  = pOD->getJobProperties ();
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;
   Device            *pDevice           = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": Enumerated: \"" << pszLibName << "\", \"" << pszJobProperties << "\"" << std::endl;
#endif

   *phmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

   if (*phmodDevice)
   {
      g_module_symbol (*phmodDevice,
                       "newDeviceW_JopProp_Advanced",
                       (gpointer *)&pfnNewDeviceWArgs);

      if (pfnNewDeviceWArgs)
      {
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

      }
   }

   return pDevice;
}

DeviceInfo * Omni::
findDeviceInfoFromShortName (PSZCRO pszShortName,
                             bool   fBuildOnly)
{
   typedef std::list <DeviceInfo *> DeviceInfoList;

   Enumeration    *pEnum               = Omni::listDevices (fBuildOnly);
   DeviceInfoList  listMatchingDevices;

   std::cerr << "Info: Trying to figure out which device library to use..." << std::endl;

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         GModule *hmodDevice = 0;
         Device  *pDevice = 0;

         pDevice = createDevice (pOD, &hmodDevice);

         if (pDevice)
         {
            if (0 == strcmp (pszShortName, pDevice->getShortName ()))
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": Found a match!" << std::endl;
#endif

               listMatchingDevices.push_back (new DeviceInfo (pDevice,
                                                              hmodDevice,
                                                              pOD));
            }
            else
            {
               delete pDevice;
               pDevice = 0;

               g_module_close (hmodDevice);
               hmodDevice = 0;

               delete pOD;
               pOD = 0;
            }
         }
      }
   }

   if (0 == listMatchingDevices.size ())
   {
      std::cerr << "Error: Could not find a device whose short name matches " << pszShortName << std::endl;
   }
   else if (1 == listMatchingDevices.size ())
   {
      return listMatchingDevices.front ();
   }
   else
   {
      std::cerr << "Error: There are multiple devices whose short names match " << pszShortName << std::endl;

      for ( DeviceInfoList::iterator next = listMatchingDevices.begin () ;
            next != listMatchingDevices.end () ;
            next++ )
      {
         DeviceInfo  *pDI        = *next;
         Device      *pDevice    = 0;
         GModule     *hmodDevice = 0;
         OmniDevice  *pOD        = 0;
         PSZRO        pszJobProp = 0;

         if (pDI)
         {
            pDevice    = pDI->getDevice ();
            hmodDevice = pDI->getHmodDevice ();
            pOD        = pDI->getOmniDevice ();

            if (pOD)
            {
               pszJobProp = pOD->getJobProperties ();
            }

            std::cerr << "Error:\t";

            if (pszJobProp)
            {
               std::cerr << "'-sproperties="
                         << pszJobProp
                         << "'";
            }

            std::cerr << " --driver"
                      << pDevice->getLibraryName ()
                      << std::endl;

            delete pDI;
         }
      }
   }

   delete pEnum;

   return 0;
}

class BuildDeviceEnumerator : public Enumeration
{
public:
   BuildDeviceEnumerator (bool fVerbose)
   {
      // Initialize our variables
      fVerbose_d           = fVerbose;
      pszLD_LIBRARY_PATH_d = getenv ("LD_LIBRARY_PATH");
      fGlobActive_d        = false;
      hLibrary_d           = 0;
      pDeviceEnum_d        = 0;

#ifndef RETAIL
      if (!pszLD_LIBRARY_PATH_d)
      {
         DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": LD_LIBRARY_PATH is empty!" << std::endl;
      }
#endif
   }

   virtual
   ~BuildDeviceEnumerator ()
   {
      // Clean up
      if (fGlobActive_d)
      {
         globfree (&globbuf_d);
         fGlobActive_d = false;
      }

      delete pDeviceEnum_d;
      pDeviceEnum_d = 0;
      if (hLibrary_d)
      {
         g_module_close (hLibrary_d);
         hLibrary_d = 0;
      }
   }

   virtual bool
   hasMoreElements ()
   {
      // Prime the pump.  That is, find at least one element.

      // Do we have an active device enumeration?
      if (pDeviceEnum_d)
      {
         if (pDeviceEnum_d->hasMoreElements ())
         {
            return true;
         }

         delete pDeviceEnum_d;
         pDeviceEnum_d = 0;
         if (hLibrary_d)
         {
            g_module_close (hLibrary_d);
            hLibrary_d = 0;
         }
      }

      // Are we in a glob?
      if (fGlobActive_d)
      {
         // Find another one
         findDeviceInGlob ();
      }

      if (  !fGlobActive_d        // Did we not find one?
         && pszLD_LIBRARY_PATH_d  // Still more paths to search
         )
      {
         // Try with another path
         pszLD_LIBRARY_PATH_d = globPath (pszLD_LIBRARY_PATH_d);
      }

      // Did we find one?
      if (fGlobActive_d)
         return true;
      else
         return false;
   }

   virtual void *
   nextElement ()
   {
      if (pDeviceEnum_d)
      {
         return pDeviceEnum_d->nextElement ();
      }
      else if (fGlobActive_d)
      {
         PSZCRO pszLibraryName = globbuf_d.gl_pathv[iCurrentFile_d];

         // See if this device has subdevices
         hLibrary_d = g_module_open (pszLibraryName, (GModuleFlags)0);

         if (hLibrary_d)
         {
            PFNENUMERATEDEVICES pfnEnumerateDevices = 0;

            g_module_symbol (hLibrary_d,
                             "getDeviceEnumeration",
                             (gpointer *)&pfnEnumerateDevices);

            if (pfnEnumerateDevices)
            {
               pDeviceEnum_d = pfnEnumerateDevices (pszLibraryName, true);
            }

            if (pDeviceEnum_d)
            {
               if (pDeviceEnum_d->hasMoreElements ())
               {
                  // Return the first subdevice
                  return pDeviceEnum_d->nextElement ();
               }
            }

            // Clean up
            delete pDeviceEnum_d;
            pDeviceEnum_d = 0;
            g_module_close (hLibrary_d);
            hLibrary_d = 0;
         }

         // Return the one that we found
         return new OmniDevice (pszLibraryName, 0);
      }
      else
      {
         // Error!
         return 0;
      }
   }

private:
   char *
   globPath (char *pszLD_LIBRARY_PATH)
   {
      char  achPath[256]; // @TBD - need max path size
      char *pszSep       = 0;
      bool  fFound       = false;
      int   rc;

      do
      {
         // See if there are two or more paths
         pszSep = strpbrk (pszLD_LIBRARY_PATH, ":");

         if (pszSep)
         {
            int iChars = pszSep - pszLD_LIBRARY_PATH;

            // Just copy the first one
            strncpy (achPath, pszLD_LIBRARY_PATH, iChars);
            achPath[iChars] = '\0';

            pszLD_LIBRARY_PATH = pszSep + 1;
         }
         else
         {
            // Copy the only one
            strcpy (achPath, pszLD_LIBRARY_PATH);

            pszLD_LIBRARY_PATH = 0;
         }

         // Add our search parameter
         strcat (achPath, "/lib*.so");

         if (fVerbose_d)
         {
            DebugOutput::getErrorStream () << "Searching: glob (" << achPath << ") = ";
         }

         // Call glob
         memset (&globbuf_d, 0, sizeof (globbuf_d));
         rc = glob (achPath, 0, NULL, &globbuf_d);

         if (fVerbose_d)
         {
            DebugOutput::getErrorStream () << rc << std::endl;
         }

         if (0 == rc)
         {
            // Call succeded
            iCurrentFile_d = -1;
            fGlobActive_d  = true;

            // See if there is a device in this directory
            fFound = findDeviceInGlob ();
         }

      } while (  !fFound            // Haven't found any yet
              && pszLD_LIBRARY_PATH // More to search
              );

      return pszLD_LIBRARY_PATH;
   }

   bool
   findDeviceInGlob (void)
   {
      bool fFound = false;

      for ( iCurrentFile_d++;
            iCurrentFile_d < (int)globbuf_d.gl_pathc;
            iCurrentFile_d++)
      {
         PSZCRO pszCurrentName = globbuf_d.gl_pathv[iCurrentFile_d];

         std::string stringSystem;

/////////stringSystem  = "strace OmniDeviceValid ";
         stringSystem  = "OmniDeviceValid ";
         if (fVerbose_d)
            stringSystem += "-v ";
         stringSystem += "\"";
         stringSystem += pszCurrentName;
         stringSystem += "\" ";
         stringSystem += VERSION;

         if (0 == Omni::my_system (stringSystem.c_str ()))
         {
            fFound = true;
         }

         if (fVerbose_d)
         {
            DebugOutput::getErrorStream () << "Checking ("
                                           << pszCurrentName
                                           << ") "
                                           << (fFound ? '+' : '-')
                                           << std::endl;
            DebugOutput::getErrorStream () << "Running: " << stringSystem << std::endl;
         }

         if (fFound)
         {
            break;
         }
      }

      if (!fFound)
      {
         globfree (&globbuf_d);
         fGlobActive_d = false;
      }

      return fFound;
   }

private:
   bool         fVerbose_d;
   char        *pszLD_LIBRARY_PATH_d;
   glob_t       globbuf_d;
   int          iCurrentFile_d;
   bool         fGlobActive_d;

   GModule     *hLibrary_d;
   Enumeration *pDeviceEnum_d;
};

class SystemDeviceEnumerator : public Enumeration
{
public:
   SystemDeviceEnumerator ()
   {
      // Initialize our variables

      // Add the Omni builtin paths
      for (int i = 0; vapszLibraryPaths[i]; i++)
      {
         std::string stringPath = vapszLibraryPaths[i];

         if (*vapszLibraryPaths[i])
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::SystemDeviceEnumerator::" << __FUNCTION__ << ": inserting (" << stringPath << ")" << std::endl;
#endif

            setDirectories_d.insert (stringPath);
         }
      }

      // Add /etc/ld.so.conf
      char           achLine[512];             // @TBD
      std::ifstream  ifIn ("/etc/ld.so.conf");

      while (ifIn.getline (achLine, sizeof (achLine)))
      {
         char *pszEnd = achLine + strlen (achLine) - 1;

         if ('/' != *pszEnd)
         {
            strcat (achLine, "/");
         }

         std::string stringLine = achLine;

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::SystemDeviceEnumerator::" << __FUNCTION__ << ": inserting (" << achLine << ")" << std::endl;
#endif

         setDirectories_d.insert (stringLine);
      }

      iterCurrent_d = setDirectories_d.begin ();

      fGlobActive_d = false;
      hLibrary_d    = 0;
      pDeviceEnum_d = 0;
   }

   virtual
   ~SystemDeviceEnumerator ()
   {
      // Clean up
      if (fGlobActive_d)
      {
         globfree (&globbuf_d);
         fGlobActive_d = false;
      }

      delete pDeviceEnum_d;
      pDeviceEnum_d = 0;
      if (hLibrary_d)
      {
         g_module_close (hLibrary_d);
         hLibrary_d = 0;
      }
   }

   virtual bool
   hasMoreElements ()
   {
      // Prime the pump.  That is, find at least one element.

      // Do we have an active device enumeration?
      if (pDeviceEnum_d)
      {
         if (pDeviceEnum_d->hasMoreElements ())
         {
            return true;
         }

         delete pDeviceEnum_d;
         pDeviceEnum_d = 0;
         if (hLibrary_d)
         {
            g_module_close (hLibrary_d);
            hLibrary_d = 0;
         }
      }

      // Are we in a glob?
      if (fGlobActive_d)
      {
         // Find another one
         findDeviceInGlob ();
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::SystemDeviceEnumerator::" << __FUNCTION__ << ": fGlobActive_d = " << fGlobActive_d << ", " << (iterCurrent_d != setDirectories_d.end ()) << std::endl;
#endif

      if (  !fGlobActive_d                           // Did we not find one?
         && iterCurrent_d != setDirectories_d.end () // Still more directories to search
         )
      {
         // Try with another path
         globPath ();
      }

      // Did we find one?
      if (fGlobActive_d)
         return true;
      else
         return false;
   }

   virtual void *
   nextElement ()
   {
      if (pDeviceEnum_d)
      {
         return pDeviceEnum_d->nextElement ();
      }
      else if (fGlobActive_d)
      {
         PSZCRO pszLibraryName = globbuf_d.gl_pathv[iCurrentFile_d];

         if (Omni::openAndTestDeviceLibrary (pszLibraryName, &hLibrary_d))
         {
            // See if this device has subdevices
            PFNENUMERATEDEVICES pfnEnumerateDevices = 0;

            g_module_symbol (hLibrary_d,
                             "getDeviceEnumeration",
                             (gpointer *)&pfnEnumerateDevices);

            if (pfnEnumerateDevices)
            {
               pDeviceEnum_d = pfnEnumerateDevices (pszLibraryName, false);
            }

            if (pDeviceEnum_d)
            {
               if (pDeviceEnum_d->hasMoreElements ())
               {
                  // Return the first subdevice
                  return pDeviceEnum_d->nextElement ();
               }
            }
         }

         // Clean up
         delete pDeviceEnum_d;
         pDeviceEnum_d = 0;
         if (hLibrary_d)
         {
            g_module_close (hLibrary_d);
            hLibrary_d = 0;
         }

         // Error!
         return 0;
      }
      else
      {
         // Error!
         return 0;
      }
   }

private:
   void
   globPath ()
   {
      bool fFound = false;
      int  rc     = 0;

      while (  !fFound                                  // Haven't found any yet
            && iterCurrent_d != setDirectories_d.end () // Haven't reached the end
            )
      {
         std::string stringGlob = *iterCurrent_d++;

         stringGlob += "lib*.so";

         // Call glob
         memset (&globbuf_d, 0, sizeof (globbuf_d));
         rc = glob (stringGlob.c_str (), 0, NULL, &globbuf_d);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::SystemDeviceEnumerator::" << __FUNCTION__ << " (" << stringGlob << ") = " << rc << std::endl;
#endif

         if (0 == rc)
         {
            // Call succeded
            iCurrentFile_d = -1;
            fGlobActive_d  = true;

            // See if there is a device in this directory
            fFound = findDeviceInGlob ();
         }
      }
   }

   bool
   findDeviceInGlob (void)
   {
      bool fFound = false;

      for ( iCurrentFile_d++;
            iCurrentFile_d < (int)globbuf_d.gl_pathc;
            iCurrentFile_d++)
      {
         PSZCRO pszCurrentName = globbuf_d.gl_pathv[iCurrentFile_d];

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::SystemDeviceEnumerator::" << __FUNCTION__ << ": findDeviceInGlob: checking (" << pszCurrentName << ")" << std::endl;
#endif

         std::string stringSystem;

         stringSystem  = "OmniDeviceValid ";
/////////stringSystem += "-v ";
         stringSystem += "\"";
         stringSystem += pszCurrentName;
         stringSystem += "\" ";
         stringSystem += VERSION;

         if (0 == Omni::my_system (stringSystem.c_str ()))
         {
            fFound = true;
            break;
         }
      }

      if (!fFound)
      {
         globfree (&globbuf_d);
         fGlobActive_d = false;
      }

      return fFound;
   }

private:
   std::set<std::string>            setDirectories_d;
   std::set<std::string>::iterator  iterCurrent_d;
   glob_t                           globbuf_d;
   int                              iCurrentFile_d;
   bool                             fGlobActive_d;

   GModule                         *hLibrary_d;
   Enumeration                     *pDeviceEnum_d;
};

Enumeration * Omni::
listDevices (bool fBuildOnly)
{
   if (fBuildOnly)
   {
#ifndef RETAIL
      return new BuildDeviceEnumerator (true);
#else
      return new BuildDeviceEnumerator (false);
#endif
   }
   else
   {
      return new SystemDeviceEnumerator ();
   }
}

OmniDevice * Omni::
findOmniDeviceEntry (PSZCRO pszShortName)
{
   PSZCRO      pszXMLFile  = openXMLFile ("devices.xml");
   XmlDocPtr   doc         = 0;
   OmniDevice *pODReturn   = 0;
   XmlNodePtr  devicesNode = 0;
   XmlNodePtr  deviceNode  = 0;

   if (!pszXMLFile)
      goto done;

   doc = XMLParseFile (pszXMLFile);
   if (!doc)
      goto done;

   devicesNode = XMLDocGetRootElement (doc);
   if (!devicesNode)
      goto done;
   if (0 != XMLStrcmp (XMLGetName (devicesNode), "devices"))
      goto done;

   deviceNode = XMLFirstNode (XMLGetChildrenNode (devicesNode));

   while (  deviceNode
         && !pODReturn
         )
   {
      if (0 == XMLStrcmp (XMLGetName (deviceNode), "device"))
      {
         XmlNodePtr elm = XMLFirstNode (XMLGetChildrenNode (deviceNode));

         while (elm)
         {
            if (0 == XMLStrcmp (XMLGetName (elm), "shortname"))
            {
               PSZRO pszNodeShortName = 0;

               pszNodeShortName = XMLNodeListGetString (XMLGetDocNode (elm),
                                                        XMLGetChildrenNode (elm),
                                                        1);

               if (pszNodeShortName)
               {
                  if (0 == XMLStrcmp (pszNodeShortName,
                                      pszShortName))
                  {
                     PSZRO pszLibraryName   = 0;
                     PSZRO pszJobProperties = 0;

                     elm = XMLFirstNode (XMLGetChildrenNode (deviceNode));

                     while (elm)
                     {
                        if (0 == XMLStrcmp (XMLGetName (elm), "libraryname"))
                        {
                           pszLibraryName = XMLNodeListGetString (XMLGetDocNode (elm),
                                                                  XMLGetChildrenNode (elm),
                                                                  1);
                        }
                        else if (0 == XMLStrcmp (XMLGetName (elm), "jobproperties"))
                        {
                           pszJobProperties = XMLNodeListGetString (XMLGetDocNode (elm),
                                                                    XMLGetChildrenNode (elm),
                                                                    1);
                        }

                        elm = XMLNextNode (elm);
                     }

                     if (  pszLibraryName
                        && pszJobProperties
                        )
                     {
                        pODReturn = new OmniDevice ((PSZRO)pszLibraryName,
                                                    (PSZRO)pszJobProperties);
                     }

                     if (pszLibraryName)
                     {
                        XMLFree ((void *)pszLibraryName);
                     }
                     if (pszJobProperties)
                     {
                        XMLFree ((void *)pszJobProperties);
                     }
                  }

                  XMLFree ((void *)pszNodeShortName);
                  break;
               }
            }

            elm = XMLNextNode (elm);
         }
      }

      // Move to the next one
      deviceNode = XMLNextNode (deviceNode);
   }

done:
   // Clean up
   if (doc)
   {
      XMLFreeDoc (doc);
   }
   XMLCleanup ();
   if (pszXMLFile)
   {
      free ((void *)pszXMLFile);
   }

   return pODReturn;
}

PSZCRO Omni::
openXMLFile (PSZCRO pszXMLFile)
{
   if (  !pszXMLFile
      || !*pszXMLFile
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": pszXMLFile = NULL" << std::endl;
#endif

      return 0;
   }

   char *pszTargetXMLFile = 0;

   for (int i = 0; vapszAllDataPaths[i]; i++)
   {
      int  cbAlloc      = 0;
      bool fSlashNeeded = false;

      if (!vapszAllDataPaths[i])
      {
         continue;
      }

      cbAlloc = strlen (pszXMLFile)
              + strlen (vapszAllDataPaths[i])
              + 1;

      if ('/' != vapszAllDataPaths[i][strlen (vapszAllDataPaths[i]) - 1])
      {
         fSlashNeeded = true;
         cbAlloc++;
      }

      pszTargetXMLFile = (char *)malloc (cbAlloc);
      if (pszTargetXMLFile)
      {
         struct stat statFile;

         strcpy (pszTargetXMLFile, vapszAllDataPaths[i]);
         if (fSlashNeeded)
         {
            strcat (pszTargetXMLFile, "/");
         }
         strcat (pszTargetXMLFile, pszXMLFile);

         if (0 == stat (pszTargetXMLFile, &statFile))
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Mapping \"" << pszXMLFile << "\" to \"" << pszTargetXMLFile << "\"." << std::endl;
#endif

            return pszTargetXMLFile;
         }

         free (pszTargetXMLFile);
         pszTargetXMLFile = 0;
      }
      else
      {
         DebugOutput::getErrorStream () << std::endl << "<<<<<<<<<<<<<<<<<<<<<< ERROR >>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
         DebugOutput::getErrorStream () << std::endl << std::endl;
         DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Error: Out of memory @ " << __LINE__ << std::endl;

         return 0;
      }
   }

   DebugOutput::getErrorStream () << std::endl << "<<<<<<<<<<<<<<<<<<<<<< ERROR >>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
   DebugOutput::getErrorStream () << std::endl << std::endl;
   DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Omni XML file (" << pszXMLFile << ") not found in the following paths:" << std::endl;
   for (int i = 0; vapszAllDataPaths[i]; i++)
   {
      if (*vapszAllDataPaths[i])
      {
         DebugOutput::getErrorStream () << "\t" << vapszAllDataPaths[i] << std::endl;
      }
      else
      {
         DebugOutput::getErrorStream () << "\t." << std::endl;
      }
   }
   DebugOutput::getErrorStream () << std::endl;

   return 0;
}

static bool
checkForXMLMasterSignature (PSZCRO pszFileName)
{
   FILE *fpFile = 0;
   bool  fFound = false;

   fpFile = fopen (pszFileName, "r");

   if (fpFile)
   {
//#define USE_DTD 1
#ifdef USE_DTD
                                    //           1         2          3         4
                                    // 0123456789012345678901234 567890123456789012 34
      static PSZCRO  pszTest        = "<!DOCTYPE Device SYSTEM \"../OmniDevice.dtd\">";
#else
                                    //           1
                                    // 012345678901234
      static PSZCRO  pszTest        = "<Device xmlns=\"";
#endif
      static int     cbTest         = strlen (pszTest);
      char           achBuffer[512];
      char          *pszBuffer      = achBuffer;
      size_t         cbBuffer       = sizeof (achBuffer);
      bool           fContinue      = true;

      do
      {
         // Read a block
         if (fread (pszBuffer, cbBuffer, 1, fpFile))
         {
            // Start at the end
            char *pszSearch = achBuffer + sizeof (achBuffer) - cbTest;

            // While within the buffer
            while (pszSearch >= achBuffer)
            {
               // Is is found
               if (  *pszSearch == *pszTest
                  && 0 == strncmp (pszSearch, pszTest, cbTest)
                  )
               {
                  fFound = true;

                  break;
               }
               else
               {
                  // Move backwards based on current character
                  switch (*pszSearch)
                  {
#ifdef USE_DTD
                  case '!': pszSearch -= 1;      break;
                  case 'D': pszSearch -= 2;      break;
                  case 'O': pszSearch -= 3;      break;
                  case 'C': pszSearch -= 4;      break;
                  case 'T': pszSearch -= 5;      break;
                  case 'Y': pszSearch -= 6;      break;
                  case 'P': pszSearch -= 7;      break;
                  case 'E': pszSearch -= 8;      break;
                  case ' ': pszSearch -= 9;      break;
                  case 'e': pszSearch -= 11;     break;
                  case 'v': pszSearch -= 12;     break;
                  case 'i': pszSearch -= 13;     break;
                  case 'c': pszSearch -= 14;     break;
                  case 'S': pszSearch -= 17;     break;
                  case 'M': pszSearch -= 22;     break;
                  case '"': pszSearch -= 24;     break;
                  case '/': pszSearch -= 27;     break;
                  case 'm': pszSearch -= 29;     break;
                  case 'n': pszSearch -= 30;     break;
                  case '.': pszSearch -= 38;     break;
                  case 'd': pszSearch -= 39;     break;
                  case 't': pszSearch -= 40;     break;
                  case '>': pszSearch -= 44;     break;
                  default:  pszSearch -= cbTest; break;
#else
                  case 'D': pszSearch -= 1;      break;
                  case 'e': pszSearch -= 2;      break;
                  case 'v': pszSearch -= 3;      break;
                  case 'i': pszSearch -= 4;      break;
                  case 'c': pszSearch -= 5;      break;
                  case ' ': pszSearch -= 7;      break;
                  case 'x': pszSearch -= 8;      break;
                  case 'm': pszSearch -= 9;      break;
                  case 'l': pszSearch -= 10;     break;
                  case 'n': pszSearch -= 11;     break;
                  case 's': pszSearch -= 12;     break;
                  case '=': pszSearch -= 13;     break;
                  case '"': pszSearch -= 14;     break;
                  default:  pszSearch -= cbTest; break;
#endif
                  }
               }
            }

            if (!fFound)
            {
               /* In order to handle the case where the search string might
               ** be spanning buffers, move the end of the buffer to the
               ** front and shrink the buffer
               */
               memmove (achBuffer,
                        achBuffer + sizeof (achBuffer) - cbTest,
                        cbTest);

               pszBuffer = achBuffer + cbTest;
               cbBuffer  = sizeof (achBuffer) - cbTest;
            }
         }
         else
         {
            // File read error!
            fContinue = false;
         }

      } while (  fContinue
              && !fFound
              );

      // Clean up
      fclose (fpFile);
   }

   return fFound;
}

class PathXMLEnumerator : public Enumeration
{
public:
   PathXMLEnumerator ()
   {
      // Initialize our variables
      iterCurrent_d = setDirectories_d.begin ();

      fGlobActive_d = false;
   }

   virtual
   ~PathXMLEnumerator ()
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::PathXMLEnumerator::~" << __FUNCTION__ << std::endl;
#endif

      // Clean up
      if (fGlobActive_d)
      {
         globfree (&globbuf_d);
         fGlobActive_d = false;
      }
   }

   virtual bool
   hasMoreElements ()
   {
      // Prime the pump.  That is, find at least one element.

      // Are we in a glob?
      if (fGlobActive_d)
      {
         // Find another one
         findDeviceInGlob ();
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::PathXMLEnumerator::" << __FUNCTION__ << ": fGlobActive_d = " << fGlobActive_d << ", " << (iterCurrent_d != setDirectories_d.end ()) << std::endl;
#endif

      if (  !fGlobActive_d                           // Did we not find one?
         && iterCurrent_d != setDirectories_d.end () // Still more directories to search
         )
      {
         // Try with another path
         globPath ();
      }

      // Did we find one?
      if (fGlobActive_d)
         return true;
      else
         return false;
   }

   virtual void *
   nextElement ()
   {
      if (fGlobActive_d)
         // Return the one that we found
         return globbuf_d.gl_pathv[iCurrentFile_d];
      else
         // Error!
         return 0;
   }

   void
   addPath (std::string stringPath)
   {
      setDirectories_d.insert (stringPath);

      iterCurrent_d = setDirectories_d.begin ();

      fGlobActive_d = false;
   }

private:
   void
   globPath ()
   {
      bool fFound = false;
      int  rc     = 0;

      while (  !fFound                                  // Haven't found any yet
            && iterCurrent_d != setDirectories_d.end () // Haven't reached the end
            )
      {
         std::string stringGlob = *iterCurrent_d++;

         stringGlob += "*.xml";

         // Call glob
         memset (&globbuf_d, 0, sizeof (globbuf_d));
         rc = glob (stringGlob.c_str (), 0, NULL, &globbuf_d);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::PathXMLEnumerator::" << __FUNCTION__ << " (" << stringGlob << ") = " << rc << std::endl;
#endif

         if (0 == rc)
         {
            // Call succeded
            iCurrentFile_d = -1;
            fGlobActive_d  = true;

            // See if there is a device in this directory
            fFound = findDeviceInGlob ();
         }
      }
   }

   bool
   findDeviceInGlob (void)
   {
      bool fFound = false;

      for ( iCurrentFile_d++;
            iCurrentFile_d < (int)globbuf_d.gl_pathc;
            iCurrentFile_d++)
      {
         PSZCRO pszCurrentName = globbuf_d.gl_pathv[iCurrentFile_d];

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::PathXMLEnumerator::" << __FUNCTION__ << ": findDeviceInGlob: checking (" << pszCurrentName << ")" << std::endl;
#endif

         fFound = checkForXMLMasterSignature (pszCurrentName);

         if (fFound)
         {
            break;
         }
      }

      if (!fFound)
      {
         globfree (&globbuf_d);
         fGlobActive_d = false;
      }

      return fFound;
   }

private:
   std::set<std::string>           setDirectories_d;
   std::set<std::string>::iterator iterCurrent_d;
   glob_t                          globbuf_d;
   int                             iCurrentFile_d;
   bool                            fGlobActive_d;
};

class DeviceListXMLEnumerator : public Enumeration
{
public:
   DeviceListXMLEnumerator ()
   {
      // Initialize our variables
      iterCurrent_d = setDirectories_d.begin ();
      fGlobActive_d = false;
      pifstreamIn_d = 0;
   }

   virtual
   ~DeviceListXMLEnumerator ()
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::DeviceListXMLEnumerator::~" << __FUNCTION__ << std::endl;
#endif

      // Clean up
      delete pifstreamIn_d; pifstreamIn_d = 0;
   }

   virtual bool
   hasMoreElements ()
   {
      // Prime the pump.  That is, find at least one element.

      // Are we in a glob?
      if (fGlobActive_d)
      {
         // Find another one
         findDeviceInGlob ();
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::DeviceListXMLEnumerator::" << __FUNCTION__ << ": fGlobActive_d = " << fGlobActive_d << ", " << (iterCurrent_d != setDirectories_d.end ()) << std::endl;
#endif

      if (  !fGlobActive_d                           // Did we not find one?
         && iterCurrent_d != setDirectories_d.end () // Still more directories to search
         )
      {
         // Try with another path
         globPath ();
      }

      // Did we find one?
      if (fGlobActive_d)
         return true;
      else
         return false;
   }

   virtual void *
   nextElement ()
   {
      if (fGlobActive_d)
      {
         // Return the one that we found
         return (void *)stringMasterXMLFileName_d.c_str ();
      }
      else
      {
         // Error!
         return 0;
      }
   }

   void
   addPath (std::string stringPath)
   {
      setDirectories_d.insert (stringPath);

      iterCurrent_d = setDirectories_d.begin ();

      fGlobActive_d = false;
   }

private:
   void
   globPath ()
   {
      bool fFound = false;

      while (  !fFound                                  // Haven't found any yet
            && iterCurrent_d != setDirectories_d.end () // Haven't reached the end
            )
      {
         std::string stringFileName = *iterCurrent_d;
         struct stat statFileName;

         stringFileName += "Device List";

         delete pifstreamIn_d; pifstreamIn_d = 0;

         if (-1 != stat (stringFileName.c_str (), &statFileName))
         {
            pifstreamIn_d = new std::ifstream (stringFileName.c_str ());

            // See if there is a device in this directory
            fFound = findDeviceInGlob ();

            if (fFound)
            {
               // Call succeded
               fGlobActive_d = true;
            }
         }

         if (!fFound)
         {
            iterCurrent_d++;
         }
      }
   }

   bool
   findDeviceInGlob (void)
   {
      char  achLine[512];   // @TBD
      bool  fFound       = false;

      if (!pifstreamIn_d)
      {
         return false;
      }

      while (pifstreamIn_d->getline (achLine, sizeof (achLine)))
      {
         if ('#' == achLine[0])
            continue;

         std::string stringFileNameMasterXML;

         stringFileNameMasterXML  = *iterCurrent_d;
         stringFileNameMasterXML += achLine;
         stringFileNameMasterXML += ".xml";

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::DeviceListXMLEnumerator::" << __FUNCTION__ << ": findDeviceInGlob: checking (" << stringFileNameMasterXML << ")" << std::endl;
#endif

         fFound = checkForXMLMasterSignature (stringFileNameMasterXML.c_str ());

         if (fFound)
         {
            stringMasterXMLFileName_d = stringFileNameMasterXML;
            break;
         }
      }

      if (!fFound)
      {
         fGlobActive_d = false;

         iterCurrent_d++;
      }

      return fFound;
   }

private:
   std::set<std::string>            setDirectories_d;
   std::set<std::string>::iterator  iterCurrent_d;
   bool                             fGlobActive_d;
   std::ifstream                   *pifstreamIn_d;
   std::string                      stringMasterXMLFileName_d;
};

Enumeration * Omni::
listXMLDevices (bool fBuildOnly)
{
   if (fBuildOnly)
   {
      DeviceListXMLEnumerator *pEnum       = 0;
      PSZCRO                   pszXML_PATH = getenv ("OMNI_DEVICELIST_PATH");

      if (!pszXML_PATH)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Error OMNI_DEVICELIST_PATH is not set!" << std::endl;
#endif
      }

      pEnum = new DeviceListXMLEnumerator ();

      if (!pEnum)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Error failed to allocate DeviceListXMLEnumerator!" << std::endl;
#endif
      }

      if (  pEnum
         && pszXML_PATH
         )
      {
         std::string            stringXMLPath = pszXML_PATH;
         std::string::size_type indexCurrent  = 0;
         std::string::size_type indexNext     = 0;

         do
         {
            std::string stringPath;

            // See if there are two or more paths
            indexNext = stringXMLPath.find (":", indexCurrent);

            if (std::string::npos != indexNext)
            {
               stringPath = stringXMLPath.substr (indexCurrent,
                                                  indexNext - indexCurrent);
            }
            else
            {
               stringPath = stringXMLPath.substr (indexCurrent);
            }

            if (0 < stringPath.length ())
            {
               if (stringPath[stringPath.length () - 1] != '/')
               {
                  stringPath += "/";
               }

#ifndef RETAIL
               if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": inserting (" << stringPath << ")" << std::endl;
#endif

               pEnum->addPath (stringPath);
            }

            indexCurrent = indexNext + 1;

         } while (std::string::npos != indexNext); // More to search
      }

      return pEnum;
   }
   else
   {
      PathXMLEnumerator *pEnum = 0;

      pEnum = new PathXMLEnumerator ();

      if (pEnum)
      {
         // Add the Omni builtin paths
         for (int i = 0; vapszAllDataPaths[i]; i++)
         {
            std::string stringPath = vapszAllDataPaths[i];

            if (*vapszAllDataPaths[i])
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": inserting (" << stringPath << ")" << std::endl;
#endif

               pEnum->addPath (stringPath);
            }
         }
      }

      return pEnum;
   }

   return 0;
}

bool Omni::
addOmniToPATH ()
{
   typedef std::map <std::string, int> PathMap;

   std::string             stringPATH;
   std::string             stringFQP;
   std::string::size_type  idxCurrent  = 0;
   std::string::size_type  idxNext;
   PathMap                 pathMapSeen;

   for (int i = 0; i < (int)dimof (vapszExecutablePaths); i++)
   {
      if (vapszExecutablePaths[i])
      {
         pathMapSeen[std::string (vapszExecutablePaths[i])] = 0;
      }
   }

   stringPATH = getenv ("PATH");

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": stringPATH = " << stringPATH << std::endl;
#endif

   while (idxCurrent < stringPATH.length ())
   {
      idxNext = stringPATH.find (':', idxCurrent);

      if (std::string::npos != idxNext)
      {
         stringFQP  = stringPATH.substr (idxCurrent, idxNext - idxCurrent);
         idxCurrent = idxNext + 1;
      }
      else
      {
         idxNext    = stringPATH.length ();
         stringFQP  = stringPATH.substr (idxCurrent, idxNext - idxCurrent);
         idxCurrent = idxNext + 1;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": stringFQP = " << stringFQP << std::endl;
#endif

      for (int i = 0; i < (int)dimof (vapszExecutablePaths); i++)
      {
         if (!vapszExecutablePaths[i])
         {
            continue;
         }

         int  cbLength       = strlen (vapszExecutablePaths[i]);
         bool fTrailingSlash = false;

         if ('/' == vapszExecutablePaths[i][cbLength - 1])
         {
            fTrailingSlash = true;
            cbLength--;
         }

         if (  (  fTrailingSlash
               && 0 == strncmp (vapszExecutablePaths[i],
                                stringFQP.c_str (),
                                cbLength)
               && 0 == strcmp (vapszExecutablePaths[i] + cbLength,
                               "/")
               )
            || (  !fTrailingSlash
               && 0 == strcmp (vapszExecutablePaths[i],
                               stringFQP.c_str ())
               )
            )
         {
            std::string stringSeen = vapszExecutablePaths[i];

            pathMapSeen[stringSeen] = 1;

#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Match " << stringSeen << std::endl;
#endif
         }
      }
   }

   bool fNewPathsAdded = false;

   for ( PathMap::iterator nextPath = pathMapSeen.begin () ;
         nextPath != pathMapSeen.end () ;
         nextPath++ )
   {
      if (!nextPath->second)
      {
         if (!fNewPathsAdded)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": Paths not seen:" << std::endl;
#endif

            fNewPathsAdded = true;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": " << nextPath->first << std::endl;
#endif

         if (':' != stringPATH[stringPATH.length () - 1])
         {
            stringPATH += ":";
         }
         stringPATH += nextPath->first;
      }
   }

   if (fNewPathsAdded)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": new stringPATH = " << stringPATH << std::endl;
#endif

      setenv ("PATH", stringPATH.c_str (), 1);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmni ()) DebugOutput::getErrorStream () << "Omni::" << __FUNCTION__ << ": PATH = " << getenv ("PATH") << std::endl;
#endif
   }

   return 0;
}

PSZCRO Omni::
quoteString (PSZRO pszString)
{
   if (!pszString)
      return 0;

   int   cbNewString = 0;
   PSZRO pszFrom     = pszString;

   while (*pszFrom)
   {
      switch (*pszFrom++)
      {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
      case '"':
      case '%':
         cbNewString += 3;
         break;
      default:
         cbNewString++;
         break;
      }
   }

   if (cbNewString)
   {
      cbNewString++;

      PSZ pszNewString = (PSZ)calloc (1, cbNewString);
      PSZ pszTo        = 0;

      if (pszNewString)
      {
         pszFrom = pszString;
         pszTo   = pszNewString;

         while (*pszFrom)
         {
            switch (*pszFrom)
            {
            case ' ':  strcat (pszTo, "%20"); pszTo += strlen (pszTo); break;
            case '\t': strcat (pszTo, "%09"); pszTo += strlen (pszTo); break;
            case '\r': strcat (pszTo, "%0D"); pszTo += strlen (pszTo); break;
            case '\n': strcat (pszTo, "%0A"); pszTo += strlen (pszTo); break;
            case '"':  strcat (pszTo, "%22"); pszTo += strlen (pszTo); break;
            case '%':  strcat (pszTo, "%25"); pszTo += strlen (pszTo); break;
            default:   *pszTo++ = *pszFrom; break;
            }
            pszFrom++;
         }

         *pszTo = '\0';

         return (PSZCRO)pszNewString;
      }
   }

   return 0;
}

PSZCRO Omni::
dequoteString (PSZRO pszString)
{
   if (!pszString)
      return 0;

   int   cbNewString = 0;
   PSZRO pszFrom     = pszString;

   while (*pszFrom)
   {
      switch (*pszFrom++)
      {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
      {
         break;
      }
      case '%':
      {
         int  iHigh  = -1;
         int  iLow   = -1;
         bool fValid = true;

         while (  *pszFrom
               && !(  -1 != iHigh
                   && -1 != iLow
                   )
               && fValid
               )
         {
            switch (*pszFrom)
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            {
               pszFrom++;
               break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
               if (-1 == iHigh)
               {
                  iHigh = *pszFrom - '0';
               }
               else if (-1 == iLow)
               {
                  iLow = *pszFrom - '0';
               }
               pszFrom++;
               break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            {
               if (-1 == iHigh)
               {
                  iHigh = 10 + *pszFrom - 'a';
               }
               else if (-1 == iLow)
               {
                  iLow = 10 + *pszFrom - 'a';
               }
               pszFrom++;
               break;
            }
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            {
               if (-1 == iHigh)
               {
                  iHigh = 10 + *pszFrom - 'A';
               }
               else if (-1 == iLow)
               {
                  iLow = 10 + *pszFrom - 'A';
               }
               pszFrom++;
               break;
            }
            default:
            {
               fValid = false;
               cbNewString++;   // '%'
               if (-1 != iHigh)
               {
                  cbNewString++;
               }
               if (-1 != iLow)
               {
                  cbNewString++;
               }
               break;
            }
            }
         }
         if (  fValid
            && -1 != iHigh
            && -1 != iLow
            )
         {
            cbNewString++;
         }
         break;
      }
      default:
      {
         cbNewString++;
         break;
      }
      }
   }

   if (cbNewString)
   {
      cbNewString++;

      PSZ pszNewString = (PSZ)calloc (1, cbNewString);
      PSZ pszTo        = 0;

      if (pszNewString)
      {
         pszFrom = pszString;
         pszTo   = pszNewString;

         while (*pszFrom)
         {
            switch (*pszFrom)
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            {
               pszFrom++;
               break;
            }
            case '%':
            {
               int  iHigh  = -1;
               int  iLow   = -1;
               char chHigh = ' ';
               char chLow  = ' ';
               bool fValid = true;

               pszFrom++;
               while (  *pszFrom
                     && !(  -1 != iHigh
                         && -1 != iLow
                         )
                     && fValid
                     )
               {
                  switch (*pszFrom)
                  {
                  case ' ':
                  case '\t':
                  case '\r':
                  case '\n':
                  {
                     pszFrom++;
                     break;
                  }
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                  {
                     if (-1 == iHigh)
                     {
                        iHigh  = *pszFrom - '0';
                        chHigh = *pszFrom;
                     }
                     else if (-1 == iLow)
                     {
                        iLow  = *pszFrom - '0';
                        chLow = *pszFrom;
                     }
                     pszFrom++;
                     break;
                  }
                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                  {
                     if (-1 == iHigh)
                     {
                        iHigh  = 10 + *pszFrom - 'a';
                        chHigh = *pszFrom;
                     }
                     else if (-1 == iLow)
                     {
                        iLow  = 10 + *pszFrom - 'a';
                        chLow = *pszFrom;
                     }
                     pszFrom++;
                     break;
                  }
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                  {
                     if (-1 == iHigh)
                     {
                        iHigh  = 10 + *pszFrom - 'A';
                        chHigh = *pszFrom;
                     }
                     else if (-1 == iLow)
                     {
                        iLow  = 10 + *pszFrom - 'A';
                        chLow = *pszFrom;
                     }
                     pszFrom++;
                     break;
                  }
                  default:
                  {
                     fValid = false;
                     *pszTo++ = '%';
                     if (-1 != iHigh)
                     {
                        *pszTo++ = chHigh;
                     }
                     if (-1 != iLow)
                     {
                        *pszTo++ = chLow;
                     }
                     break;
                  }
                  }
               }
               if (  -1 != iHigh
                  && -1 != iLow
                  )
               {
                  *pszTo++ = (char)(iHigh * 16 + iLow);
               }
               break;
            }
            default:
            {
               *pszTo++ = *pszFrom++;
               break;
            }
            }
         }

         *pszTo = '\0';

         return (PSZCRO)pszNewString;
      }
   }

   return 0;
}
