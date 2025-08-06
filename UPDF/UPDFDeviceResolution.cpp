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

#include <UPDFDeviceResolution.hpp>

UPDFDeviceResolution::
UPDFDeviceResolution (Device      *pDevice,
                      PSZRO        pszJobProperties,
                      int          iXInternalRes,
                      int          iYInternalRes,
                      BinaryData  *pbdData,
                      int          iCapabilities,
                      int          iDestinationBitsPerPel,
                      int          iScanlineMultiple,
                      XmlNodePtr   node)
   : DeviceResolution (pDevice,
                       pszJobProperties,
                       iXInternalRes,
                       iYInternalRes,
                       pbdData,
                       iCapabilities,
                       iDestinationBitsPerPel,
                       iScanlineMultiple)
{
   node_d = node;
}

UPDFDeviceResolution::
~UPDFDeviceResolution ()
{
}

static XmlNodePtr
skipInvalidResolutions (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceResolution *
createFromXMLNode (Device      *pDevice,
                   XmlNodePtr   node)
{
   UPDFDevice       *pUPDFDevice    = UPDFDevice::isAUPDFDevice (pDevice);
   int               iVirtualUnits  = 0;
   PSZRO             pszUPDFName    = 0;
   PSZRO             pszOmniJP      = 0;
   DeviceResolution *pResolutionRet = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   iVirtualUnits = pUPDFDevice->getYVirtualUnits ();

   if (iVirtualUnits == 0)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": Error: Could not find virtual units @ " << __LINE__ << std::endl;
#endif

      return 0;
   }

   if (!node)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": Error: !node" << std::endl;
#endif

      return 0;
   }

   pszUPDFName = XMLGetProp (node, "ClassifyingID");

   if (pszUPDFName)
   {
      if (UPDFDeviceResolution::mapUPDFToOmni (pszUPDFName,
                                               &pszOmniJP))
      {
         int                iXInternalRes          = 0;
         int                iYInternalRes          = 0;
         BinaryData        *pbdData                = 0;
         int                iCapabilities          = 0;
         int                iDestinationBitsPerPel = 0;
         int                iScanlineMultiple      = 1;
         std::ostringstream oss;

         oss << "Resolution=" << pszOmniJP;

         pResolutionRet = new UPDFDeviceResolution (pUPDFDevice,
                                                    pszOmniJP,
                                                    iXInternalRes,
                                                    iYInternalRes,
                                                    pbdData,
                                                    iCapabilities,
                                                    iDestinationBitsPerPel,
                                                    iScanlineMultiple,
                                                    node);

         if (pszOmniJP)
         {
            free ((void *)pszOmniJP);
         }
      }
   }

   if (pszUPDFName)
   {
      XMLFree ((void *)pszUPDFName);
   }

   return pResolutionRet;
}

DeviceResolution * UPDFDeviceResolution::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice       *pUPDFDevice       = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr        nodeResolutions   = 0;
   XmlNodePtr        nodeItem          = 0;
   XmlNodePtr        nodeFound         = 0;
   int               iXRes             = 0;
   int               iYRes             = 0;
   PSZRO             pszOmniName       = 0;
   DeviceResolution *pResolutionRet    = 0;
   char              achResolution[25];

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, &pszOmniName, &iXRes, &iYRes))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   sprintf (achResolution, "Resolution_%dx%d", iXRes, iYRes);

   nodeResolutions = findResolutions (pUPDFDevice);

   if (!nodeResolutions)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !rootDeviceResolutions" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeResolutions));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !nodeItem" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem != 0
         && !pResolutionRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, achResolution))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         pResolutionRet = createFromXMLNode (pDevice, nodeItem);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   if (!pResolutionRet)
   {
      pResolutionRet = pUPDFDevice->getDefaultResolution ();
   }

   return pResolutionRet;
}

DeviceResolution * UPDFDeviceResolution::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

PSZCRO UPDFDeviceResolution::
getDeviceID ()
{
   return 0;
}

