/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2001
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
#include "Kyocera_PCL_Instance.hpp"

#include <defines.hpp>
#include <JobProperties.hpp>
#include <GplDitherInstance.hpp>

#include <iostream>
#include <sstream>
#include <stdio.h>

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new Kyocera_PCL_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

Kyocera_PCL_Instance::
Kyocera_PCL_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::Kyocera_PCL_Instance ()" << std::endl;
#endif

   ptlPrintHead_d.x     = 0;
   ptlPrintHead_d.y     = 0;
   iUOM_d               = 0;
   iXScalingFactor_d    = 1;
   iYScalingFactor_d    = 1;
   iYUOMScalingFactor_d = 0;
   iVerticalOffset_d    = 0;
   fHaveInitialized_d   = false;
   fHaveSetupPrinter_d  = false;
   iHardwareScaling_d   = 0;
}

Kyocera_PCL_Instance::
~Kyocera_PCL_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::~Kyocera_PCL_Instance ()" << std::endl;
#endif
}

void Kyocera_PCL_Instance::
initializeInstance ()
{
   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

#ifdef DEBUG
   if (DebugOutput::shouldOutputInstance ())
   {
      if (hasDeviceOption ("DO_AUTO_ROTATE"))              DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_AUTO_ROTATE" << std::endl;
      if (hasDeviceOption ("DO_SCALABLE_FONT"))            DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SCALABLE_FONT" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_DUPLEX"))           DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_DUPLEX" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_RULES"))            DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_RULES" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_DOT_COORDS"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_DOT_COORDS" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PJL"))              DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PJL" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_BACKBIN"))          DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_BACKBIN" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_JOBOFFSET"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_JOBOFFSET" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PATTERNS"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PATTERNS" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCL5"))             DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCL5" << std::endl;
      if (hasDeviceOption ("DO_MIXED_TRAY"))               DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_MIXED_TRAY" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLOR"))            DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLOR" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PS"))               DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PS" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PAGEPROT_ESC"))     DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PAGEPROT_ESC" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_ES"))               DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_ES" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_HPGL2"))            DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_HPGL2" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_HPGL2_MC"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_HPGL2_MC" << std::endl;
      if (hasDeviceOption ("DO_PAGEPROT_AUTO"))            DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_PAGEPROT_AUTO" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COMPRESS5"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COMPRESS5" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLORSMART"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLORSMART" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_HPGL_DEFAULT"))     DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_HPGL_DEFAULT" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_OUTPUTOPTION"))     DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_OUTPUTOPTION" << std::endl;
      if (hasDeviceOption ("DO_PRINTER_DISPLAY"))          DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_PRINTER_DISPLAY" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLOR_RET"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLOR_RET" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_HGRAYSCALE"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_HGRAYSCALE" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_WATERMARK"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_WATERMARK" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_HPGL2MACRO"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_HPGL2MACRO" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PAN_SUPER"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PAN_SUPER" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_CONFIG"))           DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_CONFIG" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_HPGL2LOW"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_HPGL2LOW" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_BIDI"))             DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_BIDI" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_STAPLER"))          DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_STAPLER" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCLXL"))            DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCLXL" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_AUTOSELECT"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_AUTOSELECT" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCLXL_DEFAULT"))    DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCLXL_DEFAULT" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCLXL_1200"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCLXL_1200" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_LARGECLRFMT"))      DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_LARGECLRFMT" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_2STAPLES"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_2STAPLES" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCLXL_600"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCLXL_600" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_NEWCLR"))           DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_NEWCLR" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PJLOUTPUTBIN"))     DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PJLOUTPUTBIN" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLLATOR"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLLATOR" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_MET"))              DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_MET" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_MOPIER"))           DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_MOPIER" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_MOPIER_DEFAULT"))   DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_MOPIER_DEFAULT" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_EUROCHAR"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_EUROCHAR" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_JOB_RETENTION"))    DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_JOB_RETENTION" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCLXL_1200R"))      DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCLXL_1200R" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_FAX"))              DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_FAX" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_SORTER"))           DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_SORTER" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLOR_OEM"))        DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLOR_OEM" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLLATE"))          DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLLATE" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_QUIETRESET"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_QUIETRESET" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_EDGEPRINTING"))     DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_EDGEPRINTING" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_OPTIMIZEPQ"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_OPTIMIZEPQ" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_COLORSMART2"))      DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_COLORSMART2" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_MANUALDUPLEX"))     DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_MANUALDUPLEX" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_NOFNTMGR"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_NOFNTMGR" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PRINTERSELECTDEF")) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PRINTERSELECTDEF" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCL6_REV20"))       DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCL6_REV20" << std::endl;
      if (hasDeviceOption ("DO_SUPPORT_PCL6_INT"))         DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << "device has device option: DO_SUPPORT_PCL6_INT" << std::endl;
   }
