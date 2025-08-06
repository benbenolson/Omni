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

#include <UPDFDeviceCopies.hpp>

UPDFDeviceCopies::
UPDFDeviceCopies (Device      *pDevice,
                  PSZRO        pszJobProperties,
                  BinaryData  *data,
                  int          iMinimum,
                  int          iMaximum,
                  bool         fSimulationRequired,
                  XmlNodePtr   node)
   : DeviceCopies (pDevice,
                   pszJobProperties,
                   data,
                   iMinimum,
                   iMaximum,
                   fSimulationRequired)
{
   node_d = node;
}

UPDFDeviceCopies::
~UPDFDeviceCopies ()
{
}

DeviceCopies * UPDFDeviceCopies::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice         *pUPDFDevice          = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr          nodeCopies           = 0;
   PSZRO               pszMinimum           = 0;
   PSZRO               pszMaximum           = 0;
   int                 iMinimum             = 1;
   int                 iMaximum             = -1;
   DeviceCopies       *pCopiesRet           = 0;
   int                 iCopies              = -1;
   BinaryData         *pbdData              = 0;    // @TBD
   bool                fSimulationRequired  = true; // @TBD
   std::ostringstream  oss;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   nodeCopies = findCopies (pUPDFDevice);

   if (!nodeCopies)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": !nodeCopies" << std::endl;
#endif

      goto done;
   }

   pszMinimum = XMLGetProp (nodeCopies, "Minimum");
   pszMaximum = XMLGetProp (nodeCopies, "Maximum");

   if (!getComponents (pszJobProperties, &iCopies))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (pszMinimum)
   {
      sscanf (pszMinimum, "%d", &iMinimum);
   }
   if (pszMaximum)
   {
      sscanf (pszMaximum, "%d", &iMaximum);
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": iCopies = " << iCopies << ", iMinimum = " << iMinimum << ", iMaximum = " << iMaximum << std::endl;
#endif

   if (  (  iMinimum <= iCopies
         && iCopies  <= iMaximum
         )
      || (  iMinimum <= iCopies
         && iMaximum == -1
         )
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceCopies ())
      {
         if (pbdData)
            DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": pbdData             = " << *pbdData << std::endl;
         else
            DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
         DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": iMaximum            = " << iMaximum << std::endl;
         DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
      }
#endif

      oss << "Copies={"
          << iCopies
          << ","
          << iMinimum
          << ","
          << iMaximum
          << "}";

      // Create the object
      pCopiesRet = new UPDFDeviceCopies (pDevice,
                                         oss.str ().c_str (),
                                         pbdData,
                                         1,
                                         iMaximum,
                                         fSimulationRequired,
                                         nodeCopies);
   }

done:
   if (pszMinimum)
   {
      XMLFree ((void *)pszMinimum);
   }
   if (pszMaximum)
   {
      XMLFree ((void *)pszMaximum);
   }

   if (!pCopiesRet)
   {
      pCopiesRet = pUPDFDevice->getDefaultCopies ();
   }

   return pCopiesRet;
}

DeviceCopies * UPDFDeviceCopies::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceCopies::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeCopies  = 0;
   PSZRO       pszMinimum  = 0;
   PSZRO       pszMaximum  = 0;
   int         iCopies     = -1;
   int         iMinimum    = 1;
   int         iMaximum    = -1;
   bool        fRet        = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   nodeCopies = findCopies (pUPDFDevice);

   if (!nodeCopies)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": !nodeCopies" << std::endl;
#endif

      goto done;
   }

   pszMinimum = XMLGetProp (nodeCopies, "Minimum");
   pszMaximum = XMLGetProp (nodeCopies, "Maximum");

   if (!getComponents (pszJobProperties, &iCopies))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (pszMinimum)
   {
      sscanf (pszMinimum, "%d", &iMinimum);
   }
   if (pszMaximum)
   {
      sscanf (pszMaximum, "%d", &iMaximum);
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceCopies ()) DebugOutput::getErrorStream () << "UPDFDeviceCopies::" << __FUNCTION__ << ": iCopies = " << iCopies << ", iMinimum = " << iMinimum << ", iMaximum = " << iMaximum << std::endl;
#endif

   if (  (  iMinimum <= iCopies
         && iCopies  <= iMaximum
         )
      || (  iMinimum <= iCopies
         && iMaximum == -1
         )
      )
   {
      fRet = true;
   }

done:
   if (pszMinimum)
   {
      XMLFree ((void *)pszMinimum);
   }
   if (pszMaximum)
   {
      XMLFree ((void *)pszMaximum);
   }

   return fRet;
}

PSZCRO UPDFDeviceCopies::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceCopies::
getEnumeration (bool fInDeviceSpecific)
{
   std::ostringstream          oss;
   MultiJobPropertyEnumerator *pRet = 0;
   JobProperties              *pJP  = 0;

   pRet = new MultiJobPropertyEnumerator ();

   if (pRet)
   {
      oss << "Copies={"
          << iNumCopies_d
          << ","
          << iMinimum_d
          << ","
          << iMaximum_d
          << "}";

      pJP = new JobProperties (oss.str ().c_str ());

      pRet->addElement (pJP);
   }

   return (Enumeration *)pRet;
}

XmlNodePtr UPDFDeviceCopies::
findCopies (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeCopies = 0;

   if (!pUPDFDevice)
   {
      return nodeCopies;
   }

   if (  ((nodeCopies = FINDUDRENTRY (pUPDFDevice, nodeCopies, "PrintCapabilities",  UPDFDeviceCopies)) != 0)
      && ((nodeCopies = FINDUDRENTRY (pUPDFDevice, nodeCopies, "Features",           UPDFDeviceCopies)) != 0)
      && ((nodeCopies = FINDUDRENTRY (pUPDFDevice, nodeCopies, "Copies",             UPDFDeviceCopies)) != 0)
      )
      return nodeCopies;

   return nodeCopies;
}

#ifndef RETAIL

void UPDFDeviceCopies::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceCopies::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceCopies: "
       << DeviceCopies::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceCopies& const_self)
{
   UPDFDeviceCopies&  self = const_cast<UPDFDeviceCopies&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
