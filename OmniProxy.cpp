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
#include "OmniProxy.hpp"
#include "MonoDither.hpp"
#include "defines.hpp"

#include <cstdarg>
#include <cstdint>

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "hppcl3/bitmap.hpp"

//#define INCLUDE_2BPP  1
//#define INCLUDE_4BPP  1
//#define INCLUDE_16BPP 1

unsigned int
square (int iValue)
{
   return iValue * iValue;
}

byte
findNearestColor (RGB2 rgb, PBITMAPINFO2 pbmi2)
{
   if (8 < pbmi2->cBitCount)
      // Error!
      return 0;

   int iNumColors = 1 << pbmi2->cBitCount;

   // Find exact match
   for (int i = 0; i < iNumColors; i++)
   {
      if (  rgb.bRed   == pbmi2->argbColor[i].bRed
         && rgb.bGreen == pbmi2->argbColor[i].bGreen
         && rgb.bBlue  == pbmi2->argbColor[i].bBlue
         )
      {
         return i;
      }
   }

   // Find closest match
   unsigned int uiDistance = 4294967295U;
   int          iClosest   = 0;

   for (int i = 0; i < iNumColors; i++)
   {
      unsigned int uiThisDistance = square (rgb.bRed   - pbmi2->argbColor[i].bRed)
                                  + square (rgb.bGreen - pbmi2->argbColor[i].bGreen)
                                  + square (rgb.bBlue  - pbmi2->argbColor[i].bBlue);

      if (uiThisDistance < uiDistance)
      {
         uiDistance = uiThisDistance;
         iClosest   = i;
      }
   }

   return iClosest;
}

OmniProxy::
OmniProxy (Device *pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy::" << __FUNCTION__ << " ()" << std::endl;
#endif

   pDevice_d         = pDevice;
   pHeadersRoot_d    = 0;
   pHeadersCurrent_d = 0;
   pHeaderCurrent_d  = 0;
   fp_d              = 0;
   fd_d              = 0;
   iMaxScanLines_d   = 1024; // @TBD

   int iScanlineMultiple = pDevice_d->getScanlineMultiple ();

   if (0 != (iMaxScanLines_d % iScanlineMultiple))
   {
      iMaxScanLines_d += iScanlineMultiple - (iMaxScanLines_d % iScanlineMultiple);
   }
}

OmniProxy::
~OmniProxy ()
{
   // Clean up
   delete pDevice_d;
   pDevice_d = 0;

   freeBitmapInfo ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy::~" << __FUNCTION__ << " ()" << std::endl;
#endif
}

void OmniProxy::
allocateBitmapInfo ()
{
   // Allocate a block of BITBLT_HEADERs
   pHeadersRoot_d = (PBITBLT_HEADERS)calloc (1, BITBLT_HEADERS_BLOCK);

   if (pHeadersRoot_d)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: pHeadersRoot_d = 0x" << std::hex << (int)pHeadersRoot_d << std::dec << std::endl;
#endif

      // Initialize it
      pHeadersRoot_d->pNext    = 0;
      pHeadersRoot_d->cHeaders = BITBLT_HEADERS_COUNT;
      pHeadersCurrent_d = pHeadersRoot_d;
      pHeaderCurrent_d  = pHeadersRoot_d->aHeaders;

      // Open a temporary file
      fp_d = tmpfile ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: fp_d = 0x" << (int)fp_d << std::endl;
#endif

      if (!fp_d)
      {
         // Error!  Clean up!
         free (pHeadersRoot_d);
         pHeadersRoot_d    = 0;
         pHeadersCurrent_d = 0;
         pHeaderCurrent_d  = 0;
      }
      else
      {
         fd_d = fileno (fp_d);
#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: fd_d = " << (int)fd_d << std::endl;
#endif
      }
   }
}

void OmniProxy::
freeBitmapInfo ()
{
   // Free the headers
   if (pHeadersRoot_d)
   {
      PBITBLT_HEADERS pTmp;

      // Loop through the headers and free them
      while (pHeadersRoot_d)
      {
         pTmp           = pHeadersRoot_d;
         pHeadersRoot_d = pHeadersRoot_d->pNext;

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: freeing 0x" << std::hex << (int)pTmp << std::dec << std::endl;
#endif
         free (pTmp);
      }
   }

   // Close the file
   if (fp_d)
   {
      fclose (fp_d);
      fp_d = 0;
      fd_d = 0;
   }
}

bool OmniProxy::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
   PSZCRO pszDumpEnvironmentVar = getenv ("OMNI_DUMP_PROXY_BITMAPS");
   bool   fDumpProxyBitmaps     = false;

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy::" << __FUNCTION__ << ": (in)pszDumpEnvironmentVar = " << std::hex << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpProxyBitmaps = true;
   }

   if (fDumpProxyBitmaps)
   {
      static int  iNum = 0;
      char        achName[4 + 8 + 1];
      PNEUTRALRGB pColors = 0;

      sprintf (achName, "%04dPRXY.bmp", iNum);

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

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ())
   {
      DebugOutput::getErrorStream () << "OmniProxy::" << __FUNCTION__ << " ()"
           << "pbBits = 0x" << std::hex << (int)pbBits << std::dec
           << ", pbmi2 = {" << pbmi2->cbFix << ", " << pbmi2->cx << ", " << pbmi2->cy << ", " << pbmi2->cPlanes << ", " << pbmi2->cBitCount << "}"
           << ", rectlPageLocation = { (" << prectlPageLocation->xLeft
           << ", " << prectlPageLocation->yBottom
           << "), (" << prectlPageLocation->xRight
           << ", " << prectlPageLocation->yTop
           << ") }"
           << std::endl;
   }
