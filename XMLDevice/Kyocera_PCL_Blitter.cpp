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
#include "Kyocera_PCL_Blitter.hpp"
#include "Kyocera_PCL_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new Kyocera_PCL_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

Kyocera_PCL_Blitter::
Kyocera_PCL_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fInstanceInitialized_d  = false;
   fGraphicsHaveBeenSent_d = false;
}

Kyocera_PCL_Blitter::
~Kyocera_PCL_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void Kyocera_PCL_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   if (fInstanceInitialized_d)
      return;

   fInstanceInitialized_d = true;

   HardCopyCap     *pHCC             = getCurrentForm ()->getHardCopyCap ();
   DevicePrintMode *pDPM             = getCurrentPrintMode ();
   int              iColorTech       = pDPM->getColorTech ();
   int              iNumDstRowBytes8 = 0;

   if (iColorTech == DevicePrintMode::COLOR_TECH_K)
   {
      iNumDstRowBytes8 = (pHCC->getXPels () + 7) >> 3;
   }
   else if (iColorTech == DevicePrintMode::COLOR_TECH_RGB)
   {
      iNumDstRowBytes8 = pHCC->getXPels () * 3;
   }

   setCompressionInstance (new GplCompression (iColorTech,
                                               GplCompression::GPLCOMPRESS_RLL
                                               | GplCompression::GPLCOMPRESS_TIFF
                                               | GplCompression::GPLCOMPRESS_DELTAROW,
                                               iNumDstRowBytes8,
                                               this));
}

bool Kyocera_PCL_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "Kyocera_PCL_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   Kyocera_PCL_Instance *pInstance = dynamic_cast <Kyocera_PCL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return hp_laserjetMonoRasterize (pbBits,
                                 pbmi2,
                                 prectlPageLocation,
                                 eType);
      break;
   }

   case DevicePrintMode::COLOR_TECH_RGB:
   {
      return hp_laserjetColorRasterize (pbBits,
                                  pbmi2,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }
   }

   return true;
}

void Kyocera_PCL_Blitter::
compressionChanged (int iNewCompression)
{
   DeviceCommand *pCommands = getCommands ();
   BinaryData    *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetCompression");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::compressionChanged cmdSetCompression = " << *pCmd << ", new compression is " << iNewCompression << std::endl;
#endif

      sendPrintfToDevice (pCmd, iNewCompression);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::compressionChanged Error: There is no cmdSetCompression defined for this device!" << std::endl;
#endif
   }
}

void Kyocera_PCL_Blitter::
sendData (int         iLength,
          BinaryData *pbdData,
          int         iWhichPlane)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::sendData ("
                          << iLength
                          << ", "
                          << std::hex << (int)pbdData << std::dec
                          << ", "
                          << iWhichPlane
                          << ")"
                          << std::endl;
#endif

   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdTransferRasterBlock");

   if (pCmd)
   {
      sendPrintfToDevice (pCmd, iLength);
      sendBinaryDataToDevice (pbdData);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::sendData Error: There is no cmdTransferRasterBlock or cmdTransferRasterPlane defined for this device!" << std::endl;
#endif
   }
}

