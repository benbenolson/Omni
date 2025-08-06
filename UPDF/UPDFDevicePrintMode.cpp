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

#include <UPDFDevicePrintMode.hpp>

UPDFDevicePrintMode::
UPDFDevicePrintMode (Device      *pDevice,
                     PSZRO        pszJobProperties,
                     int          iPhysicalCount,
                     int          iLogicalCount,
                     int          iPlanes,
                     XmlNodePtr   node)
   : DevicePrintMode (pDevice,
                      pszJobProperties,
                      iPhysicalCount,
                      iLogicalCount,
                      iPlanes)
{
   node_d = node;
}

UPDFDevicePrintMode::
~UPDFDevicePrintMode ()
{
}

static XmlNodePtr
skipInvalidPrintModes (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DevicePrintMode *
createFromXMLNode (Device      *pDevice,
                   XmlNodePtr   node)
{
   UPDFDevice         *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice);
   PSZRO               pszPrintModeName = 0;
   int                 iPhysicalCount   = 0;
   int                 iLogicalCount    = 0;
   int                 iPlanes          = 0;
   std::ostringstream  oss;
   DevicePrintMode    *pPrintModeRet    = 0;

   if (!pUPDFDevice)
   {
      return 0;
   }

   pszPrintModeName = XMLGetProp (node, "ClassifyingID");

   if (pszPrintModeName)
   {
      PSZRO pszOmniJP = 0;

      if (UPDFDevicePrintMode::mapUPDFToOmni (pszPrintModeName,
                                              &pszOmniJP))
      {
         if (0 == strcmp (pszOmniJP, "PRINT_MODE_1_ANY"))
         {
            iPhysicalCount = 1;
            iLogicalCount  = 1;
            iPlanes        = 1;
         }
         else if (0 == strcmp (pszOmniJP, "PRINT_MODE_24_RGB"))
         {
            // @TBD
            iPhysicalCount = 16777216;
            iLogicalCount  = 24;
            iPlanes        = 1;
         }

         if (  iPhysicalCount != 0
            && iLogicalCount  != 0
            && iPlanes        != 0
            )
         {
            oss << "printmode=" << pszOmniJP;

            pPrintModeRet = new UPDFDevicePrintMode (pUPDFDevice,
                                                     oss.str ().c_str (),
                                                     iPhysicalCount,
                                                     iLogicalCount,
                                                     iPlanes,
                                                     node);
         }
      }
   }

   if (pszPrintModeName)
   {
      XMLFree ((void *)pszPrintModeName);
   }

   return pPrintModeRet;
}

DevicePrintMode * UPDFDevicePrintMode::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice      *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr       nodePrintModes   = 0;
   XmlNodePtr       nodeItem         = 0;
   XmlNodePtr       nodeFound        = 0;
   PSZRO            pszPrintModeName = 0;
   PSZRO            pszOmniName      = 0;
   DevicePrintMode *pPrintModeRet    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDevicePrintMode::mapOmniToUPDF (pszOmniName, &pszPrintModeName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodePrintModes = findPrintModes (pUPDFDevice);

   if (!nodePrintModes)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !nodePrintModes" << std::endl;
#endif

      return 0;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodePrintModes));

   while (  nodeItem != 0
         && !pPrintModeRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszPrintModeName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         pPrintModeRet = createFromXMLNode (pDevice, nodeItem);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   return pPrintModeRet;
}

DevicePrintMode * UPDFDevicePrintMode::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

PSZCRO UPDFDevicePrintMode::
getDeviceID ()
{
   return 0;
}

bool UPDFDevicePrintMode::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodePrintModes   = 0;
   XmlNodePtr  nodeItem         = 0;
   XmlNodePtr  nodeFound        = 0;
   PSZRO       pszPrintModeName = 0;
   PSZRO       pszOmniName      = 0;
   bool        fRet             = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDevicePrintMode::mapOmniToUPDF (pszOmniName, &pszPrintModeName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodePrintModes = findPrintModes (pUPDFDevice);

   if (!nodePrintModes)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": !nodePrintModes" << std::endl;
#endif

      return 0;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodePrintModes));

   while (  nodeItem != 0
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszPrintModeName))
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

