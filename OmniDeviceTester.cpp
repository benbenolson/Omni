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
#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <list>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "Omni.hpp"
#include "JobProperties.hpp"
#include "defines.hpp"

#include <glib.h>
#include <gmodule.h>

// typedef unsigned int ULONG;
typedef short int SHORT;
typedef unsigned short int USHORT;

/* bitmap parameterization used by GpiCreateBitmap and others */
typedef struct _BITMAPINFOHEADER         /* bmp */
{
   ULONG  cbFix;
   USHORT cx;
   USHORT cy;
   USHORT cPlanes;
   USHORT cBitCount;
} BITMAPINFOHEADER;
typedef BITMAPINFOHEADER *PBITMAPINFOHEADER;

///* RGB data for _BITMAPINFO struct */
//typedef struct _RGB              /* rgb */
//{
//   BYTE bBlue;
//   BYTE bGreen;
//   BYTE bRed;
//} RGB;

/* bitmap data used by GpiSetBitmapBits and others */
typedef struct _BITMAPINFO       /* bmi */
{
   ULONG  cbFix;
   USHORT cx;
   USHORT cy;
   USHORT cPlanes;
   USHORT cBitCount;
   RGB    argbColor[1];
} BITMAPINFO;
typedef BITMAPINFO *PBITMAPINFO;

/* Constants for compression/decompression command */

#define CBD_COMPRESSION     1L
#define CBD_DECOMPRESSION   2L
#define CBD_BITS            0L

/* Flags for compression/decompression option */

#define CBD_COLOR_CONVERSION    0x00000001L

/* Compression scheme in the ulCompression field of the bitmapinfo structure */

#define BCA_UNCOMP          0L
#define BCA_HUFFMAN1D       3L
#define BCA_RLE4            2L
#define BCA_RLE8            1L
#define BCA_RLE24           4L

#define BRU_METRIC          0L

#define BRA_BOTTOMUP        0L

#define BRH_NOTHALFTONED    0L
#define BRH_ERRORDIFFUSION  1L
#define BRH_PANDA           2L
#define BRH_SUPERCIRCLE     3L

#define BCE_PALETTE         (-1L)
#define BCE_RGB             0L

typedef struct _BITMAPINFOHEADER2        /* bmp2  */
{
   ULONG  cbFix;            /* Length of structure                    */
   ULONG  cx;               /* Bit-map width in pels                  */
   ULONG  cy;               /* Bit-map height in pels                 */
   USHORT cPlanes;          /* Number of bit planes                   */
   USHORT cBitCount;        /* Number of bits per pel within a plane  */
   ULONG  ulCompression;    /* Compression scheme used to store the bitmap */
   ULONG  cbImage;          /* Length of bit-map storage data in bytes*/
   ULONG  cxResolution;     /* x resolution of target device          */
   ULONG  cyResolution;     /* y resolution of target device          */
   ULONG  cclrUsed;         /* Number of color indices used           */
   ULONG  cclrImportant;    /* Number of important color indices      */
   USHORT usUnits;          /* Units of measure                       */
   USHORT usReserved;       /* Reserved                               */
   USHORT usRecording;      /* Recording algorithm                    */
   USHORT usRendering;      /* Halftoning algorithm                   */
   ULONG  cSize1;           /* Size value 1                           */
   ULONG  cSize2;           /* Size value 2                           */
   ULONG  ulColorEncoding;  /* Color encoding                         */
   ULONG  ulIdentifier;     /* Reserved for application use           */
} BITMAPINFOHEADER2;
typedef BITMAPINFOHEADER2 *PBITMAPINFOHEADER2;

//typedef struct _RGB2         /* rgb2 */
//{
//   BYTE bBlue;              /* Blue component of the color definition */
//   BYTE bGreen;             /* Green component of the color definition*/
//   BYTE bRed;               /* Red component of the color definition  */
//   BYTE fcOptions;          /* Reserved, must be zero                 */
//} RGB2;
//typedef RGB2 *PRGB2;

//typedef struct _BITMAPINFO2      /* bmi2 */
//{
//   ULONG  cbFix;            /* Length of fixed portion of structure   */
//   ULONG  cx;               /* Bit-map width in pels                  */
//   ULONG  cy;               /* Bit-map height in pels                 */
//   USHORT cPlanes;          /* Number of bit planes                   */
//   USHORT cBitCount;        /* Number of bits per pel within a plane  */
//   ULONG  ulCompression;    /* Compression scheme used to store the bitmap */
//   ULONG  cbImage;          /* Length of bit-map storage data in bytes*/
//   ULONG  cxResolution;     /* x resolution of target device          */
//   ULONG  cyResolution;     /* y resolution of target device          */
//   ULONG  cclrUsed;         /* Number of color indices used           */
//   ULONG  cclrImportant;    /* Number of important color indices      */
//   USHORT usUnits;          /* Units of measure                       */
//   USHORT usReserved;       /* Reserved                               */
//   USHORT usRecording;      /* Recording algorithm                    */
//   USHORT usRendering;      /* Halftoning algorithm                   */
//   ULONG  cSize1;           /* Size value 1                           */
//   ULONG  cSize2;           /* Size value 2                           */
//   ULONG  ulColorEncoding;  /* Color encoding                         */
//   ULONG  ulIdentifier;     /* Reserved for application use           */
//   RGB2   argbColor[1];     /* Color definition record                */
//} BITMAPINFO2;
//typedef BITMAPINFO2 *PBITMAPINFO2;

