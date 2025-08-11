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
#include "Epson_ESCP2_Instance.hpp"

#include <defines.hpp>
#include <JobProperties.hpp>

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new Epson_ESCP2_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

Epson_ESCP2_Instance::
Epson_ESCP2_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::Epson_ESCP2_Instance ()" << std::endl;
#endif

   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
   fUseBidirectional_d = true;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
}

Epson_ESCP2_Instance::
~Epson_ESCP2_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::~Epson_ESCP2_Instance ()" << std::endl;
#endif
}

void Epson_ESCP2_Instance::
initializeInstance ()
{
   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;
}

static PSZRO apszDeviceJobPropertyKeys[] = {
   "bidirectional"
};
#define JOBPROP_BIDIRECTIONAL apszDeviceJobPropertyKeys[0]

static PSZRO apszDeviceJobPropertyValues[] = {
   "true",
   "false"
};
#define JOBPROP_TRUE    apszDeviceJobPropertyValues[0]
#define JOBPROP_FALSE   apszDeviceJobPropertyValues[1]

std::string * Epson_ESCP2_Instance::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   oss << JOBPROP_BIDIRECTIONAL
       << "=";
   if (fUseBidirectional_d)
   {
      oss << JOBPROP_TRUE;
   }
   else
   {
      oss << JOBPROP_FALSE;
   }

   return new std::string (oss.str ());
}

bool Epson_ESCP2_Instance::
setJobProperties (PSZCRO pszJobProperties)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_BIDIRECTIONAL))
      {
         if (0 == strcmp (pszValue, JOBPROP_TRUE))
         {
            fRet                = true;
            fUseBidirectional_d = true;
         }
         else if (0 == strcmp (pszValue, JOBPROP_FALSE))
         {
            fRet                = true;
            fUseBidirectional_d = false;
         }
         else
         {
            // @TBD
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

Enumeration * Epson_ESCP2_Instance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   class BidirectionalJPEnumeration : public Enumeration
   {
   public:
      BidirectionalJPEnumeration (int cBiDis, PSZRO *aBiDis)
      {
         iBiDis_d = 0;
         cBiDis_d = cBiDis;
         aBiDis_d = aBiDis;
      }

      virtual bool hasMoreElements ()
      {
         if (iBiDis_d < cBiDis_d)
            return true;
         else
            return false;
      }

      virtual void *nextElement ()
      {
         if (iBiDis_d > cBiDis_d - 1)
            return 0;

         std::ostringstream oss;

         oss << JOBPROP_BIDIRECTIONAL
             << "="
             << aBiDis_d[iBiDis_d];

         iBiDis_d++;

         return (void *)new JobProperties (oss.str ().c_str ());
      }

   private:
      int    iBiDis_d;
      int    cBiDis_d;
      PSZRO *aBiDis_d;
   };

   Enumeration    *pEnumBidis = 0;
   EnumEnumerator *pEnumRet   = 0;

   pEnumBidis = new BidirectionalJPEnumeration (dimof (apszDeviceJobPropertyValues),
                                                apszDeviceJobPropertyValues);

   if (pEnumBidis)
   {
      pEnumRet = new EnumEnumerator ();

      if (pEnumRet)
      {
         pEnumRet->addElement (pEnumBidis);
      }
   }

   return pEnumRet;
}

std::string * Epson_ESCP2_Instance::
getJobPropertyType (PSZRO pszKey)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (pszKey, JOBPROP_BIDIRECTIONAL))
   {
      pRet = new std::string ("boolean ");
      *pRet += JOBPROP_TRUE;
   }

   return pRet;
}

std::string * Epson_ESCP2_Instance::
getJobProperty (PSZRO pszKey)
{
   if (0 == strcasecmp (pszKey, JOBPROP_BIDIRECTIONAL))
   {
      if (fUseBidirectional_d)
      {
         return new std::string (JOBPROP_TRUE);
      }
      else
      {
         return new std::string (JOBPROP_FALSE);
      }
   }

   return 0;
}

