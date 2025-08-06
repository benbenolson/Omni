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
#include <XMLDeviceNUp.hpp>

#include <JobProperties.hpp>

XMLDeviceNUp::
XMLDeviceNUp (Device                *pDevice,
              PSZRO                  pszJobProperties,
              BinaryData            *pbdData,
              bool                   fSimulationRequired,
              XmlNodePtr             node)
   : DeviceNUp (pDevice,
                pszJobProperties,
                pbdData,
                fSimulationRequired)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceNUp::
~XMLDeviceNUp ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceNUp * XMLDeviceNUp::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr   docDeviceNUps = pXMLDevice->getDocNUps ();
   XmlNodePtr  rootDeviceNUp = XMLDocGetRootElement (docDeviceNUps);
   XmlNodePtr  elmDeviceNUp  = 0;
   DeviceNUp  *pNUpRet       = 0;

   if (!rootDeviceNUp)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !rootDeviceNUp " << std::endl;
#endif

      return 0;
   }

   elmDeviceNUp = XMLFirstNode (rootDeviceNUp);
   if (!elmDeviceNUp)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !elmDeviceNUp " << std::endl;
#endif

      return 0;
   }

   int iX             = -1;
   int iY             = -1;
   int indexDirection = -1;

   if (!getComponents (pszJobProperties,
                       &iX,
                       &iY,
                       0,
                       &indexDirection))
   {
      return pXMLDevice->getDefaultNUp ();
   }

   elmDeviceNUp = XMLFirstNode (XMLGetChildrenNode (elmDeviceNUp));

   while (  elmDeviceNUp
         && !pNUpRet
         )
   {
      XmlNodePtr  elmNumberUp         = 0;
      int         iXElm               = -1;
      int         iYElm               = -1;
      PSZRO       pszDirection        = 0;
      int         indexDirectionElm   = -1;
      PSZRO       pszCommand          = 0;
      BinaryData *pbdData             = 0;
      bool        fSimulationRequired = false;

      try
      {
         elmNumberUp = XMLFirstNode (XMLGetChildrenNode (elmDeviceNUp));

         if (elmNumberUp)
         {
            // Read in the X number
            iXElm = getXMLContentInt (elmNumberUp, docDeviceNUps, "x");
            // Read in the Y number
            iYElm = getXMLContentInt (elmNumberUp, docDeviceNUps, "y");
         }

         // Read in the capability
         pszDirection = getXMLContentString (elmDeviceNUp, docDeviceNUps, "NumberUpDirection");

         if (pszDirection)
         {
            indexDirectionElm = DeviceNUp::directionIndex (pszDirection);

            XMLFree ((void *)pszDirection);
         }

         fSimulationRequired = getXMLContentBool (elmDeviceNUp, docDeviceNUps, "simulationRequired", false, false);

         if (  iXElm             == iX
            && iYElm             == iY
            && indexDirectionElm == indexDirection
            )
         {
            // Read in the command
            pszCommand = getXMLContentString (elmDeviceNUp, docDeviceNUps, "command");

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
            if (DebugOutput::shouldOutputXMLDeviceNUp ())
            {
               DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": iXElm               = " << iXElm << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": iYElm               = " << iYElm << std::endl;
               if (pbdData)
                  DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": pbdData             = " << *pbdData << std::endl;
               else
                  DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": indexDirectionElm   = " << indexDirectionElm << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
            }
#endif

            // Create the object
            pNUpRet = new XMLDeviceNUp (pDevice,
                                        pszJobProperties,
                                        pbdData,
                                        fSimulationRequired,
                                        elmDeviceNUp);
         }
      }
      catch (std::string *pstringError)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

         delete pstringError;
      }

      elmDeviceNUp = XMLNextNode (elmDeviceNUp);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": returning " << pNUpRet << std::endl;
#endif

   return pNUpRet;
}

DeviceNUp * XMLDeviceNUp::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceNUp::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceNUps = pXMLDevice->getDocNUps ();
   XmlNodePtr  rootDeviceNUp = XMLDocGetRootElement (docDeviceNUps);
   XmlNodePtr  elmDeviceNUp  = 0;
   bool        fFound        = false;

   if (!rootDeviceNUp)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !rootDeviceNUp " << std::endl;
#endif

      return false;
   }

   elmDeviceNUp = XMLFirstNode (rootDeviceNUp);
   if (!elmDeviceNUp)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !elmDeviceNUp " << std::endl;
