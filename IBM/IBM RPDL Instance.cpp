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
#include "IBM_RPDL_Instance.hpp"

#include <defines.hpp>
#include <JobProperties.hpp>

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new IBM_RPDL_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

IBM_RPDL_Instance::
IBM_RPDL_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::IBM_RPDL_Instance ()" << std::endl;
#endif

   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
}

IBM_RPDL_Instance::
~IBM_RPDL_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::~IBM_RPDL_Instance ()" << std::endl;
#endif
}

void IBM_RPDL_Instance::
initializeInstance (PSZCRO pszJobProperties)
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
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::initializeInstance ()" << std::endl;
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
writeTStateValue (std::ostringstream& oss, IBM_RPDL_Instance::TRISTATE tValue)
{
   switch (tValue)
   {
   case IBM_RPDL_Instance::TSTATE_NONE: oss << "on";   break;
   case IBM_RPDL_Instance::TSTATE_ON:   oss << "off";  break;
   case IBM_RPDL_Instance::TSTATE_OFF:  oss << "none"; break;
   default:                                            break;
   }
}

std::string * IBM_RPDL_Instance::
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

bool IBM_RPDL_Instance::
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

Enumeration * IBM_RPDL_Instance::
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

std::string * IBM_RPDL_Instance::
getJobPropertyType (PSZRO pszKey)
{
   TRISTATE state = TSTATE_NONE;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::" << __FUNCTION__ << " (" << (pszKey ? pszKey : "") << ")" << std::endl;
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

std::string * IBM_RPDL_Instance::
getJobProperty (PSZRO pszKey)
{
   TRISTATE state = TSTATE_NONE;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::" << __FUNCTION__ << " (" << (pszKey ? pszKey : "") << ")" << std::endl;
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

std::string * IBM_RPDL_Instance::
translateKeyValue (PSZRO pszKey,
                   PSZRO pszValue)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::" << __FUNCTION__ << " ("
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

bool IBM_RPDL_Instance::
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

void IBM_RPDL_Instance::
setupPrinter ()
{
   DeviceCommand     *pCommands = getCommands ();
   DeviceTray        *pDT       = getCurrentTray ();
   BinaryData        *pCmd      = 0;
   DeviceResolution  *pRes      = getCurrentResolution ();
   DeviceForm        *pDF       = getCurrentForm ();
   DeviceMedia       *pDM       = getCurrentMedia ();
   DeviceOrientation *pDO       = getCurrentOrientation ();

   std::string *pstringTray        = 0;
   std::string *pstringOrientation = 0;

   pstringTray = pDT->getInputTray ();
   pstringOrientation = pDO->getRotation ();

   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::setupPrinter ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdGraphicsMode");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdGraphicsMode= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdGraphicsUnit");
   if (pCmd)
   {
      BYTE bCmd;

      if (  400 == pRes->getYRes ()
         && 400 == pRes->getXRes ()
         )
         bCmd = '1';
      else if (  600 == pRes->getYRes()
              && 600 == pRes->getXRes()
              )
         bCmd = '3';
      else if (  1200 == pRes->getYRes ()
              && 1200 == pRes->getXRes ()
              )
         bCmd = '4';
      else
         bCmd = '3';
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdGraphicsUnit= " << *pCmd << "(" << bCmd << ")" << std::endl;
#endif
      sendPrintfToDevice (pCmd, bCmd);
   }

   pCmd = pCommands->getCommandData ("cmdSpacingUnit2");
   if (pCmd)
   {
      BYTE bCmd;

      if (  400 == pRes->getYRes ()
         && 400 == pRes->getXRes ()
         )
         bCmd = '7';
      else if (  600 == pRes->getYRes()
              && 600 == pRes->getXRes()
              )
         bCmd = '8';
      else if (  1200 == pRes->getYRes ()
              && 1200 == pRes->getXRes ()
              )
         bCmd = '9';
      else
         bCmd = '8';
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdSpacingUnit2= " << *pCmd << "(" << bCmd << ")" << std::endl;
#endif
      sendPrintfToDevice (pCmd, bCmd);
   }

   pCmd = pCommands->getCommandData ("cmdEngineResolution");
   if (pCmd)
   {
      BYTE bCmd;

      if (  400 == pRes->getYRes ()
         && 400 == pRes->getXRes ()
         )
         bCmd = '1';
      else if (  600 == pRes->getYRes()
              && 600 == pRes->getXRes()
              )
         bCmd = '3';
      else if (  1200 == pRes->getYRes ()
              && 1200 == pRes->getXRes ()
              )
         bCmd = '4';
      else
         bCmd = '3';
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdEngineResolution= " << *pCmd << "(" << bCmd << ")" << std::endl;
#endif
      sendPrintfToDevice (pCmd, bCmd);
   }

   pCmd = pCommands->getCommandData ("cmdCoordinateUnit");
   if (pCmd)
   {
      BYTE bCmd;

      if (  400 == pRes->getYRes ()
         && 400 == pRes->getXRes ()
         )
         bCmd = '2';
      else if (  600 == pRes->getYRes()
              && 600 == pRes->getXRes()
              )
         bCmd = '4';
      else if (  1200 == pRes->getYRes ()
              && 1200 == pRes->getXRes ()
              )
         bCmd = '5';
      else
         bCmd = '4';
#ifndef RETAI
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdCoordinateUnit= " << *pCmd << "(" << bCmd << ")" << std::endl;
#endif
      sendPrintfToDevice (pCmd, bCmd);
   }

   pCmd = pCommands->getCommandData ("cmdSpacingUnit");
   if (pCmd)
   {
      BYTE bCmd;

      if (  400 == pRes->getYRes ()
         && 400 == pRes->getXRes ()
         )
         bCmd = '4';
      else if (  600 == pRes->getYRes()
              && 600 == pRes->getXRes()
              )
         bCmd = '5';
      else if (  1200 == pRes->getYRes ()
              && 1200 == pRes->getXRes ()
              )
         bCmd = '6';
      else
         bCmd = '5';
#ifndef RETAI
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdSpacingUnit= " << *pCmd << "(" << bCmd << ")" << std::endl;
#endif
      sendPrintfToDevice (pCmd, bCmd);
   }

   pCmd = pCommands->getCommandData ("cmdPageLength");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageLength= " << *pCmd << "(2)" <<  std::endl;
#endif
      sendPrintfToDevice (pCmd, '2');
   }

   pCmd = pCommands->getCommandData ("cmdPrintableArea");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPrintableArea= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdPrintMode");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPrintMode= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdScale");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdScale= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   if (  pstringTray
      && 0 == pstringTray->compare ("AutoSelect")
      )
   {
      pCmd = pCommands->getCommandData ("cmdLimitLessOn");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdLimitLessOn= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      // tray select command
      pCmd = pCommands->getCommandData ("cmdAutoSelectTray");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdAutoSelectTray= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);

         pCmd = pDF->getData ();
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "form= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);

         pCmd = pDM->getData ();
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "media= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }
   }
   else
   {
      pCmd = pCommands->getCommandData ("cmdLimitLessOff");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdLimitLessOff= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      pCmd = pDT->getData ();
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdTrayChange= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }
   }

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      pCmd = pCommands->getCommandData ("cmdPortrait");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPortrait= " << *pCmd << std::endl;
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
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdLandscape= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }
   }

   pCmd = pCommands->getCommandData ("cmdWriteModeEraseWrite");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdWriteMode= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   delete pstringTray;
   delete pstringOrientation;

   fHaveSetupPrinter_d = true;
}

