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
#include "Star_ESCP2_Blitter.hpp"
#include "Star_ESCP2_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

const static bool fTestNoCompression = false;

int compressEpsonRLE (PBYTE  pbData,
                      int    cBytesInData,
                      PBYTE  pbReturn,
                      int    cBytesInReturn);

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new Star_ESCP2_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

Star_ESCP2_Blitter::
Star_ESCP2_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fInstanceInitialized_d  = false;
   fGraphicsHaveBeenSent_d = false;
   cbCompress_d            = 0;
   pbCompress_d            = 0;
}

Star_ESCP2_Blitter::
~Star_ESCP2_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   if (pbCompress_d)
   {
      free (pbCompress_d);
      pbCompress_d = 0;
      cbCompress_d = 0;
   }

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void Star_ESCP2_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   if (fInstanceInitialized_d)
      return;

   fInstanceInitialized_d = true;

   HardCopyCap *pHCC             = getCurrentForm ()->getHardCopyCap ();
   int          iNumDstRowBytes8 = (pHCC->getXPels () + 7) >> 3;

   cbCompress_d = iNumDstRowBytes8 * 24;
   cbCompress_d += cbCompress_d / 20;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::initializeInstance () cbCompress_d = " << cbCompress_d << std::endl;
#endif

   pbCompress_d = (PBYTE)malloc (cbCompress_d);

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Star_ESCP2_Blitter::initializeInstance () pbCompress_d = " << (int)pbCompress_d << std::endl;
#endif

   DeviceResolution *pDR         = getCurrentResolution ();
   DevicePrintMode  *pDPM        = getCurrentPrintMode ();
   PSZRO             pszDitherID = getCurrentDitherID ();

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