typedef struct _BITMAPFILEHEADER    /* bfh */
{
   USHORT    usType;
   ULONG     cbSize;
   SHORT     xHotspot;
   SHORT     yHotspot;
   ULONG     offBits;
   BITMAPINFOHEADER bmp;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER *PBITMAPFILEHEADER;

typedef struct _BITMAPARRAYFILEHEADER       /* bafh */
{
   USHORT    usType;
   ULONG     cbSize;
   ULONG     offNext;
   USHORT    cxDisplay;
   USHORT    cyDisplay;
   BITMAPFILEHEADER bfh;
} BITMAPARRAYFILEHEADER;
typedef BITMAPARRAYFILEHEADER *PBITMAPARRAYFILEHEADER;

typedef struct _BITMAPFILEHEADER2    /* bfh2 */
{
   USHORT    usType;
   ULONG     cbSize;
   SHORT     xHotspot;
   SHORT     yHotspot;
   ULONG     offBits;
   BITMAPINFOHEADER2 bmp2;
} BITMAPFILEHEADER2;
typedef BITMAPFILEHEADER2 *PBITMAPFILEHEADER2;

typedef struct _BITMAPARRAYFILEHEADER2    /* bafh2 */
{
   USHORT    usType;
   ULONG     cbSize;
   ULONG     offNext;
   USHORT    cxDisplay;
   USHORT    cyDisplay;
   BITMAPFILEHEADER2 bfh2;
} BITMAPARRAYFILEHEADER2;
typedef BITMAPARRAYFILEHEADER2 *PBITMAPARRAYFILEHEADER2;

/*************************************************************************
* These are the identifying values that go in the usType field of the
* BITMAPFILEHEADER(2) and BITMAPARRAYFILEHEADER(2).
* (BFT_ => Bit map File Type)
*************************************************************************/
#define BFT_ICON           0x4349   /* 'IC' */
#define BFT_BMAP           0x4d42   /* 'BM' */
#define BFT_POINTER        0x5450   /* 'PT' */
#define BFT_COLORICON      0x4943   /* 'CI' */
#define BFT_COLORPOINTER   0x5043   /* 'CP' */
#define BFT_BITMAPARRAY    0x4142   /* 'BA' */

void
bitblt (FILE   *fp,
        USHORT  usType,
        int     iMaxScanLines,
        Device *pDevice)
{
   PBYTE              pbBits              = 0;
   PBITMAPINFO2       pbmi2Printer        = 0;
   SIZEL              sizelPage           = {0, 0};
   RECTL              rectlPageLocation   = {0, 0, 0, 0};
   ULONG              cbFix;
   BITMAPFILEHEADER   bfhBitmap;
   BITMAPINFOHEADER2  bmih2Bitmap;
   int                iNumColorsBitmap;
   int                iNumColorsPrinter;
   int                iBytesToAlloc;
   byte               bWhiteIndex         = 0;
   byte               bFillColor;
   int                icbBitmapScanLine;
   int                icbPrinterScanLine;
   int                iScanlineMultiple;
   int                iNumBands;
   int                iCurrentBand;
   int                rc;

   if (!fp)
      return;

   HardCopyCap      *pHCC       = pDevice->getCurrentForm ()->getHardCopyCap ();
   DevicePrintMode  *pPrintMode = pDevice->getCurrentPrintMode ();

   /* Bump up the # of scanlines that we send to the device to be a
   ** multiple of what it requests.
   */
   iScanlineMultiple = pDevice->getScanlineMultiple ();
   if (0 != (iMaxScanLines % iScanlineMultiple))
   {
      iMaxScanLines += iScanlineMultiple - (iMaxScanLines % iScanlineMultiple);
   }
   iNumBands = (pHCC->getYPels () + iMaxScanLines - 1) / iMaxScanLines;

   bfhBitmap.usType = usType;

   rc = fread (&bfhBitmap.cbSize,
               ( sizeof (bfhBitmap.cbSize)
               + sizeof (bfhBitmap.xHotspot)
               + sizeof (bfhBitmap.yHotspot)
               + sizeof (bfhBitmap.offBits)
               ),
               1,
               fp);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bfhBitmap.usType   = 0x" << std::hex << bfhBitmap.usType << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bfhBitmap.cbSize   = 0x" << bfhBitmap.cbSize << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bfhBitmap.xHotspot = 0x" << bfhBitmap.xHotspot << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bfhBitmap.yHotspot = 0x" << bfhBitmap.yHotspot << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bfhBitmap.offBits  = 0x" << bfhBitmap.offBits << std::dec << std::endl;
   }
#endif

   rc = fread (&cbFix, sizeof (cbFix), 1, fp);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": sizeof (BITMAPINFOHEADER) = " << sizeof (BITMAPINFOHEADER) << std::endl;
#endif

   if (sizeof (BITMAPINFOHEADER) >= cbFix)
   {
      // The format is BITMAPINFOHEAHER
      // @TBD
   }
   else
   {
      // The format is BITMAPINFOHEADER2
#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cbFix           = 0x" << std::hex << cbFix << std::dec << std::endl;
#endif
      bmih2Bitmap.cbFix = cbFix;
      rc = fread (&bmih2Bitmap.cx, bmih2Bitmap.cbFix - sizeof (bmih2Bitmap.cbFix), 1, fp);

#ifndef RETAIL
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cy))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cx              = " << bmih2Bitmap.cx << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cPlanes))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cy              = " << bmih2Bitmap.cy << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cBitCount))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cPlanes         = " << bmih2Bitmap.cPlanes << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, ulCompression))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cBitCount       = " << bmih2Bitmap.cBitCount << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cbImage))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   ulCompression   = " << bmih2Bitmap.ulCompression << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cxResolution))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cbImage         = " << bmih2Bitmap.cbImage << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cyResolution))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cxResolution    = " << bmih2Bitmap.cxResolution << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cclrUsed))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cyResolution    = " << bmih2Bitmap.cyResolution << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cclrImportant))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cclrUsed        = " << bmih2Bitmap.cclrUsed << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, usUnits))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cclrImportant   = " << bmih2Bitmap.cclrImportant << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, usReserved))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   usUnits         = " << bmih2Bitmap.usUnits << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, usRecording))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   usReserved      = " << bmih2Bitmap.usReserved << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cSize1))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   usRendering     = " << bmih2Bitmap.usRendering << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, cSize2))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cSize1          = " << bmih2Bitmap.cSize1 << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, ulColorEncoding))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   cSize2          = " << bmih2Bitmap.cSize2 << std::endl;
      if (bmih2Bitmap.cbFix >= FIELDOFFSET (BITMAPINFOHEADER2, ulIdentifier))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   ulColorEncoding = " << bmih2Bitmap.ulColorEncoding << std::endl;
      if (bmih2Bitmap.cbFix >= sizeof (BITMAPINFOHEADER2))
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ":   ulIdentifier    = " << bmih2Bitmap.ulIdentifier << std::endl;
#endif

      if (  !bmih2Bitmap.cclrUsed
         && 8 >= bmih2Bitmap.cBitCount
         )
      {
         // Fix the number of colors specified in the color table
         bmih2Bitmap.cclrUsed      = 1 << bmih2Bitmap.cBitCount;
         bmih2Bitmap.cclrImportant = 0;                    // All are important
      }
   }

   iNumColorsBitmap  = 1 << bmih2Bitmap.cBitCount;
   iNumColorsPrinter = 1 << pPrintMode->getLogicalCount ();

   // Allocate a full bitmap info 2 and bitmap header message
   iBytesToAlloc = sizeof (BITMAPINFO2);
   if (256 >= iNumColorsPrinter)
      iBytesToAlloc += (iNumColorsPrinter - 1) * sizeof (RGB2);

   pbmi2Printer = (PBITMAPINFO2)calloc (1, iBytesToAlloc);
   if (!pbmi2Printer)
      goto done;

   // Initialize it
   pbmi2Printer->cbFix         = sizeof (BITMAPINFO2)
                               - sizeof (((PBITMAPINFO2)0)->argbColor)
                               ;
   pbmi2Printer->cx            = pHCC->getXPels ();
   pbmi2Printer->cy            = iMaxScanLines;
   pbmi2Printer->cPlanes       = pPrintMode->getNumPlanes ();
   pbmi2Printer->cBitCount     = pPrintMode->getLogicalCount ();
   pbmi2Printer->cclrUsed      = 1 << pbmi2Printer->cBitCount;
   pbmi2Printer->cclrImportant = 0;                            // All are important

   // Set up the color table
   switch (pbmi2Printer->cBitCount)
   {
   case 1:
   {
      pbmi2Printer->cbFix += 2 * sizeof (RGB2);
      pbmi2Printer->argbColor[0].bRed   = 0xFF;
      pbmi2Printer->argbColor[0].bGreen = 0xFF;
      pbmi2Printer->argbColor[0].bBlue  = 0xFF;
      pbmi2Printer->argbColor[1].bRed   = 0x00;
      pbmi2Printer->argbColor[1].bGreen = 0x00;
      pbmi2Printer->argbColor[1].bBlue  = 0x00;
      bWhiteIndex = 0;
      break;
   }

   case 2:
   case 4:
   case 8:
   {
      // @TBD
      break;
   }
   }

   // Read the color table, if there is one
   if (  256 >= iNumColorsBitmap
      && 256 >= iNumColorsPrinter
      )
   {
      if (sizeof (BITMAPINFOHEADER) >= cbFix)
      {
         /* We have to do this the hard way and read in the old RGB on top
         ** of the new RGB2
         */
         for (int i = 0; i < iNumColorsBitmap; i++)
         {
            rc = fread (&pbmi2Printer->argbColor[i], sizeof (RGB), 1, fp);
            if (  0xFF == pbmi2Printer->argbColor[i].bRed
               && 0xFF == pbmi2Printer->argbColor[i].bGreen
               && 0xFF == pbmi2Printer->argbColor[i].bBlue
               )
            {
               bWhiteIndex = i;
            }
         }
      }
      else
      {
         rc = fread (pbmi2Printer->argbColor, iNumColorsBitmap * sizeof (RGB2), 1, fp);

         for (int i = 0; i < iNumColorsBitmap; i++)
         {
            if (  0xFF == pbmi2Printer->argbColor[i].bRed
               && 0xFF == pbmi2Printer->argbColor[i].bGreen
               && 0xFF == pbmi2Printer->argbColor[i].bBlue
               )
            {
               bWhiteIndex = i;
               break;
            }
         }
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ())
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bWhiteIndex = " << (int)bWhiteIndex << std::endl;
      }
