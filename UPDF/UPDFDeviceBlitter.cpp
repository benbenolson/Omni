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
#include "UPDFDeviceBlitter.hpp"
#include "UPDFDeviceInstance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

UPDFDeviceBlitter::
UPDFDeviceBlitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "UPDFDeviceBlitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d      = false;
   fGraphicsHaveBeenSent_d = false;
}

UPDFDeviceBlitter::
~UPDFDeviceBlitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "UPDFDeviceBlitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "UPDFDeviceBlitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void UPDFDeviceBlitter::
initializeInstance ()
{
   HardCopyCap      *pHCC        = getCurrentForm ()->getHardCopyCap ();
   DeviceResolution *pDR         = getCurrentResolution ();
   DevicePrintMode  *pDPM        = getCurrentPrintMode ();
   PSZCRO            pszDitherID = getCurrentDitherID ();

   if (  pDPM
      && (  DevicePrintMode::COLOR_TECH_CMY  == pDPM->getColorTech ()
         || DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ()
         )
      )
   {
      int  iNumDstRowBytes8      = (pHCC->getXPels () + 7) >> 3;
      char achDitherOptions[512]; // @TBD

      sprintf (achDitherOptions,
               "fDataInRGB=true "
               "iBlackReduction=%d "
               "iColorTech=%d "
               "iNumDitherRows=%d "
               "iSrcRowPels=%d "
               "iNumDestRowBytes=%d "
               "iDestBitsPerPel=%d",
               /* @TBD iBlackReduction*/0,
               pDPM->getColorTech (),
               pDR->getScanlineMultiple (),
               pHCC->getXPels (),
               iNumDstRowBytes8,
               pDR->getDstBitsPerPel ());

      setDitherInstance (DeviceDither::createDitherInstance (pszDitherID,                 // iDitherType
                                                             pDevice_d,                   // pDevice
                                                             achDitherOptions));          // pszOptions
   }
}

bool UPDFDeviceBlitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "UPDFDeviceBlitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   UPDFDeviceInstance *pInstance = dynamic_cast <UPDFDeviceInstance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (pDevice_d->getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return updfMonoRasterize (pbBits,
                                 pbmi2,
                                 prectlPageLocation,
                                 eType);
      break;
   }

   case DevicePrintMode::COLOR_TECH_CMYK:
   case DevicePrintMode::COLOR_TECH_CMY:
   {
      return updfColorRasterize (pbBits,
                                  pbmi2,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "UPDFDeviceBlitter::rasterize Error: unknown color tech " << pDevice_d->getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return true;
}

bool UPDFDeviceBlitter::
updfMonoRasterize (PBYTE        pbBits,
                   PBITMAPINFO2 pbmi2,
                   PRECTL       prectlPageLocation,
                   BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "UPDFDeviceBlitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   UPDFDeviceInstance *pInstance = dynamic_cast <UPDFDeviceInstance *>(getInstance ());
   if (!pInstance)
      return false;

   char      *pszDumpEnvironmentVar = getenv ("DUMP_OUTGOING_BITMAPS");
   bool       fDumpOutgoingBitmaps  = false;
   static int iNum = 0;
   char       achName[4 + 1 + 3 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "UPDFP2_Blitter::updfMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   pInstance->executeEvent ("Event_RasterGraphicBand", true);

   return true;
}

bool UPDFDeviceBlitter::
updfColorRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "UPDFDeviceBlitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   UPDFDeviceInstance *pInstance = dynamic_cast <UPDFDeviceInstance *>(getInstance ());
   if (!pInstance)
      return false;

   char      *pszDumpEnvironmentVar = getenv ("DUMP_OUTGOING_BITMAPS");
   bool       fDumpOutgoingBitmaps  = false;
   static int iNum = 0;
   char       achName[4 + 1 + 3 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "UPDFDeviceBlitter::updfColorRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   return true;
}

/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool UPDFDeviceBlitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   UPDFDeviceInstance *pInstance = dynamic_cast <UPDFDeviceInstance *>(getInstance ());
   if (!pInstance)
      return false;

   return true;
}

std::string UPDFDeviceBlitter::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceBlitter: "
       << DeviceBlitter::toString (oss2) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceBlitter& const_self)
{
   UPDFDeviceBlitter& self = const_cast<UPDFDeviceBlitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