bool IBM_RPDL_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::beginJob ()" << std::endl;
#endif

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ())
   {
      std::string *pstring = 0;
      if (getCurrentOrientation ())
      {
         pstring = getCurrentOrientation ()->getRotation ();
         DebugOutput::getErrorStream () << "The Rotation is " << *pstring << std::endl;
         delete pstring;
      }
      if (getCurrentForm ())
      {
         pstring = getCurrentForm ()->getForm ();
         DebugOutput::getErrorStream () << "The Form is " << *pstring << std::endl;
         delete pstring;
      }
      if (getCurrentTray ())
      {
         pstring = getCurrentTray ()->getInputTray ();
         DebugOutput::getErrorStream () << "The InputTray is " << *pstring << std::endl;
         delete pstring;
      }
      if (getCurrentMedia ())
      {
         pstring = getCurrentMedia ()->getMedia ();
         DebugOutput::getErrorStream () << "The media is " << *pstring << std::endl;
         delete pstring;
      }
      if (getCurrentResolution ())
      {
         DebugOutput::getErrorStream () << "The Resolution is " << getCurrentResolution ()->getXRes () << "x" << getCurrentResolution ()->getYRes () << std::endl;
      }
   }
#endif

   if (hasDeviceOption ("SUPPORTS_PJL"))
   {
      pCmd = pCommands->getCommandData ("cmdPJLSignature");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPJLSignature =" << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      pCmd = pCommands->getCommandData ("cmdBeginJob");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdBeginJob =" << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      // @TBD Job prop stuff

      pCmd = pDevice_d->getCurrentOutputBin ()->getData ();
      if (pCmd)
      {

#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "OutputBin =" << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      pCmd = pCommands->getCommandData ("cmdPJLEnterLanguage");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPJLEnterLanguage= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }
   }

   pCmd = pCommands->getCommandData ("cmdChangeEmulation");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdChangeEmulation= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdGraphicsOut");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdGraphicsOut= " << *pCmd << std::endl;
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

bool IBM_RPDL_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::newFrame ()" << std::endl;
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

bool IBM_RPDL_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::newFrame (with props)" << std::endl;
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

bool IBM_RPDL_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::endJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageEject = " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdInit");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdInit = " << *pCmd <<  std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   if (hasDeviceOption ("SUPPORTS_PJL"))
   {
      pCmd = pCommands->getCommandData ("cmdPJLSignature");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPJLSignature= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      pCmd = pCommands->getCommandData ("cmdEndJob");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdEndJob= " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }

      pCmd = pCommands->getCommandData ("cmdPJLSignature");
      if (pCmd)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPJLSignature " << *pCmd << std::endl;
#endif
         sendBinaryDataToDevice (pCmd);
      }
   }

   pCmd = pCommands->getCommandData ("cmdTerm");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdTerm= " << *pCmd << std::endl;
#endif
      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool IBM_RPDL_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "IBM_RPDL_Instance::endJob ()" << std::endl;
#endif

   // @TBD
   return false;
}

#ifndef RETAIL

void IBM_RPDL_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string IBM_RPDL_Instance::
toString (std::ostringstream& oss)
{
   oss << "{IBM_RPDL_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const IBM_RPDL_Instance& const_self)
{
   IBM_RPDL_Instance& self = const_cast<IBM_RPDL_Instance&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
