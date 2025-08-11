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
 * /etc/rc.d/init.d/cups restart
 * /etc/cups/ppds.dat
 * /etc/cups/ppd/bob.ppd
 * vi /etc/cups/cupsd.conf  --  LogLevel debug2
 * http://localhost:631/
 * /usr/lib/cups/filter/
 * /usr/share/cups/model/foomatic/%driver%-%device%-cups.ppd.gz
 * vi /var/log/cups/error_log
 * lpr.cups /home/ghostscript/test/examples/tiger.ps
 * make install; /bin/cp /opt/Omni/bin/CUPSToOmni /usr/lib/cups/filter
 * unzip -c PPDs/Epson/Epson-Epson_Stylus_C60-omni-cups.ppd.gz > bob
 * (cd ..; . ./setit Epson.PDC; cd CUPS; ./GenerateOmniPPDs --buildOnly)
 *
 */
#include "Omni.hpp"

#include <sys/stat.h>
#include <sys/types.h>

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
             << "\t--buildOnly\tShows devices in the LD_LIBRARY_PATH path"
             << std::endl
             << "\t--help\t\tPrints this out"
             << std::endl;
}

bool
createDirectory (std::string stringDirectoryName)
{
   struct stat statFile;
   mode_t      mode       = S_IRUSR | S_IWUSR | S_IXUSR
                          | S_IRGRP | S_IWGRP | S_IXGRP
                          | S_IROTH | S_IXOTH;

   if (0 == stat (stringDirectoryName.c_str (), &statFile))
   {
      if (!S_ISDIR (statFile.st_mode))
      {
         std::cerr << "Error: " << stringDirectoryName << " is not a directory!" << std::endl;

         return false;
      }

      return true;
   }
   else
   {
      if (0 == mkdir (stringDirectoryName.c_str (), mode))
      {
         return true;
      }
      else
      {
         return false;
      }
   }
}

void
convertFilename (char *pszFileName)
{
   while (*pszFileName)
   {
      switch (*pszFileName)
      {
      case ' ':
         *pszFileName = '_';
      }

      pszFileName++;
   }
}

void
callDriver (PSZCRO        pszLibraryName,
            PSZCRO        pszJobProperties,
            std::string&  stringRoot,
            PSZCRO        pszProgramName,
            int           iFileNumber)
{
   Device            *pDevice           = 0;
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;
   GModule           *hmodDevice        = 0;

   hmodDevice = g_module_open (pszLibraryName, (GModuleFlags)0);

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
            std::string stringDirectory;

            std::cout << "Generating "
                      << stringRoot
                      << "/"
                      << pDevice->getDriverName ()
                      << "/"
                      << pDevice->getDriverName ()
                      << "-"
                      << pDevice->getShortName ()
                      << "-omni"
                      << "-cups.ppd.gz"
                      << std::endl;

            stringDirectory = stringRoot
                            + "/"
                            + pDevice->getDriverName ();
            convertFilename (const_cast<char*>(stringDirectory.c_str ())); // @HACK?

            if (createDirectory (stringDirectory))
            {
               std::string        stringSystem;
               std::string        stringSystemArgs;
               std::ostringstream oss;

               stringSystemArgs += stringRoot;
               stringSystemArgs += "/";
               stringSystemArgs += pDevice->getDriverName ();
               stringSystemArgs += "/";
               stringSystemArgs += pDevice->getDriverName ();
               stringSystemArgs += "-";
               stringSystemArgs += pDevice->getShortName ();
               stringSystemArgs += "-omni";
               stringSystemArgs += "-cups.ppd.gz\"";
               convertFilename (const_cast<char*>(stringSystemArgs.c_str ())); // @HACK?

               oss << "GenerateOmniPPD "
                   << "--driver \""
                   << pszLibraryName
                   << "\""
                   << " --number "
                   << iFileNumber
                   << " ";

               if (  pszJobProperties
                  && *pszJobProperties
                  )
               {
                  oss << " -sproperties=\'"
                      << pszJobProperties
                      << "\'";
               }

               oss << " | gzip -c > \""
                   << stringSystemArgs;

               stringSystem = oss.str ();

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "GenerateOmniPPDs::" << __FUNCTION__ << ": execing \"" << stringSystem << "\"" << std::endl;
#endif

               Omni::my_system (stringSystem.c_str ());
            }
            else
            {
               std::cerr << "Error: Could not create directory \"" << stringDirectory << "\"" << std::endl;
            }

            delete pDevice;
         }
         else
         {
            std::cerr << "Error: Could not create device with \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\"" << std::endl;
         }
      }

      g_module_close (hmodDevice);
   }
}

int
main (int argc, char *argv[])
{
   char *pszProgramName = argv[0];
   bool  fBuildOnly     = false;
   int   iFileNumber    = 0;
   int   rc             = 0;

   if (!g_module_supported ())
   {
      std::cerr << "Error: This program needs glib's module routines!" << std::endl;

      return 1;
   }

   std::string stringRoot = "PPDs";

   if (!createDirectory (stringRoot))
   {
      return 1;
   }
   
   std::cerr << "Created " << stringRoot << std::endl;

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp (argv[i], "--buildOnly"))
      {
         fBuildOnly = true;
      }
      else if (  0 == strcasecmp (argv[i], "--help")
              || 0 == strcasecmp (argv[i], "-?")
              )
      {
         printUsage (pszProgramName);

         return 1;
      }
   }

   Omni::initialize ();

   Enumeration *pEnum = Omni::listDevices (fBuildOnly);
   
   std::cerr << "Got devices." << std::endl;

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();
      
      std::cerr << "Got a device of some kind." << std::endl;

      if (pOD)
      {
         PSZCRO pszLibName       = pOD->getLibraryName ();
         PSZCRO pszJobProperties = pOD->getJobProperties ();
         
        std::cerr << "Got driver: " << pszLibName << std::endl;

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "GenerateOmniPPDs::" << __FUNCTION__ << ": pszLibName = " << pszLibName << ", pszJobProperties = " << (pszJobProperties ? pszJobProperties : "NULL") << std::endl;
#endif

         callDriver (pszLibName,
                     pszJobProperties,
                     stringRoot,
                     pszProgramName,
                     iFileNumber);

         iFileNumber++;

         delete pOD;
      }
      else
      {
         std::cerr << "Error: Enumeration returns an empty value." << std::endl;
      }
   }

   delete pEnum;

   Omni::terminate ();

   return rc;
}
