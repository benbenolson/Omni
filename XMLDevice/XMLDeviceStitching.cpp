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
#include <XMLDeviceStitching.hpp>

#include <JobProperties.hpp>

XMLDeviceStitching::
XMLDeviceStitching (Device                *pDevice,
                    PSZRO                  pszJobProperties,
                    BinaryData            *pbdData,
                    XmlNodePtr             node)
   : DeviceStitching (pDevice,
                      pszJobProperties,
                      pbdData)
{
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceStitching::
~XMLDeviceStitching ()
{
   if (pszDeviceID_d)
   {
      XMLFree ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceStitching * XMLDeviceStitching::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr        docDeviceStitchings = pXMLDevice->getDocStitchings ();
   XmlNodePtr       rootDeviceStitching = XMLDocGetRootElement (docDeviceStitchings);
   XmlNodePtr       elmDeviceStitching  = 0;
   DeviceStitching *pStitchingRet       = 0;

   if (!rootDeviceStitching)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !rootDeviceStitching " << std::endl;
#endif

      return 0;
   }

   elmDeviceStitching = XMLFirstNode (rootDeviceStitching);
   if (!elmDeviceStitching)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !elmDeviceStitching " << std::endl;
#endif

      return 0;
   }

   int iPosition          = -1;
   int indexReferenceEdge = -1;
   int indexType          = -1;
   int iCount             = -1;
   int iAngle             = -1;

   if (!getComponents (pszJobProperties,
                      &iPosition,
                      0,
                      &indexReferenceEdge,
                      0,
                      &indexType,
                      &iCount,
                      &iAngle))
   {
      return pXMLDevice->getDefaultStitching ();
   }

   elmDeviceStitching = XMLFirstNode (XMLGetChildrenNode (elmDeviceStitching));

   while (  elmDeviceStitching
         && !pStitchingRet
         )
   {
      int         iPositionElm          = -1;
      PSZRO       pszReferenceEdge      = 0;
      int         indexReferenceEdgeElm = -1;
      PSZRO       pszType               = 0;
      int         indexTypeElm          = -1;
      int         iCountElm             = -1;
      int         iAngleElm             = -1;
      PSZRO       pszCommand            = 0;
      BinaryData *pbdData               = 0;

      try
      {
         iPositionElm = getXMLContentInt (elmDeviceStitching, docDeviceStitchings, "StitchingPosition", true, -1);

         pszReferenceEdge = getXMLContentString (elmDeviceStitching, docDeviceStitchings, "StitchingReferenceEdge");

         if (pszReferenceEdge)
         {
            indexReferenceEdgeElm = DeviceStitching::referenceEdgeIndex (pszReferenceEdge);

            XMLFree ((void *)pszReferenceEdge);
         }

         pszType = getXMLContentString (elmDeviceStitching, docDeviceStitchings, "StitchingType");

         if (pszType)
         {
            indexTypeElm = DeviceStitching::typeIndex (pszType);

            XMLFree ((void *)pszType);
         }

         iCountElm = getXMLContentInt (elmDeviceStitching, docDeviceStitchings, "StitchingCount", true, -1);

         iAngleElm = getXMLContentInt (elmDeviceStitching, docDeviceStitchings, "StitchingAngle", true, -1);

         if (  iPositionElm          == iPosition
            && indexReferenceEdgeElm == indexReferenceEdge
            && indexTypeElm          == indexType
            && iCountElm             == iCount
            && iAngleElm             == iAngle
            )
         {
            // Read in the command
            pszCommand = getXMLContentString (elmDeviceStitching, docDeviceStitchings, "command");

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
            if (DebugOutput::shouldOutputXMLDeviceStitching ())
            {
               DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": iPosition          = " << iPosition << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": indexReferenceEdge = " << indexReferenceEdge << std::endl;
               if (pbdData)
                  DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": pbdData            = " << *pbdData << std::endl;
               else
                  DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": indexType          = " << indexType << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": iCount             = " << iCount << std::endl;
               DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": iAngle             = " << iAngle << std::endl;
            }
#endif


            // Create the object
            pStitchingRet = new XMLDeviceStitching (pDevice,
                                                    pszJobProperties,
                                                    pbdData,
                                                    elmDeviceStitching);
         }
      }
      catch (std::string *pstringError)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

         delete pstringError;
      }

      elmDeviceStitching = XMLNextNode (elmDeviceStitching);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": returning " << pStitchingRet << std::endl;
#endif

   return pStitchingRet;
}

DeviceStitching * XMLDeviceStitching::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceStitching::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceStitchings = pXMLDevice->getDocStitchings ();
   XmlNodePtr  rootDeviceStitching = XMLDocGetRootElement (docDeviceStitchings);
   XmlNodePtr  elmDeviceStitching  = 0;
   bool        fFound              = false;

   if (!rootDeviceStitching)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !rootDeviceStitching " << std::endl;
#endif

      return false;
   }

   elmDeviceStitching = XMLFirstNode (rootDeviceStitching);
   if (!elmDeviceStitching)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !elmDeviceStitching " << std::endl;