#endif
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bmih2Bitmap = { "
           << "cx = " << bmih2Bitmap.cx
           << ", cy = " << bmih2Bitmap.cy
           << ", cPlanes = " << bmih2Bitmap.cPlanes
           << ", cBitCount = " << bmih2Bitmap.cBitCount
           << ", cclrUsed = " << bmih2Bitmap.cclrUsed
           << ", cclrImportant = " << bmih2Bitmap.cclrImportant
           << " }" << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": bmi2Printer = { "
           << "cx = " << pbmi2Printer->cx
           << ", cy = " << pbmi2Printer->cy
           << ", cPlanes = " << pbmi2Printer->cPlanes
           << ", cBitCount = " << pbmi2Printer->cBitCount
           << ", cclrUsed = " << pbmi2Printer->cclrUsed
           << ", cclrImportant = " << pbmi2Printer->cclrImportant
           << " }" << std::endl;
   }
#endif

   // Scan lines are double word aligned (32-bits)
   icbPrinterScanLine = ((pbmi2Printer->cx * pbmi2Printer->cBitCount + 31) >> 5) << 2;
   icbBitmapScanLine  = ((bmih2Bitmap.cx   * bmih2Bitmap.cBitCount   + 31) >> 5) << 2;
   iBytesToAlloc      = icbPrinterScanLine * iMaxScanLines;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iBytesToAlloc = " << iBytesToAlloc <<  std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": icbPrinterScanLine = " << icbPrinterScanLine <<  std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": icbBitmapScanLine = " << icbBitmapScanLine <<  std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iMaxScanLines = " << iMaxScanLines << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iNumBands = " << iNumBands << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iScanlineMultiple = " << iScanlineMultiple << std::endl;
   }
#endif

   pbBits = (PBYTE)calloc (1, iBytesToAlloc);
   if (!pbBits)
      goto done;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": malloced " << iBytesToAlloc << " bytes at 0x" << std::hex << (int)pbBits << std::dec << std::endl;
#endif

   sizelPage.cx = pHCC->getXPels ();
   sizelPage.cy = pHCC->getYPels ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__
           << ": sizelPage = { " << sizelPage.cx << ", " << sizelPage.cy << " }"
           << std::endl;
   }
#endif

   // Determine the fill color
   bFillColor = 0xFF;

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

   case 2:
   {
      bFillColor = (bWhiteIndex << 6)
                 | (bWhiteIndex << 4)
                 | (bWhiteIndex << 2)
                 | bWhiteIndex;
      break;
   }

   case 4:
   {
      bFillColor = (bWhiteIndex << 4) | bWhiteIndex;
      break;
   }

   case 8:
   {
      bFillColor = bWhiteIndex;
      break;
   }

   case 16:
   case 24:
   {
      bFillColor = 0xFF;  // Don't use indexes
      break;
   }
   }

   // loop through the bands
   for (iCurrentBand = 0; iCurrentBand < iNumBands; iCurrentBand++)
   {
      // @TBD which way is it orientated?
      rectlPageLocation.xLeft   = 0;
      rectlPageLocation.xRight  = pbmi2Printer->cx - 1;
      rectlPageLocation.yTop    = (sizelPage.cy - 1) - iCurrentBand * iMaxScanLines;
      rectlPageLocation.yBottom = rectlPageLocation.yTop - iMaxScanLines + 1;

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ())
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Bitblt: pbBits = 0x" << std::hex << (int)pbBits
                     << ", bmi2Printer = 0x" << (int)pbmi2Printer
              << std::dec << ", sizelPage = { " << sizelPage.cx << ", " << sizelPage.cy << " }"
                     << ", rectlPageLocation = { (" << rectlPageLocation.xLeft
                     << ", " << rectlPageLocation.yBottom
                     << "), (" << rectlPageLocation.xRight
                     << ", " << rectlPageLocation.yTop
                     << ") }"
              << std::endl;
      }
#endif

      PBYTE pbScanLine = 0;

      // Read in the bitmap's scanlines
      for (int iScanLine = 0; iScanLine < iMaxScanLines; iScanLine++)
      {
         pbScanLine = pbBits + iScanLine * icbPrinterScanLine;

         // Erase the bitmap
         memset (pbScanLine, bFillColor, icbPrinterScanLine);

         // Read in a scanline
         if (  0 <= rectlPageLocation.yBottom + iScanLine
            && rectlPageLocation.yBottom + iScanLine < (int)bmih2Bitmap.cy
            )
         {
            int iBitmapScanline = bmih2Bitmap.cy
                                - ( rectlPageLocation.yBottom
                                  + iScanLine
                                  );

#ifndef RETAIL
////////////if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": seeking to scanline " << iBitmapScanline << std::endl;
#endif

            // Seek to the scanline in the file
            rc = fseek (fp,
                        -( ( iBitmapScanline
                           + 1
                           )
                         * icbBitmapScanLine
                         ),
                        SEEK_END);

            // Do we need to convert the bits?
            if (bmih2Bitmap.cBitCount == pbmi2Printer->cBitCount)
            {
               rc = fread (pbScanLine,
                           omni::min (icbPrinterScanLine,
                                      icbBitmapScanLine),
                           1,
                           fp);
            }
            else
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceTester ())
               {
                  DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Need to convert from "
                       << bmih2Bitmap.cBitCount
                       << " to "
                       << pbmi2Printer->cBitCount
                       << " bpp!" << std::endl;
               }
#endif

               // @TBD convert
            }
         }
      }

      // Send it to the device
      pDevice->rasterize (pbBits,
                          pbmi2Printer,
                          &rectlPageLocation,
                          BITBLT_BITMAP);
   }

done:
   // Clean up!
   if (pbmi2Printer)
      free (pbmi2Printer);

   if (pbBits)
      free (pbBits);
}

typedef struct _HDC {
   Device *pDevice;     // Device's handle
   PBYTE   pbScanLine;  // Scanline to draw into
   int     iCurrentY;   // scanline's y position
   RGB     rgbColor;

   // Cached items follow
   int     cx;          // PS's max x
   int     cy;          // PS's max y
   int     iBitsPerPel; // PS's color depth
   int     iXdpi;
   int     iYdpi;
} HDC, *PHDC;

void drawChar                (PHDC    pHDC,
                              int     iChar,
                              int     x,
                              int     y);
void drawChar                (PHDC    pHDC,
                              int     iChar,
                              int     x,
                              int     y,
                              int     cx,
                              int     cy);
void drawPel                 (PHDC    pHDC,
                              int     x);
void drawLine                (PHDC    pHDC,
                              int     iStartX,
                              int     iStartY,
                              int     iEndX,
                              int     iEndY);
void drawBox                 (PHDC    pHDC,
                              int     x,
                              int     y,
                              int     cx,
                              int     cy);
void drawBoxRoutine          (PHDC    pHDC);
void drawColorsRoutine       (PHDC    pHDC);
void drawPage                (int     iMaxScanLines,
                              Device *pDevice,
                              bool    fDrawBox,
                              bool    fDrawColors);

void
initializeHDC (HDC& hdc, Device *pDevice)
{
   hdc.pbScanLine = 0;
   hdc.iCurrentY  = 0;

   hdc.rgbColor.bRed   = 0x00;
   hdc.rgbColor.bGreen = 0x00;
   hdc.rgbColor.bBlue  = 0x00;

   HardCopyCap      *pHCC        = pDevice->getCurrentForm ()->getHardCopyCap ();
   DevicePrintMode  *pPrintMode  = pDevice->getCurrentPrintMode ();
   DeviceResolution *pResolution = pDevice->getCurrentResolution ();

   hdc.cx          = pHCC->getXPels ();
   hdc.cy          = pHCC->getYPels ();
   hdc.iBitsPerPel = pPrintMode->getLogicalCount ();
   hdc.iXdpi       = pResolution->getXRes ();
   hdc.iYdpi       = pResolution->getYRes ();
}

int
getCx (PHDC pHDC)
{
   return pHDC->cx;
}

int
getCy (PHDC pHDC)
{
   return pHDC->cy;
}

int
getXResolution (PHDC pHDC)
{
   return pHDC->iXdpi;
}

int
getYResolution (PHDC pHDC)
{
   return pHDC->iYdpi;
}

