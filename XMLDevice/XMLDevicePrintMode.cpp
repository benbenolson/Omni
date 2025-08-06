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
#include <XMLDevicePrintMode.hpp>

#include <JobProperties.hpp>

XMLDevicePrintMode::
XMLDevicePrintMode (Device      *pDevice,
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
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDevicePrintMode::
~XMLDevicePrintMode ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DevicePrintMode * XMLDevicePrintMode::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr        docDevicePrintModes  = pXMLDevice->getDocPrintModes ();
   XmlNodePtr       rootDevicePrintModes = XMLDocGetRootElement (docDevicePrintModes);
   XmlNodePtr       elmDevicePrintMode   = 0;
   DevicePrintMode *pPrintModeRet        = 0;

   if (!rootDevicePrintModes)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !rootDevicePrintModes " << std::endl;
#endif

      return 0;
   }

   elmDevicePrintMode = XMLFirstNode (rootDevicePrintModes);
   if (!elmDevicePrintMode)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !elmDevicePrintMode " << std::endl;
#endif

      return 0;
   }

   PSZRO pszPrintModeName = 0;

   if (!getComponents (pszJobProperties, &pszPrintModeName, 0, 0))
   {
      return pXMLDevice->getDefaultPrintMode ();
   }

   elmDevicePrintMode = XMLFirstNode (XMLGetChildrenNode (elmDevicePrintMode));

   while (  elmDevicePrintMode
         && !pPrintModeRet
         )
   {
      PSZCRO pszElmPrintModeName = getXMLContentString (elmDevicePrintMode,
                                                        docDevicePrintModes,
                                                        "name");

      if (  pszPrintModeName
         && pszElmPrintModeName
         )
      {
         if (0 == strcmp (pszPrintModeName, pszElmPrintModeName))
         {
            int iPhysicalCount = 0;
            int iLogicalCount  = 0;
            int iPlanes        = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": Creating \"" << pszElmPrintModeName << "\"" << std::endl;
#endif

               // Read in the physical count
               iPhysicalCount = getXMLContentInt (elmDevicePrintMode, docDevicePrintModes, "printModePhysicalCount");
               // Read in the logical count
               iLogicalCount = getXMLContentInt (elmDevicePrintMode, docDevicePrintModes, "printModeLogicalCount");
               // Read in the planes
               iPlanes = getXMLContentInt (elmDevicePrintMode, docDevicePrintModes, "printModePlanes");

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDevicePrintMode ())
               {
                  DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": iPhysicalCount = " << iPhysicalCount << std::endl;
                  DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": iLogicalCount = " << iLogicalCount << std::endl;
                  DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": iPlanes = " << iPlanes << std::endl;
               }
#endif

               // Create the object
               pPrintModeRet = new XMLDevicePrintMode (pDevice,
                                                       pszJobProperties,
                                                       iPhysicalCount,
                                                       iLogicalCount,
                                                       iPlanes,
                                                       elmDevicePrintMode);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmPrintModeName)
      {
         XMLFree ((void *)pszElmPrintModeName);
      }

      elmDevicePrintMode = XMLNextNode (elmDevicePrintMode);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": returning " << pPrintModeRet << std::endl;
#endif

   if (pszPrintModeName)
   {
      XMLFree ((void *)pszPrintModeName);
   }

   return pPrintModeRet;
}

DevicePrintMode * XMLDevicePrintMode::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDevicePrintMode::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDevicePrintModes  = pXMLDevice->getDocPrintModes ();
   XmlNodePtr  rootDevicePrintModes = XMLDocGetRootElement (docDevicePrintModes);
   XmlNodePtr  elmDevicePrintMode   = 0;
   bool        fFound               = false;

   if (!rootDevicePrintModes)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !rootDevicePrintModes " << std::endl;
#endif

      return false;
   }

   elmDevicePrintMode = XMLFirstNode (rootDevicePrintModes);
   if (!elmDevicePrintMode)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !elmDevicePrintMode " << std::endl;
#endif

      return false;
   }

   PSZRO pszPrintModeName = 0;

   if (!getComponents (pszJobProperties, &pszPrintModeName, 0, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return 0;
   }


   elmDevicePrintMode = XMLFirstNode (XMLGetChildrenNode (elmDevicePrintMode));

   while (  elmDevicePrintMode
         && !fFound
         )
   {
      PSZCRO pszElmPrintModeName = getXMLContentString (elmDevicePrintMode,
                                                        docDevicePrintModes,
                                                        "name");

      if (  pszPrintModeName
         && pszElmPrintModeName
         )
      {
         if (0 == strcmp (pszPrintModeName, pszElmPrintModeName))
         {
            fFound = true;
         }
      }

      if (pszElmPrintModeName)
      {
         XMLFree ((void *)pszElmPrintModeName);
      }

      elmDevicePrintMode = XMLNextNode (elmDevicePrintMode);
   }

   if (pszPrintModeName)
   {
      XMLFree ((void *)pszPrintModeName);
   }

   return fFound;
}

PSZCRO XMLDevicePrintMode::
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
   if (DebugOutput::shouldOutputXMLDevicePrintMode ()) DebugOutput::getErrorStream () << "XMLDevicePrintMode::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLPrintMode_Enumerator : public Enumeration
{
public:
   XMLPrintMode_Enumerator (Device     *pDevice,
                            XmlNodePtr  nodeItem,
                            bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d          = pXMLDevice;
      docDevicePrintModes_d = 0;
      nodeItem_d            = nodeItem;
      fInDeviceSpecific_d   = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDevicePrintModes_d = pXMLDevice->getDocPrintModes ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLPrintMode_Enumerator ()
   {
      pXMLDevice_d          = 0;
      docDevicePrintModes_d = 0;
      nodeItem_d            = 0;
      fInDeviceSpecific_d   = false;
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
         PSZRO pszPrintModeName = 0;

         if (fInDeviceSpecific_d)
         {
            pszPrintModeName = getXMLContentString (nodeItem_d,
                                                    docDevicePrintModes_d,
                                                    "deviceID");
         }

         if (!pszPrintModeName)
         {
            pszPrintModeName = getXMLContentString (nodeItem_d,
                                                    docDevicePrintModes_d,
                                                    "name");
         }

         if (pszPrintModeName)
         {
            std::ostringstream oss;

            oss << "printmode=" << pszPrintModeName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszPrintModeName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDevicePrintModes_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDevicePrintMode::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLPrintMode_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDevicePrintModes  = pXMLDevice->getDocPrintModes ();
   XmlNodePtr  rootDevicePrintModes = XMLDocGetRootElement (docDevicePrintModes);
   XmlNodePtr  elmDevicePrintMode   = 0;

   if (!rootDevicePrintModes)
      return new XMLPrintMode_Enumerator (pDevice_d, 0, false);

   elmDevicePrintMode = XMLFirstNode (rootDevicePrintModes);
   if (!elmDevicePrintMode)
      return new XMLPrintMode_Enumerator (pDevice_d, 0, false);

   elmDevicePrintMode = XMLFirstNode (XMLGetChildrenNode (elmDevicePrintMode));

   return new XMLPrintMode_Enumerator (pDevice_d, elmDevicePrintMode, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDevicePrintMode::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDevicePrintMode::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDevicePrintMode: "
       << DevicePrintMode::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDevicePrintMode& const_self)
{
   XMLDevicePrintMode& self = const_cast<XMLDevicePrintMode&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
