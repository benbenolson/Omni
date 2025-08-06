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
#include <XMLDeviceSheetCollate.hpp>

#include <JobProperties.hpp>

XMLDeviceSheetCollate::
XMLDeviceSheetCollate (Device                *pDevice,
                       PSZRO                  pszJobProperties,
                       BinaryData            *pbdData,
                       XmlNodePtr             node)
   : DeviceSheetCollate (pDevice,
                         pszJobProperties,
                         pbdData)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceSheetCollate::
~XMLDeviceSheetCollate ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceSheetCollate * XMLDeviceSheetCollate::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr           docDeviceSheetCollates = pXMLDevice->getDocSheetCollates ();
   XmlNodePtr          rootDeviceSheetCollate = XMLDocGetRootElement (docDeviceSheetCollates);
   XmlNodePtr          elmDeviceSheetCollate  = 0;
   DeviceSheetCollate *pSheetCollateRet       = 0;

   if (!rootDeviceSheetCollate)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !rootDeviceSheetCollate " << std::endl;
#endif

      return 0;
   }

   elmDeviceSheetCollate = XMLFirstNode (rootDeviceSheetCollate);
   if (!elmDeviceSheetCollate)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !elmDeviceSheetCollate " << std::endl;
#endif

      return 0;
   }

   PSZRO pszSheetCollateName = 0;

   if (!getComponents (pszJobProperties,
                       &pszSheetCollateName,
                       0))
   {
      return pXMLDevice->getDefaultSheetCollate ();
   }

   elmDeviceSheetCollate = XMLFirstNode (XMLGetChildrenNode (elmDeviceSheetCollate));

   while (  elmDeviceSheetCollate
         && !pSheetCollateRet
         )
   {
      PSZCRO pszElmSheetCollateName = getXMLContentString (elmDeviceSheetCollate,
                                                           docDeviceSheetCollates,
                                                           "name");

      if (  pszSheetCollateName
         && pszElmSheetCollateName
         )
      {
         if (0 == strcmp (pszSheetCollateName, pszElmSheetCollateName))
         {
            PSZRO       pszCommand = 0;
            BinaryData *pbdData    = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": Creating \"" << pszElmSheetCollateName << "\"" << std::endl;
#endif

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceSheetCollate, docDeviceSheetCollates, "command");

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
               if (DebugOutput::shouldOutputXMLDeviceSheetCollate ())
               {
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": pbdData                = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               }
#endif

               // Create the object
               pSheetCollateRet = new XMLDeviceSheetCollate (pDevice,
                                                             pszJobProperties,
                                                             pbdData,
                                                             elmDeviceSheetCollate);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmSheetCollateName)
      {
         XMLFree ((void *)pszElmSheetCollateName);
      }

      elmDeviceSheetCollate = XMLNextNode (elmDeviceSheetCollate);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": returning " << pSheetCollateRet << std::endl;
#endif

   if (pszSheetCollateName)
   {
      XMLFree ((void *)pszSheetCollateName);
   }

   return pSheetCollateRet;
}

DeviceSheetCollate * XMLDeviceSheetCollate::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceSheetCollate::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceSheetCollates = pXMLDevice->getDocSheetCollates ();
   XmlNodePtr  rootDeviceSheetCollate = XMLDocGetRootElement (docDeviceSheetCollates);
   XmlNodePtr  elmDeviceSheetCollate  = 0;
   bool        fFound                 = false;

   if (!rootDeviceSheetCollate)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !rootDeviceSheetCollate " << std::endl;
#endif

      return false;
   }

   elmDeviceSheetCollate = XMLFirstNode (rootDeviceSheetCollate);
   if (!elmDeviceSheetCollate)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !elmDeviceSheetCollate " << std::endl;
#endif

      return false;
   }

   PSZRO pszSheetCollateName = 0;

   if (!getComponents (pszJobProperties,
                       &pszSheetCollateName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceSheetCollate = XMLFirstNode (XMLGetChildrenNode (elmDeviceSheetCollate));

   while (  elmDeviceSheetCollate
         && !fFound
         )
   {
      PSZCRO pszElmSheetCollateName = getXMLContentString (elmDeviceSheetCollate,
                                                           docDeviceSheetCollates,
                                                           "name");

      if (  pszSheetCollateName
         && pszElmSheetCollateName
         )
      {
         if (0 == strcmp (pszSheetCollateName, pszElmSheetCollateName))
         {
            fFound = true;
         }
      }

      if (pszElmSheetCollateName)
      {
         XMLFree ((void *)pszElmSheetCollateName);
      }

      elmDeviceSheetCollate = XMLNextNode (elmDeviceSheetCollate);
   }

   if (pszSheetCollateName)
   {
      XMLFree ((void *)pszSheetCollateName);
   }

   return fFound;
}

PSZCRO XMLDeviceSheetCollate::
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
   if (DebugOutput::shouldOutputXMLDeviceSheetCollate ()) DebugOutput::getErrorStream () << "XMLDeviceSheetCollate::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLSheetCollate_Enumerator : public Enumeration
{
public:
   XMLSheetCollate_Enumerator (Device     *pDevice,
                               XmlNodePtr  nodeItem,
                               bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d             = pXMLDevice;
      docDeviceSheetCollates_d = 0;
      nodeItem_d               = nodeItem;
      fInDeviceSpecific_d      = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceSheetCollates_d = pXMLDevice->getDocSheetCollates ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLSheetCollate_Enumerator ()
   {
      pXMLDevice_d             = 0;
      docDeviceSheetCollates_d = 0;
      nodeItem_d               = 0;
      fInDeviceSpecific_d      = false;
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
         PSZRO pszSheetCollateName = 0;

         if (fInDeviceSpecific_d)
         {
            pszSheetCollateName = getXMLContentString (nodeItem_d,
                                                       docDeviceSheetCollates_d,
                                                       "deviceID");
         }

         if (!pszSheetCollateName)
         {
            pszSheetCollateName = getXMLContentString (nodeItem_d,
                                                       docDeviceSheetCollates_d,
                                                       "name");
         }

         if (pszSheetCollateName)
         {
            std::ostringstream oss;

            oss << "SheetCollate=" << pszSheetCollateName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszSheetCollateName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceSheetCollates_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceSheetCollate::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLSheetCollate_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceSheetCollates = pXMLDevice->getDocSheetCollates ();
   XmlNodePtr  rootDeviceSheetCollate = XMLDocGetRootElement (docDeviceSheetCollates);
   XmlNodePtr  elmDeviceSheetCollate  = 0;

   if (!rootDeviceSheetCollate)
      return new XMLSheetCollate_Enumerator (pDevice_d, 0, false);

   elmDeviceSheetCollate = XMLFirstNode (rootDeviceSheetCollate);
   if (!elmDeviceSheetCollate)
      return new XMLSheetCollate_Enumerator (pDevice_d, 0, false);

   elmDeviceSheetCollate = XMLFirstNode (XMLGetChildrenNode (elmDeviceSheetCollate));

   return new XMLSheetCollate_Enumerator (pDevice_d, elmDeviceSheetCollate, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceSheetCollate::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceSheetCollate::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceSheetCollate: "
       << DeviceSheetCollate::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceSheetCollate& const_self)
{
   XMLDeviceSheetCollate& self = const_cast<XMLDeviceSheetCollate&>(const_self);
   std::ostringstream     oss;

   os << self.toString (oss);

   return os;
}
