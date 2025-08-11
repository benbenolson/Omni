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
#include "bitmap.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <iostream>
#include <algorithm>

using namespace std;

const static bool fDebugOutput = false;

int chsize (int fileno, long int iSize);

Bitmap::
Bitmap (int          iCx,
        int          iCy,
        int          iBitCount,
        PNEUTRALRGB  pColors)
{
   int iNumColors = 1 << iBitCount;

   fp_d                = fopen ("tmp.bmp", "wb");
   iCx_d               = iCx;
   iCy_d               = iCy;
   iBitCount_d         = iBitCount;
   iSizeColors_d       = iNumColors * sizeof (NEUTRALRGB);
   pColors_d           = 0;

   if (8 >= iBitCount_d)
   {
      pColors_d = (PNEUTRALRGB)malloc (iSizeColors_d);
      memcpy (pColors_d, pColors, iSizeColors_d);
   }

   iSizeScanLine_d     = (iCx_d * iBitCount_d + 31) / 32 * 4;
   iScanLinesWritten_d = 0;
   fFirstTime_d        = true;
}

Bitmap::
Bitmap (int          iCx,
        int          iCy,
        int          iBitCount,
        PNEUTRALRGB2 pColors)
{
   int iNumColors = 1 << iBitCount;

   fp_d                = fopen ("tmp.bmp", "wb");
   iCx_d               = iCx;
   iCy_d               = iCy;
   iBitCount_d         = iBitCount;
   iSizeColors_d       = iNumColors * sizeof (NEUTRALRGB);
   pColors_d           = 0;

   if (8 >= iBitCount_d)
   {
      pColors_d = (PNEUTRALRGB)malloc (iSizeColors_d);

      for (int i = 0; i < iNumColors; i++)
      {
         pColors_d->bRed   = pColors[i].bRed;
         pColors_d->bGreen = pColors[i].bGreen;
         pColors_d->bBlue  = pColors[i].bBlue;
      }
   }

   iSizeScanLine_d     = (iCx_d * iBitCount_d + 31) / 32 * 4;
   iScanLinesWritten_d = 0;
   fFirstTime_d        = true;
}

Bitmap::
Bitmap (char        *pszFilename,
        int          iCx,
        int          iCy,
        int          iBitCount,
        PNEUTRALRGB  pColors)
{
   int iNumColors = 1 << iBitCount;

   fp_d                = fopen (pszFilename, "wb");
   iCx_d               = iCx;
   iCy_d               = iCy;
   iBitCount_d         = iBitCount;
   iSizeColors_d       = iNumColors * sizeof (NEUTRALRGB);
   pColors_d           = 0;

   if (8 >= iBitCount_d)
   {
      pColors_d = (PNEUTRALRGB)malloc (iSizeColors_d);

      memcpy (pColors_d, pColors, iSizeColors_d);
   }

   iSizeScanLine_d     = (iCx_d * iBitCount_d + 31) / 32 * 4;
   iScanLinesWritten_d = 0;
   fFirstTime_d        = true;
}

Bitmap::
Bitmap (char        *pszFilename,
        int          iCx,
        int          iCy,
        int          iBitCount,
        PNEUTRALRGB2 pColors)
{
   int iNumColors = 1 << iBitCount;

   fp_d                = fopen (pszFilename, "wb");
   iCx_d               = iCx;
   iCy_d               = iCy;
   iBitCount_d         = iBitCount;
   iSizeColors_d       = iNumColors * sizeof (NEUTRALRGB);
   pColors_d           = 0;

   if (8 >= iBitCount_d)
   {
      pColors_d = (PNEUTRALRGB)malloc (iSizeColors_d);

      for (int i = 0; i < iNumColors; i++)
      {
         pColors_d->bRed   = pColors[i].bRed;
         pColors_d->bGreen = pColors[i].bGreen;
         pColors_d->bBlue  = pColors[i].bBlue;
      }
   }

   iSizeScanLine_d     = (iCx_d * iBitCount_d + 31) / 32 * 4;
   iScanLinesWritten_d = 0;
   fFirstTime_d        = true;
}

Bitmap::
~Bitmap ()
{
   if (fp_d)
   {
      fclose (fp_d);
   }
   if (pColors_d)
   {
      free (pColors_d);
   }
}

void Bitmap::
addScanLine (PBYTE pbBits, int iScanLines)
{
   if (fFirstTime_d)
   {
      fFirstTime_d = false;

      sizeFile ();
      writeHeader ();
   }

   int rc;

   for (int i = 0; i < iScanLines; i++)
   {
      if (iScanLinesWritten_d > iCy_d)
         return;

      rc = fseek (fp_d,
                  -( (iScanLinesWritten_d + 1)
                   * iSizeScanLine_d
                   ),
                  SEEK_END);

      if (fDebugOutput)
      {
         if (0 != rc)
         {
            cerr << __FUNCTION__ << "::addScanLine: fseek failed, rc = " << rc << endl;
         }
      }

      rc = fwrite (pbBits + i * iSizeScanLine_d,
                   sizeof (BYTE),
                   iSizeScanLine_d,
                   fp_d);

      if (fDebugOutput)
      {
         if (rc != iSizeScanLine_d)
         {
            cerr << __FUNCTION__ << "::addScanLine: fwrite failed, rc = " << rc << endl;
         }
      }

      iScanLinesWritten_d++;
   }
}

