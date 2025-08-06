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

#include <glib.h>
#include <gmodule.h>

typedef enum _Mode {
   MODE_SHORT_NAME = 1,
   MODE_DESCRIPTIVE_NAME,
   MODE_FULL_PATH,
   MODE_GENERATE_XML_FILE
} EMODE, *PEMODE;

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << std::endl
             << "\t--short\t\tShows the device name parameter"
             << std::endl
             << "\t--descriptive\tShows the driver.device name"
             << std::endl
             << "\t--fullPath\tShows the full path"
             << std::endl
             << "\t--generateXMLFile\tGenerates an XML device mapping file"
             << std::endl
             << "\t--buildOnly\tShows devices in the LD_LIBRARY_PATH path"
             << std::endl
             << "\t--help\t\tPrints this out"
             << std::endl;
}

int
main (int argc, char *argv[])
{
///DebugOutput::setOutputOmni (false);

   if (!g_module_supported ())
   {
      std::cerr << "Error: This program needs glib's module routines!" << std::endl;

      return 1;
   }

   EMODE eMode      = MODE_SHORT_NAME;
   bool  fBuildOnly = false;

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp (argv[i], "--short"))
      {
         eMode = MODE_SHORT_NAME;
      }
      else if (0 == strcasecmp (argv[i], "--descriptive"))
      {
         eMode = MODE_DESCRIPTIVE_NAME;
      }
      else if (0 == strcasecmp (argv[i], "--fullPath"))
      {
         eMode = MODE_FULL_PATH;
      }
      else if (0 == strcasecmp (argv[i], "--generateXMLFile"))
      {
         eMode = MODE_GENERATE_XML_FILE;
      }
      else if (0 == strcasecmp (argv[i], "--buildOnly"))
      {
         fBuildOnly = true;
      }
      else if (  0 == strcasecmp (argv[i], "--help")
              || 0 == strcasecmp (argv[i], "-?")
              )
      {
         printUsage (argv[0]);

         return 1;
      }
   }

   Omni::initialize ();

   Enumeration *pEnum = Omni::listDevices (fBuildOnly);

   switch (eMode)
   {
   case MODE_GENERATE_XML_FILE:
   {
      std::cout << "<devices>" << std::endl;
      break;
   }
   default:
   {
      break;
   }
   }

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         PSZCRO pszLibName       = pOD->getLibraryName ();
         PSZCRO pszJobProperties = pOD->getJobProperties ();

         switch (eMode)
         {
         case MODE_SHORT_NAME:
         case MODE_DESCRIPTIVE_NAME:
         case MODE_GENERATE_XML_FILE:
         {
            Device            *pDevice           = 0;
            PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;
            GModule           *hmodDevice        = 0;

            hmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

            if (hmodDevice)
            {
               g_module_symbol (hmodDevice,
                                "newDeviceW_JopProp_Advanced",
                                (gpointer *)&pfnNewDeviceWArgs);

               if (pfnNewDeviceWArgs)
               {
                  pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

                  if (pDevice)
                  {
                     switch (eMode)
                     {
                     case MODE_DESCRIPTIVE_NAME:
                     {
                        std::cout << pDevice->getDriverName ()
                                  << "."
                                  << pDevice->getDeviceName ()
                                  << std::endl;
                        break;
                     }

                     case MODE_SHORT_NAME:
                     {
                        std::cout << pDevice->getShortName () << std::endl;
                        break;
                     }

                     case MODE_GENERATE_XML_FILE:
                     {
                        std::string *pstringJP = pDevice->getJobProperties ();

                        std::cout << "   <device>" << std::endl;
                        std::cout << "      <shortname>"
                                  << pDevice->getShortName ()
                                  << "</shortname>"
                                  << std::endl;
                        std::cout << "      <libraryname>"
                                  << pDevice->getLibraryName ()
                                  << "</libraryname>"
                                  << std::endl;
                        if (pstringJP)
                        {
                           std::cout << "      <jobproperties>"
                                     << *pstringJP
                                     << "</jobproperties>"
                                     << std::endl;
                        }
                        std::cout << "   </device>" << std::endl;

                        delete pstringJP;
                        break;
                     }

                     default:
                        // Should not happen!
                        break;
                     }

                     delete pDevice;
                  }
               }

               g_module_close (hmodDevice);
            }
            break;
         }

         case MODE_FULL_PATH:
         {
            std::cout << pszLibName << std::endl;
            break;
         }
         }

         delete pOD;
      }
      else
      {
         std::cerr << "Error" << std::endl;
      }
   }

   delete pEnum;

   switch (eMode)
   {
   case MODE_GENERATE_XML_FILE:
   {
      std::cout << "</devices>" << std::endl;
      break;
   }
   default:
   {
      break;
   }
   }

   Omni::terminate ();

   return 0;
}