bool Star_ESCP2_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "Star_ESCP2_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   Star_ESCP2_Instance *pInstance = dynamic_cast <Star_ESCP2_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return starMonoRasterize (pbBits,
                                 pbmi2,
                                 prectlPageLocation,
                                 eType);
      break;
   }

   case DevicePrintMode::COLOR_TECH_CMYK:
   case DevicePrintMode::COLOR_TECH_CMY:
   {
      return starColorRasterize (pbBits,
                                  pbmi2,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return true;
}

bool Star_ESCP2_Blitter::
starMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize Star_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Star_ESCP2_Instance *pInstance = dynamic_cast <Star_ESCP2_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Star_ESCP2_Blitter::starMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DeviceResolution *pDR                   = getCurrentResolution ();
   static int        iBandSizes[]          = {24, 8, 1, 0};
   static BYTE       Mask[8]               = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iBandSizeIdx          = 0,
                     iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   bool              bDirty;
   PBYTE             pbBuffer;
   int               iCompressed;
   int               iRemainder;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;
   register int      sl;

   if (  pInstance->fUseMicroweave_d
      || 360 < pDR->getYRes ()
      )
   {
      /* Micro weaving has been turned on!  We must set a band size of 1! */
      iBandSizeIdx = 2;
   }

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
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Star_ESCP2_Blitter::starMonoRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize iRemainder = " << iRemainder << std::endl;
#endif

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

   while (iNumScanLines)
   {
      while (iNumScanLines >= iBandSizes[iBandSizeIdx])
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize iScanLineY = " << iScanLineY << ", indexing to " << (iScanLineY-iBandSizes[iBandSizeIdx]+1)*cbSourceBytesInBitmap << " for " << iBandSizes[iBandSizeIdx] << " bands" << std::endl;
#endif

         pbBuffer = pbBits + ( iScanLineY
                             - iBandSizes[iBandSizeIdx]
                             + 1
                             ) * cbSourceBytesInBitmap;

         if (fBlackWhiteReversed)
         {
            for (int x = 0; x < cbSourceBytesInBitmap * iBandSizes[iBandSizeIdx]; x++)
               pbBuffer[x] = ~pbBuffer[x];
         }

         /* See if this block of raster has some bits set in it!
         */
         bDirty = false;
         for (sl = 0; sl < iBandSizes[iBandSizeIdx] && !bDirty; sl++)
         {
            register int x;

            // Check all but the last byte
            for (x = 0; x <= cbDestBytesInPrinter - 2 && !bDirty; x++)
               bDirty |= pbBits[x + (iScanLineY - sl)*cbSourceBytesInBitmap];
            // Check the remaining bits
            bDirty |= pbBits[x + (iScanLineY - sl)*cbSourceBytesInBitmap] & Mask[iRemainder];
         }

         if (bDirty)
         {
            if (!fGraphicsHaveBeenSent_d)
            {
               pCmd = pCommands->getCommandData ("cmdSetColor");

               // Start with black ink (default)
               sendPrintfToDevice (pCmd, 0);

               fGraphicsHaveBeenSent_d = true;
            }

            // Set Absolute Vertical Print Position
            pCmd = pCommands->getCommandData ("cmdSetYPos");

#ifndef RETAIL
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starMonoRasterize setting position to " << iWorldY << std::endl;
#endif

            sendPrintfToDevice (pCmd, iWorldY);

            if (fDumpOutgoingBitmaps)
            {
               outgoingBitmap.addScanLine (pbBits,
                                           iBandSizes[iBandSizeIdx],
                                           ( cy
                                           - iScanLineY
                                           + iBandSizes[iBandSizeIdx]
                                           - 2
                                           ),
                                           CMYKBitmap::BLACK);
            }

            // Send raster transfer header
            pCmd = pDR->getData ();
            sendPrintfToDevice (pCmd,
                                !fTestNoCompression,        // Compression
                                3600/pDR->getYRes (),       // V density
                                3600/pDR->getXRes (),       // H density
                                iBandSizes[iBandSizeIdx],   // V dot count
                                cx);                        // Count of raster data

            for (sl = 0; sl < iBandSizes[iBandSizeIdx]; sl++)
            {
               pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

               // Make sure that the extraneous bits are set to 0.
               pbBuffer[cbDestBytesInPrinter-1] &= Mask[iRemainder];

               if (!fTestNoCompression)
               {
                  iCompressed = compressEpsonRLE (pbBuffer,
                                                  cbDestBytesInPrinter,
                                                  pbCompress_d,
                                                  cbCompress_d);

                  BinaryData data (pbCompress_d, iCompressed);
                  sendBinaryDataToDevice (&data);
               }
               else
               {
                  BinaryData data (pbBuffer, cbDestBytesInPrinter);
                  sendBinaryDataToDevice (&data);
               }

               // Move down a scan line
               iScanLineY--;
               iWorldY++;
            }

            // End the raster line
            pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
            sendBinaryDataToDevice (pCmd);
         }
         else
         {
            iWorldY    += iBandSizes[iBandSizeIdx];
            iScanLineY -= iBandSizes[iBandSizeIdx];
         }

         // Done with a block of scan lines
         iNumScanLines -= iBandSizes[iBandSizeIdx];
      }

      // Move to the next scan line block size
      iBandSizeIdx++;
   }

   return true;
}

