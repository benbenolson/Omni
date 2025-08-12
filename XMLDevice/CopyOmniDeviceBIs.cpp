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

#include <fstream>
#include <string>
#include <list>
#include <map>
#include <ostream>

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef std::list <std::string>      FileList;
typedef std::map <std::string, bool> SeenList;

static bool fDebugOutput = false;

typedef void (*PFNHANDLER) (std::string&    stringFromFileNameXML,
                            std::string&    stringToFileNameXML,
                            PSZCRO          pszTargetXMLFile,
                            FileList&       fileList);

static int
my_system (const char *pszCommand)
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

      argv[0] = const_cast<char*>("sh");
      argv[1] = const_cast<char*>("-c");
      argv[2] = (char *)pszCommand;
      argv[3] = 0;

      execvp ("/bin/sh", argv);

      exit (1);
   }
}

void
convertFilename (char *pszFileName)
{
   while (*pszFileName)
   {
      switch (*pszFileName)
      {
      case '-':
      case '+':
      case '(':
      case ')':
      case ' ':
      case '\\':
         *pszFileName = '_';
      }

      pszFileName++;
   }
}

bool
copyFile (std::string& stringFromFileNameXML,
          std::string& stringToFileNameXML)
{
   std::ostringstream oss;
   struct stat        statFromFile;
   struct stat        statToFile;
   PSZRO              pszAlloc         = 0;
   char              *pszToFileNameXML = 0;
   bool               fSuccess         = false;

   pszAlloc = (char *)calloc (1, stringToFileNameXML.length () + 1);
   if (!pszAlloc)
   {
      return false;
   }
   pszToFileNameXML = (char *)pszAlloc;
   strcpy (pszToFileNameXML, stringToFileNameXML.c_str ());
   while (strchr (pszToFileNameXML, '/'))
   {
      pszToFileNameXML = strchr (pszToFileNameXML, '/') + 1;
   }
   convertFilename (pszToFileNameXML);
   pszToFileNameXML = (char *)pszAlloc;

   if (  0 == stat (stringFromFileNameXML.c_str (), &statFromFile)
      && 0 == stat (pszToFileNameXML,               &statToFile)
      )
   {
      fSuccess = true;
   }

   if (fSuccess)
   {
      if (fDebugOutput)
      {
         std::cerr << stringFromFileNameXML << ", st_ctime = " << statFromFile.st_ctime << std::endl;
         std::cerr << pszToFileNameXML      << ", st_ctime = " << statToFile.st_ctime   << std::endl;
      }

      if (statToFile.st_ctime >= statFromFile.st_ctime)
      {
         return false;
      }
   }

   oss << "cp \""
       << stringFromFileNameXML
       << "\" "
       << pszToFileNameXML
       << std::ends;

   if (fDebugOutput) std::cerr << "Execing: " << oss.str () << std::endl;

   // Clean up
   free ((void *)pszAlloc);

   fSuccess = 0 == my_system ((const char *)oss.str ().c_str ());

   if (fSuccess)
   {
      std::cout << oss.str () << std::endl;

      return true;
   }
   else
   {
      std::cout << "Error: Could not copy " << stringFromFileNameXML << " to here." << std::endl;

      return false;
   }
}

void
appendPreLibraryList (FileList& fileList,
                      FileList& fileHeader)
{
   std::ifstream ifLibraries1MAK ("libraries1.mak");
   char          achLine[512];                                // @TBD
   bool          fFound = false;

   while (  !fFound
         && ifLibraries1MAK.getline (achLine, sizeof (achLine))
         )
   {
      for (FileList::iterator nextFileHeader = fileHeader.begin ();
           nextFileHeader != fileHeader.end ();
           nextFileHeader++ )
      {
         if (0 != strstr (achLine, nextFileHeader->c_str ()))
         {
            fFound = true;
         }
      }

      if (!fFound)
      {
         if (fDebugOutput) std::cerr << "-" << achLine << std::endl;

         fileList.push_back (achLine);
      }
   }
}