#endif

   if (!pHeaderCurrent_d)
      return false;

   int cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   int rc;

   // fill in the record
   fflush (fp_d); pHeaderCurrent_d->pos = lseek (fd_d, 0, SEEK_END); // was: fgetpos (fp_d, &pHeaderCurrent_d->pos);
   pHeaderCurrent_d->rectlPageLocation.xLeft   = prectlPageLocation->xLeft;
   pHeaderCurrent_d->rectlPageLocation.yBottom = prectlPageLocation->yBottom;
   pHeaderCurrent_d->rectlPageLocation.xRight  = prectlPageLocation->xRight;
   pHeaderCurrent_d->rectlPageLocation.yTop    = prectlPageLocation->yTop;
   pHeaderCurrent_d->cPlanes                   = pbmi2->cPlanes;
   pHeaderCurrent_d->cBitCount                 = pbmi2->cBitCount;
   pHeaderCurrent_d->cbBitmapHeader            = pbmi2->cbFix;
   pHeaderCurrent_d->cbBitmapData              = cbSourceBytesInBitmap * (prectlPageLocation->yTop - prectlPageLocation->yBottom + 1);
   pHeaderCurrent_d->eType                     = eType;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ())
   {
      DebugOutput::getErrorStream () << "OmniProxy: pHeaderCurrent_d->pos = " << pHeaderCurrent_d->pos << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: pbmi2->cbFix = " << pbmi2->cbFix << std::endl;
   }
#endif

   if (8 >= pbmi2->cBitCount)
   {
      int iNumColors = 1 << pbmi2->cBitCount;

      pHeaderCurrent_d->cbBitmapHeader += iNumColors * sizeof (RGB2);
   }

   // Write the bitmap info struct
   rc = fwrite (pbmi2, pHeaderCurrent_d->cbBitmapHeader, 1, fp_d);

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: fwrite (...pbmi2(" << pHeaderCurrent_d->cbBitmapHeader << ")...) = " << rc << std::endl;
#endif

   // Write the bitmap bits
   rc = fwrite (pbBits, pHeaderCurrent_d->cbBitmapData, 1, fp_d);

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: fwrite (...pbBits(" << pHeaderCurrent_d->cbBitmapData << ")...) = " << rc << std::endl;
#endif

   pHeadersCurrent_d->cHeaders--;
   pHeaderCurrent_d++;

   // Have we run out of headers?
   if (0 == pHeadersCurrent_d->cHeaders)
   {
      pHeadersCurrent_d->pNext = (PBITBLT_HEADERS)calloc (1, BITBLT_HEADERS_BLOCK);

      if (pHeadersCurrent_d->pNext)
      {
         pHeadersCurrent_d = pHeadersCurrent_d->pNext;

         pHeadersCurrent_d->pNext    = 0;
         pHeadersCurrent_d->cHeaders = BITBLT_HEADERS_COUNT;

         pHeaderCurrent_d  = pHeadersCurrent_d->aHeaders;
      }
      else
      {
         pHeadersCurrent_d = 0;
      }
   }

   return true;
}

bool OmniProxy::
beginJob ()
{
   allocateBitmapInfo ();

   return pDevice_d->beginJob ();
}

bool OmniProxy::
beginJob (PSZCRO pszJobProperties)
{
   replayBitmaps ();
   allocateBitmapInfo ();

   return pDevice_d->beginJob (pszJobProperties);
}

bool OmniProxy::
newFrame ()
{
   replayBitmaps ();
   allocateBitmapInfo ();

   return pDevice_d->newFrame ();
}

bool OmniProxy::
newFrame (PSZCRO pszJobProperties)
{
   replayBitmaps ();
   allocateBitmapInfo ();

   return pDevice_d->newFrame (pszJobProperties);
}

bool OmniProxy::
endJob ()
{
   replayBitmaps ();

   return pDevice_d->endJob ();
}

bool OmniProxy::
abortJob ()
{
   return pDevice_d->abortJob ();
}

void OmniProxy::
replayBitmaps ()
{
#if 0 // @GCC
   if (!pHeaderCurrent_d)
      return;

   PBITBLT_HEADERS      pStart             = 0;
   PBITBLT_HEADER       pCurrent           = 0;
   int                  iCount             = 0;
   PBYTE                pbBitmapBits       = 0;
   PBITMAPINFO2         pbmi2Bitmap        = 0;
   int                  icbBitmapScanline  = 0;
   int                  rc;

   RECTL                rectlPageLocation  = {0, 0, 0, 0};
   PBYTE                pbPrinterBits      = 0;
   PBITMAPINFO2         pbmi2Printer       = 0;
   PBYTE                pbPrinterScanline  = 0;
   int                  iBytesToAlloc;
   int                  iNumColorsPrinter;
   int                  icbPrinterScanline;
   int                  iNumBands;
   int                  iTopY;
   byte                 bWhiteIndex        = 0;
   byte                 bFillColor         = 0;
   HardCopyCap         *pHCC;
   DevicePrintMode     *pPrintMode;
   IMAGEINFOS           ImageInfo;
   DIFFUSIONDITHERINFO  DiffInfo;
   POINTL               ptlSrcBounds;
   POINTL               ptlTrgBounds;

   pHCC = pDevice_d->getCurrentForm ()->getHardCopyCap ();
   pPrintMode = pDevice_d->getCurrentPrintMode ();

   iNumColorsPrinter = 1 << pPrintMode->getLogicalCount ();

   // Calculate the size of a full bitmap info 2 and bitmap header message
   iBytesToAlloc = sizeof (BITMAPINFO2);
   if (256 >= iNumColorsPrinter)
      iBytesToAlloc += (iNumColorsPrinter - 1) * sizeof (RGB2);

   // Allocate a bitmap info structure
   pbmi2Printer = (PBITMAPINFO2)calloc (1, iBytesToAlloc);
   if (!pbmi2Printer)
      goto done;

   // Initialize the bitmap info structure
   pbmi2Printer->cx            = pHCC->getXPels ();
   pbmi2Printer->cy            = iMaxScanLines_d;
   pbmi2Printer->cPlanes       = pPrintMode->getNumPlanes ();
   pbmi2Printer->cBitCount     = pPrintMode->getLogicalCount ();
   pbmi2Printer->cclrUsed      = 1 << pbmi2Printer->cBitCount;
   pbmi2Printer->cclrImportant = 0;                            // All are important

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniProxy ())
   {
      DebugOutput::getErrorStream () << "OmniProxy: pbmi2Printer:" << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: cx              = " << pbmi2Printer->cx << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: cy              = " << pbmi2Printer->cy << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: cPlanes         = " << pbmi2Printer->cPlanes << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: cBitCount       = " << pbmi2Printer->cBitCount << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: cclrUsed        = " << pbmi2Printer->cclrUsed << std::endl;
      DebugOutput::getErrorStream () << "OmniProxy: cclrImportant   = " << pbmi2Printer->cclrImportant << std::endl;
   }