bool Kyocera_PCL_Blitter::
hp_laserjetMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize Kyocera_PCL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Kyocera_PCL_Instance *pInstance = dynamic_cast <Kyocera_PCL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   PSZRO       pszDumpEnvironmentVar = getenv ("OMNI_DUMP_OUTGOING_BITMAPS");
   bool        fDumpOutgoingBitmaps  = false;
   static int  iNum                  = 0;
   char        achName[4 + 1 + 3 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize (out)pszDumpEnvironmentVar = " << std::hex << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   static BYTE       Mask[8]               = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx;
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
      int cyPage    = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      iWorldY       = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage    = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      iWorldY       = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize iRemainder = " << iRemainder << std::endl;
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

   for (int x = cbSourceBytesInBitmap - 1; fContinue && x >= 0; x--)
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
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize iRightmost = " << iRightmost << std::endl;
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

   moveToYPosition (iWorldY, false);

   int iSWidth, iSHeight;

   pCmd = pCommands->getCommandData ("cmdSetSourceRasterHeight");

   iSHeight = cy;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize cmdSetSourceRasterHeight = " << iSHeight << std::endl;
#endif

   sendPrintfToDevice (pCmd, iSHeight);

   pCmd = pCommands->getCommandData ("cmdSetSourceRasterWidth");

   iSWidth = iRightmost * 8;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize cmdSetSourceRasterWidth = " << iSWidth << std::endl;
#endif

   sendPrintfToDevice (pCmd, iSWidth);

   int iMode = 1;

   if (  pInstance->iXScalingFactor_d != 1
      || pInstance->iYScalingFactor_d != 1
      )
   {
      iMode = 3;
   }

   if (iMode == 3)
   {
      DeviceResolution *pDR       = getCurrentResolution ();
      double            dDWidth,
                        dDHeight;

      pCmd = pCommands->getCommandData ("cmdSetDestinationRasterHeight");

      dDHeight = cy;
      // Convert from pels to decipoints
      dDHeight *= 720.0 / pDR->getYRes ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize cmdSetDestinationRasterHeight = " << dDHeight << std::endl;
#endif

      sendPrintfToDevice (pCmd, dDHeight);

      pCmd = pCommands->getCommandData ("cmdSetDestinationRasterWidth");

      dDWidth = iSWidth;
      // Convert from pels to decipoints
      dDWidth *= 720.0 / pDR->getXRes ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize cmdSetDestinationRasterWidth = " << dDWidth << std::endl;
#endif

      sendPrintfToDevice (pCmd, dDWidth);

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize pDR->getXRes () = " << pDR->getXRes () << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize pDR->getYRes () = " << pDR->getYRes () << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize pDR->getExternalXRes () = " << pDR->getExternalXRes () << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetMonoRasterize pDR->getExternalYRes () = " << pDR->getExternalYRes () << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdBeginRasterGraphics");
   sendPrintfToDevice (pCmd, iMode);

   pCmd = pCommands->getCommandData ("cmdTransferRasterBlock");

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

      compressKRasterPlane (&data);
      // Note: Kyocera_PCL_Blitter::sendData () is called because of the
      //       compression!  Also, Kyocera_PCL_Blitter::compressionChanged ()
      //       can be called.

      // Printing a scan line moves the printer down a scan line.  Update the
      // new current position.
      pInstance->ptlPrintHead_d.y = iWorldY + 1;

      // Move down a scan line
      iScanLineY--;
      iWorldY++;
   }

   pCmd = pCommands->getCommandData ("cmdEndRasterGraphics");
   sendBinaryDataToDevice (pCmd);

   // Ending the raster graphics block will force a start raster graphics
   // block which will force a zeroing out of the seed line.  Hence, make
   // the compression instance start from scratch.
   resetCompressionMode ();

   return true;
}

