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

#include "Device.hpp"
#include "XMLInterface.hpp"

const static bool vfDebug        = false;
const static bool vfDumpFile     = true;
const static bool vfDumpElements = true;

char *
bundleStringData (XmlNodePtr nodeItem)
{
   return (char *)XMLNodeListGetString (XMLGetDocNode (nodeItem), XMLGetChildrenNode (nodeItem), 1);
}

#if 1

bool
processDevice (XmlNodePtr  nodeRoot,
               bool        fUpdate,
               const char *pszDeviceFileName)
{
   bool           fFoundDeviceCopy         = false;
   bool           fFoundDeviceNUp          = false;
   bool           fFoundDeviceOutputBin    = false;
   bool           fFoundDeviceScaling      = false;
   bool           fFoundDeviceSheetCollate = false;
   bool           fFoundDeviceSide         = false;
   bool           fFoundDeviceStitching    = false;
   bool           fFoundDeviceTrimming     = false;
   XmlNodePtr     nodeElement              = 0;
   XmlNodePtr     nodeLastHas              = 0;
   PSZRO          pszDriverName            = 0;
   bool           fRC                      = false;

   nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeRoot));

   while (nodeElement)
   {
      if (0 == XMLStrcmp (XMLGetName (nodeElement), "DriverName"))
      {
         pszDriverName = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                               XMLGetChildrenNode (nodeElement),
                                               1);
      }
      else if (0 == XMLStrcmp (XMLGetName (nodeElement), "DefaultJobProperties"))
      {
         break;
      }

      nodeElement = XMLNextNode (nodeElement);
   }

   if (nodeElement)
   {
      nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeElement));

      while (nodeElement)
      {
         PSZRO pszNewNameText = 0;

         if (0 == XMLStrcmp (XMLGetName (nodeElement), "deviceNumberUp"))
         {
            pszNewNameText = "NumberUp";
         }
         else if (0 == XMLStrcmp (XMLGetName (nodeElement), "deviceScaling"))
         {
            XmlNodePtr nodeChild = 0;

            pszNewNameText = "Scaling";

            nodeChild = XMLFirstNode (XMLGetChildrenNode (nodeElement));

            while (nodeChild)
            {
               if (0 == XMLStrcmp (XMLGetName (nodeChild), "ScalingPercentage"))
               {
                  XMLNodeSetContent (nodeChild, "100");
               }
               else if (0 == XMLStrcmp (XMLGetName (nodeChild), "ScalingType"))
               {
                  XMLNodeSetContent (nodeChild, "FitToPage");
               }

               nodeChild = XMLNextNode (nodeChild);
            }
         }
         else if (0 == XMLStrcmp (XMLGetName (nodeElement), "deviceStitching"))
         {
            pszNewNameText = "Stitching";
         }

         if (  fUpdate
            && pszNewNameText
            )
         {
            if (vfDumpElements)
               std::cout << XMLGetName (nodeElement)
                         << " -> "
                         << pszNewNameText
                         << std::endl;

            // Change the name to the newly mapped name
            XMLNodeSetName (nodeElement, pszNewNameText);

            fRC = true;
         }

         nodeElement = XMLNextNode (nodeElement);
      }
   }

   nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeRoot));

   while (nodeElement)
   {
      PSZRO pszHasFileName = 0;

      if (0 == XMLStrcmp (XMLGetName (nodeElement), "Has"))
      {
         nodeLastHas = nodeElement;

         pszHasFileName = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                XMLGetChildrenNode (nodeElement),
                                                1);

         if (pszHasFileName)
         {
            XmlDocPtr  docHas      = 0;
            XmlNodePtr nodeHasRoot = 0;

            docHas = XMLParseFile ((const char *)pszHasFileName);

            if (docHas)
            {
               nodeHasRoot = XMLFirstNode (XMLDocGetRootElement (docHas));

               if (nodeHasRoot)
               {
                  if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceCopies"))
                  {
                     fFoundDeviceCopy = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceNumberUps"))
                  {
                     fFoundDeviceNUp = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceOutputBins"))
                  {
                     fFoundDeviceOutputBin = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceScalings"))
                  {
                     fFoundDeviceScaling = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceSheetCollates"))
                  {
                     fFoundDeviceSheetCollate = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceSides"))
                  {
                     fFoundDeviceSide = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceStitchings"))
                  {
                     fFoundDeviceStitching = true;
                  }
                  else if (0 == XMLStrcmp (XMLGetName (nodeHasRoot), "deviceTrimmings"))
                  {
                     fFoundDeviceTrimming = true;
                  }
               }

               XMLFreeDoc (docHas);
            }

            XMLFree ((void *)pszHasFileName);
         }
      }

      nodeElement = XMLNextNode (nodeElement);
   }

   std::cout << "fFoundDeviceCopy         = " << fFoundDeviceCopy         << std::endl;
   std::cout << "fFoundDeviceNUp          = " << fFoundDeviceNUp          << std::endl;
   std::cout << "fFoundDeviceOutputBin    = " << fFoundDeviceOutputBin    << std::endl;
   std::cout << "fFoundDeviceScaling      = " << fFoundDeviceScaling      << std::endl;
   std::cout << "fFoundDeviceSheetCollate = " << fFoundDeviceSheetCollate << std::endl;
   std::cout << "fFoundDeviceSide         = " << fFoundDeviceSide         << std::endl;
   std::cout << "fFoundDeviceStitching    = " << fFoundDeviceStitching    << std::endl;
   std::cout << "fFoundDeviceTrimming     = " << fFoundDeviceTrimming     << std::endl;

   if (!fFoundDeviceCopy)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " Copies.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceNUp)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " NUps.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceOutputBin)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " OutputBins.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceScaling)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " Scalings.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceSheetCollate)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " SheetCollates.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceSide)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " Sides.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceStitching)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " Stitchings.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

   if (!fFoundDeviceTrimming)
   {
      XmlNodePtr         nodeNew      = 0;
      XmlNodePtr         nodeNewValue = 0;
      XmlNodePtr         nodeNewWS    = 0;
      std::ostringstream oss;

      oss << pszDriverName
          << " Trimmings.xml";

      nodeNew      = XMLNewNode (0, "Has");
      nodeNewValue = XMLNewText (oss.str ().c_str ());
      nodeNewWS    = XMLNewText ("\n   ");

      if (  nodeNew
         && nodeNewValue
         && nodeNewWS
         )
      {
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNewWS);
         nodeLastHas = XMLAddNextSibling (nodeLastHas, nodeNew);

         XMLAddChild (nodeNew, nodeNewValue);
      }
   }

#if 0
   if (nodeRoot)
   {
      XmlNodePtr  nodeDevice = XMLFirstNode (nodeRoot);
      XmlNsPtr   *pnsDevice  = 0;
      XmlNsPtr    nsElement  = 0;
      XmlNsPtr    nsOmni     = 0;

      // Get the namespaces in the <Device> element
      pnsDevice = XMLGetNsList (XMLGetDocNode (nodeDevice), nodeDevice);

      if (pnsDevice)
      {
         // Start at the beginning
         nsElement = *pnsDevice;
      }

      while (nsElement)
      {
         std::cout << "nsElement->href   = " << SAFE_PRINT_PSZ ((char *)nsElement->href) << std::endl;
         std::cout << "nsElement->prefix = " << SAFE_PRINT_PSZ ((char *)nsElement->prefix) << std::endl;

         if (0 == XMLStrcmp (nsElement->href, "http://www.ibm.com/linux/ltc/projects/omni/"))
         {
            nsOmni = nsElement;
         }

         nsElement = nsElement->next;
      }

//////if (nsOmni)
//////{
//////   XMLUnsetNsProp (nodeDevice, nsOmni, "");
//////}
   }

   if (nodeRoot)
   {
      XmlNodePtr  nodeDevice     = XMLFirstNode (nodeRoot);
      XmlAttrPtr  attrProperties = 0;

      attrProperties = nodeDevice->properties;

      while (attrProperties)
      {
         std::cout << "XMLGetName (attrProperties) = " << SAFE_PRINT_PSZ ((char *)XMLGetName (attrProperties)) << std::endl;

         attrProperties = attrProperties->next;
      }
   }

   if (nodeRoot)
   {
      XmlNodePtr nodeDevice = XMLFirstNode (nodeRoot);
      XmlNsPtr   nsNew      = 0;

      nsNew = XMLNewNs (nodeDevice, "http://www.ibm.com/linux/ltc/projects/omni/", "omni");

//    XMLSetNs (nodeDevice, nsNew);
      XMLNewNs (nodeDevice, "omni", "targetNamespace");
   }
#endif

   if (pszDriverName)
   {
      XMLFree ((void *)pszDriverName);
   }

   return fRC;
}