#endif

   // Set up the color table
   switch (pbmi2Printer->cBitCount)
   {
   case 1:
   {
      pbmi2Printer->argbColor[0].bRed   = 0xFF;
      pbmi2Printer->argbColor[0].bGreen = 0xFF;
      pbmi2Printer->argbColor[0].bBlue  = 0xFF;
      pbmi2Printer->argbColor[1].bRed   = 0x00;
      pbmi2Printer->argbColor[1].bGreen = 0x00;
      pbmi2Printer->argbColor[1].bBlue  = 0x00;
      bWhiteIndex = 0;
      break;
   }

#ifdef INCLUDE_2BPP
   case 2:
   {
      // @TBD
      break;
   }
#endif

#ifdef INCLUDE_4BPP
   case 4:
   {
      // @TBD
      break;
   }
#endif

   case 8:
   {
      // @TBD
      break;
   }
   }

   // Determine the erase color byte
   switch (pbmi2Printer->cBitCount)
   {
   case 1:
   {
      if (bWhiteIndex)
         bFillColor = 0xFF;
      else
         bFillColor = 0x00;
      break;
   }

#ifdef INCLUDE_2BPP
   case 2:
   {
      bFillColor = (bWhiteIndex << 6)
                 | (bWhiteIndex << 4)
                 | (bWhiteIndex << 2)
                 | bWhiteIndex;
      break;
   }
#endif

#ifdef INCLUDE_4BPP
   case 4:
   {
      bFillColor = (bWhiteIndex << 4) | bWhiteIndex;
      break;
   }
#endif

   case 8:
   {
      bFillColor = 0;     // @TBD
      break;
   }

#ifdef INCLUDE_16BPP
   case 16:
   {
      bFillColor = 0xFF;  // Don't use indexes
      break;
   }
#endif

   case 24:
   {
      bFillColor = 0xFF;  // Don't use indexes
      break;
   }
   }

   // Scan lines are double word aligned (32-bits)
   icbPrinterScanline = ((pbmi2Printer->cx * pbmi2Printer->cBitCount + 31) >> 5) << 2;
   iBytesToAlloc      = icbPrinterScanline * iMaxScanLines_d;

   // Allocate the printer's scan lines
   pbPrinterBits = (PBYTE)calloc (1, iBytesToAlloc);
   if (!pbPrinterBits)
      goto done;

   iNumBands = (pHCC->getYPels () + iMaxScanLines_d - 1) / iMaxScanLines_d;
   iTopY     = pHCC->getYPels ();

   // Loop through the bands
   for (int iPass = 0; iPass < iNumBands; iPass++)
   {
      // Erase the bitmap
      memset (pbPrinterBits, bFillColor, icbPrinterScanline * pbmi2Printer->cy);

      // @TBD which way is it orientated?

      // Set up the band location
      rectlPageLocation.xLeft   = 0;
      rectlPageLocation.xRight  = pbmi2Printer->cx - 1;
      rectlPageLocation.yTop    = (iTopY - 1) - iPass * iMaxScanLines_d;
      rectlPageLocation.yBottom = rectlPageLocation.yTop - iMaxScanLines_d + 1;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniProxy ())
      {
         DebugOutput::getErrorStream () << std::dec
              << "OmniProxy: Printer's band is: rectlPageLocation = { (" << rectlPageLocation.xLeft
              << ", " << rectlPageLocation.yTop
              << "), (" << rectlPageLocation.xRight
              << ", " << rectlPageLocation.yBottom
              << ") }" << std::endl;
      }
