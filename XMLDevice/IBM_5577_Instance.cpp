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
#include "IBM_5577_Instance.hpp"

#include <defines.hpp>
#include <JobProperties.hpp>

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new IBM_5577_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

IBM_5577_Instance::
IBM_5577_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::IBM_5577_Instance ()" << std::endl;
#endif

   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
}

IBM_5577_Instance::
~IBM_5577_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::~IBM_5577_Instance ()" << std::endl;
#endif
}

void IBM_5577_Instance::
initializeInstance ()
{
   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

   fRET_d         = TSTATE_NONE;
   fEconoMode_d   = TSTATE_NONE;
   fPageProtect_d = TSTATE_NONE;
   fJamRecovery_d = TSTATE_NONE;
   fStaple_d      = TSTATE_NONE;
   fOffset_d      = TSTATE_NONE;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::initializeInstance ()" << std::endl;
#endif
}

static PSZRO apszDeviceJobPropertyKeys[] = {
   "RET",
   "EconoMode",
   "PageProtect",
   "JamRecovery",
   "Staple",
   "Offset"
};
#define JOBPROP_RET              apszDeviceJobPropertyKeys[0]
#define JOBPROP_ECONOMODE        apszDeviceJobPropertyKeys[1]
#define JOBPROP_PAGEPROTECT      apszDeviceJobPropertyKeys[2]
#define JOBPROP_JAMRECOVERY      apszDeviceJobPropertyKeys[3]
#define JOBPROP_STAPLE           apszDeviceJobPropertyKeys[4]
#define JOBPROP_OFFSET           apszDeviceJobPropertyKeys[5]

void
writeTStateValue (std::ostringstream& oss, IBM_5577_Instance::TRISTATE tValue)
{
   switch (tValue)
   {
   case IBM_5577_Instance::TSTATE_NONE: oss << "on";   break;
   case IBM_5577_Instance::TSTATE_ON:   oss << "off";  break;
   case IBM_5577_Instance::TSTATE_OFF:  oss << "none"; break;
   default:                                            break;
   }
}

std::string * IBM_5577_Instance::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   oss << JOBPROP_RET
       << "=";
   writeTStateValue (oss, fRET_d);

   oss << " "
       << JOBPROP_ECONOMODE
       << "=";
   writeTStateValue (oss, fEconoMode_d);

   oss << " "
       << JOBPROP_PAGEPROTECT
       << "=";
   writeTStateValue (oss, fPageProtect_d);

   oss << " "
       << JOBPROP_JAMRECOVERY
       << "=";
   writeTStateValue (oss, fJamRecovery_d);

   oss << " "
       << JOBPROP_STAPLE
       << "=";
   writeTStateValue (oss, fStaple_d);

   oss << " "
       << JOBPROP_OFFSET
       << "=";
   writeTStateValue (oss, fOffset_d);

   return new std::string (oss.str ());
}

bool IBM_5577_Instance::
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

      if (0 == strcmp (pszKey, JOBPROP_RET))
      {
         if (0 == strcmp (pszValue, "on"))
         {
            fRET_d = TSTATE_ON;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "off"))
         {
            fRET_d = TSTATE_OFF;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "none"))
         {
            fRET_d = TSTATE_NONE;
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_ECONOMODE))
      {
         if (0 == strcmp (pszValue, "on"))
         {
            fEconoMode_d = TSTATE_ON;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "off"))
         {
            fEconoMode_d = TSTATE_OFF;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "none"))
         {
            fEconoMode_d = TSTATE_NONE;
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_PAGEPROTECT))
      {
         if (0 == strcmp (pszValue, "on"))
         {
            fPageProtect_d = TSTATE_ON;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "off"))
         {
            fPageProtect_d = TSTATE_OFF;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "none"))
         {
            fPageProtect_d = TSTATE_NONE;
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_JAMRECOVERY))
      {
         if (0 == strcmp (pszValue, "on"))
         {
            fJamRecovery_d = TSTATE_ON;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "off"))
         {
            fJamRecovery_d = TSTATE_OFF;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "none"))
         {
            fJamRecovery_d = TSTATE_NONE;
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_STAPLE))
      {
         if (0 == strcmp (pszValue, "on"))
         {
            fStaple_d = TSTATE_ON;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "off"))
         {
            fStaple_d = TSTATE_OFF;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "none"))
         {
            fStaple_d = TSTATE_NONE;
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_OFFSET))
      {
         if (0 == strcmp (pszValue, "on"))
         {
            fOffset_d = TSTATE_ON;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "off"))
         {
            fOffset_d = TSTATE_OFF;
            fRet = true;
         }
         else if (0 == strcmp (pszValue, "none"))
         {
            fOffset_d = TSTATE_NONE;
            fRet = true;
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

Enumeration * IBM_5577_Instance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   class TristateEnumerator : public Enumeration
   {
   public:
      TristateEnumerator (PSZRO pszKey)
      {
         iCount_d = 0;
         pszKey_d = pszKey;
      }

      virtual bool
      hasMoreElements ()
      {
         if (iCount_d < 3)
            return true;
         else
            return false;
      }

      virtual void *
      nextElement ()
      {
         if (iCount_d > 2)
            return 0;

         std::ostringstream oss;

         oss << pszKey_d
             << "=";

         switch (iCount_d)
         {
         case 0:  oss << "none"; break;
         case 1:  oss << "on";   break;
         case 2:  oss << "off";  break;
         default:                break;
         }

         iCount_d++;

         return (void *)new JobProperties (oss.str ().c_str ());
      }

   private:
      int   iCount_d;
      PSZRO pszKey_d;
   };

   TristateEnumerator *pEnum    = 0;
   EnumEnumerator     *pEnumRet = 0;

   pEnumRet = new EnumEnumerator ();

   if (pEnumRet)
   {
      pEnum = new TristateEnumerator (JOBPROP_RET);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
      pEnum = new TristateEnumerator (JOBPROP_ECONOMODE);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
      pEnum = new TristateEnumerator (JOBPROP_PAGEPROTECT);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
      pEnum = new TristateEnumerator (JOBPROP_JAMRECOVERY);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
      pEnum = new TristateEnumerator (JOBPROP_STAPLE);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
      pEnum = new TristateEnumerator (JOBPROP_OFFSET);
      if (pEnum)
      {
         pEnumRet->addElement (pEnum);
      }
   }

   return pEnumRet;
}

