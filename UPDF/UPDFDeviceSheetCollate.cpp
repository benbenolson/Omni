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

#include <UPDFDeviceSheetCollate.hpp>

UPDFDeviceSheetCollate::
UPDFDeviceSheetCollate (Device     *pDevice,
                        PSZRO       pszJobProperties,
                        BinaryData *pbdData,
                        XmlNodePtr  node)
   : DeviceSheetCollate (pDevice,
                         pszJobProperties,
                         pbdData)
{
   node_d = node;
}

UPDFDeviceSheetCollate::
~UPDFDeviceSheetCollate ()
{
}

static XmlNodePtr
skipInvalidSheetCollates (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceSheetCollate *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  node)
{
   UPDFDevice         *pUPDFDevice         = UPDFDevice::isAUPDFDevice (pDevice);
   PSZRO               pszSheetCollateName = 0;
   BinaryData         *pbdData             = 0; // @TBD
   std::ostringstream  oss;
   DeviceSheetCollate *pSheetCollateRet    = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   pszSheetCollateName = XMLGetProp (node, "ClassifyingID");

   if (pszSheetCollateName)
   {
      PSZRO pszOmniJP = 0;

      if (UPDFDeviceSheetCollate::mapUPDFToOmni (pszSheetCollateName,
                                                 &pszOmniJP))
      {
         oss << "SheetCollate=" << pszOmniJP;

         pSheetCollateRet = new UPDFDeviceSheetCollate (pUPDFDevice,
                                                        oss.str ().c_str (),
                                                        pbdData,
                                                        node);
      }
   }

   if (pszSheetCollateName)
   {
      XMLFree ((void *)pszSheetCollateName);
   }

   return pSheetCollateRet;
}

DeviceSheetCollate * UPDFDeviceSheetCollate::
createS (Device  *pDevice,
         PSZCRO   pszJobProperties)
{
   UPDFDevice         *pUPDFDevice         = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr          nodeSheetCollates   = 0;
   XmlNodePtr          nodeItem            = 0;
   XmlNodePtr          nodeFound           = 0;
   PSZRO               pszSheetCollateName = 0;
   PSZRO               pszOmniName         = 0;
   DeviceSheetCollate *pSheetCollateRet    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceSheetCollate::mapOmniToUPDF (pszOmniName, &pszSheetCollateName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeSheetCollates = findSheetCollates (pUPDFDevice);

   if (!nodeSheetCollates)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !nodeSheetCollates" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeSheetCollates));

   while (  nodeItem != 0
         && !pSheetCollateRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszSheetCollateName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         pSheetCollateRet = createFromXMLNode (pDevice, nodeItem);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   if (!pSheetCollateRet)
   {
      pSheetCollateRet = pUPDFDevice->getDefaultSheetCollate ();
   }

   return pSheetCollateRet;
}

DeviceSheetCollate * UPDFDeviceSheetCollate::
create (Device  *pDevice,
        PSZCRO   pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceSheetCollate::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice *pUPDFDevice         = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeSheetCollates   = 0;
   XmlNodePtr  nodeItem            = 0;
   XmlNodePtr  nodeFound           = 0;
   PSZRO       pszSheetCollateName = 0;
   PSZRO       pszOmniName         = 0;
   bool        fRet                = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceSheetCollate::mapOmniToUPDF (pszOmniName, &pszSheetCollateName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeSheetCollates = findSheetCollates (pUPDFDevice);

   if (!nodeSheetCollates)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": !nodeSheetCollates" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeSheetCollates));

   while (  nodeItem != 0
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszSheetCollateName))
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

PSZCRO UPDFDeviceSheetCollate::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceSheetCollate::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet              = 0;
   UPDFDevice                 *pUPDFDevice       = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeSheetCollates = 0;
   XmlNodePtr                  nodeSheetCollate  = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeSheetCollates = findSheetCollates (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": nodeSheetCollates = " << std::hex << (int *)nodeSheetCollates << std::dec << std::endl;
#endif

   if (!nodeSheetCollates)
   {
      goto done;
   }

   nodeSheetCollate = XMLFirstNode (XMLGetChildrenNode (nodeSheetCollates));
   nodeSheetCollate = skipInvalidSheetCollates (nodeSheetCollate);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": nodeSheetCollate = " << std::hex << (int *)nodeSheetCollate << std::dec << std::endl;
#endif

   while (nodeSheetCollate)
   {
      DeviceSheetCollate *pSheetCollate = 0;
      JobProperties      *pJP           = 0;

      pSheetCollate = createFromXMLNode (pDevice_d, nodeSheetCollate);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": pSheetCollate = " << std::hex << (int *)pSheetCollate << std::dec << std::endl;
#endif

      if (pSheetCollate)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pSheetCollate->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pSheetCollate;
      }

      nodeSheetCollate = XMLNextNode (nodeSheetCollate);
      nodeSheetCollate = skipInvalidSheetCollates (nodeSheetCollate);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceSheetCollate ()) DebugOutput::getErrorStream () << "UPDFDeviceSheetCollate::" << __FUNCTION__ << ": nodeSheetCollate = " << std::hex << (int *)nodeSheetCollate << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceSheetCollate::
findSheetCollates (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeSheetCollates = 0;

   if (!pUPDFDevice)
   {
      return nodeSheetCollates;
   }

   if (  ((nodeSheetCollates = FINDUDRENTRY (pUPDFDevice, nodeSheetCollates, "PrintCapabilities",  UPDFDeviceSheetCollate)) != 0)
      && ((nodeSheetCollates = FINDUDRENTRY (pUPDFDevice, nodeSheetCollates, "Features",           UPDFDeviceSheetCollate)) != 0)
      && ((nodeSheetCollates = FINDUDRENTRY (pUPDFDevice, nodeSheetCollates, "SheetCollate",       UPDFDeviceSheetCollate)) != 0)
      )
      return nodeSheetCollates;

   return nodeSheetCollates;
}

bool UPDFDeviceSheetCollate::
mapUPDFToOmni (PSZCRO  pszUPDFValue,
               PSZRO  *ppszOmniValue)
{
   typedef struct _SheetCollateMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SHEETCOLLATEMAPPING, *PSHEETCOLLATEMAPPING;
   SHEETCOLLATEMAPPING ammFromUPDFToOmni[] = {
      { "Collated",       "SheetCollated"    },
      { "Uncollated",     "SheetUncollated"  }
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

bool UPDFDeviceSheetCollate::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
   typedef struct _SheetCollateMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SHEETCOLLATEMAPPING, *PSHEETCOLLATEMAPPING;
   SHEETCOLLATEMAPPING ammFromOmniToUPDF[] = {
      { "SheetCollated",       "Collated"    },
      { "SheetUncollated",     "Uncollated"  }
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

void UPDFDeviceSheetCollate::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceSheetCollate::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceSheetCollate: "
       << DeviceSheetCollate::toString (oss2)
       << "}";

   return oss.str ();
}

std::ostream&
operator<< (std::ostream&                 os,
            const UPDFDeviceSheetCollate& const_self)
{
   UPDFDeviceSheetCollate& self = const_cast<UPDFDeviceSheetCollate&>(const_self);
   std::ostringstream      oss;

   os << self.toString (oss);

   return os;
}