#endif

      pStart = pHeadersRoot_d;

      // Loop through the header blocks
      do
      {
         pCurrent = pStart->aHeaders;
         iCount = BITBLT_HEADERS_COUNT - pStart->cHeaders;

         // Loop through the header records
         while (0 < iCount)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmniProxy ())
            {
               DebugOutput::getErrorStream () << "OmniProxy: xl = " << pCurrent->rectlPageLocation.xLeft   << ", xr = " << rectlPageLocation.xRight  << " = " << (pCurrent->rectlPageLocation.xLeft   > rectlPageLocation.xRight ) << std::endl;
               DebugOutput::getErrorStream () << "OmniProxy: yb = " << pCurrent->rectlPageLocation.yBottom << ", yt = " << rectlPageLocation.yTop    << " = " << (pCurrent->rectlPageLocation.yBottom > rectlPageLocation.yTop   ) << std::endl;
               DebugOutput::getErrorStream () << "OmniProxy: xr = " << pCurrent->rectlPageLocation.xRight  << ", xl = " << rectlPageLocation.xLeft   << " = " << (pCurrent->rectlPageLocation.xRight  < rectlPageLocation.xLeft  ) << std::endl;
               DebugOutput::getErrorStream () << "OmniProxy: yt = " << pCurrent->rectlPageLocation.yTop    << ", yb = " << rectlPageLocation.yBottom << " = " << (pCurrent->rectlPageLocation.yTop    < rectlPageLocation.yBottom) << std::endl;
               DebugOutput::getErrorStream () << "OmniProxy: ----- (all zeros indicate a hit) -----" << std::endl;
            }
#endif

            // Does the bitmap intersect the band?
            if (! (  pCurrent->rectlPageLocation.xLeft   > rectlPageLocation.xRight
                  || pCurrent->rectlPageLocation.yBottom > rectlPageLocation.yTop
                  || pCurrent->rectlPageLocation.xRight  < rectlPageLocation.xLeft
                  || pCurrent->rectlPageLocation.yTop    < rectlPageLocation.yBottom
                  )
               )
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputOmniProxy ())
               {
                  DebugOutput::getErrorStream () << "OmniProxy: Bitmap Record (0x" << std::hex << (int)pCurrent << ":" << std::dec << iCount << ")" << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: pos            = " << pCurrent->pos << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: .xLeft         = " << pCurrent->rectlPageLocation.xLeft << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: .yBottom       = " << pCurrent->rectlPageLocation.yBottom << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: .xRight        = " << pCurrent->rectlPageLocation.xRight << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: .yTop          = " << pCurrent->rectlPageLocation.yTop << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: cBitCount      = " << pCurrent->cBitCount << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: cPlanes        = " << pCurrent->cPlanes << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: cbBitmapHeader = " << pCurrent->cbBitmapHeader << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: cbBitmapData   = " << pCurrent->cbBitmapData << std::endl;
               }
#endif

               int   iLength;
               int   iOffset;
               int   iBump;
               void *pvAddress;

               iLength = pCurrent->cbBitmapHeader
                       + pCurrent->cbBitmapData;
               iOffset = (int)pCurrent->pos;
               iBump   = iOffset & 0xFFF;

               // Access our data
               pvAddress = mmap (0,
                                 iLength + iBump,
                                 PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE,
                                 fd_d,
                                 iOffset & 0xFFFFF000);

#ifndef RETAIL
               if (DebugOutput::shouldOutputOmniProxy ())
               {
                  DebugOutput::getErrorStream () << "OmniProxy: pvAddress = 0x" << std::hex << (int)pvAddress << std::dec << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: iOffset   = " << iOffset << std::endl;
                  DebugOutput::getErrorStream () << "OmniProxy: iBump     = " << iBump << std::endl;
               }
