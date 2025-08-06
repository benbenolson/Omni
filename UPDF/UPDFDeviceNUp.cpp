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

#include <UPDFDeviceNUp.hpp>

UPDFDeviceNUp::
UPDFDeviceNUp (Device     *pDevice,
               PSZRO       pszJobProperties,
               BinaryData *pbdData,
               bool        fSimulationRequired,
               XmlNodePtr  nodeName,
               XmlNodePtr  nodeDir)
   : DeviceNUp (pDevice,
                pszJobProperties,
                pbdData,
                fSimulationRequired)
{
   nodeName_d = nodeName;
   nodeDir_d  = nodeDir;
}

UPDFDeviceNUp::
~UPDFDeviceNUp ()
{
}

static XmlNodePtr
skipInvalidNUps (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static XmlNodePtr
skipInvalidNUpDirs (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceNUp *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  nodeNUp,
                   XmlNodePtr  nodeNUpDir)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   PSZRO       pszUPDFNUpDir        = 0;
   PSZRO       pszUPDFNUpName       = 0;
   PSZRO       pszOmniJobProperties = 0;
   BinaryData *pbdData              = 0;
   bool        fSimulationRequired  = false;
   DeviceNUp  *pNUpRet              = 0;

   pszUPDFNUpName = XMLGetProp (nodeNUp,    "ClassifyingID");
   pszUPDFNUpDir  = XMLGetProp (nodeNUpDir, "ClassifyingID");

   if (UPDFDeviceNUp::mapUPDFToOmni (pszUPDFNUpName,
                                      pszUPDFNUpDir,
                                      0,
                                      0,
                                      &pszOmniJobProperties))
   {
      pNUpRet = new UPDFDeviceNUp (pUPDFDevice,
                                   pszOmniJobProperties,
                                   pbdData,
                                   fSimulationRequired,
                                   nodeNUp,
                                   nodeNUpDir);

      if (pszOmniJobProperties)
      {
         free ((void *)pszOmniJobProperties);
      }
   }

   if (pszUPDFNUpName)
   {
      XMLFree ((void *)pszUPDFNUpName);
   }

   if (pszUPDFNUpDir)
   {
      XMLFree ((void *)pszUPDFNUpDir);
   }

   return pNUpRet;
}

DeviceNUp * UPDFDeviceNUp::
createS (Device  *pDevice,
         PSZCRO   pszJobProperties)
{
   UPDFDevice  *pUPDFDevice    = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr   nodeNUps       = 0;
   XmlNodePtr   nodeNUpDirs    = 0;
   XmlNodePtr   nodeItem       = 0;
   XmlNodePtr   nodeNUp        = 0;
   XmlNodePtr   nodeNUpDir     = 0;
   DeviceNUp   *pNUpRet        = 0;
   int          iX             = 0;
   int          iY             = 0;
   PSZRO        pszNUpDir      = 0;
   char         achNUp[25];

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &iX,
                       &iY,
                       &pszNUpDir,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   sprintf (achNUp, "NUp_%dx%d", iX, iY);

   nodeNUps    = findNUps (pUPDFDevice);
   nodeNUpDirs = findNUpDirs (pUPDFDevice);

   if (!nodeNUps)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeNUps" << std::endl;
#endif

      goto done;
   }

   if (!nodeNUpDirs)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeNUpDirs" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeNUps));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeItem in nodeNUps" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !nodeNUp
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, achNUp))
         {
            nodeNUp = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeNUpDirs));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeItem in nodeNUpDirs" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !nodeNUpDir
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszNUpDir))
         {
            nodeNUpDir = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

   if (  nodeNUp
      && nodeNUpDir
      )
   {
      pNUpRet = createFromXMLNode (pDevice,
                                   nodeNUp,
                                   nodeNUpDir);
   }

done:
   if (pszNUpDir)
   {
      free ((void *)pszNUpDir);
   }

   if (!pNUpRet)
   {
      pNUpRet = pUPDFDevice->getDefaultNUp ();
   }

   return pNUpRet;
}

