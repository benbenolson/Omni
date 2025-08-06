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
#include <XMLDeviceMedia.hpp>

#include <JobProperties.hpp>

XMLDeviceMedia::
XMLDeviceMedia (Device      *pDevice,
                PSZRO        pszJobProperties,
                BinaryData  *pbdData,
                int          iColorAdjustRequired,
                int          iAbsorption,
                XmlNodePtr   node)
   : DeviceMedia (pDevice,
                  pszJobProperties,
                  pbdData,
                  iColorAdjustRequired,
                  iAbsorption)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceMedia::
~XMLDeviceMedia ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceMedia * XMLDeviceMedia::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !pXMLDevice" << std::endl;
#endif

      return 0;
   }

   XmlDocPtr    docDeviceMedias  = pXMLDevice->getDocMedias ();
   XmlNodePtr   rootDeviceMedias = XMLDocGetRootElement (docDeviceMedias);
   XmlNodePtr   elmDeviceMedia   = 0;
   DeviceMedia *pMediaRet        = 0;

   if (!rootDeviceMedias)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !rootDeviceMedias" << std::endl;
#endif

      return 0;
   }

   elmDeviceMedia = XMLFirstNode (rootDeviceMedias);

   if (!elmDeviceMedia)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !elmDeviceMedia " << std::endl;
#endif

      return 0;
   }

   PSZRO pszMediaName = 0;

   if (!getComponents (pszJobProperties, &pszMediaName, 0))
   {
      return pXMLDevice->getDefaultMedia ();
   }

   elmDeviceMedia = XMLFirstNode (XMLGetChildrenNode (elmDeviceMedia));

   while (  elmDeviceMedia
         && !pMediaRet
         )
   {
      PSZCRO pszElmMediaName = getXMLContentString (elmDeviceMedia,
                                                    docDeviceMedias,
                                                    "name");

      if (  pszMediaName
         && pszElmMediaName
         )
      {
         if (0 == strcmp (pszMediaName, pszElmMediaName))
         {
            PSZRO       pszCommand           = 0;
            BinaryData *pbdData              = 0;
            int         iColorAdjustRequired = 0;
            PSZRO       pszAbsorption        = 0;
            int         iAbsorption          = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": Creating \"" << pszElmMediaName << "\"" << std::endl;
#endif

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceMedia, docDeviceMedias, "command");

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

               // Read in the color adjust required
               iColorAdjustRequired = getXMLContentInt (elmDeviceMedia, docDeviceMedias, "mediaColorAdjustRequired");

               // Read in the absorption
               pszAbsorption = getXMLContentString (elmDeviceMedia, docDeviceMedias, "mediaAbsorption");

               if (pszAbsorption)
               {
                  iAbsorption = DeviceMedia::getReservedValue (pszAbsorption);

                  XMLFree ((void *)pszAbsorption);
               }

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceMedia ())
               {
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": pbdData = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": iColorAdjustRequired = " << iColorAdjustRequired << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": iAbsorption = 0x" << std::hex << iAbsorption << std::dec << std::endl;
               }
#endif

               // Create the object
               pMediaRet = new XMLDeviceMedia (pDevice,
                                               pszJobProperties,
                                               pbdData,
                                               iColorAdjustRequired,
                                               iAbsorption,
                                               elmDeviceMedia);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmMediaName)
      {
         XMLFree ((void *)pszElmMediaName);
      }

      elmDeviceMedia = XMLNextNode (elmDeviceMedia);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": returning " << pMediaRet << std::endl;
#endif

   if (pszMediaName)
   {
      free ((void *)pszMediaName);
   }

   return pMediaRet;
}

DeviceMedia * XMLDeviceMedia::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceMedia::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceMedias  = pXMLDevice->getDocMedias ();
   XmlNodePtr  rootDeviceMedias = XMLDocGetRootElement (docDeviceMedias);
   XmlNodePtr  elmDeviceMedia   = 0;
   bool        fFound           = false;

   if (!rootDeviceMedias)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !rootDeviceMedias " << std::endl;
#endif

      return false;
   }

   elmDeviceMedia = XMLFirstNode (rootDeviceMedias);
   if (!elmDeviceMedia)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !elmDeviceMedia " << std::endl;
#endif

      return false;
   }

   PSZRO pszMediaName = 0;

   if (!getComponents (pszJobProperties, &pszMediaName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceMedia = XMLFirstNode (XMLGetChildrenNode (elmDeviceMedia));

   while (  elmDeviceMedia
         && !fFound
         )
   {
      PSZCRO pszElmMediaName = getXMLContentString (elmDeviceMedia,
                                                    docDeviceMedias,
                                                    "name");

      if (  pszMediaName
         && pszElmMediaName
         )
      {
         if (0 == strcmp (pszMediaName, pszElmMediaName))
         {
            fFound = true;
         }
      }

      if (pszElmMediaName)
      {
         XMLFree ((void *)pszElmMediaName);
      }

      elmDeviceMedia = XMLNextNode (elmDeviceMedia);
   }

   if (pszMediaName)
   {
      free ((void *)pszMediaName);
   }

   return fFound;
}

PSZCRO XMLDeviceMedia::
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
   if (DebugOutput::shouldOutputXMLDeviceMedia ()) DebugOutput::getErrorStream () << "XMLDeviceMedia::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLMedia_Enumerator : public Enumeration
{
public:
   XMLMedia_Enumerator (Device     *pDevice,
                        XmlNodePtr  nodeItem,
                        bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceMedias_d   = 0;
      nodeItem_d          = nodeItem;
      fInDeviceSpecific_d = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceMedias_d = pXMLDevice->getDocMedias ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLMedia_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceMedias_d   = 0;
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
         PSZRO pszMediaName = 0;

         if (fInDeviceSpecific_d)
         {
            pszMediaName = getXMLContentString (nodeItem_d,
                                                docDeviceMedias_d,
                                                "deviceID");
         }

         if (!pszMediaName)
         {
            pszMediaName = getXMLContentString (nodeItem_d,
                                                docDeviceMedias_d,
                                                "name");
         }

         if (pszMediaName)
         {
            std::ostringstream oss;

            oss << "media=" << pszMediaName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszMediaName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceMedias_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceMedia::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLMedia_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceMedias  = pXMLDevice->getDocMedias ();
   XmlNodePtr  rootDeviceMedias = XMLDocGetRootElement (docDeviceMedias);
   XmlNodePtr  elmDeviceMedia   = 0;

   if (!rootDeviceMedias)
      return new XMLMedia_Enumerator (pDevice_d, 0, false);

   elmDeviceMedia = XMLFirstNode (rootDeviceMedias);
   if (!elmDeviceMedia)
      return new XMLMedia_Enumerator (pDevice_d, 0, false);

   elmDeviceMedia = XMLFirstNode (XMLGetChildrenNode (elmDeviceMedia));

   return new XMLMedia_Enumerator (pDevice_d, elmDeviceMedia, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceMedia::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceMedia::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceMedia: "
       << DeviceMedia::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceMedia& const_self)
{
   XMLDeviceMedia&    self = const_cast<XMLDeviceMedia&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
