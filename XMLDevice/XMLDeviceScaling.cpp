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
#include <XMLDeviceScaling.hpp>

#include <JobProperties.hpp>

XMLDeviceScaling::
XMLDeviceScaling (Device                *pDevice,
                  PSZRO                  pszJobProperties,
                  BinaryData            *pbdData,
                  int                    iMinimumScale,
                  int                    iMaximumScale,
                  XmlNodePtr             node)
   : DeviceScaling (pDevice,
                    pszJobProperties,
                    pbdData,
                    iMinimumScale,
                    iMaximumScale)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceScaling::
~XMLDeviceScaling ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceScaling * XMLDeviceScaling::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr      docDeviceScalings = pXMLDevice->getDocScalings ();
   XmlNodePtr     rootDeviceScaling = XMLDocGetRootElement (docDeviceScalings);
   XmlNodePtr     elmDeviceScaling  = 0;
   DeviceScaling *pScalingRet       = 0;

   if (!rootDeviceScaling)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !rootDeviceScaling " << std::endl;
#endif

      return 0;
   }

   elmDeviceScaling = XMLFirstNode (rootDeviceScaling);
   if (!elmDeviceScaling)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !elmDeviceScaling " << std::endl;
#endif

      return 0;
   }

   PSZRO  pszScalingType     = 0;
   double dScalingPercentage = -1;

   if (!getComponents (pszJobProperties,
                       &pszScalingType,
                       0,
                       &dScalingPercentage))
   {
      return pXMLDevice->getDefaultScaling ();
   }

   elmDeviceScaling = XMLFirstNode (XMLGetChildrenNode (elmDeviceScaling));

   while (  elmDeviceScaling
         && !pScalingRet
         )
   {
      PSZCRO pszElmScalingType = getXMLContentString (elmDeviceScaling,
                                                      docDeviceScalings,
                                                      "allowedType");

      if (  pszScalingType
         && pszElmScalingType
         )
      {
         if (0 == strcmp (pszScalingType, pszElmScalingType))
         {
            int iMinimum = getXMLContentInt (elmDeviceScaling,
                                             docDeviceScalings,
                                             "minimum",
                                             true,
                                             -1);
            int iMaximum = getXMLContentInt (elmDeviceScaling,
                                             docDeviceScalings,
                                             "maximum",
                                             true,
                                             -1);

            if (  iMinimum           <= dScalingPercentage
               && dScalingPercentage <= iMaximum
               )
            {
               PSZRO       pszCommand = 0;
               BinaryData *pbdData    = 0;

               try
               {
                  // Read in the command
                  pszCommand = getXMLContentString (elmDeviceScaling, docDeviceScalings, "command");

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
                  if (DebugOutput::shouldOutputXMLDeviceScaling ())
                  {
                     if (pbdData)
                        DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": pbdData             = " << *pbdData << std::endl;
                     else
                        DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
                     DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": iMinimum            = " << iMinimum << std::endl;
                     DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": iMaximum            = " << iMaximum << std::endl;
                  }
#endif

                  // Create the object
                  pScalingRet = new XMLDeviceScaling (pDevice,
                                                      pszJobProperties,
                                                      pbdData,
                                                      iMinimum,
                                                      iMaximum,
                                                      elmDeviceScaling);
               }
               catch (std::string *pstringError)
               {
#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

                  delete pstringError;
               }
            }
         }
      }

      if (pszElmScalingType)
      {
         XMLFree ((void *)pszElmScalingType);
      }

      elmDeviceScaling = XMLNextNode (elmDeviceScaling);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": returning " << pScalingRet << std::endl;
#endif

   if (pszScalingType)
   {
      XMLFree ((void *)pszScalingType);
   }

   return pScalingRet;
}

DeviceScaling * XMLDeviceScaling::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceScaling::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceScalings = pXMLDevice->getDocScalings ();
   XmlNodePtr  rootDeviceScaling = XMLDocGetRootElement (docDeviceScalings);
   XmlNodePtr  elmDeviceScaling  = 0;
   bool        fFound            = false;

   if (!rootDeviceScaling)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !rootDeviceScaling " << std::endl;
#endif

      return false;
   }

   elmDeviceScaling = XMLFirstNode (rootDeviceScaling);
   if (!elmDeviceScaling)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !elmDeviceScaling " << std::endl;
