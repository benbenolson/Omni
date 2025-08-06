/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2002
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
#include <iostream>
#include <memory>
#include <string>

#include "OmniGSInterface.hpp"
#include <Omni.hpp>
#include <JobProperties.hpp>

using namespace std;

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << " ( --driver printer_library_name | '-sproperties=\"...\"' | printer_short_name )+"
             << std::endl;
}

int
main (int argc, char *argv[])
{
   JobProperties  jobProp;
   PSZRO          pszJobProperties = 0;
   int            rc               = 0;
   int            rc2;

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "Error: This program needs glib's module routines!" << std::endl;

      return 1;
   }

   Omni::initialize ();

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcmp ("--driver", argv[i]))
      {
         std::ostringstream  oss;
         std::string         stringOss;
         char               *pszDriverLibrary = argv[i + 1];

         pszDriverLibrary = argv[i + 1];

         if (0 != strncasecmp (argv[i + 1], "lib", 3))
         {
            oss << "lib"
                << pszDriverLibrary
                << ".so";

            stringOss        = oss.str ();
            pszDriverLibrary = (char *)stringOss.c_str ();
         }

         rc2 = GetOmniJobProperties (pszDriverLibrary, &pszJobProperties);

         cout << "rc2 = " << rc2 << ", pszJobProperties = \"" << pszJobProperties << "\"" << endl;

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
         DeviceInfo *pDI = Omni::findDeviceInfoFromShortName (argv[i], false);

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

            rc2 = GetOmniJobProperties (pDevice->getLibraryName (), &pszJobProperties);

            cout << "rc2 = " << rc2 << ", pszJobProperties = \"" << pszJobProperties << "\"" << endl;

            if (pszJobProp)
            {
               free ((void *)pszJobProp);
               pszJobProp = 0;
            }

            delete pDI;
         }
      }
   }

   rc = 0;

   // Clean up
   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
   }

   Omni::terminate ();

   return rc;
}