DeviceNUp * UPDFDeviceNUp::
create (Device  *pDevice,
        PSZCRO   pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceNUp::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice  *pUPDFDevice    = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr   nodeNUps       = 0;
   XmlNodePtr   nodeNUpDirs    = 0;
   XmlNodePtr   nodeItem       = 0;
   XmlNodePtr   nodeNUp        = 0;
   XmlNodePtr   nodeNUpDir     = 0;
   int          iX             = 0;
   int          iY             = 0;
   PSZRO        pszNUpDir      = 0;
   char         achNUp[25];
   bool         fRet           = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &iX,
                       &iY,
                       &pszNUpDir,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   sprintf (achNUp, "NUp_%dx%d", iX, iY);

   nodeNUps    = findNUps (pUPDFDevice);
   nodeNUpDirs = findNUpDirs (pUPDFDevice);

   if (!nodeNUps)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeNUps" << std::endl;
#endif

      goto done;
   }

   if (!nodeNUpDirs)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeNUpDirs" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeNUps));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeItem in nodeNUps" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !nodeNUp
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, achNUp))
         {
            nodeNUp = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeNUpDirs));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": !nodeItem in nodeNUpDirs" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !nodeNUpDir
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszNUpDir))
         {
            nodeNUpDir = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

   if (  nodeNUp
      && nodeNUpDir
      )
   {
      fRet = true;
   }

done:
   if (pszNUpDir)
   {
      free ((void *)pszNUpDir);
   }

   return fRet;
}

