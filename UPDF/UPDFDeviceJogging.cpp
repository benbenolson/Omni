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

#include <UPDFDeviceJogging.hpp>

/*
*/
static int     getOmniJoggingID             (xmlNodePtr      node);

UPDFDeviceJogging::
UPDFDeviceJogging (Device      *pDevice,
                   int          id,
                   BinaryData  *data,
                   xmlNodePtr   node)
   : DeviceJogging (pDevice,
                    id,
                    data)
{
   node_d = node;
}

UPDFDeviceJogging::
~UPDFDeviceJogging ()
{
}

DeviceJogging * UPDFDeviceJogging::
createV (int id)
{
   return UPDFDeviceJogging::create (pDevice_d, id);
}

DeviceJogging * UPDFDeviceJogging::
create (Device *pDevice,
        PSZCRO  pszId)
{
   UPDFDevice *pUPDFDevice  = UPDFDevice::isAUPDFDevice (pDevice);
   xmlNodePtr  nodeJoggings = 0;

   nodeJoggings = findJoggings (pUPDFDevice);

   if (nodeJoggings)
   {
      xmlNodePtr nodeItem  = firstNode (nodeJoggings->xmlChildrenNode);
      xmlNodePtr nodeFound = 0;

      while (nodeItem != 0)
      {
         PSZCRO pszNodeId = (PSZCRO)xmlGetProp (nodeItem, (xmlChar *)"ID");

         if (pszNodeId)
         {
            if (0 == strcmp (pszNodeId, pszId))
            {
               nodeFound = nodeItem;
            }

            free ((void *)pszNodeId);
         }

         if (nodeFound)
         {
            int id = getOmniJoggingID (nodeFound);

            if (DeviceJogging::JOGGING_UNLISTED != id)
            {
               return create (pDevice, id);
            }

            nodeFound = 0;
         }

         nodeItem = nextNode (nodeItem);
      }
   }

   return 0;
}

static int
getOmniJoggingID (xmlNodePtr node)
{
   int id = DeviceJogging::JOGGING_UNLISTED;

   if (!node)
   {
      return id;
   }

   PSZCRO pszID = (PSZCRO)xmlGetProp (node, (xmlChar *)"Predefined_ID");

   if (pszID)
   {
      id = DeviceJogging::nameToID (pszID);

      free ((void *)pszID);
   }

   return id;
}

static xmlNodePtr
skipInvalidJoggings (xmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   int id = getOmniJoggingID (node);

   while (  node
         && (id == DeviceJogging::JOGGING_UNLISTED)
         )
   {
      node = nextNode (node);
      id   = getOmniJoggingID (node);
   }

   return node;
}

static DeviceJogging *
createFromXMLNode (Device      *pDevice,
                   xmlNodePtr   node,
                   int          id)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);

   if (!pUPDFDevice)
   {
      return 0;
   }

   BinaryData  *data = 0; // @TBD

   return new UPDFDeviceJogging (pUPDFDevice,
                                 id,
                                 data,
                                 node);
}

DeviceJogging * UPDFDeviceJogging::
create (Device *pDevice, int id)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceJogging ()) DebugOutput::getErrorStream () << "UPDFDeviceJogging::" << __FUNCTION__ << ": pDevice = " << pDevice << ", id = " << id << std::endl;
#endif

   if (!pUPDFDevice)
   {
      return 0;
   }

   xmlNodePtr nodeJoggings = 0;

   nodeJoggings = findJoggings (pUPDFDevice);

   if (nodeJoggings)
   {
      xmlNodePtr nodeItem = firstNode (nodeJoggings->xmlChildrenNode);

      while (nodeItem != 0)
      {
         int iJoggingID = getOmniJoggingID (nodeItem);

         if (  id == iJoggingID
            && !(id == DeviceJogging::JOGGING_UNLISTED)
            )
         {
            return createFromXMLNode (pDevice, nodeItem, id);
         }

         nodeItem = nextNode (nodeItem);
      }

      return 0;
   }
   else
   {
      return 0;
   }
}

PSZCRO UPDFDeviceJogging::
getDeviceID ()
{
   return 0;
}

bool UPDFDeviceJogging::
isJoggingSupported (int id)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);

   if (!pUPDFDevice)
   {
      return 0;
   }

   xmlNodePtr nodeJoggings = 0;

   nodeJoggings = findJoggings (pUPDFDevice);

   if (nodeJoggings)
   {
      xmlNodePtr nodeItem = firstNode (nodeJoggings->xmlChildrenNode);

      while (nodeItem != 0)
      {
         int iJoggingID = getOmniJoggingID (nodeItem);

         if (  id == iJoggingID
            && !(id == DeviceJogging::JOGGING_UNLISTED)
            )
         {
            return true;
         }

         nodeItem = nextNode (nodeItem);
      }
   }

   return false;
}

class UPDFJogging_Enumerator : public Enumeration
{
public:
   UPDFJogging_Enumerator (Device *pDevice, xmlNodePtr nodeItem)
   {
      pDevice_d  = pDevice;
      nodeItem_d = skipInvalidJoggings (nodeItem);
   }

   virtual bool hasMoreElements ()
   {
      return nodeItem_d ? true : false;
   }

   virtual void *nextElement ()
   {
      void *pvRet = 0;

      if (nodeItem_d)
      {
         int id = getOmniJoggingID (nodeItem_d);

         pvRet = createFromXMLNode (pDevice_d,
                                    nodeItem_d,
                                    id);

         nodeItem_d = nextNode (nodeItem_d);
         nodeItem_d = skipInvalidJoggings (nodeItem_d);
      }

      return pvRet;
   }

private:
   Device     *pDevice_d;
   xmlNodePtr  nodeItem_d;
};

Enumeration * UPDFDeviceJogging::
getEnumeration ()
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);

   if (!pUPDFDevice)
   {
      return new UPDFJogging_Enumerator (pDevice_d, 0);
   }

   xmlNodePtr nodeJoggings = 0;

   nodeJoggings = findJoggings (pUPDFDevice);

   if (nodeJoggings)
   {
      xmlNodePtr nodeItem = firstNode (nodeJoggings->xmlChildrenNode);

      return new UPDFJogging_Enumerator (pDevice_d, nodeItem);
   }
   else
   {
      return new UPDFJogging_Enumerator (pDevice_d, 0);
   }
}

xmlNodePtr UPDFDeviceJogging::
findJoggings (UPDFDevice *pUPDFDevice)
{
   xmlNodePtr nodeJoggings = 0;

   if (!pUPDFDevice)
   {
      return nodeJoggings;
   }

   if (  ((nodeJoggings = FINDUDRENTRY (pUPDFDevice, nodeJoggings, "PrintCapabilities",  UPDFDeviceJogging)) != 0)
      && ((nodeJoggings = FINDUDRENTRY (pUPDFDevice, nodeJoggings, "Features",           UPDFDeviceJogging)) != 0)
      && ((nodeJoggings = FINDUDRENTRY (pUPDFDevice, nodeJoggings, "FinishingJogging",   UPDFDeviceJogging)) != 0)
      )
      return nodeJoggings;

   return nodeJoggings;
}

std::string UPDFDeviceJogging::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceJogging: "
       << DeviceJogging::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceJogging& const_self)
{
   UPDFDeviceJogging& self = const_cast<UPDFDeviceJogging&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