#endif

   int iValidUnits[] = {
       96,  100,  120,  144,  150,  160,  180,  200,
      225,  240,  288,  300,  360,  400,  450,  480,
      600,  720,  800,  900, 1200, 1440, 1800, 2400,
      3600, 7200
   };
   DeviceResolution *pDR   = getCurrentResolution ();
   int               iYRes = pDR->getExternalYRes ();
   int               iLow  = 0;
   int               iMid  = (int)dimof (iValidUnits) / 2;
   int               iHigh = (int)dimof (iValidUnits) - 1;

   // Search in the list of supported units
   while (iLow <= iHigh)
   {
      if (iYRes == iValidUnits[iMid])
      {
         // Found!
         break;
      }
      else if (iYRes < iValidUnits[iMid])
      {
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (iYRes > iValidUnits[iMid])
      {
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   iYUOMScalingFactor_d = 1;

   if (iLow > iHigh)
   {
      // Not found in the list, try for a multiple
      for (iMid = 0; iMid < (int)dimof (iValidUnits); iMid++)
      {
         if (0 == iValidUnits[iMid] % iYRes)
         {
            iYUOMScalingFactor_d = iValidUnits[iMid] / iYRes;
            break;
         }
      }

      if (iMid == (int)dimof (iValidUnits))
      {
         // Still not found.  Default to 300
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::initializeInstance: Error unable to handle " << iYRes << "!" << std::endl;
#endif

         iMid = 11; // 300
      }
   }

   HardCopyCap *pHCC     = getCurrentForm ()->getHardCopyCap ();
   int          iTopClip = (int)(((float)pHCC->getTopClip () / 25400.0) * pDR->getExternalYRes () + 0.5);

   iUOM_d            = iValidUnits[iMid];
///iVerticalOffset_d = iUOM_d / 6; // is 50 @ 300 uom
   iVerticalOffset_d = iTopClip;

   if (pDR->getExternalXRes () > pDR->getXRes ())
   {
      iXScalingFactor_d = pDR->getExternalXRes () / pDR->getXRes ();
   }
   if (pDR->getExternalYRes () > pDR->getYRes ())
   {
      iYScalingFactor_d = pDR->getExternalYRes () / pDR->getYRes ();
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::initializeInstance () iXScalingFactor_d    = " << iXScalingFactor_d << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::initializeInstance () iYScalingFactor_d    = " << iYScalingFactor_d << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::initializeInstance () iUOMYScalingFactor_d = " << iYUOMScalingFactor_d << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::initializeInstance () iUOM_d               = " << iUOM_d << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::initializeInstance () iVerticalOffset_d    = " << iVerticalOffset_d << std::endl;
#endif
}

static PSZRO apszDeviceJobPropertyKeys[] = {
   "HardwareScaling"
};
#define JOBPROP_HWSCALING apszDeviceJobPropertyKeys[0]

std::string * Kyocera_PCL_Instance::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   oss << JOBPROP_HWSCALING
       << "={"
       << iHardwareScaling_d
       << ",1,10}";           // @TBD is 10 the maximum?

   return new std::string (oss.str ());
}

bool Kyocera_PCL_Instance::
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

      if (0 == strcmp (pszKey, JOBPROP_HWSCALING))
      {
         int iValue = atoi (pszValue);

         if (iValue > 0)
         {
            DeviceResolution *pDR   = getCurrentResolution ();
            int               iXRes = pDR->getXRes ();
            int               iYRes = pDR->getYRes ();

            if (  (iXRes % iValue) == 0
               && (iYRes % iValue) == 0
               )
            {
               iHardwareScaling_d = iValue;

               pDR->setInternalXRes (iXRes / iValue);
               pDR->setInternalYRes (iYRes / iValue);

               fRet = true;
            }
         }
         else if (iValue == 0)
         {
            DeviceResolution *pDR = getCurrentResolution ();

            iHardwareScaling_d = iValue;

            pDR->setInternalXRes (pDR->getXRes ());
            pDR->setInternalYRes (pDR->getYRes ());

            fRet = true;
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

Enumeration * Kyocera_PCL_Instance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   class HWScalingJPEnumeration : public Enumeration
   {
   public:
      HWScalingJPEnumeration (int iDefault)
      {
         iDefault_d       = iDefault;
         fReturnedValue_d = false;
      }

      virtual bool hasMoreElements ()
      {
         return !fReturnedValue_d;
      }

      virtual void *nextElement ()
      {
         if (fReturnedValue_d)
            return 0;

         std::ostringstream oss;

         oss << JOBPROP_HWSCALING
             << "={"
             << iDefault_d
             << ",1,10}";

         fReturnedValue_d = true;

         return (void *)new JobProperties (oss.str ().c_str ());
      }

   private:
      int  iDefault_d;
      bool fReturnedValue_d;
   };

   Enumeration    *pEnumHWScaling = 0;
   EnumEnumerator *pEnumRet       = 0;

   pEnumHWScaling = new HWScalingJPEnumeration (iHardwareScaling_d);

   if (pEnumHWScaling)
   {
      pEnumRet = new EnumEnumerator ();

      if (pEnumRet)
      {
         pEnumRet->addElement (pEnumHWScaling);
      }
   }

   return pEnumRet;
}

std::string * Kyocera_PCL_Instance::
getJobPropertyType (PSZRO pszKey)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << " (" << (pszKey ? pszKey : "") << ")" << std::endl;
#endif

   if (0 == strcmp (JOBPROP_HWSCALING, pszKey))
   {
      std::ostringstream oss;

      oss << "integer "      // type
          << 0               // default
          << " "
          << 0;              // minimum

      std::string *pRet = new std::string (oss.str ());

      return pRet;
   }

   return 0;
}

std::string * Kyocera_PCL_Instance::
getJobProperty (PSZRO pszKey)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << " (" << (pszKey ? pszKey : "") << ")" << std::endl;
#endif

   if (0 == strcmp (JOBPROP_HWSCALING, pszKey))
   {
      std::ostringstream oss;

      oss << iHardwareScaling_d;

      std::string *pRet = new std::string (oss.str ());

      return pRet;
   }

   return 0;
}

std::string * Kyocera_PCL_Instance::
translateKeyValue (PSZRO pszKey,
                   PSZRO pszValue)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::" << __FUNCTION__ << " ("
                          << (pszKey ? pszKey : "") << ", "
                          << (pszValue ? pszValue : "") << ")"
                          << std::endl;
#endif

   PSZRO        pszRetKey = 0;
   std::string *pRet      = 0;

   if (0 == strcasecmp (pszKey, JOBPROP_HWSCALING))
   {
      pszRetKey = StringResource::getString (getLanguageResource (),
                                             StringResource::STRINGGROUP_DEVICE_COMMON,
                                             StringResource::DEVICE_COMMON_HARDWARE_SCALING);
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

bool Kyocera_PCL_Instance::
deviceOptionValid (PSZRO pszDeviceOption)
{
   if (  0 == strcmp (pszDeviceOption, "DO_AUTO_ROTATE")
      || 0 == strcmp (pszDeviceOption, "DO_SCALABLE_FONT")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_DUPLEX")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_RULES")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_DOT_COORDS")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PJL")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_BACKBIN")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_JOBOFFSET")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PATTERNS")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCL5")
      || 0 == strcmp (pszDeviceOption, "DO_MIXED_TRAY")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLOR")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PS")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PAGEPROT_ESC")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_ES")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_HPGL2")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_HPGL2_MC")
      || 0 == strcmp (pszDeviceOption, "DO_PAGEPROT_AUTO")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COMPRESS5")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLORSMART")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_HPGL_DEFAULT")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_OUTPUTOPTION")
      || 0 == strcmp (pszDeviceOption, "DO_PRINTER_DISPLAY")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLOR_RET")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_HGRAYSCALE")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_WATERMARK")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_HPGL2MACRO")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PAN_SUPER")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_CONFIG")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_HPGL2LOW")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_BIDI")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_STAPLER")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCLXL")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_AUTOSELECT")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCLXL_DEFAULT")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCLXL_1200")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_LARGECLRFMT")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_2STAPLES")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCLXL_600")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_NEWCLR")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PJLOUTPUTBIN")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLLATOR")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_MET")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_MOPIER")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_MOPIER_DEFAULT")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_EUROCHAR")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_JOB_RETENTION")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCLXL_1200R")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_FAX")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_SORTER")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLOR_OEM")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLLATE")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_QUIETRESET")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_EDGEPRINTING")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_OPTIMIZEPQ")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_COLORSMART2")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_MANUALDUPLEX")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_NOFNTMGR")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PRINTERSELECTDEF")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCL6_REV20")
      || 0 == strcmp (pszDeviceOption, "DO_SUPPORT_PCL6_INT")
      )
   {
      return true;
   }

   return false;
}

