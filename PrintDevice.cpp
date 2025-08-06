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
#include "PrintDevice.hpp"
#include "StdioFilebuf.hpp"
#include "JobProperties.hpp"

#include "hppcl3/bitmap.hpp"

#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <vector>

#include <unistd.h>

PrintDevice::
PrintDevice (PSZRO       pszDriverName,
             PSZRO       pszDeviceName,
             PSZRO       pszShortName,
             PSZRO       pszLibraryName,
             EOMNICLASS  eOmniClass,
             PSZRO       pszJobProperties)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__ << " ()" << std::endl;
#endif

   // Initialize class data
   outputStream_d              = &std::cout;
   outputStreamBuf_d           = 0;
   fShouldDeleteOutputStream_d = false;

   pfnOutputFunction_d         = 0;
   pMagicCookie_d              = 0;

   iLanguageID_d               = 0;
   pLanguage_d                 = 0;

   pszDriverName_d             = pszDriverName;
   pszDeviceName_d             = pszDeviceName;
   pszShortName_d              = pszShortName;
   pszLibraryName_d            = pszLibraryName;
   eOmniClass_d                = eOmniClass;
   pszJobProperties_d          = 0;

   lCapabilities_d             = 0;
   lRasterCapabilities_d       = 0;

   pInstance_d                 = 0;
   pBlitter_d                  = 0;
   pPDL_d                      = 0;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   pBooklet_d                  = 0;
#endif
   pCommand_d                  = 0;
#ifdef INCLUDE_JP_COMMON_COPIES
   pCopies_d                   = 0;
#endif
   pszDitherID_d               = 0;
   pForm_d                     = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   pJogging_d                  = 0;
#endif
   pMedia_d                    = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   pNUp_d                      = 0;
#endif
   pOrientation_d              = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   pOutputBin_d                = 0;
#endif
   pPrintMode_d                = 0;
   pResolution_d               = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   pScaling_d                  = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   pSheetCollate_d             = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   pSide_d                     = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   pStitching_d                = 0;
#endif
   pTray_d                     = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   pTrimming_d                 = 0;
#endif

   pData_d                     = 0;
   pString_d                   = 0;

   pszLoadedLibrary_d          = 0;
   hModLibrary_d               = 0;

   if (   pszJobProperties
      && *pszJobProperties
      )
   {
      pszJobProperties_d = (char *)malloc (strlen (pszJobProperties) + 1);
      if (pszJobProperties_d)
         strcpy (pszJobProperties_d, pszJobProperties);
   }
}

