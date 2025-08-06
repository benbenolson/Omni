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
#include "Canon_S450_Blitter.hpp"
#include "Canon_S450_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

#define BUMP_TO_NEXT_MODULUS(n,m) ((((n)+(m)-1)/(m))*(m))

const static bool fTestNoCompression = false;

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new Canon_S450_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

Canon_S450_Blitter::
Canon_S450_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d      = false;
   fGraphicsHaveBeenSent_d = false;
   iNumDstRowBytes8_d      = 0;
}

Canon_S450_Blitter::
~Canon_S450_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void Canon_S450_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::initializeInstance ()" << std::endl;
#endif

   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;
   bNotFirstPass = false;  //PAZ

   HardCopyCap      *pHCC        = getCurrentForm ()->getHardCopyCap ();
   DeviceResolution *pDR         = getCurrentResolution ();
   DevicePrintMode  *pDPM        = getCurrentPrintMode ();
   PSZRO             pszDitherID = getCurrentDitherID ();

   iNumDstRowBytes8_d = ((pHCC->getXPels () * pDR->getDstBitsPerPel ()) + 7) >> 3;

   setCompressionInstance (new GplCompression (pDPM->getColorTech (),
                                               GplCompression::GPLCOMPRESS_TIFF,
                                               iNumDstRowBytes8_d,
                                               this));

   if (  DevicePrintMode::COLOR_TECH_CMY  == pDPM->getColorTech ()
      || DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ()
      )
   {
      int  iNumDstRowBytes8      = ((pHCC->getXPels () * pDR->getDstBitsPerPel ()) + 7) >> 3; //@@PAZ
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

bool Canon_S450_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "Canon_S450_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   Canon_S450_Instance *pInstance = dynamic_cast <Canon_S450_Instance *>(getInstance ());
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
   case DevicePrintMode::COLOR_TECH_CcMmYK:
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Canon_S450_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif

      return false;
   }
   }

   return true;
}

void Canon_S450_Blitter::
compressionChanged (int iNewCompression)
{
   DeviceCommand *pCommands = getCommands ();
   BinaryData    *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetCompression");
   if (pCmd)
   {
      bool fCompressionSet = false;

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
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::compressionChanged Error: Unsupported compression! " << *pCmd << std::endl;
#endif
      }

      sendPrintfToDevice (pCmd, fCompressionSet);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::compressionChanged Error: There is no cmdSetCompression defined for this device!" << std::endl;
#endif
   }
}

void Canon_S450_Blitter::
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
      case DevicePrintMode::COLOR_PLANE_BLACK:         cPlane = 'K'; break;
      case DevicePrintMode::COLOR_PLANE_CYAN:          cPlane = 'C'; break;
      case DevicePrintMode::COLOR_PLANE_LIGHT_CYAN:    cPlane = 'c'; break;
      case DevicePrintMode::COLOR_PLANE_MAGENTA:       cPlane = 'M'; break;
      case DevicePrintMode::COLOR_PLANE_LIGHT_MAGENTA: cPlane = 'm'; break;
      case DevicePrintMode::COLOR_PLANE_YELLOW:        cPlane = 'Y'; break;
      default:
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::sendData Error: unknown iWhichPlane = " << iWhichPlane << std::endl;
#endif
         break;
      }
      }

      sendPrintfToDevice (pCmd, pbdData->getLength (), cPlane);
      sendBinaryDataToDevice (pbdData);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::sendData Error: There is no cmdTransferRasterPlane defined for this device!" << std::endl;
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::sendData Error: There is no cmdMoveToNextRasterGraphicsLine or cmdEndRasterGraphicsLine defined for this device!" << std::endl;
#endif
   }
}

bool Canon_S450_Blitter::
canonMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Canon_S450_Instance *pInstance = dynamic_cast <Canon_S450_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Canon_S450_Blitter::canonMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
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
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Canon_S450_Blitter::canonMonoRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  =  ((pbmi2->cx * getCurrentResolution()->getDstBitsPerPel() )+ 7) >> 3; // smoser
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize iRemainder = " << iRemainder << std::endl;
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
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonMonoRasterize iScanLineY = " << iScanLineY << ", indexing to " << iScanLineY*cbSourceBytesInBitmap << std::endl;
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
         // Note: Canon_S450_Blitter::sendData () is called because of the
         //       compression!  Also, Canon_S450_Blitter::compressionChanged ()
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

