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
#include <XMLDeviceSide.hpp>

#include <JobProperties.hpp>

XMLDeviceSide::
XMLDeviceSide (Device                *pDevice,
               PSZRO                  pszJobProperties,
               BinaryData            *pbdData,
               bool                   fSimulationRequired,
               XmlNodePtr             node)
   : DeviceSide (pDevice,
                 pszJobProperties,
                 pbdData,
                 fSimulationRequired)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceSide::
~XMLDeviceSide ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceSide * XMLDeviceSide::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr   docDeviceSides = pXMLDevice->getDocSides ();
   XmlNodePtr  rootDeviceSide = XMLDocGetRootElement (docDeviceSides);
   XmlNodePtr  elmDeviceSide  = 0;
   DeviceSide *pSideRet       = 0;

   if (!rootDeviceSide)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !rootDeviceSide " << std::endl;
#endif

      return 0;
   }

   elmDeviceSide = XMLFirstNode (rootDeviceSide);
   if (!elmDeviceSide)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !elmDeviceSide " << std::endl;
#endif

      return 0;
   }

   PSZRO pszSideName = 0;

   if (!getComponents (pszJobProperties,
                       &pszSideName,
                       0))
   {
      return pXMLDevice->getDefaultSide ();
   }

   elmDeviceSide = XMLFirstNode (XMLGetChildrenNode (elmDeviceSide));

   while (  elmDeviceSide
         && !pSideRet
         )
   {
      PSZCRO pszElmSideName = getXMLContentString (elmDeviceSide,
                                                   docDeviceSides,
                                                   "name");

      if (  pszSideName
         && pszElmSideName
         )
      {
         if (0 == strcmp (pszSideName, pszElmSideName))
         {
            PSZRO        pszCommand          = 0;
            BinaryData  *pbdData             = 0;
            bool         fSimulationRequired = false;

            try
            {
               // Read in the command
               pszCommand = getXMLContentString (elmDeviceSide, docDeviceSides, "command");

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

               fSimulationRequired = getXMLContentBool (elmDeviceSide, docDeviceSides, "simulationRequired", false, false);

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceSide ())
               {
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": pbdData             = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
               }
#endif

               // Create the object
               pSideRet = new XMLDeviceSide (pDevice,
                                             pszJobProperties,
                                             pbdData,
                                             fSimulationRequired,
                                             elmDeviceSide);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmSideName)
      {
         XMLFree ((void *)pszElmSideName);
      }

      elmDeviceSide = XMLNextNode (elmDeviceSide);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": returning " << pSideRet << std::endl;
#endif

   if (pszSideName)
   {
      XMLFree ((void *)pszSideName);
   }

   return pSideRet;
}

DeviceSide * XMLDeviceSide::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceSide::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceSides = pXMLDevice->getDocSides ();
   XmlNodePtr  rootDeviceSide = XMLDocGetRootElement (docDeviceSides);
   XmlNodePtr  elmDeviceSide  = 0;
   bool        fFound         = false;

   if (!rootDeviceSide)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !rootDeviceSide " << std::endl;
#endif

      return false;
   }

   elmDeviceSide = XMLFirstNode (rootDeviceSide);
   if (!elmDeviceSide)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !elmDeviceSide " << std::endl;
#endif

      return false;
   }

   PSZRO pszSideName = 0;

   if (!getComponents (pszJobProperties,
                       &pszSideName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceSide = XMLFirstNode (XMLGetChildrenNode (elmDeviceSide));

   while (  elmDeviceSide
         && !fFound
         )
   {
      PSZCRO pszElmSideName = getXMLContentString (elmDeviceSide,
                                                   docDeviceSides,
                                                   "name");

      if (  pszSideName
         && pszElmSideName
         )
      {
         if (0 == strcmp (pszSideName, pszElmSideName))
         {
            fFound = true;
         }
      }

      if (pszElmSideName)
      {
         XMLFree ((void *)pszElmSideName);
      }

      elmDeviceSide = XMLNextNode (elmDeviceSide);
   }

   if (pszSideName)
   {
      XMLFree ((void *)pszSideName);
   }

   return fFound;
}

PSZCRO XMLDeviceSide::
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
   if (DebugOutput::shouldOutputXMLDeviceSide ()) DebugOutput::getErrorStream () << "XMLDeviceSide::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLSide_Enumerator : public Enumeration
{
public:
   XMLSide_Enumerator (Device     *pDevice,
                       XmlNodePtr  nodeItem,
                       bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceSides_d    = 0;
      nodeItem_d          = nodeItem;
      fInDeviceSpecific_d = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceSides_d = pXMLDevice->getDocSides ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLSide_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceSides_d    = 0;
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
         PSZRO pszSideName = 0;

         if (fInDeviceSpecific_d)
         {
            pszSideName = getXMLContentString (nodeItem_d,
                                               docDeviceSides_d,
                                               "deviceID");
         }

         if (!pszSideName)
         {
            pszSideName = getXMLContentString (nodeItem_d,
                                               docDeviceSides_d,
                                               "name");
         }

         if (pszSideName)
         {
            std::ostringstream oss;

            oss << "Sides=" << pszSideName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszSideName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceSides_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceSide::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLSide_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceSides = pXMLDevice->getDocSides ();
   XmlNodePtr  rootDeviceSide = XMLDocGetRootElement (docDeviceSides);
   XmlNodePtr  elmDeviceSide  = 0;

   if (!rootDeviceSide)
      return new XMLSide_Enumerator (pDevice_d, 0, false);

   elmDeviceSide = XMLFirstNode (rootDeviceSide);
   if (!elmDeviceSide)
      return new XMLSide_Enumerator (pDevice_d, 0, false);

   elmDeviceSide = XMLFirstNode (XMLGetChildrenNode (elmDeviceSide));

   return new XMLSide_Enumerator (pDevice_d, elmDeviceSide, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceSide::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceSide::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceSide: "
       << DeviceSide::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceSide& const_self)
{
   XMLDeviceSide&     self = const_cast<XMLDeviceSide&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