std::string * Epson_ESCP2_Instance::
translateKeyValue (PSZRO pszKey,
                   PSZRO pszValue)
{
   PSZRO        pszRetKey   = 0;
   PSZRO        pszRetValue = 0;
   std::string *pRet        = 0;
   int          iValueEnum  = StringResource::DEVICE_COMMON_UNKNOWN;

   if (0 == strcasecmp (pszKey, JOBPROP_BIDIRECTIONAL))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_BIDIRECTIONAL);
   }

   if (pszValue)
   {
      if (0 == strcasecmp (pszValue, JOBPROP_TRUE))
         iValueEnum = StringResource::DEVICE_COMMON_TRUE;
      else if (0 == strcasecmp (pszValue, JOBPROP_FALSE))
         iValueEnum = StringResource::DEVICE_COMMON_FALSE;
   }

   if (StringResource::DEVICE_COMMON_UNKNOWN != iValueEnum)
   {
      pszRetValue = StringResource::getString (getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               iValueEnum);
   }

   if (pszRetKey)
   {
      pRet = new std::string (pszRetKey);
   }

   if (  pszRetValue
      && pRet
      )
   {
      *pRet += "=";
      *pRet += pszRetValue;
   }

   return pRet;
}

bool Epson_ESCP2_Instance::
deviceOptionValid (PSZRO pszDeviceOption)
{
   if (0 == strcmp (pszDeviceOption, "USB_PORT_SUPPORT"))
   {
      return true;
   }

   return false;
}

void Epson_ESCP2_Instance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;

   DeviceResolution *pRes      = getCurrentResolution ();
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdBeginRasterGraphics");
   if (pCmd)
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdBeginRasterGraphics defined for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdSetResolution");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd, 3600 / pRes->getYRes ());
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdSetResolution defined for this device!" << std::endl;
#endif
   }

   DeviceForm *pForm = getCurrentForm ();

   pCmd = pCommands->getCommandData ("cmdSetFormSize");
   if (pCmd)
   {
      int iUnits = pForm->getHardCopyCap ()->getYPels ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iUnits = " << iUnits << std::endl;
#endif

      sendPrintfToDevice (pCmd, iUnits);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdSetFormSize defined for this device!" << std::endl;
#endif
   }

   // @TBD
   fUseMicroweave_d = false;

   if (  720 == pRes->getXRes ()
      && 720 == pRes->getYRes ()
      )
   {
      fUseMicroweave_d = true;
   }

   pCmd = pCommands->getCommandData ("cmdBidi");
   if (pCmd)
   {
      char chState = '0';

      if (fUseBidirectional_d)
      {
         chState = '1';
      }

      if (fUseMicroweave_d)
         // Turn off bidirectional for microweaving
         chState = '0';

      sendPrintfToDevice (pCmd, chState);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdBidi defined for this device!" << std::endl;
#endif
   }

   // @TBD
///sendBinaryDataToDevice (getCurrentTray ());

   pCmd = pCommands->getCommandData ("cmdSetHardwareMicroweave");
   if (pCmd)
   {
      if (fUseMicroweave_d)
         sendPrintfToDevice (pCmd, '1');
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdSetHardwareMicroweave defined for this device!" << std::endl;
#endif
   }

   // @TBD
   pCmd = pCommands->getCommandData ("cmdSetDotSize");
   if (pCmd)
   {
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdSetDotSize defined for this device!" << std::endl;
#endif
   }
}

bool Epson_ESCP2_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::beginJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdUSBInit");

   if (  hasDeviceOption ("USB_PORT_SUPPORT")
      && pCmd
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdUSBInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdInit");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool Epson_ESCP2_Instance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::beginJob (with props)" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;
#endif

   // @TBD - reinitialize with new job properties

   // Call common code
   return beginJob ();
}

bool Epson_ESCP2_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::newFrame ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   ditherNewFrame ();

   ptlPrintHead_d.x = 0;
   ptlPrintHead_d.y = 0;

   return true;
}

bool Epson_ESCP2_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::newFrame (with props)" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;
#endif

   // @TBD - reinitialize with new job properties

   // Call common code
   return newFrame ();
}

bool Epson_ESCP2_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::endJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdTerm");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdTerm = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool Epson_ESCP2_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_ESCP2_Instance::endJob ()" << std::endl;
#endif

   int cx = getCurrentForm ()->getHardCopyCap ()->getXPels ();

   // The largest size is 24 lines
   int iSize = 24 * (cx + 7) / 8;

   PBYTE abData = new BYTE [iSize];

   memset (abData, 0, sizeof (abData));

   BinaryData data (abData, sizeof (abData));

   sendBinaryDataToDevice (&data);

   delete []abData;

   return true;
}

#ifndef RETAIL

void Epson_ESCP2_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Epson_ESCP2_Instance::
toString (std::ostringstream& oss)
{
   oss << "{Epson_ESCP2_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Epson_ESCP2_Instance& const_self)
{
   Epson_ESCP2_Instance& self = const_cast<Epson_ESCP2_Instance&>(const_self);
   std::ostringstream    oss;

   os << self.toString (oss);

   return os;
}
