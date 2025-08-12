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
#include "Device.hpp"
#include "PrintDevice.hpp"

#include <sstream>
#include <cstdint>

DeviceInstance::
DeviceInstance (PrintDevice *pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   pDevice_d = pDevice;
}

DeviceInstance::
~DeviceInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::~" << __FUNCTION__ << " ()" << std::endl;
#endif
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void DeviceInstance::
initializeInstance (PSZCRO pszJobProperties)
{
}

bool DeviceInstance::
hasError ()
{
   return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

std::string * DeviceInstance::
getJobProperties (bool fInDeviceSpecific)
{
   return new std::string ("");
}

bool DeviceInstance::
setJobProperties (PSZCRO pszJobProperties)
{
   return false;
}

Enumeration * DeviceInstance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   return new NullEnumerator ();
}

/*
 *  boolean                              "boolean true"
 *  integer with optional ranges         "integer 1 -5 42"
 *  floating point with optional ranges  "float 3.14157"
 *  string                               "string value"
 */
std::string * DeviceInstance::
getJobPropertyType (PSZCRO pszKey)
{
   return 0;
}

std::string * DeviceInstance::
getJobProperty (PSZCRO pszKey)
{
   return 0;
}

std::string * DeviceInstance::
translateKeyValue (PSZCRO pszKeyInternal,
                   PSZCRO pszValueInternal)
{
   DeviceString *pString = pDevice_d->getDeviceString ();
   std::string  *pRet    = 0;

   if (pString)
   {
      StringResource *pSR = pString->getLanguageResource ();

      if (pSR)
      {
         PSZCRO pszKeyExternal   = pSR->getStringV (StringResource::STRINGGROUP_UNKNOWN,
                                                    pszKeyInternal);
         PSZCRO pszValueExternal = pSR->getStringV (StringResource::STRINGGROUP_UNKNOWN,
                                                    pszValueInternal);

         if (pszKeyExternal)
         {
            pRet = new std::string (pszKeyExternal);
         }

         if (  pszValueExternal
            && pRet
            )
         {
            *pRet += "=";
            *pRet += pszValueExternal;
         }
      }
   }

   return pRet;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceOrientation * DeviceInstance::
getCurrentOrientation ()
{
   return pDevice_d->getCurrentOrientation ();
}

PSZCRO DeviceInstance::
getCurrentDitherID ()
{
   return pDevice_d->getCurrentDitherID ();
}

DeviceForm * DeviceInstance::
getCurrentForm ()
{
   return pDevice_d->getCurrentForm ();
}

DeviceTray * DeviceInstance::
getCurrentTray ()
{
   return pDevice_d->getCurrentTray ();
}

DeviceMedia * DeviceInstance::
getCurrentMedia ()
{
   return pDevice_d->getCurrentMedia ();
}

DeviceResolution * DeviceInstance::
getCurrentResolution ()
{
   return pDevice_d->getCurrentResolution ();
}

DeviceCommand * DeviceInstance::
getCommands ()
{
   return pDevice_d->getCommands ();
}

DeviceData * DeviceInstance::
getDeviceData ()
{
   return pDevice_d->getDeviceData ();
}

DevicePrintMode * DeviceInstance::
getCurrentPrintMode ()
{
   return pDevice_d->getCurrentPrintMode ();
}

DeviceGamma * DeviceInstance::
getCurrentGamma ()
{
   return pDevice_d->getCurrentGamma ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void DeviceInstance::
ditherNewFrame ()
{
   PrintDevice *pPrintDevice = dynamic_cast <PrintDevice *>(pDevice_d);

   if (!pPrintDevice)
      return;

   if (pPrintDevice->getDeviceBlitter ())
      pPrintDevice->getDeviceBlitter ()->ditherNewFrame ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool DeviceInstance::
hasCapability (long lMask)
{
   return pDevice_d->hasCapability (lMask);
}

bool DeviceInstance::
hasRasterCapability (long lMask)
{
   return pDevice_d->hasRasterCapability (lMask);
}

bool DeviceInstance::
hasDeviceOption (PSZCRO pszDeviceOption)
{
   return pDevice_d->hasDeviceOption (pszDeviceOption);
}

bool DeviceInstance::
deviceOptionValid (PSZCRO pszDeviceOption)
{
   return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool DeviceInstance::
setOutputStream (FILE *pFile)
{
   return true;
}

bool DeviceInstance::
setErrorStream (FILE *pFile)
{
   return true;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool DeviceInstance::
setLanguage (int iLanguageID)
{
   return true;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool DeviceInstance::
beginJob ()
{
   BinaryData *pCmd = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::beginJob ()" << std::endl;
#endif

   pCmd = getCommands ()->getCommandData ("cmdInit");

   if (pCmd)
   {
      pDevice_d->sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool DeviceInstance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::beginJob (with props)" << std::endl;
#endif

   return beginJob ();
}

bool DeviceInstance::
newFrame ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::newFrame ()" << std::endl;
#endif

   return true;
}

bool DeviceInstance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::newFrame (with props)" << std::endl;
#endif

   return newFrame ();
}

bool DeviceInstance::
endJob ()
{
   BinaryData *pCmd = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceInstance ()) DebugOutput::getErrorStream () << "DeviceInstance::endJob ()" << std::endl;
#endif

   pCmd = getCommands ()->getCommandData ("cmdTerm");

   if (pCmd)
   {
      pDevice_d->sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool DeviceInstance::
abortJob ()
{
   return true;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

StringResource * DeviceInstance::
getLanguageResource ()
{
   return pDevice_d->getLanguageResource ();
}

bool DeviceInstance::
sendBinaryDataToDevice (BinaryData *pData)
{
   return pDevice_d->sendBinaryDataToDevice (pData);
}

bool DeviceInstance::
sendBinaryDataToDevice (DeviceForm *pForm)
{
   return pDevice_d->sendBinaryDataToDevice (pForm);
}

bool DeviceInstance::
sendBinaryDataToDevice (DeviceTray *pTray)
{
   return pDevice_d->sendBinaryDataToDevice (pTray);
}

bool DeviceInstance::
sendBinaryDataToDevice (DeviceMedia *pMedia)
{
   return pDevice_d->sendBinaryDataToDevice (pMedia);
}

bool DeviceInstance::
sendBinaryDataToDevice (DeviceResolution *pResolution)
{
   return pDevice_d->sendBinaryDataToDevice (pResolution);
}

bool DeviceInstance::
sendBinaryDataToDevice (PBYTE pbData,
                        int   iLength)
{
   return pDevice_d->sendBinaryDataToDevice (pbData, iLength);
}

bool DeviceInstance::
sendPrintfToDevice (BinaryData *pData,
                                ...)
{
   va_list list;
   bool    rc;

   va_start (list, pData);

   rc = sendVPrintfToDevice (pData, list);

   va_end (list);

   return rc;
}

bool DeviceInstance::
sendVPrintfToDevice (BinaryData *pData,
                     va_list     list)
{
   return pDevice_d->sendVPrintfToDevice (pData, list);
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void DeviceInstance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceInstance::
toString (std::ostringstream& oss)
{
   oss << "{DeviceInstance: pDevice_d = " << std::hex << (uintptr_t)pDevice_d << std::dec << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceInstance& const_self)
{
   DeviceInstance&    self = const_cast<DeviceInstance&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