bool Kyocera_PCL_Blitter::
hp_laserjetColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize Kyocera_PCL_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Kyocera_PCL_Instance *pInstance = dynamic_cast <Kyocera_PCL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   PSZRO       pszDumpEnvironmentVar = getenv ("OMNI_DUMP_OUTGOING_BITMAPS");
   bool        fDumpOutgoingBitmaps  = false;
   static int  iNum                  = 0;
   char        achName[4 + 1 + 3 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize (out)pszDumpEnvironmentVar = " << std::hex << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   PBYTE             pbBuffer;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage    = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      iWorldY       = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage    = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      iWorldY       = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = pbmi2->cx * 3;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   // Find the rightmost pel of data.
   bool fSetRightmost = true;
   int  iRightmost    = -1;

   for (int x = cx - 1; x >= 0; x--)
   {
      for (iScanLineY = 0; iScanLineY < cy; iScanLineY++)
      {
         pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap + x * 3;

         byte b = *(pbBuffer);
         byte r = *(pbBuffer + 2);

         if (fSetRightmost)
         {
            if (  b               != 255
               || *(pbBuffer + 1) != 255
               || r               != 255
               )
            {
               fSetRightmost = false;
               iRightmost    = x;
            }
         }

         *pbBuffer       = r;
         *(pbBuffer + 2) = b;
      }
   }

   iRightmost++;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize iRightmost = " << iRightmost << std::endl;
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

   iScanLineY = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize iScanLineY = " << iScanLineY << std::endl;
#endif

   moveToYPosition (iWorldY, false);

   int iSWidth, iSHeight;

   pCmd = pCommands->getCommandData ("cmdSetSourceRasterHeight");

   iSHeight = cy;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize cmdSetSourceRasterHeight = " << iSHeight << std::endl;
#endif

   sendPrintfToDevice (pCmd, iSHeight);

   pCmd = pCommands->getCommandData ("cmdSetSourceRasterWidth");

   iSWidth = iRightmost * 3;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize cmdSetSourceRasterWidth = " << iSWidth << std::endl;
#endif

   sendPrintfToDevice (pCmd, iSWidth);

   int iMode = 1;

   if (  pInstance->iXScalingFactor_d != 1
      || pInstance->iYScalingFactor_d != 1
      )
   {
      iMode = 3;
   }

   if (iMode == 3)
   {
      DeviceResolution *pDR       = getCurrentResolution ();
      double            dDWidth,
                        dDHeight;

      pCmd = pCommands->getCommandData ("cmdSetDestinationRasterHeight");

      dDHeight = cy;
      // Convert from pels to decipoints
      dDHeight *= 720.0 / pDR->getYRes ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize cmdSetDestinationRasterHeight = " << dDHeight << std::endl;
#endif

      sendPrintfToDevice (pCmd, dDHeight);

      pCmd = pCommands->getCommandData ("cmdSetDestinationRasterWidth");

      dDWidth = iSWidth;
      // Convert from pels to decipoints
      dDWidth *= 720.0 / pDR->getXRes ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize cmdSetDestinationRasterWidth = " << dDWidth << std::endl;
#endif

      sendPrintfToDevice (pCmd, dDWidth);

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize pDR->getXRes () = " << pDR->getXRes () << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize pDR->getYRes () = " << pDR->getYRes () << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize pDR->getExternalXRes () = " << pDR->getExternalXRes () << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::hp_laserjetColorRasterize pDR->getExternalYRes () = " << pDR->getExternalYRes () << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdBeginRasterGraphics");
   sendPrintfToDevice (pCmd, iMode);

   pCmd = pCommands->getCommandData ("cmdTransferRasterBlock");

   for (int i = 0; i < iNumScanLines; i++)
   {
      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

      BinaryData data (pbBuffer, iRightmost);

      compressRGBRasterPlane (&data);
      // Note: Kyocera_PCL_Blitter::sendData () is called because of the
      //       compression!  Also, Kyocera_PCL_Blitter::compressionChanged ()
      //       can be called.

      // Printing a scan line moves the printer down a scan line.  Update the
      // new current position.
      pInstance->ptlPrintHead_d.y = iWorldY + 1;

      // Move down a scan line
      iScanLineY--;
      iWorldY++;
   }

   pCmd = pCommands->getCommandData ("cmdEndRasterGraphics");
   sendBinaryDataToDevice (pCmd);

   // Ending the raster graphics block will force a start raster graphics
   // block which will force a zeroing out of the seed line.  Hence, make
   // the compression instance start from scratch.
   resetCompressionMode ();

   return true;
}

/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool Kyocera_PCL_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   Kyocera_PCL_Instance *pInstance = dynamic_cast <Kyocera_PCL_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   DeviceCommand    *pCommands   = getCommands ();
   BinaryData       *pCmd        = 0;
   int               iAmount;

   if (fAbsolute)
   {
      iAmount = iWorldY;
   }
   else
   {
      iAmount = iWorldY - pInstance->ptlPrintHead_d.y;
   }

   if (0 == iAmount)
   {
      // No where to move!
      return true;
   }

   // Set Absolute Vertical Print Position
   pCmd = pCommands->getCommandData ("cmdSetYPos");
   if (!pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::moveToYPosition: There is no cmdSetYPos!" << std::endl;
#endif

      return false;
   }

   iAmount *= pInstance->iYScalingFactor_d;
   iAmount += pInstance->iVerticalOffset_d;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Kyocera_PCL_Blitter::moveToYPosition setting position to " << iAmount << std::endl;
#endif

   sendPrintfToDevice (pCmd, iAmount);

   return true;
}

#ifndef RETAIL

void Kyocera_PCL_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Kyocera_PCL_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{Kyocera_PCL_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Kyocera_PCL_Blitter& const_self)
{
   Kyocera_PCL_Blitter& self = const_cast<Kyocera_PCL_Blitter&>(const_self);
   std::ostringstream       oss;

   os << self.toString (oss);

   return os;
}
