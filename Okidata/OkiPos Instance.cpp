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
#include "OkiPos_Instance.hpp"

#include <defines.hpp>
#include <JobProperties.hpp>

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new OkiPos_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

OkiPos_Instance::
OkiPos_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::OkiPos_Instance ()" << std::endl;
#endif

   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
   fUseJournal_d       = false;
   eCutMode_d          = CUT_MODE_NONE;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
}

OkiPos_Instance::
~OkiPos_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::~OkiPos_Instance ()" << std::endl;
#endif
}

void OkiPos_Instance::
initializeInstance (PSZCRO pszJobProperties)
{
   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::initializeInstance ()" << std::endl;
#endif
}

static PSZRO apszDeviceJobPropertyKeys[] = {
   "journal",
   "cutmode"
};
#define JOBPROP_USEJOURNAL apszDeviceJobPropertyKeys[0]
#define JOBPROP_USECUTMODE apszDeviceJobPropertyKeys[1]

static PSZRO apszJournalValues[] = {
   "true",
   "false"
};
static PSZRO apszCutmodeValues[] ={
   "none",
   "full",
   "partial"
};
#define JOBPROP_TRUE    apszJournalValues[0]
#define JOBPROP_FALSE   apszJournalValues[1]
#define JOBPROP_NONE    apszCutmodeValues[0]
#define JOBPROP_FULL    apszCutmodeValues[1]
#define JOBPROP_PARTIAL apszCutmodeValues[2]

std::string * OkiPos_Instance::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   if (hasDeviceOption ("MODEL_D"))
   {
      oss << JOBPROP_USEJOURNAL
          << "=";
      if (fUseJournal_d)
      {
         oss << JOBPROP_TRUE;
      }
      else
      {
         oss << JOBPROP_FALSE;
      }

      oss << " ";
   }

   oss << JOBPROP_USECUTMODE
       << "=";
   switch (eCutMode_d)
   {
   case CUT_MODE_NONE:    oss << "none";    break;
   case CUT_MODE_FULL:    oss << "full";    break;
   case CUT_MODE_PARTIAL: oss << "partial"; break;
   default:                                 break;
   }

   return new std::string (oss.str ());
}

bool OkiPos_Instance::
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

      if (0 == strcmp (pszKey, JOBPROP_USEJOURNAL))
      {
         if (0 == strcmp (pszValue, JOBPROP_TRUE))
         {
            fUseJournal_d = true;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, JOBPROP_FALSE))
         {
            fUseJournal_d = false;
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_USECUTMODE))
      {
         if (0 == strcmp (pszValue, JOBPROP_NONE))
         {
            eCutMode_d = CUT_MODE_NONE;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, JOBPROP_FULL))
         {
            eCutMode_d = CUT_MODE_FULL;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, JOBPROP_PARTIAL))
         {
            eCutMode_d = CUT_MODE_PARTIAL;
            fRet = true;
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

Enumeration * OkiPos_Instance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   class KeyArrayEnumerator : public Enumeration
   {
   public:
      KeyArrayEnumerator (PSZRO pszKey, int cArray, PSZRO *aArray)
      {
         pszKey_d = pszKey;
         iArray_d = 0;
         cArray_d = cArray;
         aArray_d = aArray;
      }

      virtual bool hasMoreElements ()
      {
         if (iArray_d < cArray_d)
            return true;
         else
            return false;
      }

      virtual void *nextElement ()
      {
         if (iArray_d > cArray_d - 1)
            return 0;

         std::ostringstream oss;

         oss << pszKey_d
             << "="
             << aArray_d[iArray_d];

         iArray_d++;

         return (void *)new JobProperties (oss.str ().c_str ());
      }

   private:
      PSZRO  pszKey_d;
      int    iArray_d;
      int    cArray_d;
      PSZRO *aArray_d;
   };

   Enumeration       *pEnum    = 0;
   EnumEnumerator    *pEnumRet = 0;

   pEnumRet = new EnumEnumerator ();

   if (pEnumRet)
   {
      pEnum = new KeyArrayEnumerator (JOBPROP_USEJOURNAL,
                                      dimof (apszJournalValues),
                                      apszJournalValues);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }

      pEnum = new KeyArrayEnumerator (JOBPROP_USECUTMODE,
                                      dimof (apszCutmodeValues),
                                      apszCutmodeValues);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
   }

   return pEnumRet;
}

std::string * OkiPos_Instance::
getJobPropertyType (PSZRO pszKey)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (pszKey, JOBPROP_USEJOURNAL))
   {
      pRet = new std::string ("boolean ");
      *pRet += JOBPROP_FALSE;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_USECUTMODE))
   {
      pRet = new std::string ("string ");
      *pRet += JOBPROP_NONE;
   }

   return pRet;
}

std::string * OkiPos_Instance::
getJobProperty (PSZRO pszKey)
{
   if (0 == strcasecmp (pszKey, JOBPROP_USEJOURNAL))
   {
      if (fUseJournal_d)
      {
         return new std::string (JOBPROP_TRUE);
      }
      else
      {
         return new std::string (JOBPROP_FALSE);
      }
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_USECUTMODE))
   {
      switch (eCutMode_d)
      {
      case CUT_MODE_NONE:    return new std::string (JOBPROP_NONE);
      case CUT_MODE_FULL:    return new std::string (JOBPROP_FULL);
      case CUT_MODE_PARTIAL: return new std::string (JOBPROP_PARTIAL);
      }
   }

   return 0;
}