PrintDevice::
~PrintDevice ()
{
   if (pInstance_d)
   {
      delete pInstance_d;
      pInstance_d = 0;
   }

   if (pBlitter_d)
   {
      delete pBlitter_d;
      pBlitter_d = 0;
   }

   if (pPDL_d)
   {
      delete pPDL_d;
      pPDL_d = 0;
   }

   if (fShouldDeleteOutputStream_d)
   {
      delete outputStream_d;
      delete outputStreamBuf_d;
   }

   iLanguageID_d = 0;
   delete pLanguage_d;
   pLanguage_d = 0;

   cleanupProperties ();

   if (pszLoadedLibrary_d)
   {
      free (pszLoadedLibrary_d);
      pszLoadedLibrary_d = 0;
   }

   if (hModLibrary_d)
   {
      g_module_close (hModLibrary_d);
      hModLibrary_d = 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::~" << __FUNCTION__ << " ()" << std::endl;
#endif
}

bool PrintDevice::
hasError ()
{
   if (pInstance_d)
   {
      if (pInstance_d->hasError ())
      {
         return true;
      }
   }

   if (pBlitter_d)
   {
      if (pBlitter_d->hasError ())
      {
         return true;
      }
   }

   return false;
}

void PrintDevice::
initialize ()
{
   if (!pString_d)
   {
      pString_d = getDefaultString ();
   }

   iLanguageID_d = StringResource::LANGUAGE_DEFAULT;

   if (pString_d)
   {
      pString_d->setLanguage (iLanguageID_d);
   }

   pLanguage_d = StringResource::create (iLanguageID_d, pString_d);

   initializeJobProperties ();

   /* @BEGIN @HACK
   ** UPDFDeviceInstance needs the job properties string
   ** all blitter modules use the DeviceXXX objects
   */
   if (pInstance_d)
   {
      // Initialize the device instance
      pInstance_d->initializeInstance (pszJobProperties_d);
   }

   if (pBlitter_d)
   {
      // Initialize the device blitter
      pBlitter_d->initializeInstance ();
   }
   /* @END @HACK
   */
}

void PrintDevice::
cleanupProperties ()
{
   if (pszJobProperties_d)
   {
      free (pszJobProperties_d);
      pszJobProperties_d = 0;
   }

#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pBooklet_d)
   {
      delete pBooklet_d;
      pBooklet_d = 0;
   }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   if (pCopies_d)
   {
      delete pCopies_d;
      pCopies_d = 0;
   }
#endif

   if (pszDitherID_d)
   {
      free (pszDitherID_d);
      pszDitherID_d = 0;
   }

   if (pForm_d)
   {
      delete pForm_d;
      pForm_d = 0;
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pJogging_d)
   {
      delete pJogging_d;
      pJogging_d = 0;
   }
#endif

   if (pMedia_d)
   {
      delete pMedia_d;
      pMedia_d = 0;
   }

#ifdef INCLUDE_JP_COMMON_NUP
   if (pNUp_d)
   {
      delete pNUp_d;
      pNUp_d = 0;
   }
#endif

   if (pOrientation_d)
   {
      delete pOrientation_d;
      pOrientation_d = 0;
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pOutputBin_d)
   {
      delete pOutputBin_d;
      pOutputBin_d = 0;
   }
#endif

   if (pPrintMode_d)
   {
      delete pPrintMode_d;
      pPrintMode_d = 0;
   }

   if (pResolution_d)
   {
      delete pResolution_d;
      pResolution_d = 0;
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   if (pScaling_d)
   {
      delete pScaling_d;
      pScaling_d = 0;
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pSheetCollate_d)
   {
      delete pSheetCollate_d;
      pSheetCollate_d = 0;
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   if (pSide_d)
   {
      delete pSide_d;
      pSide_d = 0;
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pStitching_d)
   {
      delete pStitching_d;
      pStitching_d = 0;
   }
#endif

#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pTrimming_d)
   {
      delete pTrimming_d;
      pTrimming_d = 0;
   }
#endif

   if (pTray_d)
   {
      delete pTray_d;
      pTray_d = 0;
   }

   if (pCommand_d)
   {
      delete pCommand_d;
      pCommand_d = 0;
   }

   if (pData_d)
   {
      delete pData_d;
      pData_d = 0;
   }

   if (pString_d)
   {
      delete pString_d;
      pString_d = 0;
   }
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

PSZCRO PrintDevice::
getVersion ()
{
   return VERSION;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

PSZCRO PrintDevice::
getDriverName ()
{
   return pszDriverName_d;
}

PSZCRO PrintDevice::
getDeviceName ()
{
   return pszDeviceName_d;
}

PSZCRO PrintDevice::
getShortName ()
{
   return pszShortName_d;
}

PSZCRO PrintDevice::
getLibraryName ()
{
   return pszLibraryName_d;
}

EOMNICLASS PrintDevice::
getOmniClass ()
{
   return eOmniClass_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void PrintDevice::
setDeviceInstance (DeviceInstance *pInstance)
{
   pInstance_d = pInstance;
}

DeviceInstance * PrintDevice::
getDeviceInstance ()
{
   return pInstance_d;
}

void PrintDevice::
setDeviceBlitter (DeviceBlitter *pBlitter)
{
   pBlitter_d = pBlitter;
}

DeviceBlitter * PrintDevice::
getDeviceBlitter ()
{
   return pBlitter_d;
}

void PrintDevice::
setPDL (PDL *pPDL)
{
   pPDL_d = pPDL;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void PrintDevice::
setCapabilities (long lCapabilities)
{
   lCapabilities_d |= lCapabilities;
}

void PrintDevice::
setRasterCapabilities (long lRasterCapabilities)
{
   lRasterCapabilities_d |= lRasterCapabilities;
}

bool PrintDevice::
hasCapability (long lMask)
{
   if ((lCapabilities_d & lMask) != 0)
      return true;
   else
      return false;
}

bool PrintDevice::
hasRasterCapability (long lMask)
{
   if ((lRasterCapabilities_d & lMask) != 0)
      return true;
   else
      return false;
}

bool PrintDevice::
hasDeviceOption (PSZCRO pszDeviceOption)
{
   return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

int PrintDevice::
getScanlineMultiple ()
{
   int iScanLineMultiple = pResolution_d->getScanlineMultiple ();

   if (0 == iScanLineMultiple)
   {
      iScanLineMultiple = 1;

#ifndef RETAIL
      if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__ << " (): Error: iScanLineMultiple == 0!" << std::endl;
#endif
   }

   return iScanLineMultiple;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

static int
parseSize (char **ppszComma)
{
   int iReturn = 0;

   while (  '0' <= **ppszComma
         && **ppszComma <= '9'
         )
   {
      iReturn = iReturn * 10 + (**ppszComma - '0');

      (*ppszComma)++;
   }

   return iReturn;
}

bool PrintDevice::
initializeJobProperties ()
{
   JobProperties  jobpropDevice (pszJobProperties_d);
   std::string   *pstringDeviceJPs   = 0;
   char          *pszError           = 0;
   bool           fAltered           = false;

   // Initialize to defaults.  This is done always since you need an
   // object to create another of the same class.
#ifdef INCLUDE_JP_UPDF_BOOKLET
   pBooklet_d = getDefaultBooklet ();

   // @TBD
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   pCopies_d = getDefaultCopies ();
   if (pCopies_d)
   {
      DeviceCopies *pCopies = pCopies_d->create (this, pszJobProperties_d);

      if (pCopies)
      {
         delete pCopies_d;

         pCopies_d = pCopies;
      }

      pstringDeviceJPs = pCopies_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

   // Allocate a copy of the dither id since it is freed
   PSZCRO pszDitherID = getDefaultDitherID ();

   if (pszDitherID)
   {
      pszDitherID_d = (char *)malloc (strlen (pszDitherID) + 1);
      if (pszDitherID_d)
         strcpy (pszDitherID_d, pszDitherID);
   }

   pForm_d = getDefaultForm ();
   if (pForm_d)
   {
      DeviceForm *pForm = pForm_d->create (this, pszJobProperties_d);

      if (pForm)
      {
         delete pForm_d;

         pForm_d = pForm;
      }

      pstringDeviceJPs = pForm_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   pJogging_d = getDefaultJogging ();

   // @TBD
#endif

   pMedia_d = getDefaultMedia ();
   if (pMedia_d)
   {
      DeviceMedia *pMedia = pMedia_d->create (this, pszJobProperties_d);

      if (pMedia)
      {
         delete pMedia_d;

         pMedia_d = pMedia;
      }

      pstringDeviceJPs = pMedia_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }

#ifdef INCLUDE_JP_COMMON_NUP
   pNUp_d = getDefaultNUp ();
   if (pNUp_d)
   {
      DeviceNUp *pNUp = pNUp_d->create (this, pszJobProperties_d);

      if (pNUp)
      {
         delete pNUp_d;

         pNUp_d = pNUp;
      }

      pstringDeviceJPs = pNUp_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

   pOrientation_d = getDefaultOrientation ();
   if (pOrientation_d)
   {
      DeviceOrientation *pOrientation = pOrientation_d->create (this, pszJobProperties_d);

      if (pOrientation)
      {
         delete pOrientation_d;

         pOrientation_d = pOrientation;
      }

      pstringDeviceJPs = pOrientation_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   pOutputBin_d = getDefaultOutputBin ();
   if (pOutputBin_d)
   {
      DeviceOutputBin *pOutputBin = pOutputBin_d->create (this, pszJobProperties_d);

      if (pOutputBin)
      {
         delete pOutputBin_d;

         pOutputBin_d = pOutputBin;
      }

      pstringDeviceJPs = pOutputBin_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

   pPrintMode_d = getDefaultPrintMode ();
   if (pPrintMode_d)
   {
      DevicePrintMode *pPrintMode = pPrintMode_d->create (this, pszJobProperties_d);

      if (pPrintMode)
      {
         delete pPrintMode_d;

         pPrintMode_d = pPrintMode;
      }

      pstringDeviceJPs = pPrintMode_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }

   pResolution_d = getDefaultResolution ();
   if (pResolution_d)
   {
      DeviceResolution *pResolution = pResolution_d->create (this, pszJobProperties_d);

      if (pResolution)
      {
         delete pResolution_d;

         pResolution_d = pResolution;
      }

      pstringDeviceJPs = pResolution_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   pScaling_d = getDefaultScaling ();
   if (pScaling_d)
   {
      DeviceScaling *pScaling = pScaling_d->create (this, pszJobProperties_d);

      if (pScaling)
      {
         delete pScaling_d;

         pScaling_d = pScaling;
      }

      pstringDeviceJPs = pScaling_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   pSheetCollate_d = getDefaultSheetCollate ();
   if (pSheetCollate_d)
   {
      DeviceSheetCollate *pSheetCollate = pSheetCollate_d->create (this, pszJobProperties_d);

      if (pSheetCollate)
      {
         delete pSheetCollate_d;

         pSheetCollate_d = pSheetCollate;
      }

      pstringDeviceJPs = pSheetCollate_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   pSide_d = getDefaultSide ();
   if (pSide_d)
   {
      DeviceSide *pSide = pSide_d->create (this, pszJobProperties_d);

      if (pSide)
      {
         delete pSide_d;

         pSide_d = pSide;
      }

      pstringDeviceJPs = pSide_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   pStitching_d = getDefaultStitching ();
   if (pStitching_d)
   {
      DeviceStitching *pStitching = pStitching_d->create (this, pszJobProperties_d);

      if (pStitching)
      {
         delete pStitching_d;

         pStitching_d = pStitching;
      }

      pstringDeviceJPs = pStitching_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

   pTray_d = getDefaultTray ();
   if (pTray_d)
   {
      DeviceTray *pTray = pTray_d->create (this, pszJobProperties_d);

      if (pTray)
      {
         delete pTray_d;

         pTray_d = pTray;
      }

      pstringDeviceJPs = pTray_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   pTrimming_d = getDefaultTrimming ();
   if (pTrimming_d)
   {
      DeviceTrimming *pTrimming = pTrimming_d->create (this, pszJobProperties_d);

      if (pTrimming)
      {
         delete pTrimming_d;

         pTrimming_d = pTrimming;
      }

      pstringDeviceJPs = pTrimming_d->getJobProperties (true);
      if (pstringDeviceJPs)
      {
         jobpropDevice.setJobProperties (pstringDeviceJPs->c_str ());

         delete pstringDeviceJPs;
      }
   }
#endif

   // These are not job properties but initialize them with the rest
   if (!pCommand_d)
      pCommand_d = getDefaultCommands ();
   if (!pData_d)
      pData_d = getDefaultData ();

   if (  pszJobProperties_d
      && *pszJobProperties_d
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__ << ": The job properties on input are: " << pszJobProperties_d << std::endl;
#endif

      DebugOutput::applyAllDebugOutput (pszJobProperties_d);

      if (pInstance_d)
      {
         PSZCRO pszDeviceJPs = jobpropDevice.getJobProperties ();

         if (pszDeviceJPs)
         {
            bool rc = pInstance_d->setJobProperties (pszDeviceJPs);

#ifndef RETAIL
            if (DebugOutput::shouldOutputPrintDevice ())
               DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__
                    << ": device returned "
                    << std::dec << rc << " from setJobProperties ()" << std::endl;
#endif

            if (!rc)
            {
               fAltered = true;
            }

            free ((void *)pszDeviceJPs);
         }
      }
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputPrintDevice ())
         DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__ << ": There are no job properties on input!" << std::endl;
#endif
   }

   if (  pForm_d
      && pResolution_d
      )
   {
      pForm_d->associateWith (pResolution_d);
   }

   if (pForm_d)
   {
      HardCopyCap *pHCC = pForm_d->getHardCopyCap ();

      if (pHCC)
      {
         if (  0 == pHCC->getCx ()
            || 0 == pHCC->getCy ()
            )
         {
            if (pForm_d != getDefaultForm ())
            {
               delete pForm_d;

               pszError = "Error: new form has 0 for its size.";

               pForm_d = getDefaultForm ();

               pForm_d->associateWith (pResolution_d);

               pHCC = pForm_d->getHardCopyCap ();
            }
         }
         if (  0 == pHCC->getCx ()
            || 0 == pHCC->getCy ()
            )
         {
            pszError = "Error: form has 0 for its size.";
         }
      }
   }

   if (pszError)
   {
      DebugOutput::getErrorStream () << *this << std::endl;
      DebugOutput::getErrorStream () << pszError << std::endl;
      // @TBD - how to handle and report errors?
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ())
   {
      DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__ << ": " << this << std::endl;
   }
#endif

   return fAltered;
}

#ifdef INCLUDE_JP_UPDF_BOOKLET
DeviceBooklet * PrintDevice::
getCurrentBooklet ()
{
   return pBooklet_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
DeviceCopies * PrintDevice::
getCurrentCopies ()
{
   return pCopies_d;
}
#endif

PSZCRO PrintDevice::
getCurrentDitherID ()
{
   return pszDitherID_d;
}

DeviceDither * PrintDevice::
getCurrentDither ()
{
   return pBlitter_d->getDitherInstance ();
}

DeviceForm * PrintDevice::
getCurrentForm ()
{
   return pForm_d;
}

#ifdef INCLUDE_JP_UPDF_JOGGING
DeviceJogging * PrintDevice::
getCurrentJogging ()
{
   return pJogging_d;
}
#endif

DeviceMedia * PrintDevice::
getCurrentMedia ()
{
   return pMedia_d;
}

#ifdef INCLUDE_JP_COMMON_NUP
DeviceNUp * PrintDevice::
getCurrentNUp ()
{
   return pNUp_d;
}
#endif

DeviceOrientation * PrintDevice::
getCurrentOrientation ()
{
   return pOrientation_d;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
DeviceOutputBin * PrintDevice::
getCurrentOutputBin ()
{
   return pOutputBin_d;
}
#endif

DevicePrintMode * PrintDevice::
getCurrentPrintMode ()
{
   return pPrintMode_d;
}

DeviceResolution * PrintDevice::
getCurrentResolution ()
{
   return pResolution_d;
}

#ifdef INCLUDE_JP_COMMON_SCALING
DeviceScaling * PrintDevice::
getCurrentScaling ()
{
   return pScaling_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
DeviceSheetCollate * PrintDevice::
getCurrentSheetCollate ()
{
   return pSheetCollate_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
DeviceSide * PrintDevice::
getCurrentSide ()
{
   return pSide_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
DeviceStitching * PrintDevice::
getCurrentStitching ()
{
   return pStitching_d;
}
#endif

DeviceTray * PrintDevice::
getCurrentTray ()
{
   return pTray_d;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING
DeviceTrimming * PrintDevice::
getCurrentTrimming ()
{
   return pTrimming_d;
}
#endif

DeviceInstance * PrintDevice::
getInstance ()
{
   return pInstance_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceCommand * PrintDevice::
getCommands ()
{
   return pCommand_d;
}

DeviceData * PrintDevice::
getDeviceData ()
{
   return pData_d;
}

DeviceString * PrintDevice::
getDeviceString ()
{
   return pString_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

int PrintDevice::
getPDLLevel ()
{
   if (pPDL_d)
      return pPDL_d->getPDLLevel ();
   else
      return 0;
}

int PrintDevice::
getPDLSubLevel ()
{
   if (pPDL_d)
      return pPDL_d->getPDLSubLevel ();
   else
      return 0;
}

int PrintDevice::
getPDLMajorRevisionLevel ()
{
   if (pPDL_d)
      return pPDL_d->getPDLMajorRevisionLevel ();
   else
      return 0;
}

int PrintDevice::
getPDLMinorRevisionLevel ()
{
   if (pPDL_d)
      return pPDL_d->getPDLMinorRevisionLevel ();
   else
      return 0;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool PrintDevice::
beginJob ()
{
   return pInstance_d->beginJob ();
}

bool PrintDevice::
beginJob (PSZCRO pszJobProperties)
{
   if (pszJobProperties)
   {
      setJobProperties (pszJobProperties);
   }

   return pInstance_d->beginJob (true);
}

bool PrintDevice::
newFrame ()
{
   return pInstance_d->newFrame ();
}

bool PrintDevice::
newFrame (PSZCRO pszJobProperties)
{
   if (pszJobProperties)
   {
      setJobProperties (pszJobProperties);
   }

   return pInstance_d->newFrame (true);
}

bool PrintDevice::
endJob ()
{
   return pInstance_d->endJob ();
}

bool PrintDevice::
abortJob ()
{
   return pInstance_d->abortJob ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool PrintDevice::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ())
      DebugOutput::getErrorStream ()
           << "PrintDevice::rasterize (0x"
           << std::hex << (int)pbBits << std::dec << ", {"
           << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   PSZCRO pszDumpEnvironmentVar = getenv ("OMNI_DUMP_INCOMING_BITMAPS");
   bool   fDumpIncomingBitmaps  = false;

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::" << __FUNCTION__ << ": (in)pszDumpEnvironmentVar = " << std::hex << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpIncomingBitmaps = true;
   }

   if (fDumpIncomingBitmaps)
   {
      static int  iNum = 0;
      char        achName[4 + 6 + 1];
      PNEUTRALRGB pColors = 0;

      sprintf (achName, "%04dIN.bmp", iNum);

      if (8 >= pbmi2->cBitCount)
      {
         int iNumColors = 1 << pbmi2->cBitCount;

         pColors = (PNEUTRALRGB)malloc (sizeof (NEUTRALRGB) * iNumColors);
         if (pColors)
         {
            PRGB2 pRGBs = pbmi2->argbColor;

            for (int i = 0; i < iNumColors; i++)
            {
               pColors[i].bRed   = pRGBs[i].bRed;
               pColors[i].bGreen = pRGBs[i].bGreen;
               pColors[i].bBlue  = pRGBs[i].bBlue;
            }
         }
      }

      Bitmap dump (achName,
                   pbmi2->cx,
                   pbmi2->cy,
                   pbmi2->cBitCount,
                   pColors);
      dump.addScanLine (pbBits, pbmi2->cy);

      if (pColors)
      {
         free (pColors);
      }

      iNum++;
      if (999 < iNum)
         iNum = 0;
   }

   return pBlitter_d->rasterize (pbBits,
                                 pbmi2,
                                 prectlPageLocation,
                                 eType);
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void PrintDevice::
setOutputStream (FILE *pFile)
{
   outputStreamBuf_d           = new stdio_filebuf (pFile);
   outputStream_d              = new std::ostream (outputStreamBuf_d);
   fShouldDeleteOutputStream_d = true;

   if (pInstance_d)
   {
      pInstance_d->setOutputStream (pFile);
   }
}

void PrintDevice::
setOutputFunction (PFNOUTPUTFUNCTION  pfnOutputFunction,
                   void              *pMagicCookie)
{
   pfnOutputFunction_d = pfnOutputFunction;
   pMagicCookie_d      = pMagicCookie;
}

void PrintDevice::
setErrorStream (FILE *pFile)
{
   if (pInstance_d)
   {
      pInstance_d->setErrorStream (pFile);
   }
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool PrintDevice::
setLanguage (int iLanguageID)
{
   if (iLanguageID != iLanguageID_d)
   {
      if (pInstance_d)
      {
         pInstance_d->setLanguage (iLanguageID_d);
      }

      if (pString_d)
      {
         pString_d->setLanguage (iLanguageID_d);
      }

      StringResource *pNewLanguage = StringResource::create (iLanguageID, pString_d);

      if (pNewLanguage)
      {
         delete pLanguage_d;

         iLanguageID_d = iLanguageID;
         pLanguage_d   = pNewLanguage;

         return true;
      }
      else
      {
         return false;
      }
   }

   return true;
}

int PrintDevice::
getLanguage ()
{
   return iLanguageID_d;
}

Enumeration * PrintDevice::
getLanguages ()
{
   return StringResource::getLanguages ();
}

StringResource * PrintDevice::
getLanguageResource ()
{
   return pLanguage_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool PrintDevice::
sendBinaryDataToDevice (DeviceForm *pForm)
{
   return sendBinaryDataToDevice (pForm->getData ());
}

bool PrintDevice::
sendBinaryDataToDevice (DeviceTray *pTray)
{
   return sendBinaryDataToDevice (pTray->getData ());
}

bool PrintDevice::
sendBinaryDataToDevice (DeviceMedia *pMedia)
{
   return sendBinaryDataToDevice (pMedia->getData ());
}

bool PrintDevice::
sendBinaryDataToDevice (DeviceResolution *pResolution)
{
   return sendBinaryDataToDevice (pResolution->getData ());
}

bool PrintDevice::
sendBinaryDataToDevice (BinaryData *pData)
{
   return sendBinaryDataToDevice (pData->getData (),
                                  pData->getLength ());
}

bool PrintDevice::
sendBinaryDataToDevice (PBYTE pbData, int iLength)
{
   if (pfnOutputFunction_d)
   {
      pfnOutputFunction_d (pMagicCookie_d, (unsigned char *)pbData, iLength);
   }
   else
   {
      outputStream_d->write ((const char *)pbData, iLength);
      outputStream_d->flush ();
   }

   return true;
}

bool PrintDevice::
sendVPrintfToDevice (BinaryData *pData,
                     va_list     list)
{
   char    achCmd[10];
   char    achOutput[512];
   int     iBytesWritten = 0;
   bool    bRC           = false;
   char   *pszCommand    = (char *)pData->getData ();
   int     iLength       = pData->getLength ();
   int     iSrc, iDst;

   for (iSrc = 0, iDst = 0; iSrc < iLength; iSrc++)
   {
      achOutput[iDst] =pszCommand[iSrc];

      if (  '%' == pszCommand[iSrc]
         && '%' != pszCommand[iSrc+1]
         )
      {
         short  sLen          = 0;
         int    iValueOnStack;
         double dValueOnStack;

         achCmd[0] = pszCommand[iSrc];
         achCmd[1] = pszCommand[iSrc + 1];
         achCmd[2] = 0;

         if (  'f' == achCmd[1]
            || 'F' == achCmd[1]
            )
         {
            iValueOnStack = 0;
            dValueOnStack = va_arg (list, double);
         }
         else
         {
            iValueOnStack = va_arg (list, int);
            dValueOnStack = 0.0;
         }

         if ('d' == achCmd[1])
         {
            /* Special format character -- double word, little endian
            */
            achOutput[iDst]     = (iValueOnStack & 0x000000FF);
            achOutput[iDst + 1] = (iValueOnStack & 0x0000FF00) >> 8;
            achOutput[iDst + 2] = (iValueOnStack & 0x00FF0000) >> 16;
            achOutput[iDst + 3] = (iValueOnStack & 0xFF000000) >> 24;
            sLen                = 4;
         }
         else if ('D' == achCmd[1])
         {
            /* Special format character -- double word, big endian
            */
            achOutput[iDst]     = (iValueOnStack & 0xFF000000) >> 24;
            achOutput[iDst + 1] = (iValueOnStack & 0x00FF0000) >> 16;
            achOutput[iDst + 2] = (iValueOnStack & 0x0000FF00) >> 8;
            achOutput[iDst + 3] = (iValueOnStack & 0x000000FF);
            sLen                = 4;
         }
         else if ('w' == achCmd[1])
         {
            /* Special format character -- single word, little endian
            */
            achOutput[iDst]     = (iValueOnStack & 0x00FF);
            achOutput[iDst + 1] = (iValueOnStack & 0xFF00) >> 8;
            sLen                = 2;
         }
         else if ('W' == achCmd[1])
         {
            /* Special format character -- single word, big endian
            */
            achOutput[iDst]     = (iValueOnStack & 0xFF00) >> 8;
            achOutput[iDst + 1] = (iValueOnStack & 0x00FF);
            sLen                = 2;
         }
         else if (  'c' == achCmd[1]
                 || 'C' == achCmd[1]
                 )
         {
            /* Special format character -- single byte
            ** Need to special case this because if you use %c and give it
            ** a 0, then strlen will return 0 instead of 1.
            */
            achOutput[iDst] = (iValueOnStack & 0xFF);
            sLen            = 1;
         }
         else if (  'n' == achCmd[1]
                 || 'N' == achCmd[1]
                 )
         {
            /* Special format character -- ASCII number string
            */
            sprintf (achOutput + iDst, "%d", iValueOnStack);
            sLen = strlen (achOutput + iDst);
         }
         else if (  'f' == achCmd[1]
                 || 'F' == achCmd[1]
                 )
         {
            /* Special format character -- ASCII number string with decimal
            */
            sprintf (achOutput + iDst, "%f", dValueOnStack);
            sLen = strlen (achOutput + iDst);
         }
         else
         {
            sprintf (achOutput + iDst,
                     achCmd,
                     iValueOnStack);

            if (iValueOnStack)
               sLen = strlen (achOutput + iDst);
            else
               sLen = 1;
         }

         iBytesWritten += sLen;
         iDst += sLen;
         iSrc++;
      }
      else if (  '%' == pszCommand[iSrc]
              && '%' == pszCommand[iSrc+1]
              )
      {
         achOutput[iDst] = '%';

         iBytesWritten++;
         iDst++;
         iSrc++;
      }
      else
      {
         iBytesWritten++;
         iDst++;
      }
   }

   if (pfnOutputFunction_d)
   {
      pfnOutputFunction_d (pMagicCookie_d, (unsigned char *)achOutput, iDst);
   }
   else
   {
      outputStream_d->write (achOutput, iDst);
      outputStream_d->flush ();
   }

   return bRC;
}

bool PrintDevice::
sendPrintfToDevice (BinaryData *pData, ...)
{
   va_list list;
   bool    rc;

   va_start (list, pData);

   rc = sendVPrintfToDevice (pData, list);

   va_end (list);

   return rc;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

std::string * PrintDevice::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;
   bool               fNeedSep = false;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pBooklet_d)
   {
      std::string *pstringRet = pBooklet_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   if (pCopies_d)
   {
      std::string *pstringRet = pCopies_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

   if (pszDitherID_d)
   {
      if (fNeedSep)
      {
         oss << " ";
      }

      oss << JOBPROP_DITHER << "=" << pszDitherID_d;

      fNeedSep = true;
   }

   if (pForm_d)
   {
      std::string *pstringRet = pForm_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pJogging_d)
   {
      std::string *pstringRet = pJogging_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

   if (pMedia_d)
   {
      std::string *pstringRet = pMedia_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }

#ifdef INCLUDE_JP_COMMON_NUP
   if (pNUp_d)
   {
      std::string *pstringRet = pNUp_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

   if (pOrientation_d)
   {
      std::string *pstringRet = pOrientation_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pOutputBin_d)
   {
      std::string *pstringRet = pOutputBin_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

   if (pPrintMode_d)
   {
      std::string *pstringRet = pPrintMode_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }

   if (pResolution_d)
   {
      std::string *pstringRet = pResolution_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   if (pScaling_d)
   {
      std::string *pstringRet = pScaling_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   if (pSide_d)
   {
      std::string *pstringRet = pSide_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pStitching_d)
   {
      std::string *pstringRet = pStitching_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pTrimming_d)
   {
      std::string *pstringRet = pTrimming_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }
#endif

   if (pTray_d)
   {
      std::string *pstringRet = pTray_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (fNeedSep)
         {
            oss << " ";
         }

         oss << *pstringRet;

         fNeedSep = true;

         delete pstringRet;
      }
   }

   if (pInstance_d)
   {
      std::string *pDeviceJobProp = pInstance_d->getJobProperties (fInDeviceSpecific);

      if (pDeviceJobProp)
      {
         if (  fNeedSep
            && 0 < pDeviceJobProp->length ()
            )
         {
            oss << " ";
         }

         oss << *pDeviceJobProp;

         fNeedSep = true;

         delete pDeviceJobProp;
      }
   }

   std::string *pRet = new std::string (oss.str ());

   return pRet;
}

bool PrintDevice::
setJobProperties (PSZCRO pszJobProperties)
{
   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      // Free the old ones
      cleanupProperties ();

      // Allocate the new one
      pszJobProperties_d = (char *)malloc (strlen (pszJobProperties) + 1);
      if (pszJobProperties_d)
      {
         strcpy (pszJobProperties_d, pszJobProperties);

         return !initializeJobProperties ();
      }
   }

   return false;
}

class JPEnumerator : public EnumEnumerator
{
public:
   JPEnumerator (bool fInDeviceSpecific) // @TBD
   {
   }

   virtual
   ~JPEnumerator ()
   {
   }
};

Enumeration * PrintDevice::
listJobProperties (bool fInDeviceSpecific)
{
   JPEnumerator *pRet = new JPEnumerator (fInDeviceSpecific);

#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pBooklet_d)
   {
      pRet->addElement (pBooklet_d->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   if (pCopies_d)
   {
      pRet->addElement (pCopies_d->getEnumeration (fInDeviceSpecific));
   }
#endif
   pRet->addElement (DeviceDither::getAllEnumeration ());
   if (pForm_d)
   {
      pRet->addElement (pForm_d->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pJogging_d)
   {
      pRet->addElement (pJogging_d->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (pMedia_d)
   {
      pRet->addElement (pMedia_d->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_NUP
   if (pNUp_d)
   {
      pRet->addElement (pNUp_d->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (pOrientation_d)
   {
      pRet->addElement (pOrientation_d->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pOutputBin_d)
   {
      pRet->addElement (pOutputBin_d->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (pPrintMode_d)
   {
      pRet->addElement (pPrintMode_d->getEnumeration (fInDeviceSpecific));
   }
   if (pResolution_d)
   {
      pRet->addElement (pResolution_d->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_SCALING
   if (pScaling_d)
   {
      pRet->addElement (pScaling_d->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pSheetCollate_d)
   {
      pRet->addElement (pSheetCollate_d->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   if (pSide_d)
   {
      pRet->addElement (pSide_d->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pStitching_d)
   {
      pRet->addElement (pStitching_d->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (pTray_d)
   {
      pRet->addElement (pTray_d->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pTrimming_d)
   {
      pRet->addElement (pTrimming_d->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (pInstance_d)
   {
      Enumeration *pEnum = pInstance_d->getGroupEnumeration (fInDeviceSpecific);

      while (pEnum->hasMoreElements ())
      {
         pRet->addElement ((Enumeration *)pEnum->nextElement ());
      }

      delete pEnum;
   }

   return pRet;
}

std::string * PrintDevice::
getJobPropertyType (PSZCRO pszKey)
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pBooklet_d->handlesKey (pszKey))
   {
      return pBooklet_d->getJobPropertyType (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   if (pCopies_d->handlesKey (pszKey))
   {
      return pCopies_d->getJobPropertyType (pszKey);
   }
#endif

   if (  pszDitherID_d
      && 0 == strcmp (pszKey, JOBPROP_DITHER)
      )
   {
      std::ostringstream oss;

      oss << "string " << pszDitherID_d;

      return new std::string (oss.str ());
   }

   if (pForm_d->handlesKey (pszKey))
   {
      return pForm_d->getJobPropertyType (pszKey);
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pJogging_d->handlesKey (pszKey))
   {
      return pJogging_d->getJobPropertyType (pszKey);
   }
#endif

   if (pMedia_d->handlesKey (pszKey))
   {
      return pMedia_d->getJobPropertyType (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_NUP
   if (pNUp_d->handlesKey (pszKey))
   {
      return pNUp_d->getJobPropertyType (pszKey);
   }
#endif

   if (pOrientation_d->handlesKey (pszKey))
   {
      return pOrientation_d->getJobPropertyType (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pOutputBin_d->handlesKey (pszKey))
   {
      return pOutputBin_d->getJobPropertyType (pszKey);
   }
#endif

   if (pPrintMode_d->handlesKey (pszKey))
   {
      return pPrintMode_d->getJobPropertyType (pszKey);
   }

   if (pResolution_d->handlesKey (pszKey))
   {
      return pResolution_d->getJobPropertyType (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   if (pScaling_d->handlesKey (pszKey))
   {
      return pScaling_d->getJobPropertyType (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pSheetCollate_d->handlesKey (pszKey))
   {
      return pSheetCollate_d->getJobPropertyType (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   if (pSide_d->handlesKey (pszKey))
   {
      return pSide_d->getJobPropertyType (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pStitching_d->handlesKey (pszKey))
   {
      return pStitching_d->getJobPropertyType (pszKey);
   }
#endif

   if (pTray_d->handlesKey (pszKey))
   {
      return pTray_d->getJobPropertyType (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pTrimming_d->handlesKey (pszKey))
   {
      return pTrimming_d->getJobPropertyType (pszKey);
   }
#endif

   if (pInstance_d)
   {
      return pInstance_d->getJobPropertyType (pszKey);
   }

   return 0;
}

std::string * PrintDevice::
getJobProperty (PSZCRO pszKey)
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pBooklet_d->handlesKey (pszKey))
   {
      return pBooklet_d->getJobProperty (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   if (pCopies_d->handlesKey (pszKey))
   {
      return pCopies_d->getJobProperty (pszKey);
   }
#endif

   if (  pszDitherID_d
      && 0 == strcmp (pszKey, JOBPROP_DITHER)
      )
   {
      return new std::string (pszDitherID_d);
   }

   if (pForm_d->handlesKey (pszKey))
   {
      return pForm_d->getJobProperty (pszKey);
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pJogging_d->handlesKey (pszKey))
   {
      return pJogging_d->getJobProperty (pszKey);
   }
#endif

   if (pMedia_d->handlesKey (pszKey))
   {
      return pMedia_d->getJobProperty (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_NUP
   if (pNUp_d->handlesKey (pszKey))
   {
      return pNUp_d->getJobProperty (pszKey);
   }
#endif

   if (pOrientation_d->handlesKey (pszKey))
   {
      return pOrientation_d->getJobProperty (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pOutputBin_d->handlesKey (pszKey))
   {
      return pOutputBin_d->getJobProperty (pszKey);
   }
#endif

   if (pPrintMode_d->handlesKey (pszKey))
   {
      return pPrintMode_d->getJobProperty (pszKey);
   }

   if (pResolution_d->handlesKey (pszKey))
   {
      return pResolution_d->getJobProperty (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   if (pScaling_d->handlesKey (pszKey))
   {
      return pScaling_d->getJobProperty (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pSheetCollate_d->handlesKey (pszKey))
   {
      return pSheetCollate_d->getJobProperty (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   if (pSide_d->handlesKey (pszKey))
   {
      return pSide_d->getJobProperty (pszKey);
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pStitching_d->handlesKey (pszKey))
   {
      return pStitching_d->getJobProperty (pszKey);
   }
#endif

   if (pTray_d->handlesKey (pszKey))
   {
      return pTray_d->getJobProperty (pszKey);
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pTrimming_d->handlesKey (pszKey))
   {
      return pTrimming_d->getJobProperty (pszKey);
   }
#endif

   if (pInstance_d)
   {
      return pInstance_d->getJobProperty (pszKey);
   }

   return 0;
}

std::string * PrintDevice::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pBooklet_d->handlesKey (pszKey))
   {
      return pBooklet_d->translateKeyValue (pszKey, pszValue);
   }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   if (pCopies_d->handlesKey (pszKey))
   {
      return pCopies_d->translateKeyValue (pszKey, pszValue);
   }
#endif

   if (  pszDitherID_d
      && 0 == strcmp (pszKey, JOBPROP_DITHER)
      )
   {
      PSZCRO       pszXLateKey   = StringResource::getString (getLanguageResource (),
                                                              StringResource::STRINGGROUP_DEVICE_COMMON,
                                                              StringResource::DEVICE_COMMON_DITHER);
      PSZRO        pszXLateValue = 0;
      std::string *pRet          = 0;

      if (pszValue)
      {
         pszXLateValue = StringResource::getString (getLanguageResource (),
                                                    StringResource::STRINGGROUP_DITHERS,
                                                    pszValue);
      }

      if (pszXLateKey)
      {
         pRet = new std::string (pszXLateKey);
      }

      if (  pszXLateValue
         && pRet
         )
      {
         *pRet += "=";
         *pRet += pszXLateValue;
      }

      return pRet;
   }

   if (pForm_d->handlesKey (pszKey))
   {
      return pForm_d->translateKeyValue (pszKey, pszValue);
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pJogging_d->handlesKey (pszKey))
   {
      return pJogging_d->translateKeyValue (pszKey, pszValue);
   }
#endif

   if (pMedia_d->handlesKey (pszKey))
   {
      return pMedia_d->translateKeyValue (pszKey, pszValue);
   }

#ifdef INCLUDE_JP_COMMON_NUP
   if (pNUp_d->handlesKey (pszKey))
   {
      return pNUp_d->translateKeyValue (pszKey, pszValue);
   }
#endif

   if (pOrientation_d->handlesKey (pszKey))
   {
      return pOrientation_d->translateKeyValue (pszKey, pszValue);
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pOutputBin_d->handlesKey (pszKey))
   {
      return pOutputBin_d->translateKeyValue (pszKey, pszValue);
   }
#endif

   if (pPrintMode_d->handlesKey (pszKey))
   {
      return pPrintMode_d->translateKeyValue (pszKey, pszValue);
   }

   if (pResolution_d->handlesKey (pszKey))
   {
      return pResolution_d->translateKeyValue (pszKey, pszValue);
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   if (pScaling_d->handlesKey (pszKey))
   {
      return pScaling_d->translateKeyValue (pszKey, pszValue);
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pSheetCollate_d->handlesKey (pszKey))
   {
      return pSheetCollate_d->translateKeyValue (pszKey, pszValue);
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   if (pSide_d->handlesKey (pszKey))
   {
      return pSide_d->translateKeyValue (pszKey, pszValue);
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pStitching_d->handlesKey (pszKey))
   {
      return pStitching_d->translateKeyValue (pszKey, pszValue);
   }
#endif

   if (pTray_d->handlesKey (pszKey))
   {
      return pTray_d->translateKeyValue (pszKey, pszValue);
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pTrimming_d->handlesKey (pszKey))
   {
      return pTrimming_d->translateKeyValue (pszKey, pszValue);
   }
#endif

   if (pInstance_d)
   {
      return pInstance_d->translateKeyValue (pszKey, pszValue);
   }

   return 0;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void PrintDevice::
loadLibrary (PSZCRO pszLibraryName)
{
   if (pszLoadedLibrary_d)
   {
      free (pszLoadedLibrary_d);
      pszLoadedLibrary_d = 0;
   }

   if (  !pszLibraryName
      || !*pszLibraryName
      )
      return;

   hModLibrary_d = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::loadLibrary: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hModLibrary_d << std::dec << std::endl;
#endif

   if (!hModLibrary_d)
      // Error!
      return;

   pszLoadedLibrary_d = (char *)malloc (strlen (pszLibraryName) + 1);
   if (pszLoadedLibrary_d)
   {
      strcpy (pszLoadedLibrary_d, pszLibraryName);
   }
   else
   {
      g_module_close (hModLibrary_d);
   }
}

void * PrintDevice::
dlsym (PSZCRO pszLibraryName,
       PSZCRO pszSymbol)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::dlsym (" << pszLibraryName << ", " << pszSymbol << ")" << std::endl;
#endif

   if (!pszLibraryName)
      // Nothing loaded
      return 0;

   if (0 != strcmp (pszLibraryName, pszLoadedLibrary_d))
      // Wrong library
      return 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::dlsym hModLibrary_d = 0x" << std::hex << (int)hModLibrary_d << std::dec << std::endl;
#endif

   if (!hModLibrary_d)
      // Error!
      return 0;

   gpointer ptr = NULL;
   bool     rc;

   rc = ::g_module_symbol (hModLibrary_d, pszSymbol, &ptr);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPrintDevice ()) DebugOutput::getErrorStream () << "PrintDevice::g_module_symbol returns " << rc << std::endl;
#endif

   return ptr;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void PrintDevice::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string PrintDevice::
toString (std::ostringstream& oss)
{
   oss << "{PrintDevice: \""
       << (pszDriverName_d ? pszDriverName_d : "")
       << "."
       << (pszDeviceName_d ? pszDeviceName_d : "")
       << "\" "
       << pszLibraryName_d
       << " lCapabilities_d = 0x" << std::hex << lCapabilities_d << ", "
       << "lRasterCapabilities_d = 0x" << lRasterCapabilities_d << std::dec << ", ";

   {
      std::ostringstream oss2;
      if (pInstance_d)
         oss << "pInstance_d = " << pInstance_d->toString (oss2) << ", ";
      else
         oss << "pInstance_d = (null), ";
   }

   {
      std::ostringstream oss2;
      if (pBlitter_d)
         oss << "pBlitter_d = " << pBlitter_d->toString (oss2) << ", ";
      else
         oss << "pBlitter_d = (null), ";
   }

#ifdef INCLUDE_JP_UPDF_BOOKLET
   {
      std::ostringstream oss2;

      if (pBooklet_d)
         oss << "pBooklet_d = " << pBooklet_d->toString (oss2) << ", ";
      else
         oss << "pBooklet_d = (null), ";
   }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   {
      std::ostringstream oss2;

      if (pCopies_d)
         oss << "pCopies_d = " << pCopies_d->toString (oss2) << ", ";
      else
         oss << "pCopies_d = (null), ";
   }
#endif

   if (pszDitherID_d)
      oss << "pszDitherID_d = " << pszDitherID_d << ", ";
   else
      oss << "pszDitherID_d = (null), ";

   {
      std::ostringstream oss2;
      if (pForm_d)
         oss << "pForm_d = " << pForm_d->toString (oss2) << ", ";
      else
         oss << "pForm_d = (null), ";
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   {
      std::ostringstream oss2;
      if (pJogging_d)
         oss << "pJogging_d = " << pJogging_d->toString (oss2) << ", ";
      else
         oss << "pJogging_d = (null), ";
   }
#endif

   {
      std::ostringstream oss2;
      if (pMedia_d)
         oss << "pMedia_d = " << pMedia_d->toString (oss2) << ", ";
      else
         oss << "pMedia_d = (null), ";
   }

#ifdef INCLUDE_JP_COMMON_NUP
   {
      std::ostringstream oss2;
      if (pNUp_d)
         oss << "pNUp_d = " << pNUp_d->toString (oss2) << ", ";
      else
         oss << "pNUp_d = (null), ";
   }
#endif

   {
      std::ostringstream oss2;

      if (pOrientation_d)
         oss << "pOrientation_d = " << pOrientation_d->toString (oss2) << ", ";
      else
         oss << "pOrientation_d = (null), ";
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   {
      std::ostringstream oss2;

      if (pOutputBin_d)
         oss << "pOutputBin_d = " << pOutputBin_d->toString (oss2) << ", ";
      else
         oss << "pOutputBin_d = (null), ";
   }
#endif

   {
      std::ostringstream oss2;
      if (pPrintMode_d)
         oss << "pPrintMode_d = " << pPrintMode_d->toString (oss2) << ", ";
      else
         oss << "pPrintMode_d = (null), ";
   }

   {
      std::ostringstream oss2;
      if (pResolution_d)
         oss << "pResolution_d = " << pResolution_d->toString (oss2) << ", ";
      else
         oss << "pResolution_d = (null), ";
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   {
      std::ostringstream oss2;
      if (pScaling_d)
         oss << "pScaling_d = " << pScaling_d->toString (oss2) << ", ";
      else
         oss << "pScaling_d = (null), ";
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   {
      std::ostringstream oss2;
      if (pSide_d)
         oss << "pSide_d = " << pSide_d->toString (oss2) << ", ";
      else
         oss << "pSide_d = (null), ";
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   {
      std::ostringstream oss2;
      if (pSheetCollate_d)
         oss << "pSheetCollate_d = " << pSheetCollate_d->toString (oss2) << ", ";
      else
         oss << "pSheetCollate_d = (null), ";
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   {
      std::ostringstream oss2;
      if (pStitching_d)
         oss << "pStitching_d = " << pStitching_d->toString (oss2) << ", ";
      else
         oss << "pStitching_d = (null), ";
   }
#endif

   {
      std::ostringstream oss2;
      if (pTray_d)
         oss << "pTray_d = " << pTray_d->toString (oss2) << ", ";
      else
         oss << "pTray_d = (null), ";
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   {
      std::ostringstream oss2;
      if (pTrimming_d)
         oss << "pTrimming_d = " << pTrimming_d->toString (oss2) << ", ";
      else
         oss << "pTrimming_d = (null), ";
   }
#endif

   if (pCommand_d)
      oss << "pCommand_d = " << pCommand_d << ", ";
   else
      oss << "pCommand_d = (null), ";

   {
      std::ostringstream oss2;
      if (pData_d)
         oss << "pData_d = " << pData_d->toString (oss2) << ", ";
      else
         oss << "pData_d = (null), ";
   }

   {
      std::ostringstream oss2;
      if (pString_d)
         oss << "pString_d = " << pString_d->toString (oss2);
      else
         oss << "pString_d = (null)";
   }

   oss << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const PrintDevice& const_self)
{
   PrintDevice&       self = const_cast<PrintDevice&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