void
setColor (PHDC pHDC, RGB rgbColor)
{
   pHDC->rgbColor = rgbColor;
}

byte abCharData[][8][1] =
{
   {            //    0
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x30 }, // ..**....
      { 0x48 }, // .*..*...
      { 0x84 }, // *....*..
      { 0x84 }, // *....*..
      { 0x48 }, // .*..*...
      { 0x30 }  // ..**....
   },
   {            //    1
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x30 }, // ..**....
      { 0x70 }, // .***....
      { 0x30 }, // ..**....
      { 0x30 }, // ..**....
      { 0x30 }, // ..**....
      { 0xFC }  // ******..
   },
   {            //    2
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x78 }, // .****...
      { 0x84 }, // *....*..
      { 0x04 }, // .....*..
      { 0x38 }, // ..***...
      { 0xC0 }, // **......
      { 0xFC }  // ******..
   },
   {            //    3
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x78 }, // .****...
      { 0x84 }, // *....*..
      { 0x38 }, // ..***...
      { 0x04 }, // .....*..
      { 0x84 }, // *....*..
      { 0x78 }  // .****...
   },
   {            //    4
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x30 }, // ..**....
      { 0x50 }, // .*.*....
      { 0xFC }, // ******..
      { 0x10 }, // ...*....
      { 0x10 }, // ...*....
      { 0x10 }  // ...*....
   },
   {            //    5
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0xFC }, // ******..
      { 0x08 }, // *.......
      { 0xFC }, // *****...
      { 0x04 }, // .....*..
      { 0x84 }, // *....*..
      { 0x78 }  // .****...
   },
   {            //    6
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x30 }, // ..**....
      { 0x40 }, // .*......
      { 0xB8 }, // *.***...
      { 0xC4 }, // **...*..
      { 0x48 }, // .*..*...
      { 0x30 }  // ..**....
   },
   {            //    7
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0xFC }, // ******..
      { 0x04 }, // .....*..
      { 0x08 }, // ....*...
      { 0x10 }, // ...*....
      { 0x20 }, // ..*.....
      { 0x40 }  // .*......
   },
   {            //    8
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x78 }, // .****...
      { 0x84 }, // *....*..
      { 0x78 }, // .****...
      { 0x84 }, // *....*..
      { 0x84 }, // *....*..
      { 0x78 }  // .****...
   },
   {            //    9
      { 0x00 }, // ........
      { 0x00 }, // ........
      { 0x7C }, // .*****..
      { 0x84 }, // *....*..
      { 0x7C }, // .*****..
      { 0x04 }, // .....*..
      { 0x04 }, // .....*..
      { 0x04 }  // .....*..
   }
};

void
drawChar (PHDC pHDC,
          int  iChar,
          int  x,
          int  y)
{
   if (  '0' > iChar
      || '9' < iChar
      )
      // Out of range
      return;

   if (  pHDC->cx < x
      || 0 > x
      || pHDC->cy < y
      || 0 > y
      )
      // Off the page
      return;

   if (  y >= pHDC->iCurrentY
      && y <  pHDC->iCurrentY + 8
      )
   {
      switch (pHDC->iBitsPerPel)
      {
      case 1:
      {
         int iData = abCharData[iChar - '0'][y - pHDC->iCurrentY][0] << (8 - (x & 8));

         *(pHDC->pbScanLine + (x >> 3)    ) |= iData >> 8;
         *(pHDC->pbScanLine + (x >> 3) + 1) |= iData & 8;
         break;
      }

      case 24:
      {
         int   iData = abCharData[iChar - '0'][y - pHDC->iCurrentY][0] << (8 - (x & 8));
         PBYTE pbPel = pHDC->pbScanLine + x * 3;

         for (int iBit = (1 << 16); iBit; iBit >>= 1)
         {
            if (iData & iBit)
            {
               *pbPel++ = pHDC->rgbColor.bBlue;
               *pbPel++ = pHDC->rgbColor.bGreen;
               *pbPel++ = pHDC->rgbColor.bRed;
            }
            else
            {
               pbPel += 3;
            }
         }
         break;
      }

      default:
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": unknown bits per pel = " << pHDC->iBitsPerPel << std::endl;
#endif
         break;
      }
      }
   }
}

void
drawChar (PHDC pHDC,
          int  iChar,
          int  x,
          int  y,
          int  cx,
          int  cy)
{
   if (  '0' > iChar
      || '9' < iChar
      )
      // Out of range
      return;

   if (  pHDC->cx < x
      || 0 > x
      || pHDC->cy < y
      || 0 > y
      )
      // Off the page
      return;

   if (!(  y               <= pHDC->iCurrentY
        && pHDC->iCurrentY <= y + cy + 1
        )
      )
      // Doesn't intersect current line
      return;

   typedef struct _Point {
      int x;
      int y;
   } POINT, *PPOINT;

   int   iMidX      = x + cx / 2;
   int   iMidY      = y + cy / 2;
   int   iEndX      = x + cx;
   int   iEndY      = y + cy;
   POINT aPoints[9] = {               // The points are located as follows:
      {     x, iEndY },               //       0-----1-----2 (x+cx,y+cy)
      { iMidX, iEndY },               //       |     |     |
      { iEndX, iEndY },               //       |     |     |
      {     x, iMidY },               //       |     |     |
      { iMidX, iMidY },               //       3-----4-----5
      { iEndX, iMidY },               //       |     |     |
      {     x,     y },               //       |     |     |
      { iMidX,     y },               //       |     |     |
      { iEndX,     y }                // (x,y) 6-----7-----8
   };

   // Draw a bolder character bye drawing the same character over a number of
   // passes.
   for (int iPass = 0; iPass <= 1; iPass++)
   {
      // Draw the caracter
      switch (iChar)
      {
      case '0':
      {
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[8].x, aPoints[8].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[0].x, aPoints[0].y);
         break;
      }

      case '1':
      {
         drawLine (pHDC, aPoints[7].x, aPoints[7].y, aPoints[1].x, aPoints[1].y);
         break;
      }

      case '2':
      {
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[5].x, aPoints[5].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[3].x, aPoints[3].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[8].x, aPoints[8].y);
         break;
      }

      case '3':
      {
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[8].x, aPoints[8].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         break;
      }

      case '4':
      {
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[0].x, aPoints[0].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[2].x, aPoints[2].y);
         break;
      }

      case '5':
      {
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[0].x, aPoints[0].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[5].x, aPoints[5].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[8].x, aPoints[8].y);
         break;
      }

      case '6':
      {
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[0].x, aPoints[0].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[8].x, aPoints[8].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[5].x, aPoints[5].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         break;
      }

      case '7':
      {
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[2].x, aPoints[2].y);
         break;
      }

      case '8':
      {
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[8].x, aPoints[8].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[6].x, aPoints[6].y, aPoints[0].x, aPoints[0].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         break;
      }

      case '9':
      {
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[5].x, aPoints[5].y);
         drawLine (pHDC, aPoints[3].x, aPoints[3].y, aPoints[0].x, aPoints[0].y);
         drawLine (pHDC, aPoints[0].x, aPoints[0].y, aPoints[2].x, aPoints[2].y);
         drawLine (pHDC, aPoints[8].x, aPoints[8].y, aPoints[2].x, aPoints[2].y);
         break;
      }
      }

      // More passes left?
      if (iPass < 1)
      {
         // Simulate bold by moving the points.
         for (int i = 0; i < (int)dimof (aPoints); i++)
         {
            aPoints[i].x++;
            aPoints[i].y++;
         }
      }
   }
}

void
drawPel (PHDC pHDC,
         int  x)
{
   if (  x < 0
      || x > pHDC->cx
      )
      // Error!
      return;

   switch (pHDC->iBitsPerPel)
   {
   case 1:
   {
      static BYTE abPel[] = {
         0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
      };

      *(pHDC->pbScanLine + (x >> 3)) |= abPel[x & 7];
      break;
   }

   case 24:
   {
      PBYTE pbPel = pHDC->pbScanLine + x * 3;

      *pbPel++ = pHDC->rgbColor.bBlue;
      *pbPel++ = pHDC->rgbColor.bGreen;
      *pbPel++ = pHDC->rgbColor.bRed;
      break;
   }

   default:
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": unknown bits per pel = " << pHDC->iBitsPerPel << std::endl;
#endif
      break;
   }
   }
}

