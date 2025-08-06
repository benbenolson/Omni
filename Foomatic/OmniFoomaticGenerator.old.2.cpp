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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <fstream>

#include "Device.hpp"
#include "Omni.hpp"
#include "JobProperties.hpp"
#include "omni2foo.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <glib.h>
#include <gmodule.h>

#include <map>
#include <list>

typedef struct _DeviceListElement {
   const std::string *pstringCurrentJobProperties;
   std::string       *pstringDefaultJobProperties;
   int                iDevice;
   PSZRO              pszFoomaticDriver;
} DLE, *PDLE;

typedef std::map <std::string, PDLE>          MapDeviceList;

typedef struct _MapKeyValueElement {
   MapDeviceList  *pDeviceList;
   std::string    *pstringKeyValueTranslation;
} MKVE, *PMKVE;

typedef std::map <std::string, PMKVE>         MapKeyValue;
typedef std::map <std::string, MapKeyValue *> MapKey;

typedef struct _OmniDeviceHandle {
   GModule     *hmodDevice;
   Device      *pDevice;
   std::string *pstringDeviceName;
   std::string *pstringMakeModel;
   EOMNICLASS   eOmniClass;
} OMNIHANDLE, *POMNIHANDLE;

std::string *
getCleanName (std::string *rawName, int operation)
{
   // Clean up Foomatic names to get valid file names/Foomatic IDs or
   // the correct make/model pair
   //
   // operation = 0: Return string suitable as Foomatic ID/XML file name
   // Mangle the device name to be Make-Model_Name
   // First space (after manufacturer's name) needs to be a -, and then
   // all spaces after that need to be a _
   // Ex:  Canon-BJC-2000, Epson-Stylus_Color_740
   //
   // operation = 1: Return Make and Model, separated by a "|", names
   // cleaned up from certain weirdnesses
   std::string device = *rawName;

   // Find the first space, ie, the gap between the make and model
   // and in case of operation 0 replace the " " with a "-", in case
   // of operation 1 with a "|"
   std::string::size_type pos = device.find_first_of (" ", 0);

   if (operation == 0)
   {
      device.replace (pos, 1, "-");
   }
   else
   {
      device.replace (pos, 1, "|");
   }

   // The first word of the device name is not necessarily the
   // manufacturer's name, so determine the manufacturer here and
   // also do a clean-up of the model name
   std::string::size_type idx;

   if (device.find ("OkiP") == 0)
   {
      std::string make = "Okidata";

      // Replace the old Make/Model separator by a space and
      // build the output string according to the operation
      if (operation == 0)
      {
         if ((idx = device.find ("-")) != std::string::npos)
         {
            device.replace (idx, 1, " ");
         }

         device = make + "-" + device;
      }
      else
      {
         if ((idx = device.find ("|")) != std::string::npos)
         {
            device.replace (idx, 1, " ");
         }

         device = make + "|" + device;
      }
   }
   else if (device.find ("HP") == 0)
   {
      // Replace roman numbers in model names of HP LaserJet printers
      if ((idx = device.find ("III")) != std::string::npos)
      {
         device.replace (idx, 3, "3");
      }
      else if ((idx = device.find ("II")) != std::string::npos)
      {
         device.replace (idx, 2, "2");
      }
   }
   else if (device.find ("KS") == 0)
   {
      // "KS" is not really a manufacturer, but the name of a subdriver
      // of Omni for printers of several manufacturers.
      // Remove "KS "
      std::string s     = device.substr (3);
      std::string make;
      std::string model;

      if (  (  (s[0] >= '0')
            && (s[0] <= '9')
            )
         || (s.find ("KS") == 0)
         )
      {
         // Generic printers
         make = "Generic";

         if (s.find ("KS") == 0)
         {
            model = s.substr (3);
         }
         else
         {
            model = s;
         }

         if (model.find ("Generic") == 0)
         {
            model = model.substr (8);
         }
      }
      else if (s.find ("BJ") == 0)
      {
         // This is probably not the Canon BJ-230 because the "Canon"
         // subdriver already supports the Canon BJ-230
         make = "Others";
         model = s;
      }
      else if (s.find ("Gold Star") == 0)
      {
         make = "GoldStar";
         model = s.substr (10);
      }
      else if (  (s.find ("HDMF") == 0)
              || (s.find ("VP-") == 0)
              || (s.find ("LG ") == 0)
              )
      {
         make = "Others";
         model = s;
      }
      else
      {
         std::string::size_type idx = s.find_first_of (' ', 0);

         make = s.substr (0, idx);
         make[0] = toupper (make[0]);

         for (unsigned int i = 1;
              i < make.size ();
              i ++)
         {
            make[i] = tolower (make[i]);
         }

         model = s.substr (idx+1);
      }

      if (operation == 0)
      {
         device = make + "-" + model;
      }
      else
      {
         device = make + "|" + model;
      }
   }

   // Remove trailing spaces
   while (device[device.size () - 1] == ' ')
   {
      device = device.substr (0, device.size () - 1);
   }

   if (operation == 0)
   {
      // Replace all slashes in the model name (breaks file names)
      while ((idx = device.find ("/")) != std::string::npos)
      {
         device.replace (idx, 1, " ");
      }

      // Replace "Plus", "PLUS", and "+" by "plus"
      while ((idx = device.find ("plus")) != std::string::npos)
      {
         device.replace (idx, 4, "+");
      }

      while ((idx = device.find ("Plus")) != std::string::npos)
      {
         device.replace (idx, 4, "+");
      }

      while ((idx = device.find ("PLUS")) != std::string::npos)
      {
         device.replace (idx, 4, "+");
      }

      while ((idx = device.find (" +")) != std::string::npos)
      {
         device.replace (idx, 2, "+");
      }

      while ((idx = device.find ("+")) != std::string::npos)
      {
         device.replace (idx, 1, "plus");
      }

      // Remove "(", ")", ",", and " "
      // thanks to jason@alteredminds.com for pointing out a bug
      while ((idx = device.find_first_of ("(), ")) != std::string::npos)
      {
         device.replace (idx, 1, "_");
      }

      // Remove trailing underscores
      while (device[device.size () - 1] == '_')
      {
         device = device.substr (0, device.size () - 1);
      }
   }

   return new std::string (device);
}

std::string *
getDeviceName (Device *pDevice, omni2foo *db, int operation)
{
   // operation = 0: Return Foomatic ID (also used as the XML file name)
   // operation = 1: Return Make and Model (for printer's DB entry, only
   //                if Foomatic ID does not match)
   //
   //                If a Foomatic ID of the printer exists, no printer
   //                XML file will be created because Foomatic provides
   //                already one (empty string returned in operation 1).
   //
   //                If a printer entry does not exist, Omni must provide
   //                one, here for the manufacturer and model entry the
   //                returned string of operation 1 is used. The returned
   //                string of operation 0 provides the Foomatic ID and
   //                the file name.
   std::string devName = pDevice->getDeviceName ();

   // check the db
   std::string cleanname = *getCleanName (&devName, 0);
   std::string id        = db->getFooIDFromString (cleanname);

   // if there is no match, the string that is returned is empty
   if (id.length () > 0)
   {
      if (operation == 0)
      {
         // operation = 0:
         // found a match, return the fooid string
         return new std::string (id);
      }
      else
      {
         // operation = 1:
         // match, return empty string
         return new std::string ("");
      }
   }
   else
   {
      // operation = 0:
      // no match, return Foomatic-compliant device name
      // Mangle the device name to be Make-Model_Name
      // First space (after manufacturer's name) needs to be a -, and then
      // all spaces after that need to be a _
      // Ex:  Canon-BJC-2000, Epson-Stylus_Color_740
      //
      // operation = 1:
      // Return cleaned up make and model names

      std::string device = *getCleanName (&devName, operation);

      return new std::string (device);
   }
}

