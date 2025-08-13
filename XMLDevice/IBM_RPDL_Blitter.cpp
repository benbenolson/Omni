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
#include "IBM_RPDL_Blitter.hpp"
#include "IBM_RPDL_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new IBM_RPDL_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

IBM_RPDL_Blitter::
IBM_RPDL_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fInstanceInitialized_d  = false;
   fGraphicsHaveBeenSent_d = false;
}

IBM_RPDL_Blitter::
~IBM_RPDL_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void IBM_RPDL_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   if (fInstanceInitialized_d)
      return;

   fInstanceInitialized_d = true;

   HardCopyCap      *pHCC = getCurrentForm ()->getHardCopyCap ();
   DeviceResolution *pDR  = getCurrentResolution ();
   DevicePrintMode  *pDPM = getCurrentPrintMode ();

   PSZRO       pszDitherID = getCurrentDitherID ();

   if (  DevicePrintMode::COLOR_TECH_CMY  == pDPM->getColorTech ()
      || DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ()
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

bool IBM_RPDL_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "IBM_RPDL_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   IBM_RPDL_Instance *pInstance = dynamic_cast <IBM_RPDL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return ibmMonoRasterize (pbBits,
                                 pbmi2,
                                 prectlPageLocation,
                                 eType);
      break;
   }

   case DevicePrintMode::COLOR_TECH_RGB:
   {
      return ibmColorRasterize (pbBits,
                                  pbmi2,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }
   }

   return true;
}

bool IBM_RPDL_Blitter::
ibmMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize IBM_RPDL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   IBM_RPDL_Instance *pInstance = dynamic_cast <IBM_RPDL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   PSZRO       pszDumpEnvironmentVar = getenv ("OMNI_DUMP_OUTGOING_BITMAPS");
   bool        fDumpOutgoingBitmaps  = false;
   static int  iNum                  = 0;
   char        achName[4 + 7 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize (out)pszDumpEnvironmentVar = " << std::hex << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   static BYTE       Mask[8]               = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   PBYTE             pbBuffer;
   int               iRemainder;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      ulPageSize = cyPage;
      iWorldY    = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      ulPageSize = cxPage;
      iWorldY    = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "IBM_RPDL_Blitter::" << __FUNCTION__ << "() ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::" << __FUNCTION__ << "() iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize iRemainder = " << iRemainder << std::endl;
#endif

   // If the black and white bits are reversed (0 being black and 1 being white)
   // then invert the data.
   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
   {
      for (int y = cy - 1; y >= 0; y--)
      {
         pbBuffer = pbBits + y * cbSourceBytesInBitmap;

         for (int x = 0; x < cbSourceBytesInBitmap; x++)
            pbBuffer[x] = ~pbBuffer[x];
      }
   }

   if (iRemainder > 0)
   {
      // Make sure that the extraneous bits are set to 0.
      for (int y = cy - 1; y >= 0; y--)
      {
         pbBuffer = pbBits + y * cbSourceBytesInBitmap;

         pbBuffer[cbDestBytesInPrinter - 1] &= Mask[iRemainder];
      }
   }

   // Find the rightmost pel of data.
   bool fContinue  = true;
   int  iRightmost = -1;

#if 0 // hongoh
   for (int x = cbSourceBytesInBitmap - 1; fContinue && x >= 0; x--)
#else
   for (int x = cbDestBytesInPrinter - 1; fContinue && x >= 0; x--)
#endif
   {
      for (int y = cy - 1; fContinue && y >= 0; y--)
      {
         if (*(pbBits + y * cbSourceBytesInBitmap + x))
         {
            iRightmost = x;
            fContinue  = false;
         }
      }
   }

   iRightmost++;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize iRightmost = " << iRightmost << std::endl;
#endif

   if (0 == iRightmost)
   {
      // This buffer is blank...
      if (fDumpOutgoingBitmaps)
      {
         outgoingBitmap.addScanLine (0, 0, 0, CMYKBitmap::BLACK);
      }

      return true;
   }

   int iSWidth, iSHeight;

   iSHeight = cy;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize cmdSetSourceRasterHeight = " << iSHeight << std::endl;
#endif

   iSWidth = iRightmost * 8;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmMonoRasterize cmdSetSourceRasterWidth = " << iSWidth << std::endl;
#endif

   // Text Clipping Command
   pCmd = pCommands->getCommandData ("cmdTextClipping");
   if (pCmd)
   {
      char aCmd[64];
      sendBinaryDataToDevice (pCmd);
      sprintf (aCmd, "%d,%d,%d,%d ",
                 prectlPageLocation->xLeft,
                 iWorldY,
                 iSWidth,
                 iSHeight);
      BinaryData cmd_d ((PBYTE)aCmd, strlen (aCmd));

      sendBinaryDataToDevice (&cmd_d);
   }

   // Send Image Command
   pCmd = pCommands->getCommandData ("cmdSendImage");
   if (pCmd)
   {
      char aCmd[64];
      sendBinaryDataToDevice (pCmd);
      sprintf (aCmd, "%d,%d,%d,%d,%d,%d,%d@",
                 3,
                 iSWidth,
                 iSHeight,
                 1,
                 0,
                 prectlPageLocation->xLeft,
                 iWorldY);
      BinaryData cmd_d ((PBYTE)aCmd, strlen (aCmd));

      sendBinaryDataToDevice (&cmd_d);
   }

   for (int i = 0; i < iNumScanLines; i++)
   {
      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

      if (fDumpOutgoingBitmaps)
      {
         outgoingBitmap.addScanLine (pbBits,
                                     1,
                                     ( cy
                                     - iScanLineY
                                     - 1
                                     ),
                                     CMYKBitmap::BLACK);
      }

      BinaryData data (pbBuffer, iRightmost);
      sendBinaryDataToDevice (&data);

      // Printing a scan line moves the printer down a scan line.  Update the
      // new current position.
      pInstance->ptlPrintHead_d.y = iWorldY + 1;

      // Move down a scan line
      iScanLineY--;
      iWorldY++;
   }

   return true;
}

bool IBM_RPDL_Blitter::
ibmColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_RPDL_Blitter::ibmColorRasterize IBM_RPDL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   IBM_RPDL_Instance *pInstance = dynamic_cast <IBM_RPDL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   return true;
}

#ifndef RETAIL

void IBM_RPDL_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string IBM_RPDL_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{IBM_RPDL_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const IBM_RPDL_Blitter& const_self)
{
   IBM_RPDL_Blitter& self = const_cast<IBM_RPDL_Blitter&>(const_self);
   std::ostringstream       oss;

   os << self.toString (oss);

   return os;
}