bool Star_ESCP2_Blitter::
starColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Star_ESCP2_Instance *pInstance = dynamic_cast <Star_ESCP2_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Star_ESCP2_Blitter::starColorRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DeviceResolution *pDR                   = getCurrentResolution ();
   static int        iBandSizes[]          = { 24, 8, 1, 0 };
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iBandSizeIdx          = 0,
                     iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   int               saveCy;
   int               iMaxPass              = 4,
                     iLastColor            = -1;   /* Not a valid color to start with */
   PBYTE             pbBuffer;
   int               iCompressed;
   BinaryData       *pbdColorBits          = 0;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;
   register int      sl;

   if (  pInstance->fUseMicroweave_d
      || 360 < pDR->getYRes ()
      )
   {
      /* Micro weaving has been turned on!  We must set a band size of 1! */
      iBandSizeIdx = 2;
   }

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage    = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      ulPageSize    = cyPage;
      iWorldY       = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = std::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      int cxPage    = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      ulPageSize    = cxPage;
      iWorldY       = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Star_ESCP2_Blitter::starColorRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   saveCy = pbmi2->cy;

   // @TBD - need memory calculations for amount to send to the printer in one
   //        command.
   iBandSizeIdx = 2;
   while (iNumScanLines)
   {
      while (iNumScanLines >= iBandSizes[iBandSizeIdx])
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize indexing to " << (iScanLineY - iBandSizes[iBandSizeIdx] + 1) * cbSourceBytesInBitmap << std::endl;
#endif

         pbBuffer = pbBits + ( iScanLineY
                             - iBandSizes[iBandSizeIdx]
                             + 1
                             ) * cbSourceBytesInBitmap;

         pbmi2->cy = iBandSizes[iBandSizeIdx];
         ditherRGBtoCMYK (pbmi2, pbBuffer);

         if (ditherAllPlanesBlank ())
         {
            incrementBlankLineCount (iBandSizes[iBandSizeIdx]);
         }
         else
         {
            // Set Absolute Vertical Print Position
            pCmd = pCommands->getCommandData ("cmdSetYPos");

#ifndef RETAIL
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Star_ESCP2_Blitter::starColorRasterize setting position to " << iWorldY << std::endl;
#endif

            sendPrintfToDevice (pCmd, iWorldY);

            /* Loop through the four color passes/planes:
            **     YELLOW .. MAGENTA .. CYAN .. BLACK
            */
            for (int iPass = 0; iPass < iMaxPass; iPass++)
            {
               pCmd = pCommands->getCommandData ("cmdSetColor");

               switch (iPass)
               {
               case 0: /* YELLOW */
               {
                  if (ditherYPlaneBlank ())
                     continue;

                  pbdColorBits = getYPlane ();

                  // Need to change color modes?
                  if (iLastColor != iPass)
                     sendPrintfToDevice (pCmd, 4);
                  break;
               }

               case 1: /* MAGENTA */
               {
                  if (ditherMPlaneBlank ())
                     continue;

                  pbdColorBits = getMPlane ();

                  // Need to change color modes?
                  if (iLastColor != iPass)
                     sendPrintfToDevice (pCmd, 1);
                  break;
               }

               case 2: /* CYAN */
               {
                  if (ditherCPlaneBlank ())
                     continue;

                  pbdColorBits = getCPlane ();

                  // Need to change color modes?
                  if (iLastColor != iPass)
                     sendPrintfToDevice (pCmd, 2);
                  break;
               }

               case 3: /* BLACK */
               {
                  if (ditherKPlaneBlank ())
                     continue;

                  pbdColorBits = getKPlane ();

                  // Need to change color modes?
                  if (iLastColor != iPass)
                     sendPrintfToDevice (pCmd, 0);
                  break;
               }
               }

               iLastColor = iPass;

               if (fDumpOutgoingBitmaps)
               {
                  CMYKBitmap::PLANE eWhichPlane = CMYKBitmap::BLACK;

                  switch (iPass)
                  {
                  case 0: /* YELLOW */  eWhichPlane = CMYKBitmap::YELLOW;  break;
                  case 1: /* MAGENTA */ eWhichPlane = CMYKBitmap::MAGENTA; break;
                  case 2: /* CYAN */    eWhichPlane = CMYKBitmap::CYAN;    break;
                  case 3: /* BLACK */   eWhichPlane = CMYKBitmap::BLACK;   break;
                  }

                  outgoingBitmap.addScanLine (pbdColorBits->getData (),
                                              iBandSizes[iBandSizeIdx],
                                              ( cy
                                              - iScanLineY
                                              + iBandSizes[iBandSizeIdx]
                                              - 2
                                              ),
                                              eWhichPlane);
               }

               // Send raster transfer header
               pCmd = pDR->getData ();
               sendPrintfToDevice (pCmd,
                                   !fTestNoCompression,        // Compression
                                   3600/pDR->getYRes (),       // V density
                                   3600/pDR->getXRes (),       // H density
                                   iBandSizes[iBandSizeIdx],   // V dot count
                                   cx);                        // Count of raster data

               for (sl = 0; sl < iBandSizes[iBandSizeIdx]; sl++)
               {
                  pbBuffer = pbdColorBits->getData () + sl * cbSourceBytesInBitmap;

                  if (!fTestNoCompression)
                  {
                     iCompressed = compressEpsonRLE (pbBuffer,
                                                     cbDestBytesInPrinter,
                                                     pbCompress_d,
                                                     cbCompress_d);

                     BinaryData data (pbCompress_d, iCompressed);
                     sendBinaryDataToDevice (&data);
                  }
                  else
                  {
                     BinaryData data (pbBuffer, cbDestBytesInPrinter);
                     sendBinaryDataToDevice (&data);
                  }
               }

               // End the raster line
               pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
               sendBinaryDataToDevice (pCmd);
            }
         }

         // Done with a block of scan lines
         iNumScanLines -= iBandSizes[iBandSizeIdx];
         iScanLineY    -= iBandSizes[iBandSizeIdx];
         iWorldY       += iBandSizes[iBandSizeIdx];
      }

      // Move to the next scan line block size
      iBandSizeIdx++;
   }

   pbmi2->cy = saveCy;

   return true;
}