Enumeration * UPDFDevicePrintMode::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet           = 0;
   UPDFDevice                 *pUPDFDevice    = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodePrintModes = 0;
   XmlNodePtr                  nodePrintMode  = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodePrintModes = findPrintModes (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": nodePrintModes = " << std::hex << (int *)nodePrintModes << std::dec << std::endl;
#endif

   if (!nodePrintModes)
   {
      goto done;
   }

   nodePrintMode = XMLFirstNode (XMLGetChildrenNode (nodePrintModes));
   nodePrintMode = skipInvalidPrintModes (nodePrintMode);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": nodePrintMode = " << std::hex << (int *)nodePrintMode << std::dec << std::endl;
#endif

   while (nodePrintMode)
   {
      DevicePrintMode *pPrintMode = 0;
      JobProperties   *pJP        = 0;

      pPrintMode = createFromXMLNode (pDevice_d, nodePrintMode);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": pPrintMode = " << std::hex << (int *)pPrintMode << std::dec << std::endl;
#endif

      if (pPrintMode)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pPrintMode->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pPrintMode;
      }

      nodePrintMode = XMLNextNode (nodePrintMode);
      nodePrintMode = skipInvalidPrintModes (nodePrintMode);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevicePrintMode ()) DebugOutput::getErrorStream () << "UPDFDevicePrintMode::" << __FUNCTION__ << ": nodePrintMode = " << std::hex << (int *)nodePrintMode << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDevicePrintMode::
findPrintModes (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodePrintModes = 0;

   if (!pUPDFDevice)
   {
      return nodePrintModes;
   }

   if (  ((nodePrintModes = FINDUDRENTRY (pUPDFDevice, nodePrintModes, "PrintCapabilities", UPDFDevicePrintMode)) != 0)
      && ((nodePrintModes = FINDUDRENTRY (pUPDFDevice, nodePrintModes, "Features",          UPDFDevicePrintMode)) != 0)
      && ((nodePrintModes = FINDUDRENTRY (pUPDFDevice, nodePrintModes, "Color",             UPDFDevicePrintMode)) != 0)
      )
      return nodePrintModes;

   return nodePrintModes;
}

bool UPDFDevicePrintMode::
mapUPDFToOmni (PSZCRO  pszUPDFValue,
               PSZRO  *ppszOmniValue)
{
   if (0 == strcmp (pszUPDFValue, "Monochrome"))
   {
      if (ppszOmniValue)
      {
         *ppszOmniValue = "PRINT_MODE_1_ANY";
      }

      return true;
   }
   else if (0 == strcmp (pszUPDFValue, "Color"))
   {
      if (ppszOmniValue)
      {
         *ppszOmniValue = "PRINT_MODE_24_RGB";
      }

      return true;
   }

   return false;
}

bool UPDFDevicePrintMode::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
   if (0 == strcmp (pszOmniValue, "PRINT_MODE_1_ANY"))
   {
      if (ppszUPDFValue)
      {
         *ppszUPDFValue = "Monochrome";
      }

      return true;
   }
   else if (0 == strcmp (pszOmniValue, "PRINT_MODE_24_RGB"))
   {
      if (ppszUPDFValue)
      {
         *ppszUPDFValue = "Color";
      }

      return true;
   }

   return false;
}

#ifndef RETAIL

void UPDFDevicePrintMode::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDevicePrintMode::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDevicePrintMode: "
       << DevicePrintMode::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDevicePrintMode& const_self)
{
   UPDFDevicePrintMode& self = const_cast<UPDFDevicePrintMode&>(const_self);
   std::ostringstream   oss;

   os << self.toString (oss);

   return os;
}