void
drawLine (PHDC pHDC,
          int  iStartX,
          int  iStartY,
          int  iEndX,
          int  iEndY)
{
   // Order the points if necessary
   if (iEndX < iStartX)
   {
      int iTemp = iStartX;

      iStartX = iEndX;
      iEndX   = iTemp;
   }
   if (iEndY < iStartY)
   {
      int iTemp = iStartY;

      iStartY = iEndY;
      iEndY   = iTemp;
   }

   // Does it intersect the scanline on the x axis?
   if (  0 > iEndX
      || iStartX > pHDC->cx
      )
      return;

   // Clip the x axis
   if (0 > iStartX)
   {
      iStartX = 0;
   }
   if (pHDC->cx < iEndX)
   {
      iEndX = pHDC->cx - 1;
   }

   // Does it intersect the scanline on the y axis?
   if (  iStartY <= pHDC->iCurrentY
      && pHDC->iCurrentY <= iEndY
      )
   {
      if (iStartY == iEndY)
      {
         iStartY = omni::max (0, iStartY);
         iEndY   = omni::min (iEndY, pHDC->cy - 1);

         // Horizontal line
         for (int i = iStartX; i <= iEndX; i++)
            drawPel (pHDC, i);
      }
      else if (iStartX == iEndX)
      {
         // Vertical line
         if (  0 <= iStartX
            && iStartX <= pHDC->cx - 1
            )
         {
            drawPel (pHDC, iStartX);
         }
      }
      else
      {
         // @TDB
      }
   }
}

void
drawBox (PHDC pHDC,
         int  x,
         int  y,
         int  cx,
         int  cy)
{
   if (y == pHDC->iCurrentY)
   {
      drawLine (pHDC, x, y, x + cx - 1, y);
   }
   else if (y + cy - 1 == pHDC->iCurrentY)
   {
      drawLine (pHDC, x, y + cy - 1, x + cx - 1, y + cy - 1);
   }
   else if (  y < pHDC->iCurrentY
           && pHDC->iCurrentY < y + cy - 1
           )
   {
      drawPel (pHDC, x);
      drawPel (pHDC, x + cx - 1);
   }
}

void
drawBoxRoutine (PHDC pHDC)
{
   int cx           = getCx (pHDC);
   int cy           = getCy (pHDC);
   int iLineLengthX = getXResolution (pHDC) / 4;                         // 1/4"
   int iLineLengthY = getXResolution (pHDC) / 4;                         // 1/4"
   int iSpacer      = (3 * omni::min (iLineLengthX, iLineLengthY)) / 10; // 30%
   int iNumber      = 1;

   // Draw a box around the printable area
   drawBox (pHDC, 0, 0, cx, cy);

   // Draw lines up the left side of the page
   for (int y = 0, x = cx - 1; y < cy + iLineLengthY; y += iLineLengthY)
   {
      drawLine (pHDC, x, y, x, y + iLineLengthY);

      if (0 < iNumber / 10)
         drawChar (pHDC,
                   (iNumber / 10) + '0',
                   x - 2 * iLineLengthX - iSpacer,
                   y,
                   iLineLengthX - iSpacer,
                   iLineLengthY - iSpacer);
      drawChar (pHDC,
                (iNumber - 10 * (iNumber / 10)) + '0',
                x - iLineLengthX - iSpacer,
                y,
                iLineLengthX - iSpacer,
                iLineLengthY - iSpacer);

      x--;
      iNumber++;
   }

   iNumber = 1;

   // Draw lines across the bottom of the page
   for (int x = cx, y = 0; x > -iLineLengthX; x -= iLineLengthX)
   {
      drawLine (pHDC, x - iLineLengthX, y, x, y);

      if (0 < iNumber / 10)
         drawChar (pHDC,
                   (iNumber / 10) + '0',
                   x - iLineLengthX,
                   y + 2 * iLineLengthY + iSpacer,
                   iLineLengthX - iSpacer,
                   iLineLengthY - iSpacer);
      drawChar (pHDC,
                (iNumber - 10 * (iNumber / 10)) + '0',
                x - iLineLengthX,
                y + iLineLengthY + iSpacer,
                iLineLengthX - iSpacer,
                iLineLengthY - iSpacer);

      y++;
      iNumber++;
   }
}

void
drawColorsRoutine (PHDC pHDC)
{
   int   cx            = getCx (pHDC);
   int   cy            = getCy (pHDC);
   int   iSizeBarX     = (int)((float)cx / 5.0);
   int   iSizeSpacerX  = (int)((float)cx / 10.0);
   float flSizeBarY    = (float)cy * 0.80;
   int   iSizeUnitY    = (int)(flSizeBarY / 256.0) - 1;
   int   iStartX;
   int   iEndX;
   int   iStartY       = (int)((float)cy * 0.10);
   int   iSizeBarY     = 256 * (iSizeUnitY + 1);
   int   iCurrentY;
   RGB   rgbColor;
   RGB   rgbBlack      = { 0x00, 0x00, 0x00 };

   iStartX   = iSizeSpacerX;
   iEndX     = iStartX + iSizeBarX;
   iCurrentY = iStartY;

   rgbColor.bRed   = 0x00;
   rgbColor.bGreen = 0xFF;
   rgbColor.bBlue  = 0xFF;
   for (int i = 0; i < 256; i++)
   {
      setColor (pHDC, rgbColor);

      for (int y = 0; y < iSizeUnitY; y++, iCurrentY++)
      {
         drawLine (pHDC, iStartX, iCurrentY, iEndX, iCurrentY);
      }

      setColor (pHDC, rgbBlack);
      drawLine (pHDC, iStartX, iCurrentY, iEndX, iCurrentY);
      iCurrentY++;

      rgbColor.bRed++;
   }

   setColor (pHDC, rgbBlack);
   drawBox (pHDC, iStartX, iStartY, iSizeBarX, iSizeBarY);

   iStartX   = iEndX + iSizeSpacerX;
   iEndX     = iStartX + iSizeBarX;
   iCurrentY = iStartY;

   rgbColor.bRed   = 0xFF;
   rgbColor.bGreen = 0x00;
   rgbColor.bBlue  = 0xFF;
   for (int i = 0; i < 256; i++)
   {
      setColor (pHDC, rgbColor);

      for (int y = 0; y < iSizeUnitY; y++, iCurrentY++)
      {
         drawLine (pHDC, iStartX, iCurrentY, iEndX, iCurrentY);
      }

      setColor (pHDC, rgbBlack);
      drawLine (pHDC, iStartX, iCurrentY, iEndX, iCurrentY);
      iCurrentY++;

      rgbColor.bGreen++;
   }

   setColor (pHDC, rgbBlack);
   drawBox (pHDC, iStartX, iStartY, iSizeBarX, iSizeBarY);

   iStartX   = iEndX + iSizeSpacerX;
   iEndX     = iStartX + iSizeBarX;
   iCurrentY = iStartY;

   rgbColor.bRed   = 0xFF;
   rgbColor.bGreen = 0xFF;
   rgbColor.bBlue  = 0x00;
   for (int i = 0; i < 256; i++)
   {
      setColor (pHDC, rgbColor);

      for (int y = 0; y < iSizeUnitY; y++, iCurrentY++)
      {
         drawLine (pHDC, iStartX, iCurrentY, iEndX, iCurrentY);
      }

      setColor (pHDC, rgbBlack);
      drawLine (pHDC, iStartX, iCurrentY, iEndX, iCurrentY);
      iCurrentY++;

      rgbColor.bBlue++;
   }

   setColor (pHDC, rgbBlack);
   drawBox (pHDC, iStartX, iStartY, iSizeBarX, iSizeBarY);
}