#endif

               if (MAP_FAILED == pbmi2Bitmap)
               {
#ifndef RETAIL
                  if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << std::dec << "OmniProxy: errno = " << errno << std::endl;
#endif
               }
               else if (pvAddress)
               {
                  pbmi2Bitmap  = (PBITMAPINFO2)((PBYTE)pvAddress + iBump);
                  pbBitmapBits = (PBYTE)((PBYTE)pvAddress + iBump + pCurrent->cbBitmapHeader);

#ifndef RETAIL
                  if (DebugOutput::shouldOutputOmniProxy ())
                  {
                     DebugOutput::getErrorStream () << "OmniProxy: Bitmap Data (0x" << std::hex << (int)pbmi2Bitmap << std::dec << ")" << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cbFix           = " << pbmi2Bitmap->cbFix << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cx              = " << pbmi2Bitmap->cx << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cy              = " << pbmi2Bitmap->cy << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cPlanes         = " << pbmi2Bitmap->cPlanes << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cBitCount       = " << pbmi2Bitmap->cBitCount << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cclrUsed        = " << pbmi2Bitmap->cclrUsed << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: cclrImportant   = " << pbmi2Bitmap->cclrImportant << std::endl;
                  }
#endif

                  icbBitmapScanline = ((pbmi2Bitmap->cx * pbmi2Bitmap->cBitCount + 31) >> 5) << 2;

                  int iStartScanline = max (pCurrent->rectlPageLocation.yBottom,
                                            rectlPageLocation.yBottom);
                  int iEndScanline   = min (pCurrent->rectlPageLocation.yTop,
                                            rectlPageLocation.yTop);
                  int iStartX        = max (pCurrent->rectlPageLocation.xLeft,
                                            rectlPageLocation.xLeft);
                  int iEndX          = min (pCurrent->rectlPageLocation.xRight,
                                            rectlPageLocation.xRight);
                  int iCx            = iEndX - iStartX + 1;

#ifndef RETAIL
                  if (DebugOutput::shouldOutputOmniProxy ())
                  {
                     DebugOutput::getErrorStream () << "OmniProxy: iStartScanline = " << iStartScanline << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: iEndScanline   = " << iEndScanline << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: iStartX        = " << iStartX << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: iEndX          = " << iEndX << std::endl;
                     DebugOutput::getErrorStream () << "OmniProxy: iCx            = " << iCx << std::endl;
                  }
#endif

                  // Pre-work
                  if (  24 == pbmi2Bitmap->cBitCount
                     &&  1 == pbmi2Printer->cBitCount
                     )
                  {
                     DiffInfo.ulLength    = sizeof (DiffInfo);
                     DiffInfo.ulType      = GDM_MATRIX_DITHER;
                     DiffInfo.fOptions    = 0;
                     DiffInfo.ulIntensity = 80;
                     DiffInfo.bRedWt      = 30;
                     DiffInfo.bGreenWt    = 50;
                     DiffInfo.bBlueWt     = 20;
                     DiffInfo.bPad        = 0;

                     ptlSrcBounds.x = iCx;
                     ptlSrcBounds.y = 1;
                     ptlTrgBounds.x = iCx;
                     ptlTrgBounds.y = 1;

                     ImageInfo.pptlSrc             = &ptlSrcBounds;    // rectangle extents
                     ImageInfo.pptlDst             = &ptlTrgBounds;    // rectangle extents
                     ImageInfo.ulSrcBpp            = 24;
//                   ImageInfo.ulSrcBytesPerLine   = uiBytesPerLine;   // src bytes per line
                     ImageInfo.ulcSrcClrs          = 0;
                     ImageInfo.ulSrcClrType        = 0;
                     ImageInfo.pargb2Src           = 0;                // if srcBPP != 24
                     ImageInfo.ulTrgBpp            = 1;
//                   ImageInfo.ulTrgBytesPerLine   = pasyncDev->width + (-pasyncDev->width & 31);
                     ImageInfo.ulTrgBytesPerLine  /= 8;
                     ImageInfo.ulcTrgClrs          = 2;
                     ImageInfo.ulTrgClrType        = 0;
                     ImageInfo.pargb2Trg           = NULL;
                     ImageInfo.ulOptions           = 0;                // options
                     ImageInfo.ulPelSizeCorrection = 100;

//                   Not done since we dont want pBuffer set up to be allocated!
//                   DiffusionDither (&DiffInfo, &ImageInfo, &pBuffer, GDM_DITHER_BEGIN);
                  }

                  // Merge this bitmap into the printer's bitmap
                  for (int i = iStartScanline; i < iEndScanline; i++)
                  {
                     PBYTE pbBitmapScanline = pbBitmapBits
                                            + ( i
                                              - pCurrent->rectlPageLocation.yBottom
                                              )
                                              * icbBitmapScanline;

                     pbPrinterScanline = pbPrinterBits
                                       + ( i
                                         - rectlPageLocation.yBottom
                                         )
                                         * icbPrinterScanline;

#ifndef RETAIL
                     if (DebugOutput::shouldOutputOmniProxy ())
                     {
                        DebugOutput::getErrorStream () << "OmniProxy: from scanline = " << (i - pCurrent->rectlPageLocation.yBottom) << std::endl;
                        DebugOutput::getErrorStream () << "OmniProxy: to   scanline = " << (i - rectlPageLocation.yBottom) << std::endl;
                     }
#endif

                     if (pbmi2Bitmap->cBitCount == pbmi2Printer->cBitCount)
                     {
                        // Just copy the bits over
                        switch (pbmi2Printer->cBitCount)
                        {
                        case 1:
                        {
                           static byte abMaskStart[] = {
                           //    X     1     2     3     4     5     6     7
                              0x00, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
                           };
                           static byte abMaskEnd[]   = {
                           //    X     1     2     3     4     5     6     7
                              0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
                           };
                           int         iPos          = (iStartX & -8) >> 3;
                           int         iPelsHandled  = iCx;

                           // @TBD - color table translation
                           if (  pbmi2Bitmap->argbColor[0].bRed   == pbmi2Printer->argbColor[1].bRed
                              && pbmi2Bitmap->argbColor[0].bGreen == pbmi2Printer->argbColor[1].bGreen
                              && pbmi2Bitmap->argbColor[0].bBlue  == pbmi2Printer->argbColor[1].bBlue
                              && pbmi2Bitmap->argbColor[1].bRed   == pbmi2Printer->argbColor[0].bRed
                              && pbmi2Bitmap->argbColor[1].bGreen == pbmi2Printer->argbColor[0].bGreen
                              && pbmi2Bitmap->argbColor[1].bBlue  == pbmi2Printer->argbColor[0].bBlue
                              )
                           {
                              // Color table is reversed!
                              for (int x = 0; x < icbBitmapScanline; x++)
                              {
                                 pbBitmapScanline[x] = ~pbBitmapScanline[x];
                              }
                           }

                           // Are there bits before the start of a byte?
                           if (iStartX & 7)
                           {
                              pbPrinterScanline[iPos] |= pbBitmapScanline[iPos] & abMaskStart[iStartX & 7];

                              iPelsHandled -= (iStartX & 7);
                              iPos++;
                           }

                           // Are there full bytes?
                           if (8 <= iPelsHandled)
                           {
                              int iBytesToCopy = (iPelsHandled >> 3);

                              memcpy (pbPrinterScanline + iPos,
                                      pbBitmapScanline + iPos,
                                      iBytesToCopy);

                              iPelsHandled -= iBytesToCopy << 3;
                              iPos         += iBytesToCopy;
                           }

                           // Are there bits afterwards (should be less than a byte)?
                           if (0 < iPelsHandled)
                           {
                              pbPrinterScanline[iPos] |= pbBitmapScanline[iPos] & abMaskEnd[(iEndX + 1) & 7];
                           }
                           break;
                        }

#ifdef INCLUDE_2BPP
                        case 2:
                        {
                           static byte abMaskStart[] = {
                           //    X     1     2     3
                              0x00, 0x3F, 0x0F, 0x03
                           };
                           static byte abMaskEnd[]   = {
                           //    X     1     2     3
                              0x00, 0xC0, 0xF0, 0xF3
                           };
                           int         iPos          = (iStartX & -4) >> 2;
                           int         iPelsHandled  = iCx;

                           // Are there bits before the start of a byte?
                           if (iStartX & 3)
                           {
                              pbPrinterScanline[iPos] |= pbBitmapScanline[iPos] & abMaskStart[iStartX & 3];

                              iPelsHandled -= (iStartX & 3);
                              iPos++;
                           }

                           // Are there full bytes?
                           if (4 <= iPelsHandled)
                           {
                              int iBytesToCopy = (iPelsHandled >> 2);

                              memcpy (pbPrinterScanline + iPos,
                                      pbBitmapScanline + iPos,
                                      iBytesToCopy);

                              iPelsHandled -= iBytesToCopy << 2;
                              iPos         += iBytesToCopy;
                           }

                           // Are there bits afterwards (should be less than a byte)?
                           if (0 < iPelsHandled)
                           {
                              pbPrinterScanline[iPos] |= pbBitmapScanline[iPos] & abMaskEnd[(iEndX + 1) & 3];
                           }
                           break;
                        }
#endif

#ifdef INCLUDE_4BPP
                        case 4:
                        {
                           int iPos         = iStartX >> 1;
                           int iPelsHandled = iCx;

                           // Is there a nibble before the start of a byte?
                           if (iStartX & 1)
                           {
                              pbPrinterScanline[iPos] |= pbBitmapScanline[iPos] & 0x0F;

                              iPelsHandled--;
                              iPos++;
                           }

                           // Are there full bytes?
                           if (2 <= iPelsHandled)
                           {
                              int iBytesToCopy = (iPelsHandled >> 1);

                              memcpy (pbPrinterScanline + iPos,
                                      pbBitmapScanline + iPos,
                                      iPelsHandled);

                              iPelsHandled -= iBytesToCopy << 1;
                              iPos         += iBytesToCopy;
                           }

                           // Is there a nibble afterwards (should be 1)?
                           if (0 < iPelsHandled)
                           {
                              pbPrinterScanline[iPos] |= pbBitmapScanline[iPos] & 0xF0;
                           }
                           break;
                        }
#endif

                        case 8:
                        {
                           memcpy (pbPrinterScanline,
                                   pbBitmapScanline,
                                   iCx);
                           break;
                        }

#ifdef INCLUDE_16BPP
                        case 16:
                        {
                           memcpy (pbPrinterScanline,
                                   pbBitmapScanline,
                                   2 * iCx);
                           break;
                        }
#endif

                        case 24:
                        {
                           memcpy (pbPrinterScanline,
                                   pbBitmapScanline,
                                   3 * iCx);
                           break;
                        }
                        }
                     }
                     else if (pbmi2Printer->cBitCount > pbmi2Bitmap->cBitCount)
                     {
                        // Pad the bits when copying
                        switch (pbmi2Printer->cBitCount)
                        {
                        case 1:
                        {
                           // Impossible
                           break;
                        }

#ifdef INCLUDE_2BPP
                        case 2:
                        {
                           // @TBD
                           break;
                        }
#endif

#ifdef INCLUDE_4BPP
                        case 4:
                        {
                           // @TBD
                           break;
                        }
#endif

                        case 8:
                        {
                           switch (pbmi2Bitmap->cBitCount)
                           {
                           case 1:
                           {
                              static byte abMask[] = {
                                 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
                              };
                              byte bIndex0 = findNearestColor (pbmi2Bitmap->argbColor[0],
                                                               pbmi2Printer);
                              byte bIndex1 = findNearestColor (pbmi2Bitmap->argbColor[0],
                                                               pbmi2Printer);

                              for (int x = iStartX; x <= iEndX; x++)
                              {
                                 if (pbBitmapScanline[x >> 3] & abMask[x & 7])
                                 {
                                    pbPrinterScanline[x] = bIndex1;
                                 }
                                 else
                                 {
                                    pbPrinterScanline[x] = bIndex0;
                                 }
                              }
                              break;
                           }

#ifdef INCLUDE_2BPP
                           case 2:
                           {
                              // @TBD
                              break;
                           }
#endif

#ifdef INCLUDE_4BPP
                           case 4:
                           {
                              // @TBD
                              break;
                           }
#endif
                           }
                           break;
                        }

#ifdef INCLUDE_16BPP
                        case 16:
                        {
                           // @TBD
                           break;
                        }
#endif

                        case 24:
                        {
                           switch (pbmi2Bitmap->cBitCount)
                           {
                           case 1:
                           {
                              static byte abMask[] = {
                                 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
                              };
                              byte bR0 = pbmi2Bitmap->argbColor[0].bRed;
                              byte bG0 = pbmi2Bitmap->argbColor[0].bGreen;
                              byte bB0 = pbmi2Bitmap->argbColor[0].bBlue;
                              byte bR1 = pbmi2Bitmap->argbColor[1].bRed;
                              byte bG1 = pbmi2Bitmap->argbColor[1].bGreen;
                              byte bB1 = pbmi2Bitmap->argbColor[1].bBlue;
                              int iPos = iStartX * 3;

                              for (int x = iStartX; x <= iEndX; x++)
                              {
                                 if (pbBitmapScanline[x >> 3] & abMask[x & 7])
                                 {
                                    pbPrinterScanline[iPos++] = bB1;
                                    pbPrinterScanline[iPos++] = bG1;
                                    pbPrinterScanline[iPos++] = bR1;
                                 }
                                 else
                                 {
                                    pbPrinterScanline[iPos++] = bB0;
                                    pbPrinterScanline[iPos++] = bG0;
                                    pbPrinterScanline[iPos++] = bR0;
                                 }
                              }
                              break;
                           }

#ifdef INCLUDE_2BPP
                           case 2:
                           {
                              // @TBD
                              break;
                           }
#endif

#ifdef INCLUDE_4BPP
                           case 4:
                           {
                              // @TBD
                              break;
                           }
#endif

                           case 8:
                           {
                              int iPos = iStartX * 3;

                              for (int x = iStartX; x <= iEndX; x++)
                              {
                                 RGB2 rgb = pbmi2Bitmap->argbColor[pbBitmapScanline[x]];

                                 pbPrinterScanline[iPos++] = rgb.bBlue;
                                 pbPrinterScanline[iPos++] = rgb.bGreen;
                                 pbPrinterScanline[iPos++] = rgb.bRed;
                              }
                              break;
                           }

#ifdef INCLUDE_16BPP
                           case 16:
                           {
                              // @TBD
                              break;
                           }
#endif
                           }
                           break;
                        }
                        }
                     }
                     else // (pbmi2Printer->cBitCount < pbmi2Bitmap->cBitCount)
                     {
                        // Shrink the bits when copying
                        switch (pbmi2Printer->cBitCount)
                        {
                        case 1:
                        {
                           switch (pbmi2Bitmap->cBitCount)
                           {
                           case 24:
                           {
//                            ImageInfo.pptlSrc->y =
//                            ImageInfo.pptlDst->y = iYBand;                                       // exclusive??
//                            ImageInfo.pbSrcBits  = pBitmapTop - ((iYBand-1) * uiBytesPerLine);   // pointer to src image data
//                            ImageInfo.pbTrgBits  = pMonoData;                                    // pointer to dst image data
//
//                            DiffusionDither (&DiffInfo, &ImageInfo, &pBuffer, 0);
                              break;
                           }
                           }
                           break;
                        }

#ifdef INCLUDE_2BPP
                        case 2:
                        {
                           // @TBD
                           break;
                        }
#endif

#ifdef INCLUDE_4BPP
                        case 4:
                        {
                           // @TBD
                           break;
                        }
#endif

                        case 8:
                        {
                           // @TBD
                           break;
                        }

#ifdef INCLUDE_16BPP
                        case 16:
                        {
                           // @TBD
                           break;
                        }
#endif

                        case 24:
                        {
                           // Impossible
                           break;
                        }
                        }
                        // @TBD
                     }
                  }

                  // Post-work
                  if (  24 == pbmi2Bitmap->cBitCount
                     &&  1 == pbmi2Printer->cBitCount
                     )
                  {
//                   Not done since we dont want pBuffer freed!
//                   DiffusionDither (&DiffInfo, &ImageInfo, &pBuffer, GDM_DITHER_END);
                  }

                  // Clean up
                  rc = munmap (pvAddress, iLength + iBump);

#ifndef RETAIL
                  if (DebugOutput::shouldOutputOmniProxy ()) DebugOutput::getErrorStream () << "OmniProxy: munmap returns " << std::dec << rc << std::endl;
#endif
               }
            }

            // Move to the next record
            pCurrent++;
            iCount--;
         }

         // Move to the next header
         pStart = pStart->pNext;

      } while (pStart);

      // Send the completed bitmap to the printer
      pDevice_d->rasterize (pbPrinterBits,
                            pbmi2Printer,
                            &rectlPageLocation,
                            BITBLT_BITMAP);
   }