// Derive command-line suitable shortnames out of the longnames. Most of
// them are also equal to the shortnames used by the Foomatic options of
// other drivers (as GIMP-Print, ljet4, ...)

// Remove spaces and '#' out of a C-style string, make letters after
// former spaces capital (as long as the character before the space is not a
// number)

PSZRO
generateShortName (PSZRO pszLongName)
{
   int   j          = 0;
   int   afterspace = 0;
   int   lastnumber = 0;
   char *pszResult  = 0;

   // Allocate memory of same length of original string
   pszResult = (char *)calloc (1, sizeof (char)*(strlen (pszLongName)+1));

   for (unsigned int i = 0; i < strlen (pszLongName); i++)
   {
      if (  (pszLongName[i] != ' ')
         && (pszLongName[i] != '#')
         && (pszLongName[i] != '-') // "-" in a short name breaks the Foomatic
	                                 // filters
         )
      {
         if (afterspace)
         {
            pszResult[j] = toupper (pszLongName[i]);
            afterspace = 0;
         }
         else
         {
            pszResult[j] = pszLongName[i];
         }

         if (  (pszLongName[i] >= '0')
            && (pszLongName[i] <= '9')
            )
         {
            lastnumber = 1;
         }
         else
         {
            lastnumber = 0;
         }

         j++;
      }
      else
      {
         if (  (pszLongName[i] == ' ')
            && (!lastnumber)
            )
         {
            afterspace = 1;
         }
      }
   }

   pszResult[j] = 0;

   return pszResult;
}