void
drawPage (int     iMaxScanLines,
          Device *pDevice,
          bool    fDrawBox,
          bool    fDrawColors)
{
   PBYTE              pbBits             = 0;
   PBITMAPINFO2       pbmi2Printer       = 0;
   SIZEL              sizelPage          = {0, 0};
   RECTL              rectlPageLocation  = {0, 0, 0, 0};
   int                iNumColorsPrinter;
   int                iBytesToAlloc;
   byte               bWhiteIndex        = 0;
   int                icbPrinterScanLine;
   int                iScanlineMultiple;
   int                iNumBands;
   int                iTopY;
   HDC                hdc;
   PBYTE              pbScanLine         = 0;
   byte               bFillColor         = 0xFF;

   HardCopyCap      *pHCC       = pDevice->getCurrentForm ()->getHardCopyCap ();
   DevicePrintMode  *pPrintMode = pDevice->getCurrentPrintMode ();

   /* Bump up the # of scanlines that we send to the device to be a
   ** multiple of what it requests.
   */
   iScanlineMultiple = pDevice->getScanlineMultiple ();
   if (0 != (iMaxScanLines % iScanlineMultiple))
   {
      iMaxScanLines += iScanlineMultiple - (iMaxScanLines % iScanlineMultiple);
   }

   iNumColorsPrinter = 1 << pPrintMode->getLogicalCount ();

   // Allocate a full bitmap info 2 and bitmap header message
   iBytesToAlloc = sizeof (BITMAPINFO2);
   if (256 >= iNumColorsPrinter)
      iBytesToAlloc += (iNumColorsPrinter - 1) * sizeof (RGB2);

   pbmi2Printer = (PBITMAPINFO2)calloc (1, iBytesToAlloc);
   if (!pbmi2Printer)
      goto done;

   // Initialize it
   pbmi2Printer->cbFix         = iBytesToAlloc;
   pbmi2Printer->cx            = pHCC->getXPels ();
   pbmi2Printer->cy            = iMaxScanLines;
   pbmi2Printer->cPlanes       = pPrintMode->getNumPlanes ();
   pbmi2Printer->cBitCount     = pPrintMode->getLogicalCount ();
   pbmi2Printer->cclrUsed      = 1 << pbmi2Printer->cBitCount;
   pbmi2Printer->cclrImportant = 0;                            // All are important

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

   case 2:
   case 4:
   case 8:
   {
      // @TBD
      break;
   }
   }

   // Scan lines are double word aligned (32-bits)
   icbPrinterScanLine = ((pbmi2Printer->cx * pbmi2Printer->cBitCount + 31) >> 5) << 2;
   iBytesToAlloc      = icbPrinterScanLine * iMaxScanLines;

   sizelPage.cx = pHCC->getXPels ();
   sizelPage.cy = pHCC->getYPels ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": sizelPage = { " << sizelPage.cx << ", " << sizelPage.cy << " }" << std::endl;
#endif

   iNumBands = (sizelPage.cy + iMaxScanLines - 1) / iMaxScanLines;
   iTopY      = sizelPage.cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iBytesToAlloc = " << iBytesToAlloc << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": icbPrinterScanLine = " << icbPrinterScanLine << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iMaxScanLines = " << iMaxScanLines << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iNumBands = " << iNumBands << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": iScanlineMultiple = " << iScanlineMultiple << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__
           << ": sizelPage = { " << sizelPage.cx << ", " << sizelPage.cy << " }"
           << std::endl;
   }
#endif

   pbBits = (PBYTE)calloc (1, iBytesToAlloc);
   if (!pbBits)
      goto done;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": malloced " << iBytesToAlloc << " bytes at 0x" << std::hex << (int)pbBits << std::dec << std::endl;
#endif

   initializeHDC (hdc, pDevice);

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

   case 2:
   {
      bFillColor = (bWhiteIndex << 6)
                 | (bWhiteIndex << 4)
                 | (bWhiteIndex << 2)
                 | bWhiteIndex;
      break;
   }

   case 4:
   {
      bFillColor = (bWhiteIndex << 4) | bWhiteIndex;
      break;
   }

   case 8:
   {
      bFillColor = 0;     // @TBD
      break;
   }

   case 16:
   case 24:
   {
      bFillColor = 0xFF;  // Don't use indexes
      break;
   }
   }

   for (int iPass = iNumBands - 1; 0 <= iPass; iPass--)
   {
      // @TBD which way is it orientated?
      rectlPageLocation.xLeft   = 0;
      rectlPageLocation.xRight  = pbmi2Printer->cx - 1;
      rectlPageLocation.yBottom = iTopY - iMaxScanLines + 1;
      rectlPageLocation.yTop    = iTopY;

      iTopY -= iMaxScanLines;

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ())
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__
              << ": rectlPageLocation = { (" << rectlPageLocation.xLeft
              << ", " << rectlPageLocation.yTop
              << "), (" << rectlPageLocation.xRight
              << ", " << rectlPageLocation.yBottom
              << ") }" << std::endl;
      }
#endif

      for (int i = 0; i < iMaxScanLines; i++)
      {
         pbScanLine = pbBits + i * icbPrinterScanLine;

         // Erase the scan line
         memset (pbScanLine, bFillColor, icbPrinterScanLine);

         // Set up the hdc info
         hdc.pbScanLine = pbScanLine;
         hdc.iCurrentY  = rectlPageLocation.yBottom + i;

         if (0 > hdc.iCurrentY)
            continue;

         // Call the drawing routine
         if (fDrawBox)
            drawBoxRoutine (&hdc);
         else if (fDrawColors)
            drawColorsRoutine (&hdc);
      }

      // Send the completed bitmap to the printer
      pDevice->rasterize (pbBits,
                          pbmi2Printer,
                          &rectlPageLocation,
                          BITBLT_BITMAP);
   }

done:
   // Clean up!
   if (pbmi2Printer)
      free (pbmi2Printer);

   if (pbBits)
      free (pbBits);
}

Device *
createDevice (PSZRO     pszDriverLibrary,
              PSZCRO    pszJobProperties,
              bool      fAdvanced,
              GModule **phmodDevice)
{
#define LIBDEVICENAME3(s) STRINGIZE(s)
#define LIBDEVICENAME2(x) LIBDEVICENAME3(lib##x##.so)
#define LIBDEVICENAME1(x) LIBDEVICENAME2(x)
#define LIBDEVICENAME     LIBDEVICENAME1(DEVICENAME)

   std::ostringstream  oss;
   Device             *pDevice           = 0;
   PFNNEWDEVICE        pfnNewDevice      = 0;
   PFNNEWDEVICEWARGS   pfnNewDeviceWArgs = 0;

   if (  0 == pszDriverLibrary
      || !*pszDriverLibrary
      )
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": pszDriverLibrary is not filled in!" << std::endl;

      return 0;
   }

   if (  0 == strchr (pszDriverLibrary, '/')
      && 0 != strncasecmp (pszDriverLibrary, "lib", 3)
      )
   {
      oss << "lib"
          << pszDriverLibrary
          << ".so";

      pszDriverLibrary = (PSZRO)oss.str ().c_str ();
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Trying to load " << pszDriverLibrary << std::endl;
   }
#endif

   *phmodDevice = g_module_open (pszDriverLibrary, (GModuleFlags)0);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": *phmodDevice = " << std::hex << *phmodDevice << std::dec << std::endl;
   }
#endif

   if (!*phmodDevice)
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": g_module_error returns " << g_module_error () << std::endl;

      return 0;
   }

   // nm libEpson_Stylus_Color_860.so | grep newDevice

   // returns 00011e20 T newDeviceW_Advanced for
   // extern Device *newDevice (bool fAdvanced);
   g_module_symbol (*phmodDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": pfnNewDevice = 0x" << std::hex << (int)pfnNewDevice << std::dec << std::endl;
   }
#endif

   if (!pfnNewDevice)
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": g_module_error returns " << g_module_error () << std::endl;

      return 0;
   }

   // returns 00011d30 T newDeviceW_JopProp_Advanced for
   // extern Device *newDevice (char *pszJobProperties, bool fAdvanced);
   g_module_symbol (*phmodDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": pfnNewDeviceWArgs = 0x" << std::hex << (int)pfnNewDeviceWArgs << std::dec << std::endl;
   }
