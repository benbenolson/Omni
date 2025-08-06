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
#include <XMLDeviceOrientation.hpp>

#include <JobProperties.hpp>

XMLDeviceOrientation::
XMLDeviceOrientation (Device      *pDevice,
                      PSZRO        pszJobProperties,
                      bool         fSimulationRequired,
                      XmlNodePtr   node)
   : DeviceOrientation (pDevice,
                        pszJobProperties,
                        fSimulationRequired)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceOrientation::
~XMLDeviceOrientation ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceOrientation * XMLDeviceOrientation::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr          docDeviceOrientations  = pXMLDevice->getDocOrientations ();
   XmlNodePtr         rootDeviceOrientations = XMLDocGetRootElement (docDeviceOrientations);
   XmlNodePtr         elmDeviceOrientation   = 0;
   DeviceOrientation *pOrientationRet        = 0;

   if (!rootDeviceOrientations)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !rootDeviceOrientations " << std::endl;
#endif

      return 0;
   }

   elmDeviceOrientation = XMLFirstNode (rootDeviceOrientations);
   if (!elmDeviceOrientation)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !elmDeviceOrientation " << std::endl;
#endif

      return 0;
   }

   PSZRO pszOrientationName = 0;

   if (!getComponents (pszJobProperties, &pszOrientationName, 0))
   {
      return pXMLDevice->getDefaultOrientation ();
   }

   elmDeviceOrientation = XMLFirstNode (XMLGetChildrenNode (elmDeviceOrientation));

   while (  elmDeviceOrientation
         && !pOrientationRet
         )
   {
      PSZCRO pszElmOrientationName = getXMLContentString (elmDeviceOrientation,
                                                          docDeviceOrientations,
                                                          "name");

      if (  pszOrientationName
         && pszElmOrientationName
         )
      {
         if (0 == strcmp (pszOrientationName, pszElmOrientationName))
         {
            bool fSimulationRequired = false;

            try
            {
               fSimulationRequired = getXMLContentBool (elmDeviceOrientation, docDeviceOrientations, "simulationRequired", false, false);

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceOrientation ())
               {
                  DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
               }
#endif

               // Create the object
               pOrientationRet = new XMLDeviceOrientation (pDevice,
                                                           pszJobProperties,
                                                           fSimulationRequired,
                                                           elmDeviceOrientation);
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmOrientationName)
      {
         XMLFree ((void *)pszElmOrientationName);
      }

      elmDeviceOrientation = XMLNextNode (elmDeviceOrientation);
   }


#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": returning " << pOrientationRet << std::endl;
#endif

   if (pszOrientationName)
   {
      free ((void *)pszOrientationName);
   }

   return pOrientationRet;
}

DeviceOrientation * XMLDeviceOrientation::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceOrientation::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceOrientations  = pXMLDevice->getDocOrientations ();
   XmlNodePtr  rootDeviceOrientations = XMLDocGetRootElement (docDeviceOrientations);
   XmlNodePtr  elmDeviceOrientation   = 0;
   bool        fFound                 = false;

   if (!rootDeviceOrientations)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !rootDeviceOrientations " << std::endl;
#endif

      return false;
   }

   elmDeviceOrientation = XMLFirstNode (rootDeviceOrientations);
   if (!elmDeviceOrientation)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !elmDeviceOrientation " << std::endl;
#endif

      return false;
   }

   PSZRO pszOrientationName = 0;

   if (!getComponents (pszJobProperties, &pszOrientationName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceOrientation = XMLFirstNode (XMLGetChildrenNode (elmDeviceOrientation));

   while (  elmDeviceOrientation
         && !fFound
         )
   {
      PSZCRO pszElmOrientationName = getXMLContentString (elmDeviceOrientation,
                                                          docDeviceOrientations,
                                                          "name");

      if (  pszOrientationName
         && pszElmOrientationName
         )
      {
         if (0 == strcmp (pszOrientationName, pszElmOrientationName))
         {
            fFound = true;
         }
      }

      if (pszElmOrientationName)
      {
         XMLFree ((void *)pszElmOrientationName);
      }

      elmDeviceOrientation = XMLNextNode (elmDeviceOrientation);
   }

   if (pszOrientationName)
   {
      free ((void *)pszOrientationName);
   }

   return fFound;
}

PSZCRO XMLDeviceOrientation::
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
   if (DebugOutput::shouldOutputXMLDeviceOrientation ()) DebugOutput::getErrorStream () << "XMLDeviceOrientation::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLOrientation_Enumerator : public Enumeration
{
public:
   XMLOrientation_Enumerator (Device     *pDevice,
                              XmlNodePtr  nodeItem,
                              bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d            = pXMLDevice;
      docDeviceOrientations_d = 0;
      nodeItem_d              = nodeItem;
      fInDeviceSpecific_d     = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceOrientations_d = pXMLDevice->getDocOrientations ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLOrientation_Enumerator ()
   {
      pXMLDevice_d            = 0;
      docDeviceOrientations_d = 0;
      nodeItem_d              = 0;
      fInDeviceSpecific_d     = false;
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
         PSZRO pszOrientationName = 0;

         if (fInDeviceSpecific_d)
         {
            pszOrientationName = getXMLContentString (nodeItem_d,
                                                      docDeviceOrientations_d,
                                                      "deviceID");
         }

         if (!pszOrientationName)
         {
            pszOrientationName = getXMLContentString (nodeItem_d,
                                                      docDeviceOrientations_d,
                                                      "name");
         }

         if (pszOrientationName)
         {
            std::ostringstream oss;

            oss << "Rotation=" << pszOrientationName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszOrientationName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceOrientations_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceOrientation::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLOrientation_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceOrientations  = pXMLDevice->getDocOrientations ();
   XmlNodePtr  rootDeviceOrientations = XMLDocGetRootElement (docDeviceOrientations);
   XmlNodePtr  elmDeviceOrientation   = 0;

   if (!rootDeviceOrientations)
      return new XMLOrientation_Enumerator (pDevice_d, 0, false);

   elmDeviceOrientation = XMLFirstNode (rootDeviceOrientations);
   if (!elmDeviceOrientation)
      return new XMLOrientation_Enumerator (pDevice_d, 0, false);

   elmDeviceOrientation = XMLFirstNode (XMLGetChildrenNode (elmDeviceOrientation));

   return new XMLOrientation_Enumerator (pDevice_d, elmDeviceOrientation, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceOrientation::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceOrientation::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceOrientation: "
       << DeviceOrientation::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceOrientation& const_self)
{
   XMLDeviceOrientation& self = const_cast<XMLDeviceOrientation&>(const_self);
   std::ostringstream    oss;

   os << self.toString (oss);

   return os;
}
