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
#include <XMLDeviceResolution.hpp>

#include <JobProperties.hpp>

XMLDeviceResolution::
XMLDeviceResolution (Device      *pDevice,
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
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceResolution::
~XMLDeviceResolution ()
{
   if (pszDeviceID_d)
   {
      free ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceResolution * XMLDeviceResolution::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr         docDeviceResolutions  = pXMLDevice->getDocResolutions ();
   XmlNodePtr        rootDeviceResolutions = XMLDocGetRootElement (docDeviceResolutions);
   XmlNodePtr        elmDeviceResolution   = 0;
   DeviceResolution *pResolutionRet        = 0;

   if (!rootDeviceResolutions)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !rootDeviceResolutions " << std::endl;
#endif

      return 0;
   }

   elmDeviceResolution = XMLFirstNode (rootDeviceResolutions);
   if (!elmDeviceResolution)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !elmDeviceResolution " << std::endl;
#endif

      return 0;
   }

   PSZRO pszResolutionName = 0;

   if (!getComponents (pszJobProperties, &pszResolutionName, 0, 0))
   {
      return pXMLDevice->getDefaultResolution ();
   }

   elmDeviceResolution = XMLFirstNode (XMLGetChildrenNode (elmDeviceResolution));

   while (  elmDeviceResolution
         && !pResolutionRet
         )
   {
      PSZCRO pszElmResolutionName = getXMLContentString (elmDeviceResolution,
                                                         docDeviceResolutions,
                                                         "name");

      if (  pszResolutionName
         && pszElmResolutionName
         )
      {
         if (0 == strcmp (pszResolutionName, pszElmResolutionName))
         {
            int         iXInternalRes          = 0;
            int         iYInternalRes          = 0;
            PSZRO       pszCommand             = 0;
            BinaryData *pbdData                = 0;
            PSZRO       pszCapabilities        = 0;
            int         iCapabilities          = 0;
            int         iDestinationBitsPerPel = 0;
            int         iScanlineMultiple      = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": Creating \"" << pszElmResolutionName << "\"" << std::endl;
#endif

               // Read in the internal X resolution
               iXInternalRes = getXMLContentInt (elmDeviceResolution, docDeviceResolutions, "xInternalRes", false);
               // Read in the internal X resolution
               iYInternalRes = getXMLContentInt (elmDeviceResolution, docDeviceResolutions, "yInternalRes", false);

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceResolution, docDeviceResolutions, "command");

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

               // Read in the capability
               pszCapabilities = getXMLContentString (elmDeviceResolution, docDeviceResolutions, "resolutionCapability");

               if (pszCapabilities)
               {
                  iCapabilities = DeviceResolution::getReservedValue (pszCapabilities);

                  XMLFree ((void *)pszCapabilities);
               }

               // Read in the DestinationBitsPerPel
               iScanlineMultiple = getXMLContentInt (elmDeviceResolution, docDeviceResolutions, "resolutionDestinationBitsPerPel");
               // Read in the ScanlineMultiple
               iScanlineMultiple = getXMLContentInt (elmDeviceResolution, docDeviceResolutions, "resolutionScanlineMultiple");

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceResolution ())
               {
                  DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": iXInternalRes          = " << iXInternalRes << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": iYInternalRes          = " << iYInternalRes << std::endl;
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": pbdData                = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": iCapabilities          = " << iCapabilities << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": iDestinationBitsPerPel = " << iDestinationBitsPerPel << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": iScanlineMultiple      = " << iScanlineMultiple << std::endl;
               }
#endif

               // Create the object
               pResolutionRet = new XMLDeviceResolution (pDevice,
                                                         pszJobProperties,
                                                         iXInternalRes,
                                                         iYInternalRes,
                                                         pbdData,
                                                         iCapabilities,
                                                         iDestinationBitsPerPel,
                                                         iScanlineMultiple,
                                                         elmDeviceResolution);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmResolutionName)
      {
         XMLFree ((void *)pszElmResolutionName);
      }

      elmDeviceResolution = XMLNextNode (elmDeviceResolution);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": returning " << pResolutionRet << std::endl;
#endif

   if (pszResolutionName)
   {
      free ((void *)pszResolutionName);
   }

   return pResolutionRet;
}