void Kyocera_PCL_Instance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;

   DeviceResolution *pRes      = getCurrentResolution ();
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetUnitsOfMeasure");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd, iUOM_d);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetUnitsOfMeasure defined for this device!" << std::endl;
#endif
   }

   // Select the form
   sendBinaryDataToDevice (getCurrentForm ());

   // Select the media
   sendBinaryDataToDevice (getCurrentMedia ());

   // Select the tray
   sendBinaryDataToDevice (getCurrentTray ());

   // Select the resolution
   sendPrintfToDevice (pRes->getData (), pRes->getExternalXRes ());

   pCmd = pCommands->getCommandData ("cmdSetTopMargin");
   if (pCmd)
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetTopMargin defined for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdSetXYPos");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd,
                          0,
                          iVerticalOffset_d * iYUOMScalingFactor_d);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetXYPos defined for this device!" << std::endl;
#endif
   }

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_RGB:
   {
      pCmd = pCommands->getCommandData ("cmdConfigureImageData24BPP");
      if (pCmd)
      {
         sendBinaryDataToDevice (pCmd);

         BinaryData  *pCmd2   = 0;
         PBYTE        pbGamma = 0;
         DeviceGamma *pGamma  = getCurrentGamma ();

         pCmd    = pCommands->getCommandData ("cmdSetColorLookupTable");
         pCmd2   = pCommands->getCommandData ("cmdSetColorLookupTableDataParm");
         pbGamma = (PBYTE)malloc (256);

#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pCmd = " << *pCmd << std::endl;
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pCmd2 = " << *pCmd2 << std::endl;
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pbGamma = 0x" << std::hex << (int)pbGamma << std::dec << std::endl;
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "pGamma = " << *pGamma << std::endl;
#endif

         if (  pCmd
            && pCmd2
            && pbGamma
            )
         {
            sendPrintfToDevice (pCmd, 770); // length of following

            sendPrintfToDevice (pCmd2, 0); // RGB
            sendPrintfToDevice (pCmd2, 0); // reserved
            for (int i = 0; i < 3; i++)
            {
               switch (i)
               {
               case 0:
               {
                  // Blue gamma values
                  GplGenerateGammaCurve (pGamma->getYGamma (),
                                         pGamma->getYBias (),
                                         pbGamma);
                  break;
               }

               case 1:
               {
                  // Green gamma values
                  GplGenerateGammaCurve (pGamma->getMGamma (),
                                         pGamma->getMBias (),
                                         pbGamma);
                  break;
               }

               case 2:
               {
                  // Red gamma values
                  GplGenerateGammaCurve (pGamma->getCGamma (),
                                         pGamma->getCBias (),
                                         pbGamma);
                  break;
               }
               }

               for (int j = 0; j < 256; j++)
               {
#ifndef RETAIL
                  if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "i = " << i << ", j = " << j << ", data = " << (int)pbGamma[j] << std::endl;
#endif

                  sendPrintfToDevice (pCmd2, pbGamma[255 - j]);
               }
            }
         }

         if (pbGamma)
            free (pbGamma);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdConfigureImageData defined for this device!" << std::endl;
#endif
      }
      break;
   }
   case DevicePrintMode::COLOR_TECH_K:
   {
      pCmd = pCommands->getCommandData ("cmdConfigureImageData1BPP");
      if (pCmd)
      {
         sendBinaryDataToDevice (pCmd);
      }
      break;
   }
   }
}

bool Kyocera_PCL_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::beginJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdEnterLanguage");
   if (  hasDeviceOption ("DO_SUPPORT_PJL")
      && pCmd
      )
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Error: There is no cmdEnterLanguage defined for this device!" << std::endl;
#endif
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

bool Kyocera_PCL_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::newFrame ()" << std::endl;
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

bool Kyocera_PCL_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::newFrame (with props)" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iOrientation = " << getCurrentOrientation() << std::endl;
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

bool Kyocera_PCL_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::endJob ()" << std::endl;
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

bool Kyocera_PCL_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Instance::endJob ()" << std::endl;
#endif

   return true;
}

#ifndef RETAIL

void Kyocera_PCL_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Kyocera_PCL_Instance::
toString (std::ostringstream& oss)
{
   oss << "{Kyocera_PCL_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Kyocera_PCL_Instance& const_self)
{
   Kyocera_PCL_Instance& self = const_cast<Kyocera_PCL_Instance&>(const_self);
   std::ostringstream        oss;

   os << self.toString (oss);

   return os;
}
