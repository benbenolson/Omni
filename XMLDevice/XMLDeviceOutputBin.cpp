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
#include <XMLDeviceOutputBin.hpp>

#include <JobProperties.hpp>

XMLDeviceOutputBin::
XMLDeviceOutputBin (Device                *pDevice,
                    PSZRO                  pszJobProperties,
                    BinaryData            *pbdData,
                    XmlNodePtr             node)
   : DeviceOutputBin (pDevice,
                      pszJobProperties,
                      pbdData)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceOutputBin::
~XMLDeviceOutputBin ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceOutputBin * XMLDeviceOutputBin::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr        docDeviceOutputBins = pXMLDevice->getDocOutputBins ();
   XmlNodePtr       rootDeviceOutputBin = XMLDocGetRootElement (docDeviceOutputBins);
   XmlNodePtr       elmDeviceOutputBin  = 0;
   DeviceOutputBin *pOutputBinRet       = 0;

   if (!rootDeviceOutputBin)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !rootDeviceOutputBin " << std::endl;
#endif

      return 0;
   }

   elmDeviceOutputBin = XMLFirstNode (rootDeviceOutputBin);
   if (!elmDeviceOutputBin)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !elmDeviceOutputBin " << std::endl;
#endif

      return 0;
   }

   PSZRO pszOutputBinName = 0;

   if (!getComponents (pszJobProperties,
                       &pszOutputBinName,
                       0))
   {
      return pXMLDevice->getDefaultOutputBin ();
   }

   elmDeviceOutputBin = XMLFirstNode (XMLGetChildrenNode (elmDeviceOutputBin));

   while (  elmDeviceOutputBin
         && !pOutputBinRet
         )
   {
      PSZCRO pszElmOutputBinName = getXMLContentString (elmDeviceOutputBin,
                                                        docDeviceOutputBins,
                                                        "name");

      if (  pszOutputBinName
         && pszElmOutputBinName
         )
      {
         if (0 == strcmp (pszOutputBinName, pszElmOutputBinName))
         {
            PSZRO       pszCommand             = 0;
            BinaryData *pbdData                = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": Creating \"" << pszElmOutputBinName << "\"" << std::endl;
#endif

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceOutputBin, docDeviceOutputBins, "command");

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
               if (DebugOutput::shouldOutputXMLDeviceOutputBin ())
               {
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": pbdData                = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               }
#endif

               // Create the object
               pOutputBinRet = new XMLDeviceOutputBin (pDevice,
                                                       pszJobProperties,
                                                       pbdData,
                                                       elmDeviceOutputBin);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmOutputBinName)
      {
         XMLFree ((void *)pszElmOutputBinName);
      }

      elmDeviceOutputBin = XMLNextNode (elmDeviceOutputBin);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": returning " << pOutputBinRet << std::endl;
#endif

   if (pszOutputBinName)
   {
      free ((void *)pszOutputBinName);
   }

   return pOutputBinRet;
}

DeviceOutputBin * XMLDeviceOutputBin::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceOutputBin::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceOutputBins = pXMLDevice->getDocOutputBins ();
   XmlNodePtr  rootDeviceOutputBin = XMLDocGetRootElement (docDeviceOutputBins);
   XmlNodePtr  elmDeviceOutputBin  = 0;
   bool        fFound              = false;

   if (!rootDeviceOutputBin)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !rootDeviceOutputBin " << std::endl;
#endif

      return false;
   }

   elmDeviceOutputBin = XMLFirstNode (rootDeviceOutputBin);
   if (!elmDeviceOutputBin)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !elmDeviceOutputBin " << std::endl;
#endif

      return false;
   }

   PSZRO pszOutputBinName = 0;

   if (!getComponents (pszJobProperties,
                       &pszOutputBinName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceOutputBin = XMLFirstNode (XMLGetChildrenNode (elmDeviceOutputBin));

   while (  elmDeviceOutputBin
         && !fFound
         )
   {
      PSZCRO pszElmOutputBinName = getXMLContentString (elmDeviceOutputBin,
                                                        docDeviceOutputBins,
                                                        "name");

      if (  pszOutputBinName
         && pszElmOutputBinName
         )
      {
         if (0 == strcmp (pszOutputBinName, pszElmOutputBinName))
         {
            fFound = true;
         }
      }

      if (pszElmOutputBinName)
      {
         XMLFree ((void *)pszElmOutputBinName);
      }

      elmDeviceOutputBin = XMLNextNode (elmDeviceOutputBin);
   }

   if (pszOutputBinName)
   {
      XMLFree ((void *)pszOutputBinName);
   }

   return fFound;
}

PSZCRO XMLDeviceOutputBin::
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
   if (DebugOutput::shouldOutputXMLDeviceOutputBin ()) DebugOutput::getErrorStream () << "XMLDeviceOutputBin::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLOutputBin_Enumerator : public Enumeration
{
public:
   XMLOutputBin_Enumerator (Device     *pDevice,
                            XmlNodePtr  nodeItem,
                            bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d          = pXMLDevice;
      docDeviceOutputBins_d = 0;
      nodeItem_d            = nodeItem;
      fInDeviceSpecific_d   = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceOutputBins_d = pXMLDevice->getDocOutputBins ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual
   ~XMLOutputBin_Enumerator ()
   {
      pXMLDevice_d          = 0;
      docDeviceOutputBins_d = 0;
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
         PSZRO pszOutputBinName = 0;

         if (fInDeviceSpecific_d)
         {
            pszOutputBinName = getXMLContentString (nodeItem_d,
                                                    docDeviceOutputBins_d,
                                                    "deviceID");
         }

         if (!pszOutputBinName)
         {
            pszOutputBinName = getXMLContentString (nodeItem_d,
                                                    docDeviceOutputBins_d,
                                                    "name");
         }

         if (pszOutputBinName)
         {
            std::ostringstream oss;

            oss << "OutputBin=" << pszOutputBinName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszOutputBinName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceOutputBins_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceOutputBin::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLOutputBin_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceOutputBins = pXMLDevice->getDocOutputBins ();
   XmlNodePtr  rootDeviceOutputBin = XMLDocGetRootElement (docDeviceOutputBins);
   XmlNodePtr  elmDeviceOutputBin  = 0;

   if (!rootDeviceOutputBin)
      return new XMLOutputBin_Enumerator (pDevice_d, 0, false);

   elmDeviceOutputBin = XMLFirstNode (rootDeviceOutputBin);
   if (!elmDeviceOutputBin)
      return new XMLOutputBin_Enumerator (pDevice_d, 0, false);

   elmDeviceOutputBin = XMLFirstNode (XMLGetChildrenNode (elmDeviceOutputBin));

   return new XMLOutputBin_Enumerator (pDevice_d, elmDeviceOutputBin, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceOutputBin::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceOutputBin::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceOutputBin: "
       << DeviceOutputBin::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceOutputBin& const_self)
{
   XMLDeviceOutputBin& self = const_cast<XMLDeviceOutputBin&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