bool
createDirectory (std::string   stringDirectoryName,
                 PSZCRO        pszProgramName)
{
   struct stat statFile;
   mode_t      mode       = S_IRUSR | S_IWUSR | S_IXUSR
                          | S_IRGRP | S_IWGRP | S_IXGRP
                          | S_IROTH | S_IXOTH;

   if (0 == stat (stringDirectoryName.c_str (), &statFile))
   {
      if (!S_ISDIR (statFile.st_mode))
      {
         std::cerr << pszProgramName << ": Error: " << stringDirectoryName << " is not a directory!" << std::endl;

         return false;
      }

      return true;
   }
   else
   {
      std::cout << pszProgramName << ": Warning: creating directory \"" << stringDirectoryName << "\"" << std::endl;

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

char *
getFoomaticDriver (EOMNICLASS eOmniClass)
{
   switch (eOmniClass)
   {
   case OMNI_CLASS_COMPILED: return "omni-compiled";
   case OMNI_CLASS_XML:      return "omni-xml";
   case OMNI_CLASS_UPDF:     return "omni-updf";
   default:
   case OMNI_CLASS_UNKNOWN:  return "omni-unknown";
   }
}

std::string *
translateKeyValueList (MapKeyValue *pkeyvalueMaps,
                       POMNIHANDLE  pohDevices)
{
   MapKeyValue::iterator    nextKeyValue       = pkeyvalueMaps->begin ();
   PMKVE                    pMKVE              = nextKeyValue->second;
   MapDeviceList           *pDeviceList        = pMKVE->pDeviceList;
   MapDeviceList::iterator  nextDeviceList     = pDeviceList->begin ();
   PDLE                     pDLE               = nextDeviceList->second;
   Device                  *pDevice            = pohDevices[pDLE->iDevice].pDevice;
   std::string              stringKeyValueList = nextKeyValue->first;
   std::string             *pstringTranslation = new std::string ();
   std::string             *pRet               = 0;
   std::string::size_type   posStart           = 0;
   std::string::size_type   posEnd             = 0;

   do
   {
      std::string            stringKeyValue;
      std::string            stringKey;
      std::string::size_type posEquals      = std::string::npos;

      posEnd = stringKeyValueList.find (" ", posStart);

      if (std::string::npos == posEnd)
      {
         stringKeyValue = stringKeyValueList.substr (posStart);
      }
      else
      {
         stringKeyValue = stringKeyValueList.substr (posStart, posEnd - posStart);
      }

      posEquals = stringKeyValue.find ("=");

      if (std::string::npos != posEquals)
      {
         stringKey = stringKeyValue.substr (0, posEquals);

         pRet = pDevice->translateKeyValue (stringKey.c_str (), 0);

         if (pRet)
         {
            if ((*pstringTranslation)[0])
            {
               *pstringTranslation += " / ";
            }

            *pstringTranslation += *pRet;

            delete pRet;
         }
      }

      posStart = posEnd + 1;

   } while (std::string::npos != posEnd);

   return pstringTranslation;
}

int
main (int argc, char *argv[])
{
   bool                afOmniClassExists[OMNI_CLASS_COUNT + 1];
   PSZCRO              pszExeName                              = argv[0];
   POMNIHANDLE         pohDevices                              = 0;
   std::ofstream      *pofstream                               = 0;
   std::string         basedir                                 = "foomatic-db/db/source";
   std::string         foomaticDB                              = DEFAULT_SHARE_PATH "foo2omni";
   bool                fUseLocalBuild                          = false;
   std::ostringstream  oss;
   int                 iArgOrder                               = 200;
   MapKey              keyMap;
   MapKeyValue        *pMapKeyValue                            = 0;
   PMKVE               pMKVE                                   = 0;
   int                 iRc                                     = 1;

   for (int i = 0; i <= OMNI_CLASS_COUNT; i++)
      afOmniClassExists[i] = false;

   Omni::initialize ();

   // Command Line parsing options:
   // -b : base dir, the location where the foomatic-db dir will be made
   // -d : the foomatic id db.  it is a space separated file, specify where that file is, or use the default
   // -l : use the build root
   for (int i = 1; i < argc; i++)
   {
      if (argv[i][0] == '-')
      {
         switch (argv[i][1])
         {
         case 'D':
         case 'd':
         {
            foomaticDB = argv[i + 1];
            break;
         }

         case 'b':
         case 'B':
         {
            basedir = argv[i + 1];
            break;
         }

         case 'l':
         case 'L':
         {
            fUseLocalBuild  = true;
            foomaticDB      = "share/foo2omni";
            break;
         }
         }
      }
   }

   // Check basedir arg for trailing slash, if not present, add
   if (  basedir.length () > 0
      && basedir[basedir.length ()-1] != '/'
      )
      basedir = basedir + "/";

   // Make sure that the directories exist
   if (basedir.length () > 0)
   {
      std::string::size_type posCurrent = basedir.find ('/', 1);

      do
      {
         std::string stringDirectory;

         if (std::string::npos == posCurrent)
         {
            stringDirectory = basedir;
         }
         else
         {
            stringDirectory = basedir.substr (0, posCurrent);
         }

         if (!createDirectory (stringDirectory, pszExeName))
         {
            break;
         }

         posCurrent = basedir.find ('/', posCurrent + 1);

      } while (posCurrent != std::string::npos);
   }
   if (  !createDirectory (basedir + "printer", pszExeName)
      || !createDirectory (basedir + "driver", pszExeName)
      || !createDirectory (basedir + "opt", pszExeName)
      )
   {
      std::cerr << "Errror: Could not create the subdirectories." << std::endl;

      return __LINE__;
   }

   if (!g_module_supported ())
   {
      std::cerr << "Error: This program needs glib's module routines!" << std::endl;

      return __LINE__;
   }

   std::cout << pszExeName << ": using " << foomaticDB << std::endl;

   // Now make the omni2foo DB
   omni2foo DB (foomaticDB);

   /* Count the number of omni devices.
   */
   std::cout << pszExeName << ": Counting devices..." << std::endl;

   Enumeration *pEnum          = Omni::listDevices (fUseLocalBuild);
   int          iNumDevices    = 0;
   int          iCurrentDevice = 0;

   if (!pEnum)
   {
      std::cerr << "Error: Could not query a list of devices." << std::endl;

      return __LINE__;
   }

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         delete pOD;

         iNumDevices++;
      }
   }

   delete pEnum;

   std::cout << pszExeName << ": There are " << iNumDevices << " devices." << std::endl;

   if (!iNumDevices)
   {
      std::cout << pszExeName << ": No devices found, aborting ..." << std::endl;

      exit (1);
   }

   /* Allocate the array of devices and initialize each device.
   */
   pohDevices = new OMNIHANDLE [iNumDevices];
   pEnum      = Omni::listDevices (fUseLocalBuild);

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         PSZCRO             pszLibName        = pOD->getLibraryName ();
         PSZCRO             pszJobProperties  = pOD->getJobProperties ();
         PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

         pohDevices[iCurrentDevice].hmodDevice        = g_module_open (pszLibName, (GModuleFlags)0);
         pohDevices[iCurrentDevice].pDevice           = 0;
         pohDevices[iCurrentDevice].pstringDeviceName = 0;
         pohDevices[iCurrentDevice].pstringMakeModel  = 0;
         pohDevices[iCurrentDevice].eOmniClass        = OMNI_CLASS_UNKNOWN;

         if (pohDevices[iCurrentDevice].hmodDevice)
         {
            g_module_symbol (pohDevices[iCurrentDevice].hmodDevice,
                             "newDeviceW_JopProp_Advanced",
                             (gpointer *)&pfnNewDeviceWArgs);

            if (pfnNewDeviceWArgs)
            {
               Device *pDevice = 0;

               pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

               if (pDevice)
               {
                  std::string devName     = pDevice->getDeviceName ();
                  bool        fErrorFound = false;

                  if (!pDevice->getVersion ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No version for " << devName << std::endl;
                  }
                  if (!pDevice->getShortName ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No short name for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentOrientation ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current orientation for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentForm ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current form for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentTray ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current tray for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentMedia ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current media for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentResolution ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current resolution for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentPrintMode ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current print mode for " << devName << std::endl;
                  }
                  if (!pDevice->getCurrentDitherID ())
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No current dither id for " << devName << std::endl;
                  }
                  Enumeration *pEnum = pDevice->listJobProperties ();
                  if (!pEnum)
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No job properties for " << devName << std::endl;
                  }
                  else
                  {
                     delete pEnum;
                  }
                  std::string *pstringDeviceName = getDeviceName (pDevice, &DB, 0);
                  if (!pstringDeviceName)
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No device name " << devName << std::endl;
                  }
                  else
                  {
                     delete pstringDeviceName;
                  }
                  pstringDeviceName = getDeviceName (pDevice, &DB, 1);
                  if (!pstringDeviceName)
                  {
                     fErrorFound = true;
                     std::cerr << "Error: No make/model " << devName << std::endl;
                  }
                  else
                  {
                     delete pstringDeviceName;
                  }

                  if (fErrorFound)
                  {
                     delete pDevice;
                     pDevice = 0;
                  }
               }

               if (pDevice)
               {
                  pohDevices[iCurrentDevice].pDevice           = pDevice;
                  pohDevices[iCurrentDevice].pstringDeviceName = getDeviceName (pDevice, &DB, 0);
                  pohDevices[iCurrentDevice].pstringMakeModel  = getDeviceName (pDevice, &DB, 1);
                  pohDevices[iCurrentDevice].eOmniClass        = pDevice->getOmniClass ();

                  afOmniClassExists[pohDevices[iCurrentDevice].eOmniClass] = true;

                  std::string devName   = pDevice->getDeviceName ();
                  std::string cleanname = *getCleanName (&devName, 0);

                  std::cout << pszExeName
                            << ": Initialized "
                            << cleanname
                            << " ("
                            << iCurrentDevice + 1
                            << "/"
                            << iNumDevices
                            << ")"
                            << std::endl;
               }
               else
               {
                  std::cerr << "Error: Could not instantiate " << pszLibName << " with \"" << pszJobProperties << "\"." << std::endl;
               }
            }
            else
            {
               std::cerr << "Error: Could not find newDeviceW_JopProp_Advanced in \"" << pszLibName << "\"." << std::endl;
            }
         }
         else
         {
            std::cerr << "Error: Could not load \"" << pszLibName << "\"." << std::endl;
         }

         delete pOD;

         iCurrentDevice++;
      }
   }

   delete pEnum;

   /* Create individual printer XML files.
   */
   for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
   {
      Device          *pDevice           = pohDevices[iCurrentDevice].pDevice;

      if (!pDevice)
      {
         // Error!
         continue;
      }

      DevicePrintMode *pDPM              = pDevice->getCurrentPrintMode ();
      std::string     *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;
      std::string     *pstringMakeModel  = pohDevices[iCurrentDevice].pstringMakeModel;

      // Produce only printer XML files which Foomatic does not already
      // provide.

      if (  !pstringMakeModel
         || pstringMakeModel->empty ()
         )
      {
         std::cerr << pszExeName
                   << ": Warning \""
                   << pDevice->getDriverName ()
                   << "."
                   << pDevice->getDeviceName ()
                   << "\" MakeModel is empty!"
                   << std::endl;

         continue;
      }

      if (  !pstringDeviceName
         || pstringDeviceName->empty ()
         )
      {
         std::cerr << pszExeName
                   << ": Error \""
                   << pDevice->getDriverName ()
                   << "."
                   << pDevice->getDeviceName ()
                   << "\" DeviceName is empty!"
                   << std::endl;

         continue;
      }

      oss.str ("");
      oss << basedir
          << "printer/"
          << *pstringDeviceName
          << ".xml"
          << std::ends;

      std::ofstream ofstreamPrinter (oss.str ().c_str ());

      pofstream = &ofstreamPrinter;

      std::cout << pszExeName << ": Generating " << oss.str () << std::endl;

      *pofstream << "<printer id=\"printer/" << *pstringDeviceName << "\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      std::string::size_type idx   = pstringMakeModel->find_first_of ('|',0);
      std::string            make  = pstringMakeModel->substr (0, idx);
      std::string            model = pstringMakeModel->substr (idx+1);

      *pofstream << "  <make>" << make << "</make>" << std::endl;
      *pofstream << "  <model>" << model << "</model>" << std::endl;
      *pofstream << "  <!-- " << pDevice->getShortName () << " -->" << std::endl;
      *pofstream << "  <mechanism>" << std::endl;

      // @TBD
      if (true)
      {
         // Is the default print mode monochrome? Then we don't have a color
         // printer
         if (DevicePrintMode::COLOR_TECH_K != pDPM->getColorTech ())
         {
            // Color printer
            *pofstream << "    <color/>" << std::endl;
         }
      }
      else
      {
         // Do we have color print modes? If yes, this is a color printer
         Enumeration *pEnum       = pDPM->getEnumeration ();
         bool         fFoundColor = false;

         while (pEnum->hasMoreElements ())
         {
            DevicePrintMode *pDPMElm = (DevicePrintMode *)pEnum->nextElement ();

            if (DevicePrintMode::COLOR_TECH_K != pDPMElm->getColorTech ())
            {
               fFoundColor = true;
            }

            delete pDPMElm;

            if (fFoundColor)
            {
               break;
            }
         }

         if (fFoundColor)
         {
            // We have found a color mode
            *pofstream << "    <color/>" << std::endl;
         }

         delete pEnum;
      }

      *pofstream << "    <resolution>" << std::endl;
      *pofstream << "      <dpi>" << std::endl;
      //  Added by raharper@us.ibm.com, get the resolution for each device
      *pofstream << "        <x>" << pDevice->getCurrentResolution ()->getXRes () << "</x>" << std::endl;
      *pofstream << "        <y>" << pDevice->getCurrentResolution ()->getYRes () << "</y>" << std::endl;
      *pofstream << "      </dpi>" << std::endl;
      *pofstream << "    </resolution>" << std::endl;
      *pofstream << "    <consumables>" << std::endl;
      //  The <consumables> section can be left blank if the consumables
      //  for this printer are not known.
      *pofstream << "      <!-- no \"comments\" -->" << std::endl;
      *pofstream << "    <!--one or more \"partno\" elements.--></consumables>" << std::endl;
      *pofstream << "  </mechanism>" << std::endl;
      //  Added by raharper@us.ibm.com, changed URL
      *pofstream << "  <url>http://www.sourceforge.net/projects/omniprint</url>" << std::endl;
      *pofstream << "  <lang>" << std::endl;
      *pofstream << "    <text>" << std::endl;
      *pofstream << "      <charset>us-ascii</charset>" << std::endl;
      *pofstream << "    </text>" << std::endl;
      *pofstream << "  </lang>" << std::endl;
      *pofstream << "  <autodetect>" << std::endl;
      *pofstream << "    <!--no known parport probe information-->" << std::endl;
      *pofstream << "  </autodetect>" << std::endl;
      *pofstream << "  <functionality>D</functionality>" << std::endl;
      *pofstream << "  <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
      *pofstream << "  <unverified/><!--no \"contrib_url\"-->" << std::endl;
      *pofstream << "  <comments>" << std::endl;
      *pofstream << "    <en>" << std::endl;
      *pofstream << "    Foomatic entry automatically generated by Omni.&lt;p&gt;" << std::endl;
      *pofstream << "    Printer type and maximum resolution information" << std::endl;
      *pofstream << "    may be incorrect.&lt;p&gt;" << std::endl;
      *pofstream << "    The rating &quot;Partially&quot; is given due to the Omni code still" << std::endl;
      *pofstream << "    being under development and not supporting all features of the" << std::endl;
      *pofstream << "    supported printer models.&lt;p&gt;" << std::endl;
      *pofstream << "    </en>" << std::endl;
      *pofstream << "  </comments>" << std::endl;
      *pofstream << "</printer>" << std::endl;
   }

   /**
    * Create omni-compiled.xml
    *
    * list of device names
    */
   if (afOmniClassExists[OMNI_CLASS_COMPILED])
   {
      oss.str (""); oss << basedir << "driver/omni-compiled.xml" << std::ends;

      std::ofstream ofstreamOmniCompiled (oss.str ().c_str ());

      pofstream = &ofstreamOmniCompiled;

      std::cout << pszExeName << ": Generating " << basedir << "driver/omni-compiled.xml" << std::endl;

      *pofstream << "<driver id=\"driver/omni-compiled\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << " <name>omni-compiled</name>" << std::endl;
      *pofstream << " <url>http://www.ibm.com/linux/ltc/projects/omni/</url>" << std::endl;
      *pofstream << " <execution>" << std::endl;
      *pofstream << "  <ghostscript />" << std::endl;
      // Edited by raharper@us.ibm.com, changed the prototype to what omni wants it to look like
      *pofstream << "  <prototype>gs -q -dPARANOIDSAFER -dBATCH -dNOPAUSE -sOutputFile=- -sDEVICE=omni%B -sproperties=&quot;%A&quot;%Z -</prototype>" << std::endl;
      *pofstream << " </execution>" << std::endl;
      *pofstream << " <comments>" << std::endl;
      *pofstream << "  <en>" << std::endl;
      *pofstream << std::endl;
      *pofstream << "    &lt;b&gt;";
      *pofstream << "This page provides data for the Omni driver version ";

      bool fFound = false;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pDevice)
         {
            fFound = true;
            *pofstream << pohDevices[iCurrentDevice].pDevice->getVersion ();
            break;
         }
      }
      if (!fFound)
      {
         *pofstream << "Unknown";
      }

      *pofstream << "." << std::endl;
      *pofstream << "    &lt;/b&gt;&lt;p&gt;" << std::endl;
      *pofstream << std::endl;
      *pofstream << "    The Omni printer driver provides support for over 400 printers " << std::endl;
      *pofstream << "    using the Ghostscript framework. In addition, it provides a " << std::endl;
      *pofstream << "    model for dynamically loading printer drivers, creating new " << std::endl;
      *pofstream << "    devices by editing device description files, and simplifies " << std::endl;
      *pofstream << "    new printer driver development by allowing for the subclassing " << std::endl;
      *pofstream << "    of previous device features.&lt;p&gt;" << std::endl;
      *pofstream << std::endl;
      *pofstream << "  </en>" << std::endl;
      *pofstream << " </comments>" << std::endl;
      *pofstream << " <printers>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_COMPILED)
         {
            // Skip!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         *pofstream << "  <printer><id>printer/" << *pstringDeviceName << "</id></printer>" << std::endl;
      }

      *pofstream << " </printers>" << std::endl;
      *pofstream << "</driver>" << std::endl;
   }

   /**
    * Create omni-xml.xml
    *
    * list of device names
    */
   if (afOmniClassExists[OMNI_CLASS_XML])
   {
      oss.str (""); oss << basedir << "driver/omni-xml.xml" << std::ends;

      std::ofstream ofstreamOmniXML (oss.str ().c_str ());

      pofstream = &ofstreamOmniXML;

      std::cout << pszExeName << ": Generating " << basedir << "driver/omni-xml.xml" << std::endl;

      *pofstream << "<driver id=\"driver/omni-xml\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << " <name>omni-xml</name>" << std::endl;
      *pofstream << " <url>http://www.ibm.com/linux/ltc/projects/omni/</url>" << std::endl;
      *pofstream << " <execution>" << std::endl;
      *pofstream << "  <ghostscript />" << std::endl;
      // Edited by raharper@us.ibm.com, changed the prototype to what omni wants it to look like
      *pofstream << "  <prototype>gs -q -dPARANOIDSAFER -dBATCH -dNOPAUSE -sOutputFile=- -sDEVICE=omni%B -sproperties=&quot;%A&quot;%Z -</prototype>" << std::endl;
      *pofstream << " </execution>" << std::endl;
      *pofstream << " <comments>" << std::endl;
      *pofstream << "  <en>" << std::endl;
      *pofstream << std::endl;
      *pofstream << "    &lt;b&gt;";
      *pofstream << "This page provides data for the Omni driver version ";

      bool fFound = false;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pDevice)
         {
            fFound = true;
            *pofstream << pohDevices[iCurrentDevice].pDevice->getVersion ();
            break;
         }
      }
      if (!fFound)
      {
         *pofstream << "Unknown";
      }

      *pofstream << "." << std::endl;
      *pofstream << "    &lt;/b&gt;&lt;p&gt;" << std::endl;
      *pofstream << std::endl;
      *pofstream << "    The Omni printer driver provides support for over 400 printers " << std::endl;
      *pofstream << "    using the Ghostscript framework. In addition, it provides a " << std::endl;
      *pofstream << "    model for dynamically loading printer drivers, creating new " << std::endl;
      *pofstream << "    devices by editing device description files, and simplifies " << std::endl;
      *pofstream << "    new printer driver development by allowing for the subclassing " << std::endl;
      *pofstream << "    of previous device features.&lt;p&gt;" << std::endl;
      *pofstream << std::endl;
      *pofstream << "  </en>" << std::endl;
      *pofstream << " </comments>" << std::endl;
      *pofstream << " <printers>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_XML)
         {
            // Skip!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         *pofstream << "  <printer><id>printer/" << *pstringDeviceName << "</id></printer>" << std::endl;
      }

      *pofstream << " </printers>" << std::endl;
      *pofstream << "</driver>" << std::endl;
   }

   /**
    * Create omni-updf.xml
    *
    * list of device names
    */
   if (afOmniClassExists[OMNI_CLASS_UPDF])
   {
      oss.str (""); oss << basedir << "driver/omni-updf.xml" << std::ends;

      std::ofstream ofstreamOmniUPDF (oss.str ().c_str ());

      pofstream = &ofstreamOmniUPDF;

      std::cout << pszExeName << ": Generating " << basedir << "driver/omni-updf.xml" << std::endl;

      *pofstream << "<driver id=\"driver/omni-updf\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << " <name>omni-updf</name>" << std::endl;
      *pofstream << " <url>http://www.ibm.com/linux/ltc/projects/omni/</url>" << std::endl;
      *pofstream << " <execution>" << std::endl;
      *pofstream << "  <ghostscript />" << std::endl;
      // Edited by raharper@us.ibm.com, changed the prototype to what omni wants it to look like
      *pofstream << "  <prototype>gs -q -dPARANOIDSAFER -dBATCH -dNOPAUSE -sOutputFile=- -sDEVICE=omni%B -sproperties=&quot;%A&quot;%Z -</prototype>" << std::endl;
      *pofstream << " </execution>" << std::endl;
      *pofstream << " <comments>" << std::endl;
      *pofstream << "  <en>" << std::endl;
      *pofstream << std::endl;
      *pofstream << "    &lt;b&gt;";
      *pofstream << "This page provides data for the Omni driver version ";

      bool fFound = false;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pDevice)
         {
            fFound = true;
            *pofstream << pohDevices[iCurrentDevice].pDevice->getVersion ();
            break;
         }
      }
      if (!fFound)
      {
         *pofstream << "Unknown";
      }

      *pofstream << "." << std::endl;
      *pofstream << "    &lt;/b&gt;&lt;p&gt;" << std::endl;
      *pofstream << std::endl;
      *pofstream << "    The Omni printer driver provides support for over 400 printers " << std::endl;
      *pofstream << "    using the Ghostscript framework. In addition, it provides a " << std::endl;
      *pofstream << "    model for dynamically loading printer drivers, creating new " << std::endl;
      *pofstream << "    devices by editing device description files, and simplifies " << std::endl;
      *pofstream << "    new printer driver development by allowing for the subclassing " << std::endl;
      *pofstream << "    of previous device features.&lt;p&gt;" << std::endl;
      *pofstream << std::endl;
      *pofstream << "  </en>" << std::endl;
      *pofstream << " </comments>" << std::endl;
      *pofstream << " <printers>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_UPDF)
         {
            // Skip!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         *pofstream << "  <printer><id>printer/" << *pstringDeviceName << "</id></printer>" << std::endl;
      }

      *pofstream << " </printers>" << std::endl;
      *pofstream << "</driver>" << std::endl;
   }

   /**
    * Create omni-compiled-print-model.xml
    *
    * list of device names
    */
   if (afOmniClassExists[OMNI_CLASS_COMPILED])
   {
      oss.str (""); oss << basedir << "opt/omni-compiled-print-model.xml" << std::ends;

      std::ofstream ofstreamPrintModel (oss.str ().c_str ());

      pofstream = &ofstreamPrintModel;

      std::cout << pszExeName << ": Generating " << basedir << "opt/omni-compiled-print-model.xml" << std::endl;

      *pofstream << "<option type=\"enum\" id=\"opt/omni-compiled-print-model\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << "  <!-- A multilingual <comments> block can appear here, too;" << std::endl;
      *pofstream << "       it should be treated as documentation for the user. -->" << std::endl;
      *pofstream << "  <arg_longname>" << std::endl;
      *pofstream << "   <en>Printer Model</en>" << std::endl;
      *pofstream << "  </arg_longname>" << std::endl;
      *pofstream << "  <arg_shortname>" << std::endl;
      *pofstream << "   <en>Model</en><!-- backends only know <en> shortnames! -->" << std::endl;
      *pofstream << "  </arg_shortname>" << std::endl;
      *pofstream << "  <arg_execution>" << std::endl;
      *pofstream << "   <arg_order>10</arg_order>" << std::endl;
      *pofstream << "   <arg_spot>B</arg_spot>" << std::endl;
      *pofstream << "   <arg_required />" << std::endl;
      *pofstream << "   <arg_substitution />" << std::endl;
      *pofstream << "   <arg_proto> -sDeviceName=%s</arg_proto>" << std::endl;
      *pofstream << "  </arg_execution>" << std::endl;
      *pofstream << "" << std::endl;
      *pofstream << "  <constraints>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_COMPILED)
         {
            // Skip!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         *pofstream << "    <constraint sense='true'>" << std::endl;
         *pofstream << "      <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "      <printer>printer/" << *pstringDeviceName << "</printer>" << std::endl;
         *pofstream << "      <arg_defval>" << *pstringDeviceName << "</arg_defval>" << std::endl; // @TBD
         *pofstream << "    </constraint>" << std::endl;
      }

      *pofstream << "  </constraints>" << std::endl;
      *pofstream << std::endl;
      *pofstream << "  <enum_vals>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_COMPILED)
         {
            // Skip!
            continue;
         }

         Device *pDevice = pohDevices[iCurrentDevice].pDevice;

         if (!pDevice)
         {
            // Error!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         //raharper@us.ibm.com took out the ev/omni-
         *pofstream << "    <enum_val id='" << *pstringDeviceName << "'>" << std::endl;
         *pofstream << "      <ev_longname><en>" << *pstringDeviceName << "</en></ev_longname>" << std::endl;
         *pofstream << "      <ev_shortname><en>" << pDevice->getShortName () << "</en></ev_shortname>" << std::endl;
         *pofstream << "      <ev_driverval>" << pDevice->getShortName () << "</ev_driverval>" << std::endl;
         *pofstream << "      <constraints>" << std::endl;
         *pofstream << "        <!-- Assume the option doesn't apply... -->" << std::endl;
         *pofstream << "        <constraint sense='false'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "        <!-- ...except to these: -->" << std::endl;
         *pofstream << "        <constraint sense='true'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "          <printer>printer/" << *pstringDeviceName << "</printer>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "      </constraints>" << std::endl;
         *pofstream << "    </enum_val>" << std::endl;
      }

      *pofstream << "  </enum_vals>" << std::endl;
      *pofstream << "</option>" << std::endl;
   }

   /**
    * Create omni-print-model.xml
    *
    * list of device names
    */
   if (afOmniClassExists[OMNI_CLASS_XML])
   {
      oss.str (""); oss << basedir << "opt/omni-print-model.xml" << std::ends;

      std::ofstream ofstreamPrintModel (oss.str ().c_str ());

      pofstream = &ofstreamPrintModel;

      std::cout << pszExeName << ": Generating " << basedir << "opt/omni-print-model.xml" << std::endl;

      *pofstream << "<option type=\"enum\" id=\"opt/omni-print-model\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << "  <!-- A multilingual <comments> block can appear here, too;" << std::endl;
      *pofstream << "       it should be treated as documentation for the user. -->" << std::endl;
      *pofstream << "  <arg_longname>" << std::endl;
      *pofstream << "   <en>Printer Model</en>" << std::endl;
      *pofstream << "  </arg_longname>" << std::endl;
      *pofstream << "  <arg_shortname>" << std::endl;
      *pofstream << "   <en>Model</en><!-- backends only know <en> shortnames! -->" << std::endl;
      *pofstream << "  </arg_shortname>" << std::endl;
      *pofstream << "  <arg_execution>" << std::endl;
      *pofstream << "   <arg_order>10</arg_order>" << std::endl;
      *pofstream << "   <arg_spot>B</arg_spot>" << std::endl;
      *pofstream << "   <arg_required />" << std::endl;
      *pofstream << "   <arg_substitution />" << std::endl;
      *pofstream << "   <arg_proto> -sDeviceName=%s</arg_proto>" << std::endl;
      *pofstream << "  </arg_execution>" << std::endl;
      *pofstream << "" << std::endl;
      *pofstream << "  <constraints>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_XML)
         {
            // Skip!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         *pofstream << "    <constraint sense='true'>" << std::endl;
         *pofstream << "      <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "      <printer>printer/" << *pstringDeviceName << "</printer>" << std::endl;
         *pofstream << "      <arg_defval>" << *pstringDeviceName << "</arg_defval>" << std::endl; // @TBD
         *pofstream << "    </constraint>" << std::endl;
      }

      *pofstream << "  </constraints>" << std::endl;
      *pofstream << std::endl;
      *pofstream << "  <enum_vals>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_XML)
         {
            // Skip!
            continue;
         }

         Device *pDevice = pohDevices[iCurrentDevice].pDevice;

         if (!pDevice)
         {
            // Error!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         //raharper@us.ibm.com took out the ev/omni-
         *pofstream << "    <enum_val id='" << *pstringDeviceName << "'>" << std::endl;
         *pofstream << "      <ev_longname><en>" << *pstringDeviceName << "</en></ev_longname>" << std::endl;
         *pofstream << "      <ev_shortname><en>" << pDevice->getLibraryName () << "</en></ev_shortname>" << std::endl;
         *pofstream << "      <ev_driverval>" << pDevice->getLibraryName () << "</ev_driverval>" << std::endl;
         *pofstream << "      <constraints>" << std::endl;
         *pofstream << "        <!-- Assume the option doesn't apply... -->" << std::endl;
         *pofstream << "        <constraint sense='false'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "        <!-- ...except to these: -->" << std::endl;
         *pofstream << "        <constraint sense='true'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "          <printer>printer/" << *pstringDeviceName << "</printer>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "      </constraints>" << std::endl;
         *pofstream << "    </enum_val>" << std::endl;
      }

      *pofstream << "  </enum_vals>" << std::endl;
      *pofstream << "</option>" << std::endl;
   }

   /**
    * Create omni-updf-print-model.xml
    *
    * list of device names
    */
   if (afOmniClassExists[OMNI_CLASS_UPDF])
   {
      oss.str (""); oss << basedir << "opt/omni-updf-print-model.xml" << std::ends;

      std::ofstream ofstreamPrintModel (oss.str ().c_str ());

      pofstream = &ofstreamPrintModel;

      std::cout << pszExeName << ": Generating " << basedir << "opt/omni-updf-print-model.xml" << std::endl;

      *pofstream << "<option type=\"enum\" id=\"opt/omni-updf-print-model\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << "  <!-- A multilingual <comments> block can appear here, too;" << std::endl;
      *pofstream << "       it should be treated as documentation for the user. -->" << std::endl;
      *pofstream << "  <arg_longname>" << std::endl;
      *pofstream << "   <en>Printer Model</en>" << std::endl;
      *pofstream << "  </arg_longname>" << std::endl;
      *pofstream << "  <arg_shortname>" << std::endl;
      *pofstream << "   <en>Model</en><!-- backends only know <en> shortnames! -->" << std::endl;
      *pofstream << "  </arg_shortname>" << std::endl;
      *pofstream << "  <arg_execution>" << std::endl;
      *pofstream << "   <arg_order>10</arg_order>" << std::endl;
      *pofstream << "   <arg_spot>B</arg_spot>" << std::endl;
      *pofstream << "   <arg_required />" << std::endl;
      *pofstream << "   <arg_substitution />" << std::endl;
      *pofstream << "   <arg_proto> -sDeviceName=%s</arg_proto>" << std::endl;
      *pofstream << "  </arg_execution>" << std::endl;
      *pofstream << "" << std::endl;
      *pofstream << "  <constraints>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_UPDF)
         {
            // Skip!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         *pofstream << "    <constraint sense='true'>" << std::endl;
         *pofstream << "      <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "      <printer>printer/" << *pstringDeviceName << "</printer>" << std::endl;
         *pofstream << "      <arg_defval>" << *pstringDeviceName << "</arg_defval>" << std::endl; // @TBD
         *pofstream << "    </constraint>" << std::endl;
      }

      *pofstream << "  </constraints>" << std::endl;
      *pofstream << std::endl;
      *pofstream << "  <enum_vals>" << std::endl;

      for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
      {
         if (!pohDevices[iCurrentDevice].pstringDeviceName)
         {
            // Error!
            continue;
         }
         if (pohDevices[iCurrentDevice].eOmniClass != OMNI_CLASS_UPDF)
         {
            // Skip!
            continue;
         }

         Device *pDevice = pohDevices[iCurrentDevice].pDevice;

         if (!pDevice)
         {
            // Error!
            continue;
         }

         std::string *pstringDeviceName = pohDevices[iCurrentDevice].pstringDeviceName;

         //raharper@us.ibm.com took out the ev/omni-
         *pofstream << "    <enum_val id='" << *pstringDeviceName << "'>" << std::endl;
         *pofstream << "      <ev_longname><en>" << *pstringDeviceName << "</en></ev_longname>" << std::endl;
         *pofstream << "      <ev_shortname><en>" << pDevice->getLibraryName () << "</en></ev_shortname>" << std::endl;
         *pofstream << "      <ev_driverval>" << pDevice->getLibraryName () << "</ev_driverval>" << std::endl;
         *pofstream << "      <constraints>" << std::endl;
         *pofstream << "        <!-- Assume the option doesn't apply... -->" << std::endl;
         *pofstream << "        <constraint sense='false'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "        <!-- ...except to these: -->" << std::endl;
         *pofstream << "        <constraint sense='true'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass) << "</driver>" << std::endl;
         *pofstream << "          <printer>printer/" << *pstringDeviceName << "</printer>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "      </constraints>" << std::endl;
         *pofstream << "    </enum_val>" << std::endl;
      }

      *pofstream << "  </enum_vals>" << std::endl;
      *pofstream << "</option>" << std::endl;
   }

   /* Generate all of the option files
   */
   // Loop through all devices
   for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
   {
      if (!pohDevices[iCurrentDevice].pstringDeviceName)
      {
         // Error!
         continue;
      }

      Device      *pDevice       = pohDevices[iCurrentDevice].pDevice;
      Enumeration *pEnumJPGroups = 0;
      Enumeration *pEnumJPGroup  = 0;

      // List all job properties for the device
      pEnumJPGroups = pDevice->listJobProperties (false);

      if (!pEnumJPGroups)
      {
         std::cerr << "Error: listJobProperties returns NULL!" << std::endl;

         continue;
      }

      while (  pEnumJPGroups
            && pEnumJPGroups->hasMoreElements ()
            )
      {
         // Get the next job property group
         pEnumJPGroup = (Enumeration *)pEnumJPGroups->nextElement ();

         while (pEnumJPGroup->hasMoreElements ())
         {
            JobProperties         *pJPs                       = 0;
            JobPropertyEnumerator *pEnumJPs                   = 0;
            std::string            stringDefaultJobProperties;
            std::string            stringKeysValues;
            std::string            stringKeys;
            std::string            stringXlatedKeysValues;
            bool                   fMultiJobProperties        = false;

            // Get the job properties in the group
            pJPs = (JobProperties *)pEnumJPGroup->nextElement ();
            if (pJPs)
            {
               pEnumJPs = pJPs->getEnumeration ();
            }
            else
            {
               std::cerr << "Error: no job properties returned from enumeration" << std::endl;
            }

            while (  pEnumJPs
                  && pEnumJPs->hasMoreElements ()
                  )
            {
               PSZRO        pszKey         = pEnumJPs->getCurrentKey   ();
               PSZRO        pszValue       = pEnumJPs->getCurrentValue ();
               std::string *pstringDefault = 0;
               std::string *pstringXLate   = 0;
               bool         fFoundSpace    = false;

               if (strchr (pszValue, ' '))
               {
                  fFoundSpace = true;
               }

               // @HACK Foomatic needs something called PageSize or it will
               //       insert fake forms into the PPD file!
               if (0 == strcmp (pszKey, "Form"))
               {
                  stringKeys += "PageSize";
               }
               else
               {
                  // Create a key only string (Key1Key2...KeyN)
                  stringKeys += pszKey;
               }

               // Create a key=value string (Key1=Value1 Key2=Value2 ... KeyN=ValueN)
               if (stringKeysValues[0])
               {
                  stringKeysValues += " ";
               }
               stringKeysValues += pszKey;
               stringKeysValues += "=";
               if (fFoundSpace)
               {
                  stringKeysValues += "\"";
               }
               stringKeysValues += pszValue;
               if (fFoundSpace)
               {
                  stringKeysValues += "\"";
               }

               // Create a default job properties string
               pstringDefault = pDevice->getJobProperty (pszKey);

               if (pstringDefault)
               {
                  if (stringDefaultJobProperties[0])
                  {
                     stringDefaultJobProperties += " ";
                  }
                  stringDefaultJobProperties += pszKey;
                  stringDefaultJobProperties += "=";
                  if (fFoundSpace)
                  {
                     stringDefaultJobProperties += "\"";
                  }
                  stringDefaultJobProperties += *pstringDefault;
                  if (fFoundSpace)
                  {
                     stringDefaultJobProperties += "\"";
                  }

                  delete pstringDefault;
               }

#if 0
               // Create a translated key=value string
               pstringXLate = pDevice->translateKeyValue (pszKey, pszValue);

               if (pstringXLate)
               {
                  if (stringXlatedKeysValues[0])
                  {
                     stringXlatedKeysValues += " ";
                  }
                  stringXlatedKeysValues += *pstringXLate;

                  delete pstringXLate;
               }

               pEnumJPs->nextElement ();
#else
               pEnumJPs->nextElement ();
               if (!fMultiJobProperties)
                  fMultiJobProperties = pEnumJPs->hasMoreElements ();

               // Create a translated key=value string
               pstringXLate = pDevice->translateKeyValue (pszKey, pszValue);

               if (pstringXLate)
               {
                  if (stringXlatedKeysValues[0])
                  {
                     stringXlatedKeysValues += " ";
                  }

                  if (fMultiJobProperties)
                  {
                     stringXlatedKeysValues += *pstringXLate;
                  }
                  else
                  {
                     std::string::size_type pos = pstringXLate->find ("=");

                     if (pos != std::string::npos)
                     {
                        stringXlatedKeysValues += pstringXLate->substr (pos + 1);
                     }
                     else
                     {
                        std::cerr << "Error: pstringXLate = "
                                  << *pstringXLate
                                  << ", pszKey = "
                                  << pszKey
                                  << ", pszValue = "
                                  << pszValue
                                  << std::endl;
                     }
                  }

                  delete pstringXLate;
               }
#endif
            }

            delete pEnumJPs;
            delete pJPs;

            // Get a KeyValue map
            pMapKeyValue = keyMap[stringKeys];

            if (!pMapKeyValue)
            {
               pMapKeyValue       = new MapKeyValue ();
               keyMap[stringKeys] = pMapKeyValue;
            }

            if (pMapKeyValue)
            {
               // Get a MapKeyValue element
               pMKVE = (*pMapKeyValue)[stringKeysValues];

               if (!pMKVE)
               {
                  pMKVE = (PMKVE)calloc (1, sizeof (MKVE));

                  if (pMKVE)
                  {
                     pMKVE->pDeviceList                = new MapDeviceList ();
                     pMKVE->pstringKeyValueTranslation = new std::string (stringXlatedKeysValues);
                     (*pMapKeyValue)[stringKeysValues] = pMKVE;
                  }
               }

               if (pMKVE)
               {
                  MapKeyValue::iterator elm  = pMapKeyValue->find (stringKeysValues);
                  PDLE                  pDLE = (PDLE)calloc (1, sizeof (DLE));

                  // Add the element to the device list
                  if (pDLE)
                  {
                     pDLE->pstringCurrentJobProperties = &elm->first;
                     pDLE->pstringDefaultJobProperties = new std::string (stringDefaultJobProperties);
                     pDLE->iDevice                     = iCurrentDevice;
                     pDLE->pszFoomaticDriver           = getFoomaticDriver (pohDevices[iCurrentDevice].eOmniClass);

                     (*pMKVE->pDeviceList)[*pohDevices[iCurrentDevice].pstringDeviceName] = pDLE;
                  }
               }
            }
         }

         delete pEnumJPGroup;
      }
   }