std::string * IBM_5577_Instance::
getJobPropertyType (PSZRO pszKey)
{
   TRISTATE state = TSTATE_NONE;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::" << __FUNCTION__ << " (" << (pszKey ? pszKey : "") << ")" << std::endl;
#endif

   if (0 == strcasecmp (pszKey, JOBPROP_RET))
   {
      state = fRET_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_ECONOMODE))
   {
      state = fEconoMode_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_PAGEPROTECT))
   {
      state = fPageProtect_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_JAMRECOVERY))
   {
      state = fJamRecovery_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_STAPLE))
   {
      state = fStaple_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_OFFSET))
   {
      state = fOffset_d;
   }
   else
   {
      return 0;
   }

   switch (fOffset_d)
   {
   case TSTATE_NONE: return new std::string ("string none");
   case TSTATE_ON:   return new std::string ("string on");
   case TSTATE_OFF:  return new std::string ("string off");
   default:          return 0;
   }

   return 0;
}

std::string * IBM_5577_Instance::
getJobProperty (PSZRO pszKey)
{
   TRISTATE state = TSTATE_NONE;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::" << __FUNCTION__ << " (" << (pszKey ? pszKey : "") << ")" << std::endl;
#endif

   if (0 == strcasecmp (pszKey, JOBPROP_RET))
   {
      state = fRET_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_ECONOMODE))
   {
      state = fEconoMode_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_PAGEPROTECT))
   {
      state = fPageProtect_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_JAMRECOVERY))
   {
      state = fJamRecovery_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_STAPLE))
   {
      state = fStaple_d;
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_OFFSET))
   {
      state = fOffset_d;
   }
   else
   {
      return 0;
   }

   switch (fOffset_d)
   {
   case TSTATE_NONE: return new std::string ("none");
   case TSTATE_ON:   return new std::string ("on");
   case TSTATE_OFF:  return new std::string ("off");
   default:          return 0;
   }

   return 0;
}

std::string * IBM_5577_Instance::
translateKeyValue (PSZRO pszKey,
                   PSZRO pszValue)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::" << __FUNCTION__ << " ("
                          << (pszKey ? pszKey : "") << ", "
                          << (pszValue ? pszValue : "") << ")"
                          << std::endl;
#endif

   PSZRO        pszRetKey = 0;
   std::string *pRet      = 0;

   if (0 == strcasecmp (pszKey, JOBPROP_RET))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_RET);
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_ECONOMODE))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_ECONO_MODE);
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_PAGEPROTECT))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_PAGE_PROTECT);
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_JAMRECOVERY))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_JAM_RECOVERY);
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_STAPLE))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_STAPLE);
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_OFFSET))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_OFFSET);
   }

   if (pszRetKey)
   {
      pRet = new std::string (pszRetKey);
   }

   if (  pszValue
      && pRet
      )
   {
      *pRet += "=";
      *pRet += pszValue;
   }

   return pRet;
}

bool IBM_5577_Instance::
deviceOptionValid (PSZRO pszDeviceOption)
{
   if (  0 == strcmp (pszDeviceOption, "SUPPORTS_PJL")
      || 0 == strcmp (pszDeviceOption, "SUPPORTS_EMULATIONMODE")
      )
   {
      return true;
   }

   return false;
}

void IBM_5577_Instance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;
}

bool IBM_5577_Instance::
beginJob ()
{
   DeviceCommand *pCommands = getCommands ();
   BinaryData    *pCmd      = 0;
   DeviceForm    *pDF       = getCurrentForm ();
   DeviceTray    *pDT       = getCurrentTray ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::beginJob ()" << std::endl;
#endif

   if (hasDeviceOption ("SUPPORTS_PJL"))
   {
      pCmd = pCommands->getCommandData ("cmdPJLSignature");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPJLSignature = " << *pCmd << std::endl;
#endif

         sendBinaryDataToDevice (pCmd);
      }

      pCmd = pCommands->getCommandData ("cmdBeginJob");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdBeginJob = " << *pCmd << std::endl;
#endif

         sendBinaryDataToDevice (pCmd);
      }

      // @TBD Job prop stuff

      pCmd = pCommands->getCommandData ("cmdEnterLanguage");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdEnterLanguage = " << *pCmd << std::endl;