#endif

   if (!pfnNewDeviceWArgs)
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": g_module_error returns " << g_module_error () << std::endl;

      return 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": ********** Creating device with passed in properties **********" << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": The Properties are: " << pszJobProperties << std::endl;
   }
#endif

   pDevice = pfnNewDeviceWArgs (pszJobProperties, fAdvanced);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      if (pDevice)
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": pfnNewDeviceWArgs (...) returns " << *pDevice << std::endl;
      }
      else
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": pfnNewDeviceWArgs (...) returns NULL!" << std::endl;
      }
   }
#endif

   return pDevice;
}

Device *
createPDCDevice (PSZCRO    pszDriverLibrary,
                 PSZCRO    pszJobProperties,
                 bool      fAdvanced,
                 int       fdStdOut,
                 int       fdStdErr)
{
#ifdef PDC_INTERFACE_ENABLED
   return new OmniPDCProxy (0,                // client exe to spawn
                            pszDriverLibrary, // device name
                            pszJobProperties, // job properties
                            fAdvanced,        // is renderer
                            fdStdOut,         // stdout
                            fdStdErr);        // stderr
#else
   return 0;
#endif
}

void
printUsage (char *pszExeName)
{
   DebugOutput::getErrorStream () << "Usage: " << pszExeName << " required_parms optional_parms" << std::endl
        << "   where required_parms are:" << std::endl
        << "      --bmp bitmapfile.bmp OR --drawbox OR --drawcolors" << std::endl
        << "      --driver libDriverName.so" << std::endl
        << "   where optional_parms are:" << std::endl
        << "      -sproperties='...'" << std::endl
        << "      --UsePDC" << std::endl
        << "      --!UsePDC" << std::endl
        << "      --Advanced" << std::endl
        << "      --!Advanced" << std::endl
        << "      --cout output.file" << std::endl
        << "      --cerr error.file" << std::endl
        << "      --scanlines number_of_scanlines" << std::endl;
}

int
drawPrint (bool  fUsePDC,
           PSZRO pszDriverLibrary,
           PSZRO pszJobProperties,
           bool  fAdvanced,
           bool  fDrawBox,
           bool  fDrawColors,
           PSZ   pszBitmapFilename,
           bool  fDisplayVersion,
           FILE *pfpOut,
           FILE *pfpErr,
           int   iNumScanLines)
{
   GModule       *hmodDevice        = 0;
   Device        *pDevice           = 0;
   int            rc                = 0;

   if (fUsePDC)
   {
      pDevice = createPDCDevice (pszDriverLibrary,
                                 pszJobProperties,
                                 fAdvanced,
                                 fileno (pfpOut),
                                 pfpErr ? fileno (pfpErr) : STDERR_FILENO);
   }
   else
   {
      pDevice = createDevice (pszDriverLibrary,
                              pszJobProperties,
                              fAdvanced,
                              &hmodDevice);
   }

   Omni::setOutputStream (pDevice, pfpOut);
   if (pfpErr)
      Omni::setErrorStream (pDevice, pfpErr);

   if (!pDevice)
   {
      rc = 7;
      goto BUGOUT;
   }

   if (pDevice->hasError ())
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ())
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": hasError () = " << pDevice->hasError () << std::endl;
#endif

      rc = 8;
      goto BUGOUT;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      std::string *pstringJobProp = 0;
      HardCopyCap *pHCC           = pDevice->getCurrentForm ()->getHardCopyCap ();

      pstringJobProp = pDevice->getJobProperties ();

      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": The device's job properties are: " << *pstringJobProp << std::endl;

      delete pstringJobProp;

      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Form cx    = " << pHCC->getCx () << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Form cy    = " << pHCC->getCy () << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Form xPels = " << pHCC->getXPels () << std::endl;
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Form yPels = " << pHCC->getYPels () << std::endl;
   }
#endif

   if (  fDrawBox
      || fDrawColors
      )
   {
      if (pDevice->beginJob ())
      {
         drawPage (iNumScanLines, pDevice, fDrawBox, fDrawColors);

         pDevice->endJob ();
      }
      else
      {
         DebugOutput::getErrorStream () << "Error: Could not start the page!" << std::endl;

         rc = 9;
         goto BUGOUT;
      }
   }
   else if (pszBitmapFilename)
   {
      FILE *fpBitmap = 0;

      fpBitmap = fopen (pszBitmapFilename, "rb");
      if (fpBitmap)
      {
         USHORT usType = 0;
         int    rc;

         rc = fread (&usType, sizeof (usType), 1, fpBitmap);

         if (BFT_BMAP == usType)
         {
            if (pDevice->beginJob ())
            {
               bitblt (fpBitmap, usType, iNumScanLines, pDevice);

               pDevice->endJob ();
            }
            else
            {
               DebugOutput::getErrorStream () << "Error: Could not start the page!" << std::endl;

               rc = 10;
               goto BUGOUT;
            }
         }
         else
         {
            DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Error: " << pszBitmapFilename << " is not a valid bitmap because usType = 0x" << std::hex << usType << std::dec << std::endl;

            rc = 11;
            goto BUGOUT;
         }

         fclose (fpBitmap);
         fpBitmap = 0;
      }
      else
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Error: cannot open " << pszBitmapFilename << " for reading" << std::endl;
      }
   }

   if (fDisplayVersion)
   {
      std::cerr << pDevice->getVersion () << std::endl;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ())
   {
      DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": In " << __FUNCTION__ << ", after delete pDevice;" << std::endl;
   }
#endif

BUGOUT:
   // Clean up!

   delete pDevice;

   if (hmodDevice)
   {
#ifndef RETAIL
      int rc2 =
#endif
                g_module_close (hmodDevice);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ())
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": g_module_close returns " << rc2 << std::endl;
      }
#endif
   }

   if (pfpOut)
   {
      fclose (pfpOut);
      pfpOut = 0;
   }

   if (pfpErr)
   {
      fclose (pfpErr);
      pfpErr = 0;
   }

   return rc;
}