void
appendPostLibraryList (FileList& fileList,
                       FileList& fileHeader)
{
   typedef enum {
      BEFORE_NAME,
      IN_NAME,
      AFTER_NAME
   } ESTATE;
   std::ifstream ifLibraries1MAK ("libraries1.mak");
   char          achLine[512];                                     // @TBD
   ESTATE        eState                             = BEFORE_NAME;

   while (ifLibraries1MAK.getline (achLine, sizeof (achLine)))
   {
      switch (eState)
      {
      case BEFORE_NAME:
      {
         for (FileList::iterator nextFileHeader = fileHeader.begin ();
              nextFileHeader != fileHeader.end ();
              nextFileHeader++ )
         {
            if (0 != strstr (achLine, nextFileHeader->c_str ()))
               eState = IN_NAME;
         }
         break;
      }
      case IN_NAME:
      {
         bool fNotSeen = true;

         for (FileList::iterator nextFileHeader = fileHeader.begin ();
              nextFileHeader != fileHeader.end ();
              nextFileHeader++ )
         {
            if (  fNotSeen
               && 0 != strstr (achLine, nextFileHeader->c_str ()) // seen
               )
               fNotSeen = false;
         }
         if (fNotSeen)
         {
            eState = AFTER_NAME;
         }
         break;
      }
      case AFTER_NAME:
      {
         break;
      }
      }

      if (AFTER_NAME == eState)
      {
         if (fDebugOutput) std::cerr << "+" << achLine << std::endl;

         fileList.push_back (achLine);
      }
   }
}

void
processDeviceList (std::string&     stringFileNameDeviceList,
                   const char      *pszDirectory,
                   const char      *pszTargetRoot,
                   FileList&        fileList,
                   PFNHANDLER       pfnHandler)
{
   std::ifstream ifIn (stringFileNameDeviceList.c_str ());
   SeenList      seenList;
   char          achLine[512];   // @TBD

   while (ifIn.getline (achLine, sizeof (achLine)))
   {
      if ('#' == achLine[0])
         continue;

      if (fDebugOutput) std::cerr << "Read \"" << achLine << "\"" << std::endl;

      std::string stringFileNameMasterXML;
      std::string stringFromFileNameXML;
      std::string stringToFileNameXML;
      XmlDocPtr   docDevice               = 0;
      XmlNodePtr  nodeRoot                = 0;
      XmlNodePtr  nodeElm                 = 0;

      stringFileNameMasterXML  = "../";
      stringFileNameMasterXML += pszDirectory;
      stringFileNameMasterXML += "/";
      stringFileNameMasterXML += achLine;
      stringFileNameMasterXML += ".xml";

      docDevice = XMLParseFile (stringFileNameMasterXML.c_str ());

      if (!docDevice)
      {
         std::cerr << "Error: Missing document for " << stringFileNameMasterXML << std::endl;

         goto done;
      }

      nodeRoot = XMLDocGetRootElement (docDevice);

      if (!nodeRoot)
      {
         std::cerr << "Error: Missing root for " << stringFileNameMasterXML << std::endl;

         goto done;
      }

      nodeElm = XMLFirstNode (nodeRoot);

      if (!nodeElm)
      {
         std::cerr << "Error: Missing XMLFirstNode for " << stringFileNameMasterXML << std::endl;

         goto done;
      }

      if (0 != strcmp (XMLGetName (nodeElm), "Device"))
      {
         std::cerr << "Error: Missing Device for " << stringFileNameMasterXML << std::endl;

         goto done;
      }

      if (nodeElm)
      {
         nodeElm = XMLFirstNode (XMLGetChildrenNode (nodeElm));
      }

      if (fDebugOutput)
      {
         XMLPrintNode (stderr, nodeElm);
      }

      while (nodeElm)
      {
         if (  0 == strcmp (XMLGetName (nodeElm), "Instance")
            || 0 == strcmp (XMLGetName (nodeElm), "Blitter")
            )
         {
            PSZRO pszTargetXMLFile = 0;

            pszTargetXMLFile = XMLNodeListGetString (docDevice,
                                                     XMLGetChildrenNode (nodeElm),
                                                     1);

            if (pszTargetXMLFile)
            {
               stringFromFileNameXML  = "../";
               stringFromFileNameXML += pszDirectory;
               stringFromFileNameXML += "/";
               stringFromFileNameXML += pszTargetXMLFile;

               stringToFileNameXML    = pszTargetRoot;
               stringToFileNameXML   += pszTargetXMLFile;

               std::string stringSeenFile = pszTargetXMLFile;

               if (!seenList[stringSeenFile])
               {
                  seenList[stringSeenFile] = true;

                  pfnHandler (stringFromFileNameXML,
                              stringToFileNameXML,
                              pszTargetXMLFile,
                              fileList);
               }

               XMLFree ((void *)pszTargetXMLFile);
            }
         }

         nodeElm = XMLNextNode (nodeElm);
      }

done:
      if (docDevice)
      {
         XMLFreeDoc (docDevice);
      }
   }
}

