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
#include "Canon_BJC_8500_Blitter.hpp"
#include "Canon_BJC_8500_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

#define BUMP_TO_NEXT_MODULUS(n,m) ((((n)+(m)-1)/(m))*(m))

const static bool fTestNoCompression = false;

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new Canon_BJC_8500_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

Canon_BJC_8500_Blitter::
Canon_BJC_8500_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d      = false;
   fGraphicsHaveBeenSent_d = false;
   iNumDstRowBytes8_d      = 0;
   pGamma_d                = 0;
}

Canon_BJC_8500_Blitter::
~Canon_BJC_8500_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

   if (pGamma_d)
   {
      delete pGamma_d;
      pGamma_d = 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void Canon_BJC_8500_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::initializeInstance ()" << std::endl;
#endif

   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

   HardCopyCap      *pHCC        = getCurrentForm ()->getHardCopyCap ();
   DeviceResolution *pDR         = getCurrentResolution ();
   DevicePrintMode  *pDPM        = getCurrentPrintMode ();
   PSZRO             pszDitherID = getCurrentDitherID ();

   iNumDstRowBytes8_d = (pHCC->getXPels () + 7) >> 3;

   setCompressionInstance (new GplCompression (pDPM->getColorTech (),
                                               GplCompression::GPLCOMPRESS_TIFF,
                                               iNumDstRowBytes8_d,
                                               this));

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

bool Canon_BJC_8500_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "Canon_BJC_8500_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   Canon_BJC_8500_Instance *pInstance = dynamic_cast <Canon_BJC_8500_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return canonMonoRasterize (pbBits,
                                 pbmi2,
                                 prectlPageLocation,
                                 eType);
      break;
   }

   case DevicePrintMode::COLOR_TECH_CMY:
   case DevicePrintMode::COLOR_TECH_CMYK:
   {
      return canonColorRasterize (pbBits,
                                  pbmi2,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Canon_BJC_8500_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif

      return false;
   }
   }

   return true;
}

void Canon_BJC_8500_Blitter::
compressionChanged (int iNewCompression)
{
   DeviceCommand *pCommands = getCommands ();
   BinaryData    *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetCompression");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::compressionChanged cmdSetCompression = " << *pCmd << std::endl;
#endif

      bool fCompressionSet = true;

      if (GplCompression::GPLCOMPRESS_TIFF == iNewCompression)
      {
         fCompressionSet = true;
      }
      else if (GplCompression::GPLCOMPRESS_NONE == iNewCompression)
      {
         fCompressionSet = false;
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::compressionChanged Error: Unsupported compression! " << *pCmd << std::endl;
#endif
      }

      sendPrintfToDevice (pCmd, fCompressionSet);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::compressionChanged Error: There is no cmdSetCompression defined for this device!" << std::endl;
#endif
   }
}

void Canon_BJC_8500_Blitter::
sendData (int         iLength,
          BinaryData *pbdData,
          int         iWhichPlane)
{
   DeviceCommand   *pCommands = getCommands ();
   BinaryData      *pCmd      = 0;
   DevicePrintMode *pDPM      = getCurrentPrintMode ();

   pCmd = pCommands->getCommandData ("cmdTransferRasterPlane");
   if (pCmd)
   {
      char cPlane = 'K';

      switch (iWhichPlane)
      {
      case DevicePrintMode::COLOR_PLANE_BLACK:   cPlane = 'K'; break;
      case DevicePrintMode::COLOR_PLANE_CYAN:    cPlane = 'C'; break;
      case DevicePrintMode::COLOR_PLANE_MAGENTA: cPlane = 'M'; break;
      case DevicePrintMode::COLOR_PLANE_YELLOW:  cPlane = 'Y'; break;
      default:
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::sendData Error: unknown iWhichPlane = " << iWhichPlane << std::endl;
#endif

         break;
      }
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::sendData cPlane = " << cPlane << std::endl;
#endif

      sendPrintfToDevice (pCmd, pbdData->getLength (), cPlane);
      sendBinaryDataToDevice (pbdData);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::sendData Error: There is no cmdTransferRasterPlane defined for this device!" << std::endl;
#endif
   }

   bool fLastPlane = false;

   if (DevicePrintMode::COLOR_TECH_CMY == pDPM->getColorTech ())
   {
      if (DevicePrintMode::COLOR_PLANE_YELLOW == iWhichPlane)
      {
         fLastPlane = true;
      }
   }
   else if (DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ())
   {
      if (DevicePrintMode::COLOR_PLANE_BLACK == iWhichPlane)
      {
         fLastPlane = true;
      }
   }
   else // if (pDPM->isID (DevicePrintMode::PRINT_MODE_1_ANY))
   {
      fLastPlane = true;
   }

   if (fLastPlane)
   {
      pCmd = pCommands->getCommandData ("cmdMoveToNextRasterGraphicsLine");
   }
   else
   {
      pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
   }

   if (pCmd)
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::sendData Error: There is no cmdMoveToNextRasterGraphicsLine or cmdEndRasterGraphicsLine defined for this device!" << std::endl;
#endif
   }
}