bool Canon_S450_Blitter::
canonColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Canon_S450_Instance *pInstance = dynamic_cast <Canon_S450_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "Canon_S450_Blitter::canonColorRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
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
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Canon_S450_Blitter::canonColorRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  =  ((pbmi2->cx * getCurrentResolution()->getDstBitsPerPel() )+ 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   saveCy = pbmi2->cy;

   // smoser 1440
   DeviceResolution *pDR = getCurrentResolution ();

   if (pDR->getXRes () == 1440)
   {
      /*
         for 1440, you must use "Offset data transmission".
         Each color is 112 lines off of the color "in front of it"
         they're ordered Black, Cyan, Magenta ,Yellow
         ie, the first line of Yellow is sent with the 113th line of Cyan, as
         is the 107th line of Magenta
      */
      long lBandMemSize;
      int  iStartBufferLines = 1;   //@@START
      long lStartBandMemSize;       //@@START
      int  dataTransOffset   = 112;
      int  countC            = 0;
      int  countY            = 0;
      int  countM            = 0;
      bool lastBand          = ( (ulPageSize - 1) == (iWorldY + iNumScanLines));

      lBandMemSize      = cbDestBytesInPrinter * cy;    // allocate memory to hold previous band
      lStartBandMemSize = cbDestBytesInPrinter * (iStartBufferLines + cy);  //@@START
      int oldBandSize   = (dataTransOffset * cbDestBytesInPrinter);

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - 1440 pass" << std::endl;
#endif

      if (!bNotFirstPass)
      {
         // first time through get the memory
         currentCPlane = (PBYTE) malloc(lStartBandMemSize);  // Allocate memory for current
         currentMPlane = (PBYTE) malloc(lStartBandMemSize);
         currentYPlane = (PBYTE) malloc(lStartBandMemSize);
         pbSaveCurrentC = currentCPlane;
         pbSaveCurrentM = currentMPlane;
         pbSaveCurrentY = currentYPlane;

#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - allocated lStartBandMemSize" << std::endl;
#endif

         // some other first time only setup
         countC = 0;
         countM = -dataTransOffset;
         countY = -(2*dataTransOffset);
         pbOldCPlane =0;
         pbOldYPlane =0;

         pbOldYend  = 0;
         pbOldMend  = 0;
         blankLinep = 0;

         currentYPlaneStart = currentYPlane;
         currentMPlaneStart = currentMPlane;
         currentCPlaneStart = currentCPlane;

#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - currentCPlane= " << (int) currentCPlane << std::endl;
#endif

         // need to allocate the old band memory, set it up later
         pbOldMPlane = (PBYTE)malloc(oldBandSize);
         pbSaveOldM = pbOldMPlane;

#ifndef RETAIL
         if (!pbOldMPlane)
         {
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Error getting pbOldMPlane" << std::endl;
         }
#endif

         pbOldYPlane = (PBYTE) malloc(oldBandSize*2);
         pbSaveOldY = pbOldYPlane;

#ifndef RETAIL
         if (!pbOldYPlane)
         {
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Error getting pbOldYPlane" << std::endl;
         }
#endif

         pbOldMend=pbOldMPlane+oldBandSize;
         pbOldYend=pbOldYPlane+(oldBandSize*2);

      }
      else
      {
         currentMPlane=pbOldMPlane;
         currentYPlane=pbOldYPlane;
      }

      if (currentCPlane)
      {
         memset (currentCPlaneStart, 0, lStartBandMemSize);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::ERROR C" << std::endl;
#endif
      }
      if (currentMPlane)
      {
         memset (currentMPlaneStart, 0, lStartBandMemSize);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::ERROR M" << std::endl;
#endif
      }
      if (currentYPlane)
      {
         memset (currentYPlaneStart, 0, lStartBandMemSize);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::ERROR Y" << std::endl;
#endif
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize memset lStartBandMemSize" << std::endl;
#endif

      // set up the current*Plane with memory for this band
      int origNumScanLines = iNumScanLines;

      while (iNumScanLines)
      {
         pbBuffer  = pbBits + iScanLineY * cbSourceBytesInBitmap;
         pbmi2->cy = 1;

         ditherRGBtoCMYK (pbmi2, pbBuffer);

         if (!ditherAllPlanesBlank())
         {
            if (!ditherYPlaneBlank())
            {
               memcpy (currentYPlaneStart + ((origNumScanLines-iNumScanLines)*cbDestBytesInPrinter),getYPlane()->getData(),pbmi2->cy*cbDestBytesInPrinter);
            }
            if (!ditherCPlaneBlank())
            {
               memcpy (currentCPlaneStart + ((origNumScanLines-iNumScanLines)*cbDestBytesInPrinter),getCPlane()->getData(),pbmi2->cy*cbDestBytesInPrinter);
            }
            if (!ditherMPlaneBlank())
            {
               memcpy (currentMPlaneStart + ((origNumScanLines-iNumScanLines)*cbDestBytesInPrinter),getMPlane()->getData(),pbmi2->cy*cbDestBytesInPrinter);
            }
         }

         // Set Relative Vertical Print Position

         // Move down a scan line
         iScanLineY--;
         iWorldY++;

         // Done with a block of scan lines
         iNumScanLines--;
      }

      blankLinep = (PBYTE)malloc(cbDestBytesInPrinter);

      memset (blankLinep,0,cbDestBytesInPrinter);

      currentCPlane = currentCPlaneStart;

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - currentCPlane1 =  " << (int) currentCPlane << std::endl;
#endif

      bool       sentRasterLine                        = true;
      int        countBlankLines                       = 0;
      bool       blankLine                             = true;
      bool       blankC,
                 blankM,
                 blankY;
      BinaryData bdC (blankLinep,cbDestBytesInPrinter);
      BinaryData bdY (blankLinep,cbDestBytesInPrinter);
      BinaryData bdM (blankLinep,cbDestBytesInPrinter);

      blankLine = blankC = blankM = blankY = true;

      for(int currentLine = 0; currentLine < cy; currentLine++)
      {
         blankLine =true; blankC = true; blankM = true; blankY = true;

         if (  countC < 0
            || countM < 0
            || countY < 0
            )
         {
            blankLine = false;
         }
         if (countC >= 0)
         {
            if (memcmp (currentCPlane, blankLinep, cbDestBytesInPrinter) != 0)
            {
               bdC.setData(currentCPlane);

               blankLine = false;
               blankC    = false;
            }

            currentCPlane += cbDestBytesInPrinter;
         }
         if (countM >= 0)
         {
            if (currentMPlane == pbOldMend)
            {
               currentMPlane = currentMPlaneStart;
            }
            if (memcmp (currentMPlane, blankLinep, cbDestBytesInPrinter) != 0)
            {
               bdM.setData (currentMPlane);

               blankLine = false;
               blankM    = false;
            }

            currentMPlane += cbDestBytesInPrinter;
         }
         if (countY >= 0)
         {
            if (currentYPlane == pbOldYend)
            {
               currentYPlane = currentYPlaneStart;
            }
            if (memcmp (currentYPlane, blankLinep, cbDestBytesInPrinter) != 0)
            {
               bdY.setData (currentYPlane);

               blankLine = false;
               blankY    = false;
            }

            currentYPlane += cbDestBytesInPrinter;
         }

         if (blankLine)
         {
            if (  !sentRasterLine
               && countBlankLines == 0)
            {
               moveToYPosition (1, true);
            }

            countBlankLines++;
         }
         else
         {
            if (countBlankLines > 0)
            {
               moveToYPosition (countBlankLines, true);

               countBlankLines = 0;
            }
            else if (!sentRasterLine)
            {
               moveToYPosition (1, true);
            }
         }

         if (!blankC)
         {
            compressCRasterPlane (&bdC);
         }
         if (!blankM)
         {
            compressMRasterPlane (&bdM);
         }
         if (!blankY)
         {
            compressYRasterPlane (&bdY);
            sentRasterLine = true;
         }
         else
         {
            sentRasterLine = false;
         }

         countC++;
         countM++;
         countY++;
      }

      if (countBlankLines > 0)
      {
         moveToYPosition (countBlankLines, true);

         countBlankLines = 0;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - going through Last " << std::endl;
#endif

      if (lastBand)
      {
         for (int i = 0; i < dataTransOffset * 2; i++)
         {
            if (i <= dataTransOffset * 1)
            {
               if (currentMPlane == pbOldMend)
               {
                  currentMPlane = currentMPlaneStart;
               }
               if (memcmp (currentMPlane, blankLinep, cbDestBytesInPrinter) != 0)
               {
                  bdM.setData (currentMPlane);

                  blankLine = false;
                  blankM    = false;
               }
            }
            if (i <= dataTransOffset * 2)
            {
               if (currentYPlane == pbOldYend)
               {
                  currentYPlane = currentYPlaneStart;
               }
               if (memcmp (currentYPlane, blankLinep, cbDestBytesInPrinter) != 0)
               {
                  bdY.setData (currentYPlane);

                  blankLine = false;
                  blankY    = false;
               }

               currentYPlane += cbDestBytesInPrinter;
            }
            if (!blankM)
            {
               compressMRasterPlane (&bdM);
            }
            if (!blankY)
            {
               compressYRasterPlane (&bdY);

               sentRasterLine = true;
            }
            if (!sentRasterLine)
            {
               moveToYPosition (1, true);
            }
         }
         if (blankLinep)
         {
#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Freeing blankLinep" << std::endl;
#endif
             free (blankLinep);
         }
         if (pbSaveCurrentC)
         {
#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Freeing currentCPlane = " << (int) currentCPlane << std::endl;
#endif
             free (pbSaveCurrentC);
         }
         if (pbSaveCurrentM)
         {
#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Freeing currentMPlane " << std::endl;
#endif
             free (pbSaveCurrentM);
         }
         if (pbSaveCurrentY)
         {
#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Freeing currentYPlane" << std::endl;
#endif
             free (pbSaveCurrentY);
         }
         if (pbSaveOldM)
         {
#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Freeing pbOldMPlane " << std::endl;
#endif
             free (pbSaveOldM);
         }
         if (pbSaveOldY)
         {
#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Freeing pbOldYPlane " << std::endl;
#endif
             free (pbSaveOldY);
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize - Done Freeing plane memory " << std::endl;
#endif
      }
      else
      {
         // now copy off the part of the band that we need
         // from current position to end of memory for each color
         memcpy(pbOldYPlane,currentYPlane,oldBandSize*2);
         memcpy(pbOldMPlane,currentMPlane,oldBandSize);
      }

      bNotFirstPass = true;

      pbmi2->cy = saveCy;

      return true;
      // end 1440
   }
   else
   {
      while (iNumScanLines)
      {
         pbBuffer  = pbBits + iScanLineY * cbSourceBytesInBitmap;
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
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Canon_S450_Blitter::canonColorRasterize iScanLineY = " << iScanLineY << ", indexing to " << iScanLineY*cbSourceBytesInBitmap << std::endl;
#endif

            // Set Relative Vertical Print Position
            moveToYPosition (iWorldY, false);

            compressYRasterPlane (getYPlane ());
            compressMRasterPlane (getMPlane ());
            compressCRasterPlane (getCPlane ());

            if (DevicePrintMode::COLOR_TECH_CcMmYK == pDPM->getColorTech ())
            {
               compressLCRasterPlane (getLCPlane ());
               compressLMRasterPlane (getLMPlane ());
            }
            // Check to make sure that the printer has a black cartridge.
            if (  DevicePrintMode::COLOR_TECH_CMYK   == pDPM->getColorTech ()
               || DevicePrintMode::COLOR_TECH_CcMmYK == pDPM->getColorTech ()
               )
            {
                  compressKRasterPlane (getKPlane ());
            }

            // Note: Canon_S450_Blitter::sendData () is called because of the
            //       compression!  Also, Canon_S450_Blitter::compressionChanged ()
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
}

bool Canon_S450_Blitter::
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
bool Canon_S450_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   Canon_S450_Instance *pInstance = dynamic_cast <Canon_S450_Instance *>(getInstance ());
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

      sendPrintfToDevice (pCmd, iTemp);

      iAmount -= iTemp;
   }

   pInstance->ptlPrintHead_d.y = iWorldY;

   return true;
}

#ifndef RETAIL

void Canon_S450_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Canon_S450_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{Canon_S450_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Canon_S450_Blitter& const_self)
{
   Canon_S450_Blitter& self = const_cast<Canon_S450_Blitter&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
