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
#include "CMYKbitmap.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <iostream>

using namespace std;

const static bool fDebugOutput = false;

extern int chsize (int fileno, long int iSize);

CMYKBitmap::
CMYKBitmap (int          iCx,
            int          iCy)
{
   strcpy (achFileName_d, "tmp.bmp");
   fp_d                = 0;
   iCx_d               = iCx;
   iCy_d               = iCy;
   iBitCount_d         = 8;
   iSizeScanLineIn_d   = (iCx_d * 1 + 7) >> 3;
   iSizeScanLineOut_d  = ((iCx_d * iBitCount_d + 31) >> 5) << 2;
   pbScanLine_d        = 0;
   fFirstTime_d        = true;
}

CMYKBitmap::
CMYKBitmap (char        *pszFilename,
            int          iCx,
            int          iCy)
{
   strcpy (achFileName_d, pszFilename);
   fp_d                = 0;
   iCx_d               = iCx;
   iCy_d               = iCy;
   iBitCount_d         = 8;
   iSizeScanLineIn_d   = (iCx_d * 1 + 7) >> 3;
   iSizeScanLineOut_d  = ((iCx_d * iBitCount_d + 31) >> 5) << 2;
   pbScanLine_d        = 0;
   fFirstTime_d        = true;
}

CMYKBitmap::
~CMYKBitmap ()
{
   if (fp_d)
   {
      fclose (fp_d);
   }

   if (pbScanLine_d)
   {
      free (pbScanLine_d);
      pbScanLine_d = 0;
   }
}

void CMYKBitmap::
addScanLine (PBYTE pbBits,
             int   iScanLines,
             int   iYPos,
             PLANE eWhichPlane)
{
   if (fFirstTime_d)
   {
      fFirstTime_d = false;

      pbScanLine_d = (PBYTE)malloc (iSizeScanLineOut_d);
      memset (pbScanLine_d, 0, iSizeScanLineOut_d);

      fp_d = fopen (achFileName_d, "w+b");

      if (fp_d)
      {
         sizeFile ();
         writeHeader ();
      }
   }

   if (!fp_d)
      return;

   if (fDebugOutput) cerr << dec << "iScanLines = " << iScanLines << endl;
   if (fDebugOutput) cerr << dec << "iYPos = " << iYPos << endl;

   int rc;

   // Do we start above the bitmap?
   if (iYPos > iCy_d)
   {
      // Look at the ones that intersect on our bitmap
      iScanLines -= (iYPos - iCy_d + 1);
      pbBits     += iSizeScanLineIn_d * (iYPos - iCy_d + 1);

      // Start on the topmost scanline
      iYPos = iCy_d - 1;

      // Did any make it?
      if (0 > iScanLines)
         return;
   }

   iYPos++; // Seeking calculations are offset by one

   for (int i = 0; i < iScanLines; i++, iYPos--)
   {
      if (1 > iYPos)
         return;

      // Move to the scanline in the file
      rc = fseek (fp_d,
                  -( iYPos
                   * iSizeScanLineOut_d
                   ),
                  SEEK_END);

      // Read it in
      rc = fread (pbScanLine_d,
                  sizeof (BYTE),
                  iSizeScanLineOut_d,
                  fp_d);

      // Move to the scanline in the file
      rc = fseek (fp_d,
                  -( iYPos
                   * iSizeScanLineOut_d
                   ),
                  SEEK_END);

      // Modify it
      for (int j = 0; j < iCx_d; j++)
      {
         static BYTE abBit[] = {
            0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
         };

         if (*(pbBits + (j >> 3)) & abBit[j & 7])
         {
            PBYTE pbPel = pbScanLine_d + j;

            switch (eWhichPlane)
            {
            case CYAN:    *pbPel |= 0x08; break;
            case MAGENTA: *pbPel |= 0x04; break;
            case YELLOW:  *pbPel |= 0x02; break;
            case BLACK:   *pbPel |= 0x01; break;
            }
         }
      }

      // Write it back out
      rc = fwrite (pbScanLine_d,
                   sizeof (BYTE),
                   iSizeScanLineOut_d,
                   fp_d);

//    rc = fflush (fp_d);

      pbBits += iSizeScanLineIn_d;
   }
}