bool Canon_BJC_8500_Blitter::
canonMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Canon_BJC_8500_Instance *pInstance = dynamic_cast <Canon_BJC_8500_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Canon_BJC_8500_Blitter::canonMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
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
   bool              bDirty;
   PBYTE             pbBuffer;
   int               iRemainder;

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
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Canon_BJC_8500_Blitter::canonMonoRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize iRemainder = " << iRemainder << std::endl;
#endif

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

   while (iNumScanLines)
   {
      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

      if (fBlackWhiteReversed)
      {
         for (int x = 0; x < cbSourceBytesInBitmap; x++)
            pbBuffer[x] = ~pbBuffer[x];
      }

      /* See if this block of raster has some bits set in it!
      */
      bDirty = false;

      register int x;

      // Check all but the last byte
      for (x = 0; x <= cbDestBytesInPrinter - 2 && !bDirty; x++)
         bDirty |= pbBuffer[x];
      // Check the remaining bits
      bDirty |= pbBuffer[x] & Mask[iRemainder];

      if (bDirty)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonMonoRasterize iScanLineY = " << iScanLineY << ", indexing to " << iScanLineY*cbSourceBytesInBitmap << std::endl;
#endif

         // Set Relative Vertical Print Position
         moveToYPosition (iWorldY, false);

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

         // Make sure that the extraneous bits are set to 0.
         pbBuffer[cbDestBytesInPrinter-1] &= Mask[iRemainder];

         BinaryData data (pbBuffer, cbDestBytesInPrinter);

         compressKRasterPlane (&data);
         // Note: Canon_BJC_8500_Blitter::sendData () is called because of the
         //       compression!  Also, Canon_BJC_8500_Blitter::compressionChanged ()
         //       can be called.

         // Move down a scan line
         iScanLineY--;
         iWorldY++;

         // Printing a scan line moves the printer down a scan line.  Update the
         // new current position.
         pInstance->ptlPrintHead_d.y = iWorldY;
      }
      else
      {
         // Not dirty... just move down a scan line.
         iWorldY++;
         iScanLineY--;
      }

      // Done with a block of scan lines
      iNumScanLines--;
   }

   return true;
}

bool Canon_BJC_8500_Blitter::
canonColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Canon_BJC_8500_Instance *pInstance = dynamic_cast <Canon_BJC_8500_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Canon_BJC_8500_Blitter::canonColorRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DevicePrintMode  *pDPM                  = getCurrentPrintMode ();
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
   int               saveCy;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage    = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      ulPageSize    = cyPage;
      iWorldY       = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage    = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      ulPageSize    = cxPage;
      iWorldY       = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Canon_BJC_8500_Blitter::canonColorRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize iRemainder = " << iRemainder << std::endl;