#else

bool
processDevice (XmlNodePtr  nodeRoot,
               bool        fUpdate,
               const char *pszDeviceFileName)
{
   XmlNodePtr     nodeElement              = 0;
   bool           fRC                      = false;

   nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeRoot));

   while (nodeElement)
   {
      if (0 == XMLStrcmp (XMLGetName (nodeElement), "DefaultJobProperties"))
      {
         break;
      }

      nodeElement = XMLNextNode (nodeElement);
   }

   if (nodeElement)
   {
      nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeElement));

      while (nodeElement)
      {
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "Scaling"))
         {
            XmlNodePtr nodeChild = 0;

            nodeChild = XMLFirstNode (XMLGetChildrenNode (nodeElement));

            while (nodeChild)
            {
               if (0 == XMLStrcmp (XMLGetName (nodeChild), "ScalingType"))
               {
                  XMLNodeSetContent (nodeChild, "None");

                  fRC = true;
               }

               nodeChild = XMLNextNode (nodeChild);
            }
         }

         nodeElement = XMLNextNode (nodeElement);
      }
   }

   return fRC;
}


#endif

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
   else if (0 == XMLStrcmp ("Device", XMLGetName (nodeRoot)))
   {
      fRC = processDevice (nodeRoot, fUpdate, pszFileName);
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