done:
   // Clean up!
   freeBitmapInfo ();

   if (pbmi2Printer)
   {
      free (pbmi2Printer);
      pbmi2Printer = 0;
   }

   if (pbPrinterBits)
   {
      free (pbPrinterBits);
      pbPrinterBits = 0;
   }
#endif
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void OmniProxy::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string OmniProxy::
toString (std::ostringstream& oss)
{
      oss << "{OmniProxy: pDevice_d = " << *pDevice_d
        << ", pHeadersRoot_d = 0x" << std::hex << reinterpret_cast<uintptr_t>(pHeadersRoot_d) << std::dec
        << ", pHeadersCurrent_d = 0x" << reinterpret_cast<uintptr_t>(pHeadersCurrent_d)
        << ", pHeaderCurrent_d = 0x" << reinterpret_cast<uintptr_t>(pHeaderCurrent_d)
        << ", fp_d = 0x" << reinterpret_cast<uintptr_t>(fp_d)
        << "}";

   return oss.str ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool OmniProxy::
hasError ()
{
   return pDevice_d->hasError ();
}

void OmniProxy::
initialize ()
{
   pDevice_d->initialize ();
}

PSZCRO OmniProxy::
getVersion ()
{
   return pDevice_d->getVersion ();
}

PSZCRO OmniProxy::
getDriverName ()
{
   return pDevice_d->getDriverName ();
}

PSZCRO OmniProxy::
getDeviceName ()
{
   return pDevice_d->getDeviceName ();
}

PSZCRO OmniProxy::
getShortName ()
{
   return pDevice_d->getShortName ();
}

PSZCRO OmniProxy::
getLibraryName ()
{
   return pDevice_d->getLibraryName ();
}

EOMNICLASS OmniProxy::
getOmniClass ()
{
   return pDevice_d->getOmniClass ();
}

bool OmniProxy::
hasCapability (long lMask)
{
   return pDevice_d->hasCapability (lMask);
}

bool OmniProxy::
hasRasterCapability (long lMask)
{
   return pDevice_d->hasRasterCapability (lMask);
}

bool OmniProxy::
hasDeviceOption (PSZCRO pszDeviceOption)
{
   return pDevice_d->hasDeviceOption (pszDeviceOption);
}

int OmniProxy::
getScanlineMultiple ()
{
   return pDevice_d->getScanlineMultiple ();
}

#ifdef INCLUDE_JP_UPDF_BOOKLET
DeviceBooklet * OmniProxy::
getCurrentBooklet ()
{
   return pDevice_d->getCurrentBooklet ();
}
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
DeviceCopies * OmniProxy::
getCurrentCopies ()
{
   return pDevice_d->getCurrentCopies ();
}
#endif

PSZCRO OmniProxy::
getCurrentDitherID ()
{
   return pDevice_d->getCurrentDitherID ();
}

DeviceForm * OmniProxy::
getCurrentForm ()
{
   return pDevice_d->getCurrentForm ();
}

#ifdef INCLUDE_JP_UPDF_JOGGING
DeviceJogging * OmniProxy::
getCurrentJogging ()
{
   return pDevice_d->getCurrentJogging ();
}
#endif

DeviceMedia * OmniProxy::
getCurrentMedia ()
{
   return pDevice_d->getCurrentMedia ();
}

#ifdef INCLUDE_JP_COMMON_NUP
DeviceNUp * OmniProxy::
getCurrentNUp ()
{
   return pDevice_d->getCurrentNUp ();
}
#endif

DeviceOrientation * OmniProxy::
getCurrentOrientation ()
{
   return pDevice_d->getCurrentOrientation ();
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
DeviceOutputBin * OmniProxy::
getCurrentOutputBin ()
{
   return pDevice_d->getCurrentOutputBin ();
}
#endif

DevicePrintMode * OmniProxy::
getCurrentPrintMode ()
{
   return pDevice_d->getCurrentPrintMode ();
}

DeviceResolution * OmniProxy::
getCurrentResolution ()
{
   return pDevice_d->getCurrentResolution ();
}

#ifdef INCLUDE_JP_COMMON_SCALING
DeviceScaling * OmniProxy::
getCurrentScaling ()
{
   return pDevice_d->getCurrentScaling ();
}
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
DeviceSheetCollate * OmniProxy::
getCurrentSheetCollate ()
{
   return pDevice_d->getCurrentSheetCollate ();
}
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
DeviceSide * OmniProxy::
getCurrentSide ()
{
   return pDevice_d->getCurrentSide ();
}
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
DeviceStitching * OmniProxy::
getCurrentStitching ()
{
   return pDevice_d->getCurrentStitching ();
}
#endif

DeviceTray * OmniProxy::
getCurrentTray ()
{
   return pDevice_d->getCurrentTray ();
}

#ifdef INCLUDE_JP_COMMON_TRIMMING
DeviceTrimming * OmniProxy::
getCurrentTrimming ()
{
   return pDevice_d->getCurrentTrimming ();
}
#endif

DeviceGamma * OmniProxy::
getCurrentGamma ()
{
   return pDevice_d->getCurrentGamma ();
}

DeviceInstance * OmniProxy::
getInstance ()
{
   return pDevice_d->getInstance ();
}

int OmniProxy::
getPDLLevel ()
{
   return pDevice_d->getPDLLevel ();
}

int OmniProxy::
getPDLSubLevel ()
{
   return pDevice_d->getPDLSubLevel ();
}

int OmniProxy::
getPDLMajorRevisionLevel ()
{
   return pDevice_d->getPDLMajorRevisionLevel ();
}

int OmniProxy::
getPDLMinorRevisionLevel ()
{
   return pDevice_d->getPDLMinorRevisionLevel ();
}

void OmniProxy::
setOutputStream (FILE *pFile)
{
   return pDevice_d->setOutputStream (pFile);
}

void OmniProxy::
setOutputFunction (PFNOUTPUTFUNCTION  pfnOutputFunction,
                   void              *pMagicCookie)
{
   pDevice_d->setOutputFunction (pfnOutputFunction,
                                 pMagicCookie);
}

void OmniProxy::
setErrorStream (FILE *pFile)
{
   return pDevice_d->setErrorStream (pFile);
}

bool OmniProxy::
setLanguage (int iLanguageID)
{
   return pDevice_d->setLanguage (iLanguageID);
}

int OmniProxy::
getLanguage ()
{
   return pDevice_d->getLanguage ();
}

Enumeration * OmniProxy::
getLanguages ()
{
   return pDevice_d->getLanguages ();
}

StringResource * OmniProxy::
getLanguageResource ()
{
   return pDevice_d->getLanguageResource ();
}

std::string * OmniProxy::
getJobProperties (bool fInDeviceSpecific)
{
   return pDevice_d->getJobProperties (fInDeviceSpecific);
}

bool OmniProxy::
setJobProperties (PSZCRO pszJobProperties)
{
   return pDevice_d->setJobProperties (pszJobProperties);
}

Enumeration * OmniProxy::
listJobProperties (bool fInDeviceSpecific)
{
   return pDevice_d->listJobProperties (fInDeviceSpecific);
}

std::string * OmniProxy::
getJobPropertyType (PSZCRO pszKey)
{
   return pDevice_d->getJobPropertyType (pszKey);
}

std::string * OmniProxy::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   return pDevice_d->translateKeyValue (pszKey, pszValue);
}

std::string * OmniProxy::
getJobProperty (PSZCRO pszKey)
{
   return pDevice_d->getJobProperty (pszKey);
}
