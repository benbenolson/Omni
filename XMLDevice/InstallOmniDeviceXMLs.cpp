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
#include <DebugOutput.hpp>

#include "XMLDevice.hpp"
#include "Omni.hpp"

#include <fstream>
#include <string>
#include <sys/stat.h>

void
copyFile (std::string& stringFromFileName,
          std::string& stringToFileName)
{
   const char *pszFromFileName = stringFromFileName.c_str ();
   const char *pszToFileName   = stringToFileName.c_str ();
   struct stat statFile;

   if (-1 == stat (pszToFileName, &statFile))
   {
      std::ifstream ifFileFrom (pszFromFileName, std::ios::in  | std::ios::binary);
      std::ofstream ofFileTo   (pszToFileName,   std::ios::out | std::ios::binary);

      // copy the file in one line o' code!
      ofFileTo << ifFileFrom.rdbuf ();

      // @TBD - need better error detection
      if (-1 == stat (pszToFileName, &statFile))
      {
         std::cerr << "Error: cp "
                   << "\""
                   << stringFromFileName
                   << "\""
                   << " "
                   << "\""
                   << stringToFileName
                   << "\""
                   << std::endl;
      }
      else
      {
         std::cout << "cp "
                   << "\""
                   << stringFromFileName
                   << "\""
                   << " "
                   << "\""
                   << stringToFileName
                   << "\""
                   << std::endl;
      }
   }
}

int
main (int argc, char *argv[])
{
   char         achLine[512]; // @TBD
   PSZRO        pszTargetRoot    = DEFAULT_SHARE_PATH;
   std::string  stringTargetRoot;

   if (1 == argc)
   {
      std::cout << "Usage: " << argv[0] << "[-r root] directory [directory]*" << std::endl;
   }

   for (int i = 1; i < argc; i++)
   {
      if ('-' == argv[i][0])
      {
         switch (argv[i][1])
         {
         case 'r':
         case 'R':
            stringTargetRoot = argv[++i];

            if (  stringTargetRoot.length () > 0
               && stringTargetRoot[stringTargetRoot.length ()-1] != '/'
               )
               stringTargetRoot = stringTargetRoot + "/";

            pszTargetRoot = (char *)stringTargetRoot.c_str ();
            break;
         }
         continue;
      }

      std::string stringFileNameDeviceList ("../");

      stringFileNameDeviceList += argv[i];
      stringFileNameDeviceList += "/Device List";

      std::ifstream ifIn (stringFileNameDeviceList.c_str ());

      while (0 < ifIn.getline (achLine, sizeof (achLine)))
      {
         if (  '#' != achLine[0]
            && sizeof (achLine) - 4 > strlen (achLine)
            )
         {
            std::string stringFileNameMasterXML;
            std::string stringToFileNameXML;
            std::string stringFromFileNameXML;
            XmlDocPtr   docDevice               = 0;
            XmlNodePtr  nodeRoot                = 0;
            XmlNodePtr  nodeElm                 = 0;
            bool        fSuccess                = false;

            stringFileNameMasterXML  = "../";
            stringFileNameMasterXML += argv[i];
            stringFileNameMasterXML += "/";
            stringFileNameMasterXML += achLine;
            stringFileNameMasterXML += ".xml";

            docDevice = XMLParseFile (stringFileNameMasterXML.c_str ());

            if (!docDevice)
               goto done;

            nodeRoot = XMLDocGetRootElement (docDevice);

            if (!nodeRoot)
               goto done;

            nodeElm = XMLFirstNode (nodeRoot);

            if (!nodeElm)
               goto done;

            if (0 != strcmp (XMLGetName (nodeElm), "Device"))
               goto done;

            if (nodeElm)
               nodeElm = XMLFirstNode (XMLGetChildrenNode (nodeElm));

            while (nodeElm)
            {
               if (  0 == strcmp (XMLGetName (nodeElm), "Has")
                  || 0 == strcmp (XMLGetName (nodeElm), "Uses")
                  )
               {
                  PSZRO pszTargetXMLFile = 0;

                  pszTargetXMLFile = XMLNodeListGetString (docDevice,
                                                           XMLGetChildrenNode (nodeElm),
                                                           1);

                  if (pszTargetXMLFile)
                  {
                     XmlDocPtr docFile = 0;

                     stringFromFileNameXML  = "../";
                     stringFromFileNameXML += argv[i];
                     stringFromFileNameXML += "/";
                     stringFromFileNameXML += pszTargetXMLFile;

                     docFile = XMLParseFile (stringFromFileNameXML.c_str ());

                     if (docFile)
                     {
                        stringToFileNameXML  = pszTargetRoot;
                        stringToFileNameXML += pszTargetXMLFile;

                        copyFile (stringFromFileNameXML, stringToFileNameXML);

                        XMLFreeDoc (docFile);
                     }

                     XMLFree ((void *)pszTargetXMLFile);
                  }
               }

               nodeElm = XMLNextNode (nodeElm);
            }

            fSuccess = true;

done:
            if (fSuccess)
            {
               stringToFileNameXML  = pszTargetRoot;
               stringToFileNameXML += achLine;
               stringToFileNameXML += ".xml";

               copyFile (stringFileNameMasterXML, stringToFileNameXML);
            }
            else
            {
               std::cout << "Error: " << stringFileNameMasterXML << std::endl;
            }

            if (docDevice)
            {
               XMLFreeDoc (docDevice);
            }
         }
      }
   }

   return 0;
}
