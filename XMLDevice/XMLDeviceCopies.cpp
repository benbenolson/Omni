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
#include <XMLDeviceCopies.hpp>

#include <JobProperties.hpp>

XMLDeviceCopies::
XMLDeviceCopies (Device                *pDevice,
                 PSZRO                  pszJobProperties,
                 BinaryData            *pbdData,
                 int                    iMinimum,
                 int                    iMaximum,
                 bool                   fSimulationRequired,
                 XmlNodePtr             node)
   : DeviceCopies (pDevice,
                   pszJobProperties,
                   pbdData,
                   iMinimum,
                   iMaximum,
                   fSimulationRequired)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceCopies::
~XMLDeviceCopies ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceCopies * XMLDeviceCopies::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr     docDeviceCopies  = pXMLDevice->getDocCopies ();
   XmlNodePtr    rootDeviceCopies = XMLDocGetRootElement (docDeviceCopies);
   XmlNodePtr    elmDeviceCopies  = 0;
   DeviceCopies *pCopiesRet       = 0;

   if (!rootDeviceCopies)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !rootDeviceCopies " << std::endl;
#endif

      return 0;
   }

   elmDeviceCopies = XMLFirstNode (rootDeviceCopies);
   if (!elmDeviceCopies)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !elmDeviceCopies " << std::endl;
#endif

      return 0;
   }

   int iCopies = -1;

   if (!getComponents (pszJobProperties, &iCopies))
   {
      return pXMLDevice->getDefaultCopies ();
   }

   elmDeviceCopies = XMLFirstNode (XMLGetChildrenNode (elmDeviceCopies));

   while (  elmDeviceCopies
         && !pCopiesRet
         )
   {
      int iMinimum = getXMLContentInt (elmDeviceCopies,
                                       docDeviceCopies,
                                       "minimum",
                                       true,
                                       -1);
      int iMaximum = getXMLContentInt (elmDeviceCopies,
                                       docDeviceCopies,
                                       "maximum",
                                       true,
                                       -1);

      if (  iMinimum <= iCopies
         && iCopies  <= iMaximum
         )
      {
         PSZRO       pszCommand          = 0;
         BinaryData *pbdData             = 0;
         bool        fSimulationRequired = false;

         try
         {
            // Read in the command
            pszCommand = getXMLContentString (elmDeviceCopies, docDeviceCopies, "command");

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
               pszCommand = 0;
            }

            fSimulationRequired = getXMLContentBool (elmDeviceCopies, docDeviceCopies, "simulationRequired", false, false);

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDeviceCopies ())
            {
               if (pbdData)
                  DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": pbdData             = " << *pbdData << std::endl;
               else
                  DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": iMinimum            = " << iMinimum << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": iMaximum            = " << iMaximum << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
            }
#endif

            // Create the object
            pCopiesRet = new XMLDeviceCopies (pDevice,
                                              pszJobProperties,
                                              pbdData,
                                              iMinimum,
                                              iMaximum,
                                              fSimulationRequired,
                                              elmDeviceCopies);
         }
         catch (std::string *pstringError)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

            delete pstringError;
         }
      }

      elmDeviceCopies = XMLNextNode (elmDeviceCopies);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": returning " << pCopiesRet << std::endl;
#endif

   return pCopiesRet;
}

DeviceCopies * XMLDeviceCopies::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceCopies::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceCopies  = pXMLDevice->getDocCopies ();
   XmlNodePtr  rootDeviceCopies = XMLDocGetRootElement (docDeviceCopies);
   XmlNodePtr  elmDeviceCopies  = 0;
   bool        fFound           = false;

   if (!rootDeviceCopies)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !rootDeviceCopies " << std::endl;
#endif

      return false;
   }

   elmDeviceCopies = XMLFirstNode (rootDeviceCopies);
   if (!elmDeviceCopies)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !elmDeviceCopies " << std::endl;