void Bitmap::
sizeFile ()
{
   int    iMaxSize;
   int    rc;

#ifdef OS2

   iMaxSize = sizeof (OS2BITMAPFILEHEADER)
            + iCy_d * iSizeScanLine_d;

   if (8 >= iBitCount_d)
      iMaxSize += (1 << iBitCount_d) * sizeof (OS2RGB);

#else

   iMaxSize = sizeof (WINBITMAPFILEHEADER)
            + sizeof (WINBITMAPINFOHEADER)
            + iCy_d * iSizeScanLine_d;

   if (8 >= iBitCount_d)
      iMaxSize += (1 << iBitCount_d) * sizeof (WINRGBQUAD);

#endif

   if (fDebugOutput) cerr << "Setting fp_d to " << iMaxSize << " bytes in size!" << endl;
   rc = chsize (fileno (fp_d), iMaxSize);
}

void Bitmap::
writeHeader ()
{
   int rc;

#ifdef OS2

   OS2BITMAPFILEHEADER bfh;
   int                 iNumColors;

   bfh.usType        = OS2BFT_BMAP;
   bfh.cbSize        = sizeof (bfh);
   bfh.xHotspot      = 0;
   bfh.yHotspot      = 0;
   bfh.offBits       = sizeof (bfh);

   bfh.bmp.cbFix     = sizeof (bfh.bmp);
   bfh.bmp.cx        = iCx_d;
   bfh.bmp.cy        = iCy_d;
   bfh.bmp.cBitCount = iBitCount_d;
   bfh.bmp.cPlanes   = 1;

   iNumColors = 1 << bfh.bmp.cBitCount;
   if (fDebugOutput)
   {
      MY_PRINT_VAR (iNumColors);
   }

   if (8 >= iBitCount_d)
   {
      bfh.offBits += iNumColors * sizeof (OS2RGB);
   }

   // Write the bitmap file header out
   rc = fwrite (&bfh, sizeof (bfh), 1, fp_d);

   if (fDebugOutput)
   {
      if (rc != 1)
      {
         cerr << __FUNCTION__ << "::addScanLine: fwrite failed, rc = " << rc << endl;
      }
   }

   // Write the bitmap color table out
   if (8 >= iBitCount_d)
   {
      rc = fwrite (pColors_d, iSizeColors_d, 1, fp_d);

      if (fDebugOutput)
      {
         if (rc != 1)
         {
            cerr << __FUNCTION__ << "::addScanLine: fwrite failed, rc = " << rc << endl;
         }
      }
   }

#else

   WINBITMAPFILEHEADER bfh;
   WINBITMAPINFOHEADER bih;
   int                 iNumColors;

   iNumColors = 1 << iBitCount_d;
   if (fDebugOutput)
   {
      MY_PRINT_VAR (iNumColors);
   }

   bfh.bfType          = WINBFT_BMAP;
   bfh.bfSize          = sizeof (bfh) + sizeof (bih);
   bfh.bfReserved1     = 0;
   bfh.bfReserved2     = 0;
   bfh.bfOffBits       = sizeof (bfh) + sizeof (bih);

   bih.biSize          = sizeof (bih);
   bih.biWidth         = iCx_d;
   bih.biHeight        = iCy_d;
   bih.biPlanes        = 1;
   bih.biBitCount      = iBitCount_d;
   bih.biCompression   = 0;
   bih.biSizeImage     = iCy_d * iSizeScanLine_d;
   bih.biXPelsPerMeter = 1;
   bih.biYPelsPerMeter = 1;
   bih.biClrUsed       = 0; // @BUG - Gimp 1.2 chokes on this - iNumColors;
   bih.biClrImportant  = 0; // @BUG - Gimp 1.2 chokes on this - iNumColors;

   if (8 >= iBitCount_d)
   {
      bfh.bfOffBits += iNumColors * sizeof (WINRGBQUAD);
      bfh.bfSize    += iNumColors * sizeof (WINRGBQUAD);
   }

   bfh.bfSize += iCy_d * iSizeScanLine_d;

   // Write the bitmap file header out
   rc = fwrite (&bfh, sizeof (bfh), 1, fp_d);

   if (fDebugOutput)
   {
      if (rc != 1)
      {
         cerr << __FUNCTION__ << "::addScanLine: fwrite failed, rc = " << rc << endl;
      }
   }

   // Write the bitmap info header out
   rc = fwrite (&bih, sizeof (bih), 1, fp_d);

   if (fDebugOutput)
   {
      if (rc != 1)
      {
         cerr << __FUNCTION__ << "::addScanLine: fwrite failed, rc = " << rc << endl;
      }
   }

   // Write the bitmap color table out
   if (8 >= iBitCount_d)
   {
      WINRGBQUAD rgb;

      rgb.rgbReserved = 0;

      for (int i = 0; i < iNumColors; i++)
      {
         rgb.rgbRed   = pColors_d[i].bRed;
         rgb.rgbGreen = pColors_d[i].bGreen;
         rgb.rgbBlue  = pColors_d[i].bBlue;

         rc = fwrite (&rgb, sizeof (rgb), 1, fp_d);

         if (fDebugOutput)
         {
            if (rc != 1)
            {
               cerr << __FUNCTION__ << "::addScanLine: fwrite failed, rc = " << rc << endl;
            }
         }
      }
   }

#endif
}

int
chsize (int fileno, long int iSize)
{
   byte abData[512];
   int  rc;

   memset (abData, 0, sizeof (abData));

   int iPos = lseek (fileno, 0, SEEK_END);

   while (iPos < iSize)
   {
      rc = write (fileno, abData, omni::min ((iSize - iPos), (long int)sizeof (abData)));
      if (-1 == rc)
         return rc;

      iPos += rc;
   }

   lseek (fileno, 0, SEEK_SET);

   return 0;
}
