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
#include "Omni.hpp"

void
printUsage (int argc, char *argv[])
{
   std::cerr << "Error: Usage: "
             << argv[0]
             << " [-v] libDeviceName.so [VERSION]"
             << std::endl;
   std::cerr << "       Command line is ";

   for (int i = 0; i < argc; i++)
   {
      std::cerr << "\""
                << argv[i]
                << "\"";
   }

   std::cerr << std::endl;
}

/* Usage:
**
**   OmniDeviceValid [-v] libDeviceName.so [VERSION]
*/
int
main (int argc, char *argv[])
{
   GModule *hDevice        = 0;
   int      rc             = 1;
   bool     fVerbose       = false;
   char    *pszLibraryName = 0;
   char    *pszVersion     = 0;
   int      i              = 1;

   if (2 > argc)
   {
      // No parameters
      printUsage (argc, argv);

      return rc;
   }

   if (  0 == strcasecmp (argv[i], "-v")
      || 0 == strcasecmp (argv[i], "-V")
      )
   {
      fVerbose = true;
      i++;
   }

   if (1 > argc - i)
   {
      printUsage (argc, argv);

      return rc;
   }

   Omni::initialize ();

   pszLibraryName = argv[i++];

   hDevice = g_module_open (pszLibraryName, (GModuleFlags)0);

   if (!hDevice)
   {
      if (fVerbose)
      {
         // @HACK
         if (!(  NULL != strstr (pszLibraryName, "Instance")
              && NULL != strstr (pszLibraryName, "Blitter")
              )
            )
         {
            std::cerr << "Error: " << g_module_error () << std::endl;
         }
      }
   }

///std::cout << (getenv ("LD_LIBRARY_PATH") ? getenv ("LD_LIBRARY_PATH") : "NULL")
///          << ", pszLibraryName = "
///          << pszLibraryName
///          << ", hDevice = "
///          << hDevice
///          << ", fVerbose = "
///          << fVerbose
///          << ", argc = "
///          << argc
///          << ", "
///          << (1 <= argc - i)
///          << ", "
///          << (strstr (pszLibraryName, "Instance") ? "I" : "")
///          << ", "
///          << (strstr (pszLibraryName, "Blitter") ? "B" : "")
///          << std::endl;

   if (hDevice)
   {
      if (1 <= argc - i)
      {
         pszVersion = argv[i++];

         if (Omni::libraryValid (hDevice, pszLibraryName, pszVersion, fVerbose))
         {
            rc = 0;
         }
      }

      g_module_close (hDevice);
   }

   Omni::terminate ();

   return rc;
}