int
main (int argc, char *argv[])
{
#define MAX_SCAN_LINES 32
#define JP_BLOCK_SIZE  256
   PSZ            pszCout           = 0;
   PSZ            pszCerr           = 0;
   FILE          *pfpOut            = 0;
   FILE          *pfpErr            = 0;
   PSZ            pszBitmapFilename = 0;
   char          *pszDriverLibrary  = 0;
   PSZRO          pszJobProperties  = 0;
   JobProperties  jobProp;
   int            iNumScanLines     = MAX_SCAN_LINES;
   bool           fDrawBox          = false;
   bool           fDrawColors       = false;
   bool           fDisplayVersion   = false;
   bool           fUsePDC           = false;
   bool           fAdvanced         = true;
   bool           fBuildOnly        = false;
   bool           fAll              = false;
   int            rc                = 0;

   Omni::initialize ();

   if (!g_module_supported ())
   {
      std::cerr << "This program needs glib's module routines!" << std::endl;

      rc = 1;
      goto BUGOUT;
   }

#ifdef DEBUG
   /* HACK:  Work around gdb debugging of a program that writes to both
   **        stdout and stderr.
   */
   pszCout = "output.prn";
   pszCerr = "output.err";
#endif

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcmp ("--version", argv[i]))
      {
         fDisplayVersion = true;
      }
      else if (0 == strcmp ("--bmp", argv[i]))
      {
         pszBitmapFilename = argv[++i];
         fDrawBox          = false;
         fDrawColors       = false;
      }
      else if (0 == strcmp ("--cout", argv[i]))
      {
         pszCout = argv[++i];
      }
      else if (0 == strcmp ("--cerr", argv[i]))
      {
         pszCerr = argv[++i];
      }
      else if (0 == strcasecmp ("--driver", argv[i]))
      {
         std::ostringstream  oss;
         const char         *pszNewDriverLibrary = argv[i + 1];

         if ( 0 == strchr (pszNewDriverLibrary, '/')
            && 0 != strncasecmp (pszNewDriverLibrary, "lib", 3)
            )
         {
            oss << "lib"
                << pszNewDriverLibrary
                << ".so";

            pszNewDriverLibrary = oss.str ().c_str ();
         }

         if (pszDriverLibrary)
         {
            free (pszDriverLibrary);
            pszDriverLibrary = 0;
         }
         pszDriverLibrary = (char *)malloc (strlen (pszNewDriverLibrary) + 1);
         if (pszDriverLibrary)
         {
            strcpy (pszDriverLibrary, pszNewDriverLibrary);
         }
         else
         {
            std::cerr << "Error:  Out of memory" << std::endl;

            rc = 2;
            goto BUGOUT;
         }

         i++;
      }
      else if (0 == strcasecmp ("--scanlines", argv[i]))
      {
         iNumScanLines = atoi (argv[++i]);
      }
      else if (0 == strcasecmp ("--drawbox", argv[i]))
      {
         fDrawBox = true;
      }
      else if (0 == strcasecmp ("--drawcolors", argv[i]))
      {
         fDrawColors = true;
      }
      else if (0 == strcasecmp ("--UsePDC", argv[i]))
      {
         fUsePDC = true;
      }
      else if (0 == strcasecmp ("--!UsePDC", argv[i]))
      {
         fUsePDC = false;
      }
      else if (0 == strcasecmp ("--Advanced", argv[i]))
      {
         fAdvanced = true;
      }
      else if (0 == strcasecmp ("--!Advanced", argv[i]))
      {
         fAdvanced = false;
      }
      else if (0 == strcasecmp ("--buildOnly", argv[i]))
      {
         fBuildOnly = true;
      }
      else if (0 == strncasecmp ("-sproperties=", argv[i], 13))
      {
         char *pszJobProp = argv[i] + 13;

         jobProp.setJobProperties (pszJobProp);
      }
      else if (0 == strcasecmp (argv[i], "--all"))
      {
         fAll = true;
      }
      else
      {
         DeviceInfo *pDI = Omni::findDeviceInfoFromShortName (argv[i], fBuildOnly);

         if (pDI)
         {
            Device     *pDevice     = 0;
            GModule    *hmodDevice  = 0;
            OmniDevice *pOD         = 0;
            PSZRO       pszJobProp  = 0;

            pDevice    = pDI->getDevice ();
            hmodDevice = pDI->getHmodDevice ();
            pOD        = pDI->getOmniDevice ();

            if (pOD)
            {
               pszJobProp = pOD->getJobProperties ();

               jobProp.setJobProperties (pszJobProp);

               pszJobProp = jobProp.getJobProperties ();
            }

            if (pDevice)
            {
               PSZCRO pszDrivName = pDevice->getLibraryName ();

               if (pszDriverLibrary)
               {
                  free (pszDriverLibrary);
                  pszDriverLibrary = 0;
               }
               pszDriverLibrary = (char *)malloc (strlen (pszDrivName) + 1);
               if (pszDriverLibrary)
               {
                  strcpy (pszDriverLibrary, pszDrivName);
               }
               else
               {
                  std::cerr << "Error:  Out of memory" << std::endl;

                  rc = 2;
                  goto BUGOUT;
               }

               std::cerr << "Info: For faster results use:"
                         << std::endl
                         << "Info:\t"
                         << argv[0];

               if (pszJobProp)
               {
                  std::cerr << " '-sproperties="
                            << pszJobProp
                            << "'";
               }

               std::cerr << " --driver "
                         << pDevice->getLibraryName ()
                         << " ..."
                         << std::endl;
            }

            if (pszJobProp)
            {
               free ((void *)pszJobProp);
               pszJobProp = 0;
            }

            delete pDI;
         }
      }
   }

   if (pszCout)
   {
      unlink (pszCout);

      pfpOut = fopen (pszCout, "wb");

      if (0 == pfpOut)
      {
         DebugOutput::getErrorStream () << "Error:  Could not open " << pszCout << " for writing!" << std::endl;

         goto BUGOUT;
      }
   }

   if (pszCerr)
   {
      unlink (pszCerr);

      pfpErr = fopen (pszCerr, "w");

      Omni::setErrorStream (0, pfpErr);
   }

   if (  !fDrawBox
      && !fDrawColors
      && 0 == pszBitmapFilename
      )
   {
      std::cerr << "Info:  Defaulting to drawing a box (--drawbox).  Other choices are:"
                << std::endl
                << "\t--bmp file" << std::endl
                << "\t--drawbox" << std::endl
                << "\t--drawcolors" << std::endl;

      fDrawBox = true;
   }

   if (!fAll)
   {
      // Check for required parameters
      if (0 == pszDriverLibrary)
      {
         if (0 == pszDriverLibrary)
         {
            std::cerr << "Error:  Must specify a driver library" << std::endl;
         }

         printUsage (argv[0]);

         rc = 5;
         goto BUGOUT;
      }

      pszJobProperties = jobProp.getJobProperties ();

#ifndef RETAIL
      jobProp.applyAllDebugOutput ();

      if (DebugOutput::shouldOutputDeviceTester ())
      {
         DebugOutput::getErrorStream () << "DeviceTester::" << __FUNCTION__ << ": Creating the device with: " << pszJobProperties << std::endl;
      }
#endif

      drawPrint (fUsePDC,
                 (PSZRO)pszDriverLibrary,
                 pszJobProperties,
                 fAdvanced,
                 fDrawBox,
                 fDrawColors,
                 pszBitmapFilename,
                 fDisplayVersion,
                 pfpOut,
                 pfpErr,
                 iNumScanLines);
   }
   else
   {
      Enumeration *pEnum = Omni::listDevices (fBuildOnly);

      while (pEnum->hasMoreElements ())
      {
         OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

         if (pOD)
         {
            PSZCRO             pszLibName  = pOD->getLibraryName ();
            PSZRO              pszJobProps = pOD->getJobProperties ();
            JobProperties      jobPropNew  = jobProp;
            std::ostringstream oss;

            jobPropNew.setJobProperties (pszJobProps);

            pszJobProps = jobPropNew.getJobProperties ();

            std::cerr << "Printing: " << pszLibName << ", \"" << pszJobProps << "\"" << std::endl;

            oss << "OmniDeviceTester ";
            if (fDisplayVersion)
            {
               oss << " --version";
            }
            if (pszBitmapFilename)
            {
               oss << " --bmp " << pszBitmapFilename;
            }
            if (pszCout)
            {
               oss << " --cout " << pszCout;
            }
            if (pszCerr)
            {
               oss << " --cerr " << pszCerr;
            }
            oss << " --driver " << pszLibName;
            if (pszJobProps)
            {
               oss << " -sproperties='" << pszJobProps << "'";
            }
            oss << " --scanlines " << iNumScanLines;
            if (fDrawBox)
            {
               oss << " --drawbox";
            }
            if (fDrawColors)
            {
               oss << " --drawcolors";
            }
            if (fUsePDC)
            {
               oss << " --fUsePDC";
            }
            if (fAdvanced)
            {
               oss << " --Advanced";
            }
            if (fBuildOnly)
            {
               oss << " --buildOnly";
            }

            std::cerr << oss.str () << std::endl;

            Omni::my_system (oss.str ().c_str ());

            if (pszJobProps)
            {
               free ((void*)pszJobProps);
               pszJobProps = 0;
            }

            delete pOD;
         }
         else
         {
            std::cerr << "Error: No OmniDevice was returned!" << std::endl;

            rc = 1;
         }
      }

      delete pEnum;
   }

   if (  pszCout
      || pszCerr
      )
   {
      struct stat statOut;
      struct stat statErr;
      int         cbOut    = 0;
      int         cbErr    = 0;

      if (  pszCout
         && 0 == stat (pszCout, &statOut)
         )
      {
         cbOut = statOut.st_size;
      }
      if (  pszCerr
         && 0 == stat (pszCerr, &statErr)
         )
      {
         cbErr = statErr.st_size;
      }

      std::cerr << "out = " << cbOut << ", err = " << cbErr << std::endl;
   }

   rc = 0;

BUGOUT:
   // Clean up!

   if (pszDriverLibrary)
   {
      free (pszDriverLibrary);
      pszDriverLibrary = 0;
   }
   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
      pszJobProperties = 0;
   }

   Omni::terminate ();

   return rc;
}
