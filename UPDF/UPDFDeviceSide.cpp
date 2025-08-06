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

#include <UPDFDeviceSide.hpp>

UPDFDeviceSide::
UPDFDeviceSide (Device      *pDevice,
                PSZRO        pszJobProperties,
                BinaryData  *pbdData,
                bool         fSimulationRequired,
                XmlNodePtr   node)
   : DeviceSide (pDevice,
                 pszJobProperties,
                 pbdData,
                 fSimulationRequired)
{
   node_d = node;
}

UPDFDeviceSide::
~UPDFDeviceSide ()
{
}

static XmlNodePtr
skipInvalidSides (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceSide *
createFromXMLNode (Device      *pDevice,
                   XmlNodePtr   node)
{
   UPDFDevice         *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice);
   bool                fDeviceFeature   = true;
   PSZRO               pszSideName      = 0;
   PSZRO               pszDeviceFeature = 0;
   BinaryData         *pbdData          = 0; // @TBD
   std::ostringstream  oss;
   DeviceSide         *pSideRet         = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   pszSideName      = XMLGetProp (node, "ClassifyingID");
   pszDeviceFeature = XMLGetProp (node, "DeviceFeature");

   if (pszDeviceFeature)
   {
      if (0 == strcasecmp (pszDeviceFeature, "false"))
      {
         fDeviceFeature = false;
      }

      XMLFree ((void *)pszDeviceFeature);
   }

   if (pszSideName)
   {
      PSZRO pszOmniJP = 0;

      if (UPDFDeviceSide::mapUPDFToOmni (pszSideName,
                                         &pszOmniJP))
      {
         oss << "Sides=" << pszOmniJP;

         pSideRet = new UPDFDeviceSide (pUPDFDevice,
                                        oss.str ().c_str (),
                                        pbdData,
                                        !fDeviceFeature,
                                        node);
      }
   }

   if (pszSideName)
   {
      XMLFree ((void *)pszSideName);
   }

   return pSideRet;
}

DeviceSide * UPDFDeviceSide::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr  nodeSides   = 0;
   XmlNodePtr  nodeItem    = 0;
   XmlNodePtr  nodeFound   = 0;
   PSZRO       pszSideName = 0;
   PSZRO       pszOmniName = 0;
   DeviceSide *pSideRet    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceSide::mapOmniToUPDF (pszOmniName, &pszSideName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeSides = findSides (pUPDFDevice);

   if (!nodeSides)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !nodeSides" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeSides));

   while (  nodeItem != 0
         && !pSideRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszSideName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         pSideRet = createFromXMLNode (pDevice, nodeItem);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   if (!pSideRet)
   {
      pSideRet = pUPDFDevice->getDefaultSide ();
   }

   return pSideRet;
}

DeviceSide * UPDFDeviceSide::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceSide::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeSides   = 0;
   XmlNodePtr  nodeItem    = 0;
   XmlNodePtr  nodeFound   = 0;
   PSZRO       pszSideName = 0;
   PSZRO       pszOmniName = 0;
   bool        fRet        = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceSide::mapOmniToUPDF (pszOmniName, &pszSideName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeSides = findSides (pUPDFDevice);

   if (!nodeSides)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": !nodeSides" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeSides));

   while (  nodeItem != 0
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszSideName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         fRet = true;
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   return fRet;
}

PSZCRO UPDFDeviceSide::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceSide::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet        = 0;
   UPDFDevice                 *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeSides   = 0;
   XmlNodePtr                  nodeSide    = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeSides = findSides (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": nodeSides = " << std::hex << (int *)nodeSides << std::dec << std::endl;
#endif

   if (!nodeSides)
   {
      goto done;
   }

   nodeSide = XMLFirstNode (XMLGetChildrenNode (nodeSides));
   nodeSide = skipInvalidSides (nodeSide);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": nodeSide = " << std::hex << (int *)nodeSide << std::dec << std::endl;
#endif

   while (nodeSide)
   {
      DeviceSide    *pSide = 0;
      JobProperties *pJP   = 0;

      pSide = createFromXMLNode (pDevice_d, nodeSide);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": pSide = " << std::hex << (int *)pSide << std::dec << std::endl;
#endif

      if (pSide)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pSide->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pSide;
      }

      nodeSide = XMLNextNode (nodeSide);
      nodeSide = skipInvalidSides (nodeSide);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSide ()) DebugOutput::getErrorStream () << "UPDFDeviceSide::" << __FUNCTION__ << ": nodeSide = " << std::hex << (int *)nodeSide << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceSide::
findSides (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeSides = 0;

   if (!pUPDFDevice)
   {
      return nodeSides;
   }

   if (  ((nodeSides = FINDUDRENTRY (pUPDFDevice, nodeSides, "PrintCapabilities",  UPDFDeviceSide)) != 0)
      && ((nodeSides = FINDUDRENTRY (pUPDFDevice, nodeSides, "Features",           UPDFDeviceSide)) != 0)
      && ((nodeSides = FINDUDRENTRY (pUPDFDevice, nodeSides, "Sides",              UPDFDeviceSide)) != 0)
      )
      return nodeSides;

   return nodeSides;
}

bool UPDFDeviceSide::
mapUPDFToOmni (PSZCRO  pszUPDFValue,
               PSZRO  *ppszOmniValue)
{
   typedef struct _SideMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SIDEMAPPING, *PSIDEMAPPING;
   SIDEMAPPING ammFromUPDFToOmni[] = {
      { "OneSided",          "OneSidedFront"  },
      { "TwoSidedLongEdge",  "TwoSidedFlipY"  },
      { "TwoSidedShortEdge", "TwoSidedFlipX"  }
   };
   int iLow                 = 0;
   int iMid                 = 0;
   int iHigh                = 0;
   int iResult;

   iMid  = (int)dimof (ammFromUPDFToOmni) / 2;
   iHigh = (int)dimof (ammFromUPDFToOmni) - 1;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszUPDFValue, ammFromUPDFToOmni[iMid].pszFrom);

      if (0 == iResult)
      {
         if (ppszOmniValue)
         {
            *ppszOmniValue = ammFromUPDFToOmni[iMid].pszTo;
         }

         return true;
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return false;
}

bool UPDFDeviceSide::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
   typedef struct _SideMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SIDEMAPPING, *PSIDEMAPPING;
   SIDEMAPPING ammFromOmniToUPDF[] = {
      { "OneSidedFront",     "OneSided"          },
      { "TwoSidedFlipX",     "TwoSidedShortEdge" },
      { "TwoSidedFlipY",     "TwoSidedLongEdge"  }
   };
   int iLow                 = 0;
   int iMid                 = 0;
   int iHigh                = 0;
   int iResult;

   iMid  = (int)dimof (ammFromOmniToUPDF) / 2;
   iHigh = (int)dimof (ammFromOmniToUPDF) - 1;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszOmniValue, ammFromOmniToUPDF[iMid].pszFrom);

      if (0 == iResult)
      {
         if (ppszUPDFValue)
         {
            *ppszUPDFValue = ammFromOmniToUPDF[iMid].pszTo;
         }

         return true;
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return false;
}

#ifndef RETAIL

void UPDFDeviceSide::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceSide::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceSide: "
       << DeviceSide::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceSide& const_self)
{
   UPDFDeviceSide&    self = const_cast<UPDFDeviceSide&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