PSZCRO UPDFDeviceNUp::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceNUp::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet        = 0;
   UPDFDevice                 *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeNUps    = 0;
   XmlNodePtr                  nodeNUp     = 0;
   XmlNodePtr                  nodeNUpDirs = 0;
   XmlNodePtr                  nodeNUpDir  = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeNUps = findNUps (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": nodeNUps = " << std::hex << (int *)nodeNUps << std::dec << std::endl;
#endif

   if (!nodeNUps)
   {
      goto done;
   }

   nodeNUp = XMLFirstNode (XMLGetChildrenNode (nodeNUps));
   nodeNUp = skipInvalidNUps (nodeNUp);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": nodeNUp = " << std::hex << (int *)nodeNUp << std::dec << std::endl;
#endif

   nodeNUpDirs = findNUpDirs (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": nodeNUpDirs = " << std::hex << (int *)nodeNUpDirs << std::dec << std::endl;
#endif

   if (!nodeNUpDirs)
   {
      goto done;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": nodeNUpDir = " << std::hex << (int *)nodeNUpDir << std::dec << std::endl;
#endif

   while (nodeNUp)
   {
      nodeNUpDir = XMLFirstNode (XMLGetChildrenNode (nodeNUpDirs));
      nodeNUpDir = skipInvalidNUpDirs (nodeNUpDir);

      while (nodeNUpDir)
      {
         DeviceNUp     *pNUp = 0;
         JobProperties *pJP  = 0;

         pNUp = createFromXMLNode (pDevice_d, nodeNUp, nodeNUpDir);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": pNUp = " << std::hex << (int *)pNUp << std::dec << std::endl;
#endif

         if (pNUp)
         {
            std::string *pstringJPs = 0;

            pstringJPs = pNUp->getJobProperties (fInDeviceSpecific);

            if (pstringJPs)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

               pJP = new JobProperties (pstringJPs->c_str ());

               pRet->addElement (pJP);

               delete pstringJPs;
            }

            delete pNUp;
         }

         nodeNUpDir = XMLNextNode (nodeNUpDir);
         nodeNUpDir = skipInvalidNUpDirs (nodeNUpDir);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUpDir::" << __FUNCTION__ << ": nodeNUpDir = " << std::hex << (int *)nodeNUpDir << std::dec << std::endl;
#endif
      }

      nodeNUp = XMLNextNode (nodeNUp);
      nodeNUp = skipInvalidNUps (nodeNUp);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceNUp ()) DebugOutput::getErrorStream () << "UPDFDeviceNUp::" << __FUNCTION__ << ": nodeNUp = " << std::hex << (int *)nodeNUp << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceNUp::
findNUps (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeNUps = 0;

   if (!pUPDFDevice)
   {
      return nodeNUps;
   }

   if (  ((nodeNUps = FINDUDRENTRY (pUPDFDevice, nodeNUps, "PrintCapabilities",  UPDFDeviceNUp)) != 0)
      && ((nodeNUps = FINDUDRENTRY (pUPDFDevice, nodeNUps, "Features",           UPDFDeviceNUp)) != 0)
      && ((nodeNUps = FINDUDRENTRY (pUPDFDevice, nodeNUps, "NumberUp",           UPDFDeviceNUp)) != 0)
      )
      return nodeNUps;

   return nodeNUps;
}

XmlNodePtr UPDFDeviceNUp::
findNUpDirs (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeNUpDirs = 0;

   if (!pUPDFDevice)
   {
      return nodeNUpDirs;
   }

   if (  ((nodeNUpDirs = FINDUDRENTRY (pUPDFDevice, nodeNUpDirs, "PrintCapabilities",             UPDFDeviceNUp)) != 0)
      && ((nodeNUpDirs = FINDUDRENTRY (pUPDFDevice, nodeNUpDirs, "Features",                      UPDFDeviceNUp)) != 0)
      && ((nodeNUpDirs = FINDUDRENTRY (pUPDFDevice, nodeNUpDirs, "PresentationDirectionNumberUp", UPDFDeviceNUp)) != 0)
      )
      return nodeNUpDirs;

   return nodeNUpDirs;
}

bool UPDFDeviceNUp::
mapUPDFToOmni (PSZCRO  pszUPDFNUp,
               PSZCRO  pszUPDFNUpDirection,
               int    *piX,
               int    *piY,
               PSZRO  *ppszOmniJobProperties)
{
   std::ostringstream oss;
   bool               fRC = true;

   if (  pszUPDFNUp
      && *pszUPDFNUp
      )
   {
      if (0 == strcmp (pszUPDFNUp, "NUp_1x1"))
      {
         oss << "NumberUp=1x1";
         if (piX)
         {
            *piX = 1;
         }
         if (piY)
         {
            *piY = 1;
         }
      }
      else if (0 == strcmp (pszUPDFNUp, "NUp_2x1"))
      {
         oss << "NumberUp=2x1";
         if (piX)
         {
            *piX = 2;
         }
         if (piY)
         {
            *piY = 1;
         }
      }
      else if (0 == strcmp (pszUPDFNUp, "NUp_2x2"))
      {
         oss << "NumberUp=2x2";
         if (piX)
         {
            *piX = 2;
         }
         if (piY)
         {
            *piY = 2;
         }
      }
      else if (0 == strcmp (pszUPDFNUp, "NUp_3x2"))
      {
         oss << "NumberUp=3x2";
         if (piX)
         {
            *piX = 3;
         }
         if (piY)
         {
            *piY = 2;
         }
      }
      else if (0 == strcmp (pszUPDFNUp, "NUp_3x3"))
      {
         oss << "NumberUp=3x3";
         if (piX)
         {
            *piX = 3;
         }
         if (piY)
         {
            *piY = 3;
         }
      }
      else if (0 == strcmp (pszUPDFNUp, "NUp_4x4"))
      {
         oss << "NumberUp=4x4";
         if (piX)
         {
            *piX = 4;
         }
         if (piY)
         {
            *piY = 4;
         }
      }
      else
      {
         fRC = false;
      }
   }
   if (  pszUPDFNUpDirection
      && *pszUPDFNUpDirection
      )
   {
      if (oss.str ()[0])
      {
         oss << " ";
      }
      if (  0 == strcmp (pszUPDFNUpDirection, "TobottomToleft")
         || 0 == strcmp (pszUPDFNUpDirection, "TobottomToright")
         || 0 == strcmp (pszUPDFNUpDirection, "ToleftTobottom")
         || 0 == strcmp (pszUPDFNUpDirection, "ToleftTotop")
         || 0 == strcmp (pszUPDFNUpDirection, "TorightTobottom")
         || 0 == strcmp (pszUPDFNUpDirection, "TorightTotop")
         || 0 == strcmp (pszUPDFNUpDirection, "TotopToleft")
         || 0 == strcmp (pszUPDFNUpDirection, "TotopToright")
         )
      {
         oss << "NumberUpDirection=" << pszUPDFNUpDirection;
      }
      else
      {
         fRC = false;
      }
   }

   if (ppszOmniJobProperties)
   {
      std::string stringOmniJobProperties = oss.str ();

      *ppszOmniJobProperties = (PSZRO)malloc (stringOmniJobProperties.length () + 1);

      if (*ppszOmniJobProperties)
      {
         strcpy ((PSZ)*ppszOmniJobProperties, (PSZ)stringOmniJobProperties.c_str ());
      }
   }

   return fRC;
}

#ifndef RETAIL

void UPDFDeviceNUp::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceNUp::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceNUp: "
       << DeviceNUp::toString (oss2)
       << "}";

   return oss.str ();
}

std::ostream&
operator<< (std::ostream&        os,
            const UPDFDeviceNUp& const_self)
{
   UPDFDeviceNUp&     self = const_cast<UPDFDeviceNUp&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