///#ifndef RETAIL
///      std::cout << "================================" << std::endl;
///#endif

   for ( MapKey::iterator nextKey = keyMap.begin ();
         nextKey != keyMap.end ();
         nextKey++ )
   {
      MapDeviceList   uniqueDeviceList;
      MapKeyValue    *pMapKeyValue       = nextKey->second;
      std::string    *pstringTranslation = 0;

///#ifndef RETAIL
///         std::cout << nextKey->first << std::endl;
///#endif

      oss.str ("");
      oss << basedir
          << "opt/omni-"
          << nextKey->first
          << ".xml"
          << std::ends;

      std::ofstream ofstreamOption (oss.str ().c_str ());

      pofstream = &ofstreamOption;

      std::cout << pszExeName << ": Generating " << oss.str () << std::endl;

      *pofstream << "<option type=\"enum\" id=\"opt/omni-" << nextKey->first << "\">" << std::endl;

      if (pofstream->bad ())
      {
         std::cerr << pszExeName << ": Error: could not write to file." << std::endl;

         iRc = __LINE__;
         goto done;
      }

      *pofstream << "  <!-- A multilingual <comments> block can appear here, too;" << std::endl;
      *pofstream << "       it should be treated as documentation for the user. -->" << std::endl;
      *pofstream << "  <arg_longname>" << std::endl;

      pstringTranslation = translateKeyValueList (pMapKeyValue, pohDevices);

      *pofstream << "   <en>";
      if (pstringTranslation)
      {
         *pofstream << *pstringTranslation;

         delete pstringTranslation;
      }
      else
      {
         *pofstream << "Error";
      }
      *pofstream << "</en>" << std::endl;

      *pofstream << "  </arg_longname>" << std::endl;
      *pofstream << "  <arg_shortname>" << std::endl;
      *pofstream << "   <en>" << nextKey->first << "</en><!-- backends only know <en> shortnames! -->" << std::endl;
      *pofstream << "  </arg_shortname>" << std::endl;
      *pofstream << "  <arg_execution>" << std::endl;
      *pofstream << "   <arg_order>" << iArgOrder << "</arg_order>" << std::endl;
      iArgOrder += 10;
      *pofstream << "   <arg_spot>A</arg_spot>" << std::endl;
      // @HACK
      if (0 == strcmp (nextKey->first.c_str (), "XMLMasterFile"))
      {
         *pofstream << "   <arg_required />" << std::endl;
      }
      *pofstream << "   <arg_substitution />" << std::endl;
      *pofstream << "   <arg_proto>%s </arg_proto>" << std::endl;
      *pofstream << "  </arg_execution>" << std::endl;
      *pofstream << std::endl;

///#ifndef RETAIL
///         for ( MapKeyValue::iterator nextKeyValue = pMapKeyValue->begin ();
///               nextKeyValue != pMapKeyValue->end ();
///               nextKeyValue++ )
///         {
///            PMKVE          pMKVE       = nextKeyValue->second;
///            MapDeviceList *pDeviceList = pMKVE->pDeviceList;
///
///            std::cout << "   " << nextKeyValue->first << std::endl;
///
///            for ( MapDeviceList::iterator nextDeviceList = pDeviceList->begin ();
///                  nextDeviceList != pDeviceList->end ();
///                  nextDeviceList++ )
///            {
///               std::cout << "      " << nextDeviceList->first << std::endl;
///            }
///         }
///#endif

      // Create a list of unique devices for a MapKey value
      for ( MapKeyValue::iterator nextKeyValue = pMapKeyValue->begin ();
            nextKeyValue != pMapKeyValue->end ();
            nextKeyValue++ )
      {
         PMKVE          pMKVE       = nextKeyValue->second;
         MapDeviceList *pDeviceList = pMKVE->pDeviceList;

         for ( MapDeviceList::iterator nextDeviceList = pDeviceList->begin ();
               nextDeviceList != pDeviceList->end ();
               nextDeviceList++ )
         {
            uniqueDeviceList[nextDeviceList->first] = nextDeviceList->second;
         }
      }

      // Put in the default values for printers
      *pofstream << "  <constraints>" << std::endl;

      for ( MapDeviceList::iterator nextUniqueDevice = uniqueDeviceList.begin ();
            nextUniqueDevice != uniqueDeviceList.end ();
            nextUniqueDevice++ )
      {
         PDLE pDLE = nextUniqueDevice->second;

         if (pDLE)
         {
            *pofstream << "    <constraint sense='true'>" << std::endl;
            *pofstream << "      <driver>" << getFoomaticDriver (pohDevices[pDLE->iDevice].eOmniClass) << "</driver>" << std::endl;
            *pofstream << "      <printer>printer/" << nextUniqueDevice->first << "</printer>" << std::endl;
            *pofstream << "      <arg_defval>" << *pDLE->pstringDefaultJobProperties << "</arg_defval>" << std::endl;
            *pofstream << "    </constraint>" << std::endl;
         }
      }

      *pofstream << "  </constraints>" << std::endl;

      // Put in all of the options for the printers
      *pofstream << std::endl;
      *pofstream << "  <enum_vals>" << std::endl;

      for ( MapKeyValue::iterator nextKeyValue = pMapKeyValue->begin ();
            nextKeyValue != pMapKeyValue->end ();
            nextKeyValue++ )
      {
         PMKVE          pMKVE       = nextKeyValue->second;
         MapDeviceList *pDeviceList = pMKVE->pDeviceList;

         *pofstream << "    <enum_val id='" << nextKeyValue->first << "'>" << std::endl;
         *pofstream << "      <ev_longname><en>" << *pMKVE->pstringKeyValueTranslation << "</en></ev_longname>" << std::endl;
         *pofstream << "      <ev_shortname><en>" << nextKeyValue->first << "</en></ev_shortname>" << std::endl;
         *pofstream << "      <ev_driverval>" << nextKeyValue->first << "</ev_driverval>" << std::endl;
         *pofstream << "      <constraints>" << std::endl;
         *pofstream << "        <!-- Assume the option doesn't apply... -->" << std::endl;
         *pofstream << "        <constraint sense='false'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (OMNI_CLASS_COMPILED) << "</driver>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "        <constraint sense='false'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (OMNI_CLASS_XML) << "</driver>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "        <constraint sense='false'>" << std::endl;
         *pofstream << "          <driver>" << getFoomaticDriver (OMNI_CLASS_UPDF) << "</driver>" << std::endl;
         *pofstream << "        </constraint>" << std::endl;
         *pofstream << "        <!-- ...except to these: -->" << std::endl;

         for ( MapDeviceList::iterator nextDeviceList = pDeviceList->begin ();
               nextDeviceList != pDeviceList->end ();
               nextDeviceList++ )
         {
            PDLE pDLE = nextDeviceList->second;

            *pofstream << "        <constraint sense='true'>" << std::endl;
            *pofstream << "          <driver>" << getFoomaticDriver (pohDevices[pDLE->iDevice].eOmniClass) << "</driver>" << std::endl;
            *pofstream << "          <printer>printer/" << nextDeviceList->first << "</printer>" << std::endl;
            *pofstream << "        </constraint>" << std::endl;
         }

         *pofstream << "      </constraints>" << std::endl;
         *pofstream << "    </enum_val>" << std::endl;
      }

      *pofstream << "  </enum_vals>" << std::endl;
      *pofstream << "</option>" << std::endl;
   }

   // Clean up!
   for ( MapKey::iterator nextKey = keyMap.begin ();
         nextKey != keyMap.end ();
         nextKey++ )
   {
      MapKeyValue *pMapKeyValue = nextKey->second;

      for ( MapKeyValue::iterator nextKeyValue = pMapKeyValue->begin ();
            nextKeyValue != pMapKeyValue->end ();
            nextKeyValue++ )
      {
         PMKVE          pMKVE       = nextKeyValue->second;
         MapDeviceList *pDeviceList = pMKVE->pDeviceList;

         for ( MapDeviceList::iterator nextDeviceList = pDeviceList->begin ();
               nextDeviceList != pDeviceList->end ();
               nextDeviceList++ )
         {
            PDLE pDLE = nextDeviceList->second;

            if (pDLE)
            {
               delete pDLE->pstringDefaultJobProperties;

               free ((void *)pDLE);
            }
         }

         delete pDeviceList;
      }

      delete pMapKeyValue;
   }

   iRc = 0;

done:
   // Clean up
   for (iCurrentDevice = 0; iCurrentDevice < iNumDevices; iCurrentDevice++)
   {
      delete pohDevices[iCurrentDevice].pstringDeviceName;
      delete pohDevices[iCurrentDevice].pDevice;
      g_module_close (pohDevices[iCurrentDevice].hmodDevice);
   }

   delete[] pohDevices;

   Omni::terminate ();

   std::cout << pszExeName << ": Finished!" << std::endl;

   return iRc;
}
