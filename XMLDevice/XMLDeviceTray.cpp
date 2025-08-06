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
#include <XMLDeviceTray.hpp>

#include <JobProperties.hpp>

XMLDeviceTray::
XMLDeviceTray (Device      *pDevice,
               PSZRO        pszJobProperties,
               int          iType,
               BinaryData  *pbdData,
               XmlNodePtr   node)
   : DeviceTray (pDevice,
                 pszJobProperties,
                 iType,
                 pbdData)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceTray::
~XMLDeviceTray ()
{
   if (pszDeviceID_d)
   {
      free ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceTray * XMLDeviceTray::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr   docDeviceTrays  = pXMLDevice->getDocTrays ();
   XmlNodePtr  rootDeviceTrays = XMLDocGetRootElement (docDeviceTrays);
   XmlNodePtr  elmDeviceTray   = 0;
   DeviceTray *pTrayRet        = 0;

   if (!rootDeviceTrays)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !rootDeviceTrays " << std::endl;
#endif

      return 0;
   }

   elmDeviceTray = XMLFirstNode (rootDeviceTrays);
   if (!elmDeviceTray)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !elmDeviceTray " << std::endl;
#endif

      return 0;
   }

   PSZRO pszTrayName = 0;

   if (!getComponents (pszJobProperties, &pszTrayName, 0))
   {
      return pXMLDevice->getDefaultTray ();
   }

   elmDeviceTray = XMLFirstNode (XMLGetChildrenNode (elmDeviceTray));

   while (  elmDeviceTray
         && !pTrayRet
         )
   {
      PSZCRO pszElmTrayName = getXMLContentString (elmDeviceTray,
                                                   docDeviceTrays,
                                                   "name");

      if (  pszTrayName
         && pszElmTrayName
         )
      {
         if (0 == strcmp (pszTrayName, pszElmTrayName))
         {
            PSZRO       pszType    = 0;
            int         iType      = 0;
            PSZRO       pszCommand = 0;
            BinaryData *pbdData    = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": Creating \"" << pszElmTrayName << "\"" << std::endl;
#endif

               // Read in the Type
               pszType = getXMLContentString (elmDeviceTray, docDeviceTrays, "trayType");

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": pszType = " << pszType << std::endl;
#endif

               if (pszType)
               {
                  iType = DeviceTray::getReservedValue (pszType);

#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": iType = 0x" << std::hex << iType << std::dec << std::endl;
#endif

                  XMLFree ((void *)pszType);
               }

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceTray, docDeviceTrays, "command");

               if (pszCommand)
               {
                  byte       *pbData          = 0;
                  int         cbData          = 0;

                  if (XMLDevice::parseBinaryData (pszCommand,
                                                  &pbData,
                                                  &cbData))
                  {
                     pbdData = new BinaryDataDelete (pbData, cbData);
                  }

                  XMLFree ((void *)pszCommand);
               }

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTray ())
               {
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": pbdData = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               }
#endif

               // Create the object
               pTrayRet = new XMLDeviceTray (pDevice,
                                             pszJobProperties,
                                             iType,
                                             pbdData,
                                             elmDeviceTray);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmTrayName)
      {
         XMLFree ((void *)pszElmTrayName);
      }

      elmDeviceTray = XMLNextNode (elmDeviceTray);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": returning " << pTrayRet << std::endl;
#endif

   if (pszTrayName)
   {
      free ((void *)pszTrayName);
   }

   return pTrayRet;
}

DeviceTray * XMLDeviceTray::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceTray::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceTrays  = pXMLDevice->getDocTrays ();
   XmlNodePtr  rootDeviceTrays = XMLDocGetRootElement (docDeviceTrays);
   XmlNodePtr  elmDeviceTray   = 0;
   bool        fFound          = false;

   if (!rootDeviceTrays)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !rootDeviceTrays " << std::endl;
#endif

      return false;
   }

   elmDeviceTray = XMLFirstNode (rootDeviceTrays);
   if (!elmDeviceTray)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !elmDeviceTray " << std::endl;
#endif

      return false;
   }

   PSZRO pszTrayName = 0;

   if (!getComponents (pszJobProperties, &pszTrayName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceTray = XMLFirstNode (XMLGetChildrenNode (elmDeviceTray));

   while (  elmDeviceTray
         && !fFound
         )
   {
      PSZCRO pszElmTrayName = getXMLContentString (elmDeviceTray,
                                                   docDeviceTrays,
                                                   "name");

      if (  pszTrayName
         && pszElmTrayName
         )
      {
         if (0 == strcmp (pszTrayName, pszElmTrayName))
         {
            fFound = true;
         }
      }

      if (pszElmTrayName)
      {
         XMLFree ((void *)pszElmTrayName);
      }

      elmDeviceTray = XMLNextNode (elmDeviceTray);
   }

   if (pszTrayName)
   {
      XMLFree ((void *)pszTrayName);
   }

   return fFound;
}

PSZCRO XMLDeviceTray::
getDeviceID ()
{
   if (!pszDeviceID_d)
   {
      if (node_d)
      {
         pszDeviceID_d = getXMLContentString (node_d, XMLGetDocNode (node_d), "deviceID");
      }
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceTray ()) DebugOutput::getErrorStream () << "XMLDeviceTray::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLTray_Enumerator : public Enumeration
{
public:
   XMLTray_Enumerator (Device     *pDevice,
                       XmlNodePtr  nodeItem,
                       bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceTrays_d    = 0;
      nodeItem_d          = nodeItem;
      fInDeviceSpecific_d = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceTrays_d = pXMLDevice->getDocTrays ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLTray_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceTrays_d    = 0;
      nodeItem_d          = 0;
      fInDeviceSpecific_d = false;
   }

   virtual bool
   hasMoreElements ()
   {
      return nodeItem_d ? true : false;
   }

   virtual void *
   nextElement ()
   {
      void *pvRet = 0;

      if (nodeItem_d)
      {
         PSZRO pszTrayName = 0;

         if (fInDeviceSpecific_d)
         {
            pszTrayName = getXMLContentString (nodeItem_d,
                                               docDeviceTrays_d,
                                               "deviceID");
         }

         if (!pszTrayName)
         {
            pszTrayName = getXMLContentString (nodeItem_d,
                                               docDeviceTrays_d,
                                               "name");
         }

         if (pszTrayName)
         {
            std::ostringstream oss;

            oss << "InputTray=" << pszTrayName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszTrayName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceTrays_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceTray::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLTray_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceTrays  = pXMLDevice->getDocTrays ();
   XmlNodePtr  rootDeviceTrays = XMLDocGetRootElement (docDeviceTrays);
   XmlNodePtr  elmDeviceTray   = 0;

   if (!rootDeviceTrays)
      return new XMLTray_Enumerator (pDevice_d, 0, false);

   elmDeviceTray = XMLFirstNode (rootDeviceTrays);
   if (!elmDeviceTray)
      return new XMLTray_Enumerator (pDevice_d, 0, false);

   elmDeviceTray = XMLFirstNode (XMLGetChildrenNode (elmDeviceTray));

   return new XMLTray_Enumerator (pDevice_d, elmDeviceTray, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceTray::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceTray::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceTray: "
       << DeviceTray::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceTray& const_self)
{
   XMLDeviceTray&     self = const_cast<XMLDeviceTray&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