bool UPDFDeviceResolution::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice *pUPDFDevice       = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeResolutions   = 0;
   XmlNodePtr  nodeItem          = 0;
   XmlNodePtr  nodeFound         = 0;
   int         iXRes             = 0;
   int         iYRes             = 0;
   PSZRO       pszOmniName       = 0;
   char        achResolution[25];
   bool        fRet              = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, &pszOmniName, &iXRes, &iYRes))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   sprintf (achResolution, "Resolution_%dx%d", iXRes, iYRes);

   nodeResolutions = findResolutions (pUPDFDevice);

   if (!nodeResolutions)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !rootDeviceResolutions" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeResolutions));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": !nodeItem" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem != 0
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, achResolution))
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

Enumeration * UPDFDeviceResolution::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet            = 0;
   UPDFDevice                 *pUPDFDevice     = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeResolutions = 0;
   XmlNodePtr                  nodeResolution  = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeResolutions = findResolutions (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": nodeResolutions = " << std::hex << (int *)nodeResolutions << std::dec << std::endl;
#endif

   if (!nodeResolutions)
   {
      goto done;
   }

   nodeResolution = XMLFirstNode (XMLGetChildrenNode (nodeResolutions));
   nodeResolution = skipInvalidResolutions (nodeResolution);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": nodeResolution = " << std::hex << (int *)nodeResolution << std::dec << std::endl;
#endif

   while (nodeResolution)
   {
      DeviceResolution *pResolution = 0;
      JobProperties    *pJP         = 0;

      pResolution = createFromXMLNode (pDevice_d, nodeResolution);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": pResolution = " << std::hex << (int *)pResolution << std::dec << std::endl;
#endif

      if (pResolution)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pResolution->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pResolution;
      }

      nodeResolution = XMLNextNode (nodeResolution);
      nodeResolution = skipInvalidResolutions (nodeResolution);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": nodeResolution = " << std::hex << (int *)nodeResolution << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceResolution::
findResolutions (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeResolutions = 0;

   if (!pUPDFDevice)
   {
      return nodeResolutions;
   }

   if (  ((nodeResolutions = FINDUDRENTRY (pUPDFDevice, nodeResolutions, "PrintCapabilities", UPDFDeviceResolution)) != 0)
      && ((nodeResolutions = FINDUDRENTRY (pUPDFDevice, nodeResolutions, "Features",          UPDFDeviceResolution)) != 0)
      && ((nodeResolutions = FINDUDRENTRY (pUPDFDevice, nodeResolutions, "PrinterResolution", UPDFDeviceResolution)) != 0)
      )
      return nodeResolutions;

   return nodeResolutions;
}

bool UPDFDeviceResolution::
mapUPDFToOmni (PSZCRO  pszUPDFName,
               PSZRO  *ppszOmniJobProperties)
{
   int                iXRes = 0,
                      iYRes = 0,
                      iRet  = 0;
   std::ostringstream oss;
   bool               fRet  = false;

   if (  !pszUPDFName
      || !*pszUPDFName
      )
   {
      return fRet;
   }

   iRet = sscanf (pszUPDFName, "Resolution_%dx%d", &iXRes, &iYRes);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": iXRes = " << iXRes << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": iYRes = " << iYRes << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceResolution ()) DebugOutput::getErrorStream () << "UPDFDeviceResolution::" << __FUNCTION__ << ": iRet  = " << iRet << std::endl;
#endif

   if (  iRet == 2
      && iXRes
      && iYRes
      )
   {
#if 0 // @HACK
      iXRes = iVirtualUnits / iXRes;
      iYRes = iVirtualUnits / iYRes;
#endif

      oss << "Resolution="
          << iXRes
          << "x"
          << iYRes;

      fRet = true;
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

   return fRet;
}

#if 0
bool UPDFDeviceResolution::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
}
#endif

#ifndef RETAIL

void UPDFDeviceResolution::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceResolution::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceResolution: "
       << DeviceResolution::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceResolution& const_self)
{
   UPDFDeviceResolution& self = const_cast<UPDFDeviceResolution&>(const_self);
   std::ostringstream    oss;

   os << self.toString (oss);

   return os;
}