#endif

      return false;
   }

   int iPosition          = -1;
   int indexReferenceEdge = -1;
   int indexType          = -1;
   int iCount             = -1;
   int iAngle             = -1;

   if (!getComponents (pszJobProperties,
                      &iPosition,
                      0,
                      &indexReferenceEdge,
                      0,
                      &indexType,
                      &iCount,
                      &iAngle))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceStitching = XMLFirstNode (XMLGetChildrenNode (elmDeviceStitching));

   while (  elmDeviceStitching
         && !fFound
         )
   {
      int         iPositionElm          = -1;
      PSZRO       pszReferenceEdge      = 0;
      int         indexReferenceEdgeElm = -1;
      PSZRO       pszType               = 0;
      int         indexTypeElm          = -1;
      int         iCountElm             = -1;
      int         iAngleElm             = -1;

      try
      {
         iPositionElm = getXMLContentInt (elmDeviceStitching, docDeviceStitchings, "StitchingPosition", true, -1);

         pszReferenceEdge = getXMLContentString (elmDeviceStitching, docDeviceStitchings, "StitchingReferenceEdge");

         if (pszReferenceEdge)
         {
            indexReferenceEdgeElm = DeviceStitching::referenceEdgeIndex (pszReferenceEdge);

            XMLFree ((void *)pszReferenceEdge);
         }

         pszType = getXMLContentString (elmDeviceStitching, docDeviceStitchings, "StitchingType");

         if (pszType)
         {
            indexTypeElm = DeviceStitching::typeIndex (pszType);

            XMLFree ((void *)pszType);
         }

         iCountElm = getXMLContentInt (elmDeviceStitching, docDeviceStitchings, "StitchingCount", true, -1);

         iAngleElm = getXMLContentInt (elmDeviceStitching, docDeviceStitchings, "StitchingAngle", true, -1);

         if (  iPositionElm          == iPosition
            && indexReferenceEdgeElm == indexReferenceEdge
            && indexTypeElm          == indexType
            && iCountElm             == iCount
            && iAngleElm             == iAngle
            )
         {
            fFound = true;
         }
      }
      catch (std::string *pstringError)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

         delete pstringError;
      }

      elmDeviceStitching = XMLNextNode (elmDeviceStitching);
   }

   return fFound;
}

PSZCRO XMLDeviceStitching::
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
   if (DebugOutput::shouldOutputXMLDeviceStitching ()) DebugOutput::getErrorStream () << "XMLDeviceStitching::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLStitching_Enumerator : public Enumeration
{
public:
   XMLStitching_Enumerator (Device     *pDevice,
                            XmlNodePtr  nodeItem,
                            bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d          = pXMLDevice;
      docDeviceStitchings_d = 0;
      nodeItem_d            = nodeItem;
      fInDeviceSpecific_d   = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceStitchings_d = pXMLDevice->getDocStitchings ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLStitching_Enumerator ()
   {
      pXMLDevice_d          = 0;
      docDeviceStitchings_d = 0;
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
         PSZRO               pszDeviceID  = 0;
         std::ostringstream  oss;
         bool                fHandled     = false;

         if (fInDeviceSpecific_d)
         {
            pszDeviceID = getXMLContentString (nodeItem_d,
                                               docDeviceStitchings_d,
                                               "deviceID");

            if (pszDeviceID)
            {
               oss << "Stitching"
                   << "="
                   << pszDeviceID;

               XMLFree ((void *)pszDeviceID);

               fHandled = true;
            }
         }

         if (!fHandled)
         {
            std::string *pstringStitchingPosition      = 0;
            std::string *pstringStitchingReferenceEdge = 0;
            std::string *pstringStitchingType          = 0;
            std::string *pstringStitchingCount         = 0;
            std::string *pstringStitchingAngle         = 0;

            pstringStitchingPosition      = XMLDevice::getXMLJobProperties (nodeItem_d,
                                                                            docDeviceStitchings_d,
                                                                            "StitchingPosition");
            pstringStitchingReferenceEdge = XMLDevice::getXMLJobProperties (nodeItem_d,
                                                                            docDeviceStitchings_d,
                                                                            "StitchingReferenceEdge");
            pstringStitchingType          = XMLDevice::getXMLJobProperties (nodeItem_d,
                                                                            docDeviceStitchings_d,
                                                                            "StitchingType");
            pstringStitchingCount         = XMLDevice::getXMLJobProperties (nodeItem_d,
                                                                            docDeviceStitchings_d,
                                                                            "StitchingCount");
            pstringStitchingAngle         = XMLDevice::getXMLJobProperties (nodeItem_d,
                                                                            docDeviceStitchings_d,
                                                                            "StitchingAngle");

            if (  pstringStitchingPosition
               && pstringStitchingReferenceEdge
               && pstringStitchingType
               && pstringStitchingCount
               && pstringStitchingAngle
               )
            {
               oss << *pstringStitchingPosition
                   << " "
                   << *pstringStitchingReferenceEdge
                   << " "
                   << *pstringStitchingType
                   << " "
                   << *pstringStitchingCount
                   << " "
                   << *pstringStitchingAngle;

               fHandled = true;
            }

            delete pstringStitchingPosition;
            delete pstringStitchingReferenceEdge;
            delete pstringStitchingType;
            delete pstringStitchingCount;
            delete pstringStitchingAngle;
         }

         if (fHandled)
         {
            pvRet = (void *)new JobProperties (oss.str ().c_str ());
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceStitchings_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceStitching::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLStitching_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceStitchings = pXMLDevice->getDocStitchings ();
   XmlNodePtr  rootDeviceStitching = XMLDocGetRootElement (docDeviceStitchings);
   XmlNodePtr  elmDeviceStitching  = 0;

   if (!rootDeviceStitching)
      return new XMLStitching_Enumerator (pDevice_d, 0, false);

   elmDeviceStitching = XMLFirstNode (rootDeviceStitching);
   if (!elmDeviceStitching)
      return new XMLStitching_Enumerator (pDevice_d, 0, false);

   elmDeviceStitching = XMLFirstNode (XMLGetChildrenNode (elmDeviceStitching));

   return new XMLStitching_Enumerator (pDevice_d, elmDeviceStitching, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceStitching::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceStitching::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceStitching: "
       << DeviceStitching::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceStitching& const_self)
{
   XMLDeviceStitching& self = const_cast<XMLDeviceStitching&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