#endif

   saveCy = pbmi2->cy;

   while (iNumScanLines)
   {
      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

      pbmi2->cy = 1;
      ditherRGBtoCMYK (pbmi2, pbBuffer);

      if (ditherAllPlanesBlank ())
      {
         // Not dirty... just move down a scan line.
         iWorldY++;
         iScanLineY--;
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::canonColorRasterize iScanLineY = " << iScanLineY << ", indexing to " << iScanLineY*cbSourceBytesInBitmap << std::endl;
#endif

         // Set Relative Vertical Print Position
         moveToYPosition (iWorldY, false);

         compressYRasterPlane (getYPlane ());
         compressMRasterPlane (getMPlane ());
         compressCRasterPlane (getCPlane ());

         // Check to make sure that the printer has a black cartridge.
         if (DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ())
         {
            compressKRasterPlane (getKPlane ());
         }

         // Note: Canon_BJC_8500_Blitter::sendData () is called because of the
         //       compression!  Also, Canon_BJC_8500_Blitter::compressionChanged ()
         //       can be called.

         if (fDumpOutgoingBitmaps)
         {
            int iMaxPass = 4;

            /* Loop through the four color passes/planes:
            **     YELLOW .. MAGENTA .. CYAN .. BLACK
            */
            for (int iPass = 0; iPass < iMaxPass; iPass++)
            {
               switch (iPass)
               {
               case 0: /* YELLOW */
               {
                  if (ditherYPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getYPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::YELLOW);
                  break;
               }

               case 1: /* MAGENTA */
               {
                  if (ditherMPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getMPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::MAGENTA);
                  break;
               }

               case 2: /* CYAN */
               {
                  if (ditherCPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getCPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::CYAN);
                  break;
               }

               case 3: /* BLACK */
               {
                  if (ditherKPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getKPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::BLACK);
                  break;
               }
               }
            }
         }

         // Move down a scan line
         iScanLineY--;
         iWorldY++;

         // Printing a scan line moves the printer down a scan line.  Update the
         // new current position.
         pInstance->ptlPrintHead_d.y = iWorldY;
      }

      // Done with a block of scan lines
      iNumScanLines--;
   }

    pbmi2->cy = saveCy;

   return true;
}

bool Canon_BJC_8500_Blitter::
setCompression (bool fCompressed)
{
   DeviceCommand *pCommands = getCommands ();
   BinaryData    *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetCompression");
   if (!pCmd)
   {
      // Error!
      return false;
   }

   sendPrintfToDevice (pCmd, fCompressed ? 1 : 0);

   return true;
}

/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool Canon_BJC_8500_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   Canon_BJC_8500_Instance *pInstance = dynamic_cast <Canon_BJC_8500_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;
   int               iAmount;

   if (fAbsolute)
   {
      iAmount = iWorldY;
   }
   else
   {
      if (pInstance->ptlPrintHead_d.y == iWorldY)
      {
         // No movement required
         return true;
      }

      if (pInstance->ptlPrintHead_d.y > iWorldY)
      {
         // Wants to move backwards!
         return false;
      }

      iAmount = iWorldY - pInstance->ptlPrintHead_d.y;
   }

   pCmd = pCommands->getCommandData ("cmdSetYPos");
   if (!pCmd)
   {
      // Error!
      return false;
   }

   // Raster Skip (RSK) can only handle up to 0x17ff scan lines (at 360 dpi
   // this translates to about 17"). If the count is higher, split it into
   // multiple ESC sequences (unlikely but nevertheless possible)
   while (0 < iAmount)
   {
      int iTemp = omni::min (iAmount, 0x17FF);

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_BJC_8500_Blitter::moveToYPosition raster skipping to " << iTemp << std::endl;
#endif

      sendPrintfToDevice (pCmd, iTemp);

      iAmount -= iTemp;
   }

   pInstance->ptlPrintHead_d.y = iWorldY;

   return true;
}

#ifndef RETAIL

void Canon_BJC_8500_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Canon_BJC_8500_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{Canon_BJC_8500_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Canon_BJC_8500_Blitter& const_self)
{
   Canon_BJC_8500_Blitter& self = const_cast<Canon_BJC_8500_Blitter&>(const_self);
   std::ostringstream      oss;

   os << self.toString (oss);

   return os;
}