DeviceResolution * XMLDeviceResolution::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceResolution::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceResolutions  = pXMLDevice->getDocResolutions ();
   XmlNodePtr  rootDeviceResolutions = XMLDocGetRootElement (docDeviceResolutions);
   XmlNodePtr  elmDeviceResolution   = 0;
   bool        fFound                = false;

   if (!rootDeviceResolutions)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !rootDeviceResolutions " << std::endl;
#endif

      return false;
   }

   elmDeviceResolution = XMLFirstNode (rootDeviceResolutions);
   if (!elmDeviceResolution)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !elmDeviceResolution " << std::endl;
#endif

      return false;
   }

   PSZRO pszResolutionName = 0;

   if (!getComponents (pszJobProperties, &pszResolutionName, 0, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceResolution = XMLFirstNode (XMLGetChildrenNode (elmDeviceResolution));

   while (  elmDeviceResolution
         && !fFound
         )
   {
      PSZCRO pszElmResolutionName = getXMLContentString (elmDeviceResolution,
                                                         docDeviceResolutions,
                                                         "name");

      if (  pszResolutionName
         && pszElmResolutionName
         )
      {
         if (0 == strcmp (pszResolutionName, pszElmResolutionName))
         {
            fFound = true;
         }
      }

      if (pszElmResolutionName)
      {
         XMLFree ((void *)pszElmResolutionName);
      }

      elmDeviceResolution = XMLNextNode (elmDeviceResolution);
   }

   if (pszResolutionName)
   {
      free ((void *)pszResolutionName);
   }

   return fFound;
}

PSZCRO XMLDeviceResolution::
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
   if (DebugOutput::shouldOutputXMLDeviceResolution ()) DebugOutput::getErrorStream () << "XMLDeviceResolution::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLResolution_Enumerator : public Enumeration
{
public:
   XMLResolution_Enumerator (Device     *pDevice,
                             XmlNodePtr  nodeItem,
                             bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d           = pXMLDevice;
      docDeviceResolutions_d = 0;
      nodeItem_d             = nodeItem;
      fInDeviceSpecific_d    = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceResolutions_d = pXMLDevice->getDocResolutions ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLResolution_Enumerator ()
   {
      pXMLDevice_d           = 0;
      docDeviceResolutions_d = 0;
      nodeItem_d             = 0;
      fInDeviceSpecific_d    = false;
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
         PSZRO pszResolutionName = 0;

         if (fInDeviceSpecific_d)
         {
            pszResolutionName = getXMLContentString (nodeItem_d,
                                                     docDeviceResolutions_d,
                                                     "deviceID");
         }

         if (!pszResolutionName)
         {
            pszResolutionName = getXMLContentString (nodeItem_d,
                                                     docDeviceResolutions_d,
                                                     "name");
         }

         if (pszResolutionName)
         {
            std::ostringstream oss;

            oss << "Resolution=" << pszResolutionName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszResolutionName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceResolutions_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceResolution::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLResolution_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceResolutions  = pXMLDevice->getDocResolutions ();
   XmlNodePtr  rootDeviceResolutions = XMLDocGetRootElement (docDeviceResolutions);
   XmlNodePtr  elmDeviceResolution   = 0;

   if (!rootDeviceResolutions)
      return new XMLResolution_Enumerator (pDevice_d, 0, false);

   elmDeviceResolution = XMLFirstNode (rootDeviceResolutions);
   if (!elmDeviceResolution)
      return new XMLResolution_Enumerator (pDevice_d, 0, false);

   elmDeviceResolution = XMLFirstNode (XMLGetChildrenNode (elmDeviceResolution));

   return new XMLResolution_Enumerator (pDevice_d, elmDeviceResolution, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceResolution::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceResolution::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceResolution: "
       << DeviceResolution::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceResolution& const_self)
{
   XMLDeviceResolution& self = const_cast<XMLDeviceResolution&>(const_self);
   std::ostringstream   oss;

   os << self.toString (oss);

   return os;
}