#endif

      return false;
   }

   int iX             = -1;
   int iY             = -1;
   int indexDirection = -1;

   if (!getComponents (pszJobProperties,
                       &iX,
                       &iY,
                       0,
                       &indexDirection))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceNUp = XMLFirstNode (XMLGetChildrenNode (elmDeviceNUp));

   while (  elmDeviceNUp
         && !fFound
         )
   {
      XmlNodePtr  elmNumberUp       = 0;
      int         iXElm             = -1;
      int         iYElm             = -1;
      PSZRO       pszDirection      = 0;
      int         indexDirectionElm = -1;

      try
      {
         elmNumberUp = XMLFirstNode (XMLGetChildrenNode (elmDeviceNUp));

         if (elmNumberUp)
         {
            // Read in the X number
            iXElm = getXMLContentInt (elmNumberUp, docDeviceNUps, "x");
            // Read in the Y number
            iYElm = getXMLContentInt (elmNumberUp, docDeviceNUps, "y");
         }

         // Read in the capability
         pszDirection = getXMLContentString (elmDeviceNUp, docDeviceNUps, "NumberUpDirection");

         if (pszDirection)
         {
            indexDirectionElm = DeviceNUp::directionIndex (pszDirection);

            XMLFree ((void *)pszDirection);
         }

         if (  iXElm             == iX
            && iYElm             == iY
            && indexDirectionElm == indexDirection
            )
         {
            fFound = true;
         }
      }
      catch (std::string *pstringError)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

         delete pstringError;
      }

      elmDeviceNUp = XMLNextNode (elmDeviceNUp);
   }

   return fFound;
}

PSZCRO XMLDeviceNUp::
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
   if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": returning " << SAFE_PRINT_PSZ (pszDeviceID_d) << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLNUp_Enumerator : public Enumeration
{
public:
   XMLNUp_Enumerator (Device     *pDevice,
                      XmlNodePtr  nodeItem,
                      bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceNUps_d     = 0;
      nodeItem_d          = nodeItem;
      fInDeviceSpecific_d = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceNUps_d = pXMLDevice->getDocNUps ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLNUp_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceNUps_d     = 0;
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
         int                 iXElm        = -1;
         int                 iYElm        = -1;
         std::string        *pstringNUpPD = 0;
         PSZRO               pszDeviceID  = 0;
         std::ostringstream  oss;
         bool                fHandled     = false;
         XmlNodePtr          elmNumberUp  = 0;

         try
         {
            if (fInDeviceSpecific_d)
            {
               pszDeviceID = getXMLContentString (nodeItem_d,
                                                  docDeviceNUps_d,
                                                  "deviceID");

               if (pszDeviceID)
               {
                  oss << "NumberUp"
                      << "="
                      << pszDeviceID;

                  XMLFree ((void *)pszDeviceID);

                  fHandled = true;
               }
            }

            if (!fHandled)
            {
               elmNumberUp = XMLFirstNode (XMLGetChildrenNode (nodeItem_d));

               if (elmNumberUp)
               {
                  // Read in the X number
                  iXElm = getXMLContentInt (elmNumberUp, docDeviceNUps_d, "x");
                  // Read in the Y number
                  iYElm = getXMLContentInt (elmNumberUp, docDeviceNUps_d, "y");
               }

               pstringNUpPD = XMLDevice::getXMLJobProperties (nodeItem_d,
                                                              docDeviceNUps_d,
                                                              "NumberUpDirection");

               if (  iXElm > 0
                  && iYElm > 0
                  && pstringNUpPD
                  )
               {
                  oss << "NumberUp="
                      << iXElm
                      << "X"
                      << iYElm
                      << " "
                      << *pstringNUpPD;

                  fHandled = true;
               }
            }

            if (fHandled)
            {
               pvRet = (void *)new JobProperties (oss.str ().c_str ());
            }
         }
         catch (std::string *pstringError)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDeviceNUp ()) DebugOutput::getErrorStream () << "XMLDeviceNUp::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

            delete pstringError;
         }

         delete pstringNUpPD;

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceNUps_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceNUp::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLNUp_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceNUps = pXMLDevice->getDocNUps ();
   XmlNodePtr  rootDeviceNUp = XMLDocGetRootElement (docDeviceNUps);
   XmlNodePtr  elmDeviceNUp  = 0;

   if (!rootDeviceNUp)
      return new XMLNUp_Enumerator (pDevice_d, 0, false);

   elmDeviceNUp = XMLFirstNode (rootDeviceNUp);
   if (!elmDeviceNUp)
      return new XMLNUp_Enumerator (pDevice_d, 0, false);

   elmDeviceNUp = XMLFirstNode (XMLGetChildrenNode (elmDeviceNUp));

   return new XMLNUp_Enumerator (pDevice_d, elmDeviceNUp, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceNUp::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceNUp::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceNUp: "
       << DeviceNUp::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceNUp& const_self)
{
   XMLDeviceNUp&   self = const_cast<XMLDeviceNUp&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