#endif

      return false;
   }

   int iCopies = -1;

   if (!getComponents (pszJobProperties, &iCopies))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceCopies = XMLFirstNode (XMLGetChildrenNode (elmDeviceCopies));

   while (  elmDeviceCopies
         && !fFound
         )
   {
      int iMinimum = getXMLContentInt (elmDeviceCopies,
                                       docDeviceCopies,
                                       "minimum",
                                       true,
                                       -1);
      int iMaximum = getXMLContentInt (elmDeviceCopies,
                                       docDeviceCopies,
                                       "maximum",
                                       true,
                                       -1);

      if (  iMinimum <= iCopies
         && iCopies  <= iMaximum
         )
      {
         fFound = true;
      }

      elmDeviceCopies = XMLNextNode (elmDeviceCopies);
   }

   return fFound;
}

PSZCRO XMLDeviceCopies::
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
   if (DebugOutput::shouldOutputXMLDeviceCopies ()) DebugOutput::getErrorStream () << "XMLDeviceCopies::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLCopies_Enumerator : public Enumeration
{
public:
   XMLCopies_Enumerator (Device     *pDevice,
                         XmlNodePtr  nodeItem,
                         int         iNumCopies,
                         int         iMinimum,
                         int         iMaximum,
                         bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceCopies_d   = 0;
      nodeItem_d          = nodeItem;
      iNumCopies_d        = iNumCopies;
      iMinimum_d          = iMinimum;
      iMaximum_d          = iMaximum;
      fInDeviceSpecific_d = fInDeviceSpecific;
      fReturnedValue_d    = false;

      if (pXMLDevice)
      {
         docDeviceCopies_d = pXMLDevice->getDocCopies ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLCopies_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceCopies_d   = 0;
      nodeItem_d          = 0;
      iNumCopies_d        = 0;
      iMinimum_d          = 0;
      iMaximum_d          = 0;
      fInDeviceSpecific_d = false;
      fReturnedValue_d    = true;
   }

   virtual bool hasMoreElements ()
   {
      return !fReturnedValue_d;
   }

   virtual void *nextElement ()
   {
      void *pvRet = 0;

      if (  nodeItem_d
         && !fReturnedValue_d
         )
      {
         std::ostringstream oss;

         oss << "Copies=";

         if (fInDeviceSpecific_d)
         {
            PSZRO pszDeviceID = 0;

            pszDeviceID = getXMLContentString (nodeItem_d,
                                               docDeviceCopies_d,
                                               "deviceID");

            if (pszDeviceID)
            {
               oss << pszDeviceID;

               fReturnedValue_d = true;

               XMLFree ((void *)pszDeviceID);
            }
         }

         if (!fReturnedValue_d)
         {
            oss << "{"
                << iNumCopies_d
                << ","
                << iMinimum_d
                << ","
                << iMaximum_d
                << "}";

            fReturnedValue_d = true;
         }

         stringReturn_d = oss.str ();

         pvRet = (void *)new JobProperties (stringReturn_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceCopies_d;
   XmlNodePtr  nodeItem_d;
   int         iNumCopies_d;
   int         iMinimum_d;
   int         iMaximum_d;
   bool        fInDeviceSpecific_d;
   bool        fReturnedValue_d;
   std::string stringReturn_d;
};

Enumeration * XMLDeviceCopies::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLCopies_Enumerator (pDevice_d, 0, 0, 0, 0, false);

   XmlDocPtr   docDeviceCopies  = pXMLDevice->getDocCopies ();
   XmlNodePtr  rootDeviceCopies = XMLDocGetRootElement (docDeviceCopies);
   XmlNodePtr  elmDeviceCopies  = 0;

   if (!rootDeviceCopies)
      return new XMLCopies_Enumerator (pDevice_d, 0, 0, 0, 0, false);

   elmDeviceCopies = XMLFirstNode (rootDeviceCopies);
   if (!elmDeviceCopies)
      return new XMLCopies_Enumerator (pDevice_d, 0, 0, 0, 0, false);

   elmDeviceCopies = XMLFirstNode (XMLGetChildrenNode (elmDeviceCopies));

   return new XMLCopies_Enumerator (pDevice_d,
                                    elmDeviceCopies,
                                    iNumCopies_d,
                                    iMinimum_d,
                                    iMaximum_d,
                                    fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceCopies::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceCopies::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceCopies: "
       << DeviceCopies::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceCopies& const_self)
{
   XMLDeviceCopies&   self = const_cast<XMLDeviceCopies&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
