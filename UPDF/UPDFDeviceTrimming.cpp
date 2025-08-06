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

#include <UPDFDeviceTrimming.hpp>

UPDFDeviceTrimming::
UPDFDeviceTrimming (Device     *pDevice,
                    PSZRO       pszJobProperties,
                    BinaryData *pbdData,
                    XmlNodePtr  node)
   : DeviceTrimming (pDevice,
                     pszJobProperties,
                     pbdData)
{
   node_d = node;
}

UPDFDeviceTrimming::
~UPDFDeviceTrimming ()
{
}

static XmlNodePtr
skipInvalidTrimmings (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceTrimming *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  node)
{
   return 0;
}

DeviceTrimming * UPDFDeviceTrimming::
createS (Device  *pDevice,
         PSZCRO   pszJobProperties)
{
   return 0;
}

DeviceTrimming * UPDFDeviceTrimming::
create (Device  *pDevice,
        PSZCRO   pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceTrimming::
isSupported (PSZCRO pszJobProperties)
{
   return false;
}

PSZCRO UPDFDeviceTrimming::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceTrimming::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet          = 0;
   UPDFDevice                 *pUPDFDevice   = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeTrimmings = 0;
   XmlNodePtr                  nodeTrimming  = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTrimming ()) DebugOutput::getErrorStream () << "UPDFDeviceTrimming::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeTrimmings = findTrimmings (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTrimming ()) DebugOutput::getErrorStream () << "UPDFDeviceTrimming::" << __FUNCTION__ << ": nodeTrimmings = " << std::hex << (int *)nodeTrimmings << std::dec << std::endl;
#endif

   if (!nodeTrimmings)
   {
      goto done;
   }

   nodeTrimming = XMLFirstNode (XMLGetChildrenNode (nodeTrimmings));
   nodeTrimming = skipInvalidTrimmings (nodeTrimming);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTrimming ()) DebugOutput::getErrorStream () << "UPDFDeviceTrimming::" << __FUNCTION__ << ": nodeTrimming = " << std::hex << (int *)nodeTrimming << std::dec << std::endl;
#endif

   while (nodeTrimming)
   {
      DeviceTrimming *pTrimming = 0;
      JobProperties  *pJP       = 0;

      pTrimming = createFromXMLNode (pDevice_d, nodeTrimming);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTrimming ()) DebugOutput::getErrorStream () << "UPDFDeviceTrimming::" << __FUNCTION__ << ": pTrimming = " << std::hex << (int *)pTrimming << std::dec << std::endl;
#endif

      if (pTrimming)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pTrimming->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceTrimming ()) DebugOutput::getErrorStream () << "UPDFDeviceTrimming::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pTrimming;
      }

      nodeTrimming = XMLNextNode (nodeTrimming);
      nodeTrimming = skipInvalidTrimmings (nodeTrimming);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTrimming ()) DebugOutput::getErrorStream () << "UPDFDeviceTrimming::" << __FUNCTION__ << ": nodeTrimming = " << std::hex << (int *)nodeTrimming << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceTrimming::
findTrimmings (UPDFDevice *pUPDFDevice)
{
   return 0;
}

#ifndef RETAIL

void UPDFDeviceTrimming::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceTrimming::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceTrimming: "
       << DeviceTrimming::toString (oss2)
       << "}";

   return oss.str ();
}

std::ostream&
operator<< (std::ostream&             os,
            const UPDFDeviceTrimming& const_self)
{
   UPDFDeviceTrimming& self = const_cast<UPDFDeviceTrimming&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