#ifndef RETAIL

void Star_ESCP2_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Star_ESCP2_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{Star_ESCP2_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Star_ESCP2_Blitter& const_self)
{
   Star_ESCP2_Blitter& self = const_cast<Star_ESCP2_Blitter&>(const_self);
   std::ostringstream   oss;

   os << self.toString (oss);

   return os;
}

// @TBD Move to GplCompress
/****************************************************************************/
/* PROCEDURE NAME : CompressEpsonRLE                                        */
/* AUTHOR         : Mark Hamzy                                              */
/* DATE WRITTEN   : 4/10/94                                                 */
/* DESCRIPTION    :                                                         */
/*                  Data is organized as counter bytes followed by          */
/*                  data bytes.  Two types of counter bytes can be          */
/*                  used: repeat counters and data-length counters.         */
/*                                                                          */
/*                  repeat counters - specify the number of times (minus 1) */
/*                                    to repeat the following single byte   */
/*                                    of data.                              */
/*                                    -1 <= repeat counter <= -127          */
/*                                                                          */
/*                  data-length counters - specify the number of            */
/*                                    bytes (minus 1) of print data         */
/*                                    data following the counter.           */
/*                                    This data is only printed once.       */
/*                                                                          */
/*                           0 <= data-length counter <= 127                */
/*                                                                          */
/*     The first byte of compressed data must be a counter.                 */
/*     EX: -3 0 1 60 61 -4 15 expands into 0 0 0 0 60 61 15 15 15 15 15     */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                pbData,        // raster data to compress                 */
/*                pbReturn,      // compressed data will be written         */
/*                iTotalBytes )  // count of bytes in                       */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int
compressEpsonRLE (PBYTE  pbData,
                  int    cBytesInData,
                  PBYTE  pbReturn,
                  int    cBytesInReturn)
{
   int            iFrom = 0,
                  iTo   = 0,
                  iDups = 0;
   unsigned char  uchData;
   unsigned char *puchCount;

   /* Test for a special cases
   */
   if (0 > cBytesInData)
   {
      // Negative count
      return 0;
   }

   // loop through the buffer
   while (iFrom < cBytesInData)
   {
      /* There have to be two or more chars to examine
      ** cBytesInData is 1 based and iFrom is 0 based
      */
      if (1 == cBytesInData - iFrom)
      {
         // just one character to compress
         *(pbReturn + iTo) = (unsigned char)'\0';
         iTo++;
         *(pbReturn + iTo) = (unsigned char)pbData[iFrom];
         iTo++;

         return iTo;
      }

      uchData = pbData[iFrom];         // seed start point
      iFrom++;                         // look at the next char

      // is the next byte equal?
      if (uchData == pbData[iFrom])
      {
         iFrom++;                      // point to the next data byte
         iDups = 2;                    // initial count of dups

         // count dups and not go off the end
         while (  iFrom < cBytesInData
               && uchData == pbData[iFrom]
               && iDups < 128
               )
         {
            iDups++;
            iFrom++;
         }

         *(pbReturn + iTo) = (unsigned char)(256 - iDups + 1); // return count
         iTo++;
         *(pbReturn + iTo) = (unsigned char)uchData;           // return data
         iTo++;
      }
      else
      {
         iDups = 1;                    // initial count of non dups
         puchCount = pbReturn + iTo;   // Remember initial location of count
         iTo++;                        // Reserve its space

         *(pbReturn + iTo) = (unsigned char)uchData; // copy the data
         iTo++;                                      // move to next output

          // count non dups and not go off the end
         while (  iFrom < cBytesInData
               && iDups < 128
               )
         {
            if (  iFrom == cBytesInData - 1          // We are at the end
               || pbData[iFrom] != pbData[iFrom + 1] // Still different
               )
            {
               // copy the data
               *(pbReturn + iTo) = (unsigned char)*(pbData + iFrom);
               iTo++;
               iDups++;
            }
            else
               // Not not the same... ok not different
               break;

            iFrom++;
         }

         *puchCount = (unsigned char)(iDups - 1);      // update count
      }
   }

   return iTo;
}