#endif

      return false;
   }

   PSZRO  pszScalingType     = 0;
   double dScalingPercentage = -1;

   if (!getComponents (pszJobProperties,
                       &pszScalingType,
                       0,
                       &dScalingPercentage))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceScaling = XMLFirstNode (XMLGetChildrenNode (elmDeviceScaling));

   while (  elmDeviceScaling
         && !fFound
         )
   {
      PSZCRO pszElmScalingType = getXMLContentString (elmDeviceScaling,
                                                      docDeviceScalings,
                                                      "allowedType");

      if (  pszScalingType
         && pszElmScalingType
         )
      {
         if (0 == strcmp (pszScalingType, pszElmScalingType))
         {
            int iMinimum = getXMLContentInt (elmDeviceScaling,
                                             docDeviceScalings,
                                             "minimum",
                                             true,
                                             -1);
            int iMaximum = getXMLContentInt (elmDeviceScaling,
                                             docDeviceScalings,
                                             "maximum",
                                             true,
                                             -1);

            if (  iMinimum           <= dScalingPercentage
               && dScalingPercentage <= iMaximum
               )
            {
               fFound = true;
            }
         }
      }

      if (pszElmScalingType)
      {
         XMLFree ((void *)pszElmScalingType);
      }

      elmDeviceScaling = XMLNextNode (elmDeviceScaling);
   }

   if (pszScalingType)
   {
      XMLFree ((void *)pszScalingType);
   }

   return fFound;
}

PSZCRO XMLDeviceScaling::
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
   if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLScaling_Enumerator : public Enumeration
{
public:
   XMLScaling_Enumerator (Device     *pDevice,
                          XmlNodePtr  nodeItem,
                          bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceScalings_d = 0;
      nodeItem_d          = nodeItem;
      fInDeviceSpecific_d = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceScalings_d = pXMLDevice->getDocScalings ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLScaling_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceScalings_d = 0;
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
         PSZRO               pszDeviceID  = 0;
         std::ostringstream  oss;
         bool                fHandled     = false;

         try
         {
            if (fInDeviceSpecific_d)
            {
               pszDeviceID = getXMLContentString (nodeItem_d,
                                                  docDeviceScalings_d,
                                                  "deviceID");

               if (pszDeviceID)
               {
                  oss << "Scaling"
                      << "="
                      << pszDeviceID;

                  XMLFree ((void *)pszDeviceID);

                  fHandled = true;
               }
            }

            if (!fHandled)
            {
               PSZCRO pszScalingType = getXMLContentString (nodeItem_d,
                                                            docDeviceScalings_d,
                                                            "allowedType");
               int    iMinimum       = getXMLContentInt (nodeItem_d,
                                                         docDeviceScalings_d,
                                                         "minimum",
                                                         true,
                                                         -1);
               int    iMaximum       = getXMLContentInt (nodeItem_d,
                                                         docDeviceScalings_d,
                                                         "maximum",
                                                         true,
                                                         -1);
               int    iDefault       = getXMLContentInt (nodeItem_d,
                                                         docDeviceScalings_d,
                                                         "default",
                                                         true,
                                                         -1);

               if (  pszScalingType
                  && -1 != iMinimum
                  && -1 != iMaximum
                  && -1 != iDefault
                  )
               {
                  oss << "ScalingPercentage={"
                      << iDefault
                      << ","
                      << iMinimum
                      << ","
                      << iMaximum
                      << "} ScalingType="
                      << pszScalingType;

                  fHandled = true;
               }

               if (pszScalingType)
               {
                  free ((void *)pszScalingType);
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
            if (DebugOutput::shouldOutputXMLDeviceScaling ()) DebugOutput::getErrorStream () << "XMLDeviceScaling::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

            delete pstringError;
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceScalings_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceScaling::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLScaling_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceScalings = pXMLDevice->getDocScalings ();
   XmlNodePtr  rootDeviceScaling = XMLDocGetRootElement (docDeviceScalings);
   XmlNodePtr  elmDeviceScaling  = 0;

   if (!rootDeviceScaling)
      return new XMLScaling_Enumerator (pDevice_d, 0, false);

   elmDeviceScaling = XMLFirstNode (rootDeviceScaling);
   if (!elmDeviceScaling)
      return new XMLScaling_Enumerator (pDevice_d, 0, false);

   elmDeviceScaling = XMLFirstNode (XMLGetChildrenNode (elmDeviceScaling));

   return new XMLScaling_Enumerator (pDevice_d, elmDeviceScaling, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceScaling::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceScaling::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceScaling: "
       << DeviceScaling::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceScaling& const_self)
{
   XMLDeviceScaling&  self = const_cast<XMLDeviceScaling&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