void CMYKBitmap::
sizeFile ()
{
   int    iMaxSize;
   int    rc;

#ifdef OS2

   iMaxSize = sizeof (OS2BITMAPFILEHEADER)
            + iCy_d * iSizeScanLineOut_d;

   if (8 >= iBitCount_d)
      iMaxSize += (1 << iBitCount_d) * sizeof (OS2RGB);

#else

   iMaxSize = sizeof (WINBITMAPFILEHEADER)
            + sizeof (WINBITMAPINFOHEADER)
            + iCy_d * iSizeScanLineOut_d;

   if (8 >= iBitCount_d)
      iMaxSize += (1 << iBitCount_d) * sizeof (WINRGBQUAD);

#endif

   if (fDebugOutput) cerr << "Setting fp_d to " << iMaxSize << " bytes in size!" << endl;
   rc = chsize (fileno (fp_d), iMaxSize);
}

void CMYKBitmap::
writeHeader ()
{
   int rc;

#ifdef OS2

   OS2BITMAPFILEHEADER bfh;

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

   // Write the bitmap file header out
   rc = fwrite (&bfh, sizeof (bfh), 1, fp_d);

   // Write the bitmap color table out
   OS2RGB argb[256];
   // @TBD

#else

   WINBITMAPFILEHEADER bfh;
   WINBITMAPINFOHEADER bih;
   int                 iNumColors;

   iNumColors = 1 << iBitCount_d;

   bfh.bfType          = WINBFT_BMAP;
   bfh.bfSize          = sizeof (bfh) + sizeof (bih);
   bfh.bfReserved1     = 0;
   bfh.bfReserved2     = 0;
   bfh.bfOffBits       = sizeof (bfh) + sizeof (bih);

   bih.biSize          = sizeof (bih); // For now
   bih.biWidth         = iCx_d;
   bih.biHeight        = iCy_d;
   bih.biPlanes        = 1;
   bih.biBitCount      = iBitCount_d;
   bih.biCompression   = 0;
   bih.biSizeImage     = iCy_d * iSizeScanLineOut_d;
   bih.biXPelsPerMeter = 1;
   bih.biYPelsPerMeter = 1;
   bih.biClrUsed       = iNumColors;
   bih.biClrImportant  = iNumColors;

   // Account for the color table if there is one
   if (8 >= iBitCount_d)
   {
      bfh.bfOffBits += iNumColors * sizeof (WINRGBQUAD);
      bfh.bfSize    += iNumColors * sizeof (WINRGBQUAD);
   }

   // Include the scan lines, of course
   bfh.bfSize += iCy_d * iSizeScanLineOut_d;

   // Write the bitmap file header out
   rc = fwrite (&bfh, sizeof (bfh), 1, fp_d);
   // Write the bitmap info header out
   rc = fwrite (&bih, sizeof (bih), 1, fp_d);

   // Write the bitmap color table out
   if (8 >= iBitCount_d)
   {
      WINRGBQUAD argb[256] = {
      // CMYK       B     G     R   opt                B     G     R   opt
      /* 0000 */{ 0xFF, 0xFF, 0xFF, 0 }, /* 0001 */{ 0x00, 0x00, 0x00, 0 },
      /* 0010 */{ 0x00, 0xFF, 0xFF, 0 }, /* 0011 */{ 0x00, 0x00, 0x00, 0 },
      /* 0100 */{ 0xFF, 0x00, 0xFF, 0 }, /* 0101 */{ 0x00, 0x00, 0x00, 0 },
      /* 0110 */{ 0x00, 0x00, 0xFF, 0 }, /* 0111 */{ 0x00, 0x00, 0x00, 0 },
      /* 1000 */{ 0xFF, 0xFF, 0x00, 0 }, /* 1001 */{ 0x00, 0x00, 0x00, 0 },
      /* 1010 */{ 0x00, 0xFF, 0x00, 0 }, /* 1011 */{ 0x00, 0x00, 0x00, 0 },
      /* 1100 */{ 0xFF, 0x00, 0x00, 0 }, /* 1101 */{ 0x00, 0x00, 0x00, 0 },
      /* 1110 */{ 0x00, 0x00, 0x00, 0 }, /* 1111 */{ 0x00, 0x00, 0x00, 0 }
      };

      rc = fwrite (argb, sizeof (argb[0]), dimof (argb), fp_d);
   }

#endif

   rc = fflush (fp_d);
}
