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
#include "UPDFDeviceOrientation.hpp"

UPDFDeviceOrientation::
UPDFDeviceOrientation (Device     *pDevice,
                       PSZRO       pszJobProperties,
                       bool        fSimulationRequired,
                       XmlNodePtr  nodeOrientation,
                       XmlNodePtr  nodeRotation)
   : DeviceOrientation (pDevice,
                        pszJobProperties,
                        fSimulationRequired)
{
   nodeOrientation_d = nodeOrientation;
   nodeRotation_d    = nodeRotation;
}

UPDFDeviceOrientation::
~UPDFDeviceOrientation ()
{
}

static XmlNodePtr
skipInvalidOrientations (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static XmlNodePtr
skipInvalidRotations (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceOrientation *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  nodeOrientation,
                   XmlNodePtr  nodeRotation)
{
   UPDFDevice         *pUPDFDevice        = UPDFDevice::isAUPDFDevice (pDevice);
   PSZRO               pszOrientationName = 0;
   PSZRO               pszRotationName    = 0;
   bool                fDeviceFeature     = true;
   PSZRO               pszDeviceFeature   = 0;
   std::ostringstream  oss;
   DeviceOrientation  *pOrientationRet    = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   if (  !nodeOrientation
      || !nodeRotation
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !nodeOrientation || !nodeRotation" << std::endl;
#endif

      return 0;
   }

   pszOrientationName = XMLGetProp (nodeOrientation, "ClassifyingID");
   pszRotationName    = XMLGetProp (nodeRotation,    "ClassifyingID");
   pszDeviceFeature   = XMLGetProp (nodeOrientation, "DeviceFeature");

   if (pszDeviceFeature)
   {
      if (0 == strcasecmp (pszDeviceFeature, "false"))
      {
         fDeviceFeature = false;
      }

      XMLFree ((void *)pszDeviceFeature);
   }

   if (  pszOrientationName
      && pszRotationName
      )
   {
      PSZRO pszOmniJP = 0;

      if (UPDFDeviceOrientation::mapUPDFToOmni (pszOrientationName,
                                                pszRotationName,
                                                &pszOmniJP))
      {
         oss << "Rotation=" << pszOmniJP;

         pOrientationRet = new UPDFDeviceOrientation (pDevice,
                                                      oss.str ().c_str (),
                                                      !fDeviceFeature,
                                                      nodeOrientation,
                                                      nodeRotation);
      }
   }

   if (pszOrientationName)
   {
      XMLFree ((void *)pszOrientationName);
   }
   if (pszRotationName)
   {
      XMLFree ((void *)pszRotationName);
   }

   return pOrientationRet;
}

DeviceOrientation * UPDFDeviceOrientation::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice        *pUPDFDevice        = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr         nodeOrientations   = 0;
   XmlNodePtr         nodeRotations      = 0;
   XmlNodePtr         nodeOrientation    = 0;
   XmlNodePtr         nodeRotation       = 0;
   XmlNodePtr         nodeOrientationElm = 0;
   XmlNodePtr         nodeRotationElm    = 0;
   PSZRO              pszOmniName        = 0;
   PSZRO              pszOrientationName = 0;
   PSZRO              pszRotationName    = 0;
   DeviceOrientation *pOrientationRet    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, &pszOmniName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceOrientation::mapOmniToUPDF (pszOmniName, &pszOrientationName, &pszRotationName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeOrientations = findOrientations (pUPDFDevice);
   nodeRotations    = findRotations (pUPDFDevice);

   if (  !nodeOrientations
      || !nodeRotations
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !nodeOrientations || !nodeRotations" << std::endl;
#endif

      goto done;
   }

   nodeOrientationElm = XMLFirstNode (XMLGetChildrenNode (nodeOrientations));
   nodeRotationElm    = XMLFirstNode (XMLGetChildrenNode (nodeRotations));

   if (  !nodeOrientationElm
      || !nodeRotationElm
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !nodeOrientationElm || !nodeRotationElm" << std::endl;
#endif

      goto done;
   }

   while (  nodeOrientationElm != 0
         && nodeOrientation    == 0
         )
   {
      PSZCRO pszNodeID = XMLGetProp (nodeOrientationElm, "ClassifyingID");

      if (pszNodeID)
      {
         if (0 == strcmp (pszNodeID, pszOrientationName))
         {
            nodeOrientation = nodeOrientationElm;
         }

         XMLFree ((void *)pszNodeID);
      }

      nodeOrientationElm = XMLNextNode (nodeOrientationElm);
   }
   while (  nodeRotationElm != 0
         && nodeRotation    == 0
         )
   {
      PSZCRO pszNodeID = XMLGetProp (nodeRotationElm, "ClassifyingID");

      if (pszNodeID)
      {
         if (0 == strcmp (pszNodeID, pszRotationName))
         {
            nodeRotation = nodeRotationElm;
         }

         XMLFree ((void *)pszNodeID);
      }

      nodeRotationElm = XMLNextNode (nodeRotationElm);
   }

   if (  nodeOrientation
      && nodeRotation
      )
   {
      pOrientationRet = createFromXMLNode (pDevice, nodeOrientation, nodeRotation);
   }

done:
   if (!pOrientationRet)
   {
      pOrientationRet = pUPDFDevice->getDefaultOrientation ();
   }

   return pOrientationRet;
}

DeviceOrientation * UPDFDeviceOrientation::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceOrientation::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice        *pUPDFDevice        = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr         nodeOrientations   = 0;
   XmlNodePtr         nodeRotations      = 0;
   XmlNodePtr         nodeOrientation    = 0;
   XmlNodePtr         nodeRotation       = 0;
   XmlNodePtr         nodeOrientationElm = 0;
   XmlNodePtr         nodeRotationElm    = 0;
   PSZRO              pszOmniName        = 0;
   PSZRO              pszOrientationName = 0;
   PSZRO              pszRotationName    = 0;
   bool               fRet               = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, &pszOmniName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceOrientation::mapOmniToUPDF (pszOmniName, &pszOrientationName, &pszRotationName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeOrientations = findOrientations (pUPDFDevice);
   nodeRotations    = findRotations (pUPDFDevice);

   if (  !nodeOrientations
      || !nodeRotations
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !nodeOrientations || !nodeRotations" << std::endl;
#endif

      goto done;
   }

   nodeOrientationElm = XMLFirstNode (XMLGetChildrenNode (nodeOrientations));
   nodeRotationElm    = XMLFirstNode (XMLGetChildrenNode (nodeRotations));

   if (  !nodeOrientationElm
      || !nodeRotationElm
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": !nodeOrientationElm || !nodeRotationElm" << std::endl;
#endif

      goto done;
   }

   while (  nodeOrientationElm != 0
         && nodeOrientation    == 0
         )
   {
      PSZCRO pszNodeID = XMLGetProp (nodeOrientationElm, "ClassifyingID");

      if (pszNodeID)
      {
         if (0 == strcmp (pszNodeID, pszOrientationName))
         {
            nodeOrientation = nodeOrientationElm;
         }

         XMLFree ((void *)pszNodeID);
      }

      nodeOrientationElm = XMLNextNode (nodeOrientationElm);
   }
   while (  nodeRotationElm != 0
         && nodeRotation    == 0
         )
   {
      PSZCRO pszNodeID = XMLGetProp (nodeRotationElm, "ClassifyingID");

      if (pszNodeID)
      {
         if (0 == strcmp (pszNodeID, pszRotationName))
         {
            nodeRotation = nodeRotationElm;
         }

         XMLFree ((void *)pszNodeID);
      }

      nodeRotationElm = XMLNextNode (nodeRotationElm);
   }

   if (  nodeOrientation
      && nodeRotation
      )
   {
      fRet = true;
   }

done:
   return fRet;
}

PSZCRO UPDFDeviceOrientation::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceOrientation::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet             = 0;
   UPDFDevice                 *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeOrientations = 0;
   XmlNodePtr                  nodeOrientation  = 0;
   XmlNodePtr                  nodeRotations    = 0;
   XmlNodePtr                  nodeRotation     = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeOrientations = findOrientations (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": nodeOrientations = " << std::hex << (int *)nodeOrientations << std::dec << std::endl;
#endif

   if (!nodeOrientations)
   {
      goto done;
   }

   nodeOrientation = XMLFirstNode (XMLGetChildrenNode (nodeOrientations));
   nodeOrientation = skipInvalidOrientations (nodeOrientation);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": nodeOrientation = " << std::hex << (int *)nodeOrientation << std::dec << std::endl;
#endif

   nodeRotations = findRotations (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": nodeRotations = " << std::hex << (int *)nodeRotations << std::dec << std::endl;
#endif

   if (!nodeRotations)
   {
      goto done;
   }

   while (nodeOrientation)
   {
      nodeRotation = XMLFirstNode (XMLGetChildrenNode (nodeRotations));
      nodeRotation = skipInvalidRotations (nodeRotation);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": nodeRotation = " << std::hex << (int *)nodeRotation << std::dec << std::endl;
#endif

      while (nodeRotation)
      {
         DeviceOrientation *pOrientation = 0;
         JobProperties     *pJP          = 0;

         pOrientation = createFromXMLNode (pDevice_d, nodeOrientation, nodeRotation);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": pOrientation = " << std::hex << (int *)pOrientation << std::dec << std::endl;
#endif

         if (pOrientation)
         {
            std::string *pstringJPs = 0;

            pstringJPs = pOrientation->getJobProperties (fInDeviceSpecific);

            if (pstringJPs)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

               pJP = new JobProperties (pstringJPs->c_str ());

               pRet->addElement (pJP);

               delete pstringJPs;
            }

            delete pOrientation;
         }

         nodeRotation = XMLNextNode (nodeRotation);
         nodeRotation = skipInvalidRotations (nodeRotation);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": nodeRotation = " << std::hex << (int *)nodeRotation << std::dec << std::endl;
#endif
      }

      nodeOrientation = XMLNextNode (nodeOrientation);
      nodeOrientation = skipInvalidOrientations (nodeOrientation);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOrientation ()) DebugOutput::getErrorStream () << "UPDFDeviceOrientation::" << __FUNCTION__ << ": nodeOrientation = " << std::hex << (int *)nodeOrientation << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceOrientation::
findOrientations (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeOrientations = 0;

   if (!pUPDFDevice)
   {
      return nodeOrientations;
   }

   if (  ((nodeOrientations = FINDUDRENTRY (pUPDFDevice, nodeOrientations, "PrintCapabilities",    UPDFDeviceOrientation)) != 0)
      && ((nodeOrientations = FINDUDRENTRY (pUPDFDevice, nodeOrientations, "Features",             UPDFDeviceOrientation)) != 0)
      && ((nodeOrientations = FINDUDRENTRY (pUPDFDevice, nodeOrientations, "OrientationRequested", UPDFDeviceOrientation)) != 0)
      )
      return nodeOrientations;

   return nodeOrientations;
}

XmlNodePtr UPDFDeviceOrientation::
findRotations (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeRotations = 0;

   if (!pUPDFDevice)
   {
      return nodeRotations;
   }

   if (  ((nodeRotations = FINDUDRENTRY (pUPDFDevice, nodeRotations, "PrintCapabilities",  UPDFDeviceOrientation)) != 0)
      && ((nodeRotations = FINDUDRENTRY (pUPDFDevice, nodeRotations, "Features",           UPDFDeviceOrientation)) != 0)
      && ((nodeRotations = FINDUDRENTRY (pUPDFDevice, nodeRotations, "MediaPageRotation",  UPDFDeviceOrientation)) != 0)
      )
      return nodeRotations;

   return nodeRotations;
}

bool UPDFDeviceOrientation::
mapUPDFToOmni (PSZCRO  pszOrientation,
               PSZCRO  pszRotation,
               PSZRO  *ppszOmniJobProperties)
{
   if (  0 == strcmp (pszOrientation, "Portrait")
      && 0 == strcmp (pszRotation, "standard")
      )
   {
      if (ppszOmniJobProperties)
      {
         *ppszOmniJobProperties = "Portrait";
      }

      return true;
   }
   else if (  0 == strcmp (pszOrientation, "Landscape")
           && 0 == strcmp (pszRotation, "standard")
           )
   {
      if (ppszOmniJobProperties)
      {
         *ppszOmniJobProperties = "Landscape";
      }

      return true;
   }
   else if (  0 == strcmp (pszOrientation, "Portrait")
           && 0 == strcmp (pszRotation, "reverse")
           )
   {
      if (ppszOmniJobProperties)
      {
         *ppszOmniJobProperties = "ReversePortrait";
      }

      return true;
   }
   else if (  0 == strcmp (pszOrientation, "Landscape")
           && 0 == strcmp (pszRotation, "reverse")
           )
   {
      if (ppszOmniJobProperties)
      {
         *ppszOmniJobProperties = "ReverseLandscape";
      }

      return true;
   }

   return false;
}

bool UPDFDeviceOrientation::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszOrientation,
               PSZRO  *ppszRotation)
{
   if (0 == strcmp (pszOmniValue, "Portrait"))
   {
      if (ppszOrientation)
      {
         *ppszOrientation = "Portrait";
      }
      if (ppszRotation)
      {
         *ppszRotation    = "standard";
      }

      return true;
   }
   else if (0 == strcmp (pszOmniValue, "Landscape"))
   {
      if (ppszOrientation)
      {
         *ppszOrientation = "Landscape";
      }
      if (ppszRotation)
      {
         *ppszRotation    = "standard";
      }

      return true;
   }
   else if (0 == strcmp (pszOmniValue, "ReversePortrait"))
   {
      if (ppszOrientation)
      {
         *ppszOrientation = "Portrait";
      }
      if (ppszRotation)
      {
         *ppszRotation    = "reverse";
      }

      return true;
   }
   else if (0 == strcmp (pszOmniValue, "ReverseLandscape"))
   {
      if (ppszOrientation)
      {
         *ppszOrientation = "Landscape";
      }
      if (ppszRotation)
      {
         *ppszRotation    = "reverse";
      }

      return true;
   }

   return false;
}

#ifndef RETAIL

void UPDFDeviceOrientation::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceOrientation::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceOrientation: "
       << DeviceOrientation::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceOrientation& const_self)
{
   UPDFDeviceOrientation& self = const_cast<UPDFDeviceOrientation&>(const_self);
   std::ostringstream     oss;

   os << self.toString (oss);

   return os;
}
