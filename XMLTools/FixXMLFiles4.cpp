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
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdarg.h>
#include <glob.h>

#include "defines.hpp"
#include "XMLInterface.hpp"

const static bool vfDebug        = false;
const static bool vfDumpFile     = false;
const static bool vfDumpElements = true;

char *
bundleStringData (XmlNodePtr nodeItem)
{
   return (char *)XMLNodeListGetString (XMLGetDocNode (nodeItem), XMLGetChildrenNode (nodeItem), 1);
}

bool
processGammas (XmlNodePtr  nodeGammas,
               bool        fUpdate,
               const char *pszGammaFileName)
{
   XmlNodePtr     nodeGamma      = 0;
   XmlNodePtr     nodeElement    = 0;
   XmlDocPtr      docResolutions = 0;
   bool           fRet           = true;

   while (nodeGammas)
   {
      nodeGamma = XMLFirstNode (XMLGetChildrenNode (nodeGammas));

      while (nodeGamma)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeGamma));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (  0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableResolution")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableMedia")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTablePrintMode")
               )
            {
               XmlNodePtr     nodeNew          = 0;
               PSZRO          pszNameText      = 0;
               PSZRO          pszNewNameText   = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               if (0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableResolution"))
               {
                  pszNewNameText = "Resolution";
               }
               else if (0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableMedia"))
               {
                  pszNewNameText = "media";
               }
               else if (0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTablePrintMode"))
               {
                  pszNewNameText = "printmode";
               }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements)
                     std::cout << "<"
                               << XMLGetName (nodeElement)
                               << ">"
                               << pszNameText
                               << "</"
                               << XMLGetName (nodeElement)
                               << "> -> <"
                               << pszNewNameText
                               << ">"
                               << pszNameText
                               << "</"
                               << pszNewNameText
                               << ">"
                               << std::endl;

                  // Change the name to the newly mapped name
                  XMLNodeSetName (nodeElement, pszNewNameText);

                  fRet = true;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeGamma = XMLNextNode (nodeGamma);
      }

      nodeGammas = XMLNextNode (nodeGammas);
   }

   if (docResolutions)
   {
      XMLFreeDoc (docResolutions);
   }

   return fRet;
}

bool
processFile (const char *pszFileName,
             bool        fUpdate)
{
   XmlDocPtr           doc             = 0;
   XmlNodePtr          nodeRoot        = 0;
   bool                fRC             = false;

   doc = XMLParseFile (pszFileName);

   if (vfDebug) std::cerr << "doc = " << std::hex << (int)doc << std::dec << std::endl;

   if (!doc)
   {
      return false;
   }

   nodeRoot = XMLFirstNode (XMLDocGetRootElement (doc));
   if (!nodeRoot)
   {
      return false;
   }
   else if (0 == XMLStrcmp ("deviceGammaTables", XMLGetName (nodeRoot)))
   {
      fRC = processGammas (nodeRoot, fUpdate, pszFileName);
   }

   if (  fRC
      && vfDumpFile
      )
   {
      XMLDocDump (stdout, doc);
   }

   if (  fRC
      && fUpdate
      )
   {
      XMLSaveFile (pszFileName, doc);
   }

   XMLFreeDoc (doc);

   return true;
}

int
main (int argc, char *argv[])
{
   std::ostringstream oss;
   glob_t             globbuf;
   int                rc;

   // Call glob
   memset (&globbuf, 0, sizeof (globbuf));

   oss << "*.xml"
       << std::ends;

   rc = glob (oss.str ().c_str (), 0, NULL, &globbuf);

   if (rc)
   {
      std::cerr << "Error: glob (" << oss.str () << ") failed. rc = " << rc << std::endl;

      return __LINE__;
   }

   if (0 == globbuf.gl_pathc)
   {
      std::cerr << "Error: glob (" << oss.str () << ") returned 0 files!" << std::endl;

      return __LINE__;
   }

   // Call succeded
   for (int i = 0; i < (int)globbuf.gl_pathc; i++)
   {
      std::cout << globbuf.gl_pathv[i] << std::endl;

      processFile (globbuf.gl_pathv[i], true);
   }

   globfree (&globbuf);

   return 0;
}
