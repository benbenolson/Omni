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
#include <XMLDeviceTrimming.hpp>

#include <JobProperties.hpp>

XMLDeviceTrimming::
XMLDeviceTrimming (Device                *pDevice,
                   PSZRO                  pszJobProperties,
                   BinaryData            *pbdData,
                   XmlNodePtr             node)
   : DeviceTrimming (pDevice,
                     pszJobProperties,
                     pbdData)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceTrimming::
~XMLDeviceTrimming ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceTrimming * XMLDeviceTrimming::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr       docDeviceTrimmings = pXMLDevice->getDocTrimmings ();
   XmlNodePtr      rootDeviceTrimming = XMLDocGetRootElement (docDeviceTrimmings);
   XmlNodePtr      elmDeviceTrimming  = 0;
   DeviceTrimming *pTrimmingRet       = 0;

   if (!rootDeviceTrimming)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !rootDeviceTrimming " << std::endl;
#endif

      return 0;
   }

   elmDeviceTrimming = XMLFirstNode (rootDeviceTrimming);
   if (!elmDeviceTrimming)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !elmDeviceTrimming " << std::endl;
#endif

      return 0;
   }

   PSZRO pszTrimmingName = 0;

   if (!getComponents (pszJobProperties,
                       &pszTrimmingName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return 0;
   }

   elmDeviceTrimming = XMLFirstNode (XMLGetChildrenNode (elmDeviceTrimming));

   while (  elmDeviceTrimming
         && !pTrimmingRet
         )
   {
      PSZCRO pszElmTrimmingName = getXMLContentString (elmDeviceTrimming,
                                                       docDeviceTrimmings,
                                                       "name");

      if (  pszTrimmingName
         && pszElmTrimmingName
         )
      {
         if (0 == strcmp (pszTrimmingName, pszElmTrimmingName))
         {
            PSZRO       pszCommand = 0;
            BinaryData *pbdData    = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": Creating \"" << pszElmTrimmingName << "\"" << std::endl;
#endif

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceTrimming, docDeviceTrimmings, "command");

               if (pszCommand)
               {
                  byte *pbData = 0;
                  int   cbData = 0;

                  if (XMLDevice::parseBinaryData (pszCommand,
                                                  &pbData,
                                                  &cbData))
                  {
                     pbdData = new BinaryDataDelete (pbData, cbData);
                  }

                  XMLFree ((void *)pszCommand);
               }

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTrimming ())
               {
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": pbdData                = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               }
#endif

               // Create the object
               pTrimmingRet = new XMLDeviceTrimming (pDevice,
                                                             pszJobProperties,
                                                             pbdData,
                                                             elmDeviceTrimming);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmTrimmingName)
      {
         XMLFree ((void *)pszElmTrimmingName);
      }

      elmDeviceTrimming = XMLNextNode (elmDeviceTrimming);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": returning " << pTrimmingRet << std::endl;
#endif

   if (pszTrimmingName)
   {
      XMLFree ((void *)pszTrimmingName);
   }

   return pTrimmingRet;
}

DeviceTrimming * XMLDeviceTrimming::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceTrimming::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceTrimmings = pXMLDevice->getDocTrimmings ();
   XmlNodePtr  rootDeviceTrimming = XMLDocGetRootElement (docDeviceTrimmings);
   XmlNodePtr  elmDeviceTrimming  = 0;
   bool        fFound             = false;

   if (!rootDeviceTrimming)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !rootDeviceTrimming " << std::endl;
#endif

      return false;
   }

   elmDeviceTrimming = XMLFirstNode (rootDeviceTrimming);
   if (!elmDeviceTrimming)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !elmDeviceTrimming " << std::endl;
#endif

      return false;
   }

   PSZRO pszTrimmingName = 0;

   if (!getComponents (pszJobProperties,
                       &pszTrimmingName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceTrimming = XMLFirstNode (XMLGetChildrenNode (elmDeviceTrimming));

   while (  elmDeviceTrimming
         && !fFound
         )
   {
      PSZCRO pszElmTrimmingName = getXMLContentString (elmDeviceTrimming,
                                                       docDeviceTrimmings,
                                                       "name");

      if (  pszTrimmingName
         && pszElmTrimmingName
         )
      {
         if (0 == strcmp (pszTrimmingName, pszElmTrimmingName))
         {
            fFound = true;
         }
      }

      if (pszElmTrimmingName)
      {
         XMLFree ((void *)pszElmTrimmingName);
      }

      elmDeviceTrimming = XMLNextNode (elmDeviceTrimming);
   }

   if (pszTrimmingName)
   {
      XMLFree ((void *)pszTrimmingName);
   }

   return fFound;
}

PSZCRO XMLDeviceTrimming::
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
   if (DebugOutput::shouldOutputXMLDeviceTrimming ()) DebugOutput::getErrorStream () << "XMLDeviceTrimming::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLTrimming_Enumerator : public Enumeration
{
public:
   XMLTrimming_Enumerator (Device     *pDevice,
                           XmlNodePtr  nodeItem,
                           bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d         = pXMLDevice;
      docDeviceTrimmings_d = 0;
      nodeItem_d           = nodeItem;
      fInDeviceSpecific_d  = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceTrimmings_d = pXMLDevice->getDocTrimmings ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLTrimming_Enumerator ()
   {
      pXMLDevice_d         = 0;
      docDeviceTrimmings_d = 0;
      nodeItem_d           = 0;
      fInDeviceSpecific_d  = false;
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
         PSZRO pszTrimmingName = 0;

         if (fInDeviceSpecific_d)
         {
            pszTrimmingName = getXMLContentString (nodeItem_d,
                                                   docDeviceTrimmings_d,
                                                   "deviceID");
         }

         if (!pszTrimmingName)
         {
            pszTrimmingName = getXMLContentString (nodeItem_d,
                                                   docDeviceTrimmings_d,
                                                   "name");
         }

         if (pszTrimmingName)
         {
            std::ostringstream oss;

            oss << "Trimming=" << pszTrimmingName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszTrimmingName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceTrimmings_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceTrimming::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLTrimming_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceTrimmings = pXMLDevice->getDocTrimmings ();
   XmlNodePtr  rootDeviceTrimming = XMLDocGetRootElement (docDeviceTrimmings);
   XmlNodePtr  elmDeviceTrimming  = 0;

   if (!rootDeviceTrimming)
      return new XMLTrimming_Enumerator (pDevice_d, 0, false);

   elmDeviceTrimming = XMLFirstNode (rootDeviceTrimming);
   if (!elmDeviceTrimming)
      return new XMLTrimming_Enumerator (pDevice_d, 0, false);

   elmDeviceTrimming = XMLFirstNode (XMLGetChildrenNode (elmDeviceTrimming));

   return new XMLTrimming_Enumerator (pDevice_d, elmDeviceTrimming, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceTrimming::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceTrimming::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceTrimming: "
       << DeviceTrimming::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceTrimming& const_self)
{
   XMLDeviceTrimming& self = const_cast<XMLDeviceTrimming&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