#endif

         sendBinaryDataToDevice (pCmd);
      }

      // @TBD Job prop stuff
   }
   else if (hasDeviceOption ("SUPPORTS_EMULATIONMODE"))
   {
      pCmd = pCommands->getCommandData ("cmd5577Mode");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmd5577Mode = " << *pCmd << std::endl;
#endif

         sendBinaryDataToDevice (pCmd);
      }
   }

   pCmd = pCommands->getCommandData ("cmdInit");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   // @TBD hardware copies

   DeviceOrientation *pDO                = getCurrentOrientation ();
   std::string       *pstringOrientation = 0;

   pstringOrientation = pDO->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      pCmd = pCommands->getCommandData ("cmdPortrait");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPortrait = " << *pCmd << std::endl;
#endif

         sendBinaryDataToDevice (pCmd);
      }
   }
   else if (  !pstringOrientation
           || 0 == pstringOrientation->compare ("Landscape")
           )
   {
      pCmd = pCommands->getCommandData ("cmdLandscape");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdLandscape = " << *pCmd << std::endl;
#endif

         sendBinaryDataToDevice (pCmd);
      }
   }

   delete pstringOrientation;

   /* form select */
   pCmd = pDF->getData ();
   sendBinaryDataToDevice (pCmd);

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pDT = " << *pDT << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pDF = " << *pDF << std::endl;
#endif

   std::string *pstringTray = 0;

   pstringTray = pDT->getInputTray ();

   /* tray select */
   if (  pstringTray
      && 0 == pstringTray->compare ("TRAY_AUTO")
      && 0 == pstringTray->compare ("TRAY_FRONT_CONTINUOUS") // for 5577 continuous mode
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::beginJob2 ()" << std::endl;
#endif

      unsigned char tray;
      unsigned int  length      = 0;
      unsigned char n1          = 0,
                    n2          = 0;        // for 5577 form length set
      std::string  *pstringForm = 0;

      pstringForm = pDF->getForm ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pDF = " << *pDF << std::endl;
#endif

      if (pstringForm)
      {
         if (  0 == pstringForm->compare ("FORM_A3")
            || 0 == pstringForm->compare ("FORM_JIS_B4")
            || 0 == pstringForm->compare ("FORM_A4")
            || 0 == pstringForm->compare ("FORM_JIS_B5")
            || 0 == pstringForm->compare ("FORM_A5")
            || 0 == pstringForm->compare ("FORM_LETTER")
            || 0 == pstringForm->compare ("FORM_LEGAL")
            || 0 == pstringForm->compare ("FORM_HAGAKI_CARD")
            )
         {
            tray = 0x02;
         }
         else if (  0 == pstringForm->compare ("FORM_10_X_11")
                 || 0 == pstringForm->compare ("FORM_11_X_12")
                 || 0 == pstringForm->compare ("FORM_11_X_15")
                 || 0 == pstringForm->compare ("FORM_12_X_19")
                 || 0 == pstringForm->compare ("FORM_15_X_11")
                 || 0 == pstringForm->compare ("FORM_5_X_7")
                 )
         {
            tray = 0x01;

            length = (unsigned int)((double)pDF->getCy() / 25400.0 * 6.0);
            n1 = (length & 0xff00) >> 8;
            n2 = (length & 0x00ff);
 #ifndef RETAIL
            if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "size Cx = " << pDF->getCx() << std::endl;
            if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "size Cy = " << pDF->getCy() << std::endl;
            if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "length  = " << length << std::endl;
 #endif
         }
         else
         {
            tray = 0x01;
         }

         pCmd = pCommands->getCommandData ("cmdRenTanChg");
         sendPrintfToDevice (pCmd, tray);

         if (length)
         {
            pCmd = pCommands->getCommandData ("cmdFormLength");
            sendPrintfToDevice (pCmd, n1, n2);
         }

         delete pstringForm;
      }
   }
   else
   {
      pCmd = pDT->getData ();
      sendBinaryDataToDevice (pCmd);
   }

   delete pstringTray;

   return true;
}

bool IBM_5577_Instance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::beginJob (with props)" << std::endl;
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

bool IBM_5577_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::newFrame ()" << std::endl;
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

bool IBM_5577_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::newFrame (with props)" << std::endl;
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

bool IBM_5577_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::endJob ()" << std::endl;
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

bool IBM_5577_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_5577_Instance::endJob ()" << std::endl;
#endif

   // @TBD
   return false;
}

#ifndef RETAIL

void IBM_5577_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string IBM_5577_Instance::
toString (std::ostringstream& oss)
{
   oss << "{IBM_5577_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const IBM_5577_Instance& const_self)
{
   IBM_5577_Instance& self = const_cast<IBM_5577_Instance&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