void
handleHeader (std::string&  stringFromFileNameXML,
              std::string&  stringToFileNameXML,
              PSZCRO        pszTargetXMLFile,
              FileList&     fileList)
{
   std::string            stringTargetXMLFile;
   std::string            stringSearch;
   std::string::size_type pos;

   stringTargetXMLFile = pszTargetXMLFile;

   pos = stringTargetXMLFile.rfind (" ");

   if (std::string::npos == pos)
   {
      pos  = stringTargetXMLFile.length ();
   }

   stringSearch  = "lib";
   stringSearch += stringTargetXMLFile.substr (0, pos);
   stringSearch += "_";

   while ((pos = stringSearch.find (" ")) != std::string::npos)
   {
      stringSearch.replace (pos, 1, "_");
   }

   fileList.push_back (stringSearch);
}

void
handleFile (std::string&  stringFromFileNameXML,
            std::string&  stringToFileNameXML,
            PSZCRO        pszTargetXMLFilePure,
            FileList&     fileList)
{
   if (  !pszTargetXMLFilePure
      || !*pszTargetXMLFilePure
      )
   {
      return;
   }

   char *pszTargetXMLFile = 0;

   pszTargetXMLFile = (char *)calloc (1, strlen (pszTargetXMLFilePure) + 1);

   if (!pszTargetXMLFile)
   {
      return;
   }
   strcpy (pszTargetXMLFile, pszTargetXMLFilePure);

   copyFile (stringFromFileNameXML, stringToFileNameXML);
   convertFilename (pszTargetXMLFile);

///ofLibraries1MAK << "CLEANFILES += " << pszTargetXMLFile << std::endl;

   char *pszDot = strstr (pszTargetXMLFile, ".cpp");

   if (pszDot)
   {
      std::ostringstream oss;

      *pszDot = '\0';

      oss << "pkglib_LTLIBRARIES += lib"
          << pszTargetXMLFile
          << ".la";

      fileList.push_back (oss.str ());

      oss.str ("");
      oss << "lib"
          << pszTargetXMLFile
          << "_la_SOURCES = "
          << pszTargetXMLFile
          << ".cpp";

      fileList.push_back (oss.str ());

      oss.str ("");
      oss << "lib"
          << pszTargetXMLFile
          << "_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@ @XML_LIBS@";

      fileList.push_back (oss.str ());
   }

   free ((void *)pszTargetXMLFile);
}

int
main (int argc, char *argv[])
{
   const char *pszTargetRoot = "./";
   FileList    fileList;
   FileList    fileHeader;

   if (1 == argc)
   {
      std::cout << "Usage: " << argv[0] << " [-r root] directory [directory]*" << std::endl;
   }

   for (int i = 1; i < argc; i++)
   {
      if ('-' == argv[i][0])
      {
         switch (argv[i][1])
         {
         case 'r':
         case 'R':
            pszTargetRoot = argv[++i];
            break;
         }
         continue;
      }

      std::string stringFileNameDeviceList ("../");

      stringFileNameDeviceList += argv[i];
      stringFileNameDeviceList += "/Device List";

      processDeviceList (stringFileNameDeviceList,
                         argv[i],
                         pszTargetRoot,
                         fileHeader,
                         handleHeader);

      fileList.clear ();
      fileHeader.sort ();
      fileHeader.unique ();

      appendPreLibraryList (fileList, fileHeader);

      processDeviceList (stringFileNameDeviceList,
                         argv[i],
                         pszTargetRoot,
                         fileList,
                         handleFile);

      appendPostLibraryList (fileList, fileHeader);

      std::ofstream ofLibraries1MAK ("libraries1.mak", std::ios::out | std::ios::trunc);

      for (FileList::iterator nextFileList = fileList.begin ();
           nextFileList != fileList.end ();
           nextFileList++ )
      {
         ofLibraries1MAK << *nextFileList << std::endl;
      }
   }

   return 0;
}