std::string * OkiPos_Instance::
translateKeyValue (PSZRO pszKey,
                   PSZRO pszValue)
{
   PSZRO        pszRetKey   = 0;
   PSZRO        pszRetValue = 0;
   std::string *pRet        = 0;
   int          iValueEnum  = StringResource::DEVICE_COMMON_UNKNOWN;

   if (0 == strcasecmp (pszKey, JOBPROP_USEJOURNAL))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_JOURNAL);
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_USECUTMODE))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_CUTMODE);
   }

   if (pszValue)
   {
      if (0 == strcasecmp (pszValue, JOBPROP_TRUE))
         iValueEnum = StringResource::DEVICE_COMMON_TRUE;
      else if (0 == strcasecmp (pszValue, JOBPROP_FALSE))
         iValueEnum = StringResource::DEVICE_COMMON_FALSE;
      else if (0 == strcasecmp (pszValue, JOBPROP_NONE))
         iValueEnum = StringResource::DEVICE_COMMON_NONE;
      else if (0 == strcasecmp (pszValue, JOBPROP_FULL))
         iValueEnum = StringResource::DEVICE_COMMON_FULL;
      else if (0 == strcasecmp (pszValue, JOBPROP_PARTIAL))
         iValueEnum = StringResource::DEVICE_COMMON_PARTIAL;
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

bool OkiPos_Instance::
deviceOptionValid (PSZRO pszDeviceOption)
{
   if (  0 == strcmp (pszDeviceOption, "MODEL_S")
      || 0 == strcmp (pszDeviceOption, "MODEL_D")
      )
   {
      return true;
   }

   return false;
}

void OkiPos_Instance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;

   DeviceResolution *pRes      = getCurrentResolution ();
   DeviceForm       *pDF       = getCurrentForm ();
   HardCopyCap      *pHCC      = pDF->getHardCopyCap ();
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

   // @TBD
///sendBinaryDataToDevice (getCurrentTray ());

   pCmd = pCommands->getCommandData ("cmdSetLineSpacing144inch");
   if (pCmd)
   {
      int iSpacing = 144 * pRes->getScanlineMultiple () / pRes->getYRes ();

      sendPrintfToDevice (pCmd, iSpacing);
   }

   int iLines = (int)(6.0 * (float)pHCC->getCy () / 25400.0);

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iLines = " << iLines << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdSetPageLengthInLines");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd, iLines);
   }

   if (fUseJournal_d)
   {
      pCmd = pCommands->getCommandData ("cmdSetJournalMode");
      if (pCmd)
      {
         sendPrintfToDevice (pCmd, 1);
      }
   }

   DebugOutput::getErrorStream () << "******* " << pHCC->getXPels () << std::endl;
}

bool OkiPos_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::beginJob ()" << std::endl;
#endif

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

bool OkiPos_Instance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::beginJob (with props)" << std::endl;
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

bool OkiPos_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::newFrame ()" << std::endl;
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

bool OkiPos_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::newFrame (with props)" << std::endl;
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

bool OkiPos_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::endJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   // If the user wants to cut the receipt, then advance the roll and cut.
   pCmd = 0;
   switch (eCutMode_d)
   {
   case CUT_MODE_FULL:    pCmd = pCommands->getCommandData ("cmdFullCut"); break;
   case CUT_MODE_PARTIAL: pCmd = pCommands->getCommandData ("cmdPartialCut"); break;
   case CUT_MODE_NONE:    /* Error! */ break;
   }
   if (pCmd)
   {
      DeviceForm  *pDF       = getCurrentForm ();
      HardCopyCap *pHCC      = pDF->getHardCopyCap ();
      int          iLength   = 0;
      BinaryData  *pCmdSpace = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdXXXCut = " << *pCmd << std::endl;
#endif

      iLength = (pHCC->getTopClip () + pHCC->getBottomClip ()) * 144 / 25400;

      DebugOutput::getErrorStream () << "iLength = " << iLength << std::endl;

      pCmdSpace = pCommands->getCommandData ("cmdSetLineSpacing144inch");
      if (pCmdSpace)
      {
         sendPrintfToDevice (pCmdSpace, iLength);
      }
      pCmdSpace = pCommands->getCommandData ("cmdMoveToNextRasterGraphicsLine");
      if (pCmdSpace)
      {
         sendBinaryDataToDevice (pCmdSpace);
      }
      pCmdSpace = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
      if (pCmdSpace)
      {
         sendBinaryDataToDevice (pCmdSpace);
      }

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

bool OkiPos_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OkiPos_Instance::endJob ()" << std::endl;
#endif

   int cx = getCurrentForm ()->getHardCopyCap ()->getXPels ();

   // The largest size is 24 lines
   int iSize = 24 * (cx + 7) / 8;

   PBYTE abData = new BYTE [iSize];

   memset (abData, 0, sizeof (abData));

   BinaryData data (abData, sizeof (abData));

   sendBinaryDataToDevice (&data);

   delete[] abData;

   return true;
}

#ifndef RETAIL

void OkiPos_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string OkiPos_Instance::
toString (std::ostringstream& oss)
{
   oss << "{OkiPos_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const OkiPos_Instance& const_self)
{
   OkiPos_Instance&   self = const_cast<OkiPos_Instance&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
