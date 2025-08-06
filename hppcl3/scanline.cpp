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
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sstream>
#include <unistd.h>
#include <assert.h>

#include "hppcl3.hpp"
#include "scanline.hpp"

/* Function prototypes...
*/
int           NumBits            (int      iMaxValue);
int           chsize             (int      fileno,
                                  long int iSize);

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
ScanLines ::
ScanLines (PBYTE pbData, int iLength)
{
#define ENDIAN_SWAP_SHORT(s) (((0x00FF & s) << 8) | ((0xFF00 & s) >> 8))

#pragma pack(1)
   typedef struct _CommandLayout {
      unsigned char  uFormat;
      unsigned char  uComponents;

      unsigned short usKXRes;
      unsigned short usKYRes;
      unsigned short usKNumComponents;

      unsigned short usCXRes;
      unsigned short usCYRes;
      unsigned short usCNumComponents;

      unsigned short usMXRes;
      unsigned short usMYRes;
      unsigned short usMNumComponents;

      unsigned short usYXRes;
      unsigned short usYYRes;
      unsigned short usYNumComponents;

   } COMMANDLAYOUT, *PCOMMANDLAYOUT;
#pragma pack()

   PCOMMANDLAYOUT  pLayout = (PCOMMANDLAYOUT)pbData;

   pMonoBitmap_d            = 0;
   pColorBitmap_d           = 0;

   cxMono_d                 = 0;
   cyMono_d                 = 0;
   cxColor_d                = 0;
   cyColor_d                = 0;

   fInitialized_d           = false;
   iCurrentFormSize_d       = 0;

   iSizeMonoScanLine_d      = 0;
   iMonoScanLinesWritten_d  = 0;
   iSizeColorScanLine_d     = 0;
   iColorScanLinesWritten_d = 0;

   iSizeExtColorScanLine_d  = 0;
   pbExtColorScanLine_d     = 0;

   iSizeExtMonoScanLine_d   = 0;
   pbExtMonoScanLine_d      = 0;

   iExtMonoBitsPerPel       = 0;
   iExtColorBitsPerPel      = 0;

   eScanLineOrder_d         = ORDER_UNKNOWN;

   iCompressionLevel_d      = 0;
   cbLastScanline_d         = 0;
   pbLastScanline_d         = 0;

   iWhichPlane_d            = 0;

   iNumComponents_d         = 0;

   pbKHigh_d                = 0;
   pbKLow_d                 = 0;
   iKXRes_d                 = 0;
   iKYRes_d                 = 0;
   iKIntensityLevels_d      = 0;

   pbCHigh_d                = 0;
   pbCLow_d                 = 0;
   iCXRes_d                 = 0;
   iCYRes_d                 = 0;
   iCIntensityLevels_d      = 0;

   pbMHigh_d                = 0;
   pbMLow_d                 = 0;
   iMXRes_d                 = 0;
   iMYRes_d                 = 0;
   iMIntensityLevels_d      = 0;

   pbYHigh_d                = 0;
   pbYLow_d                 = 0;
   iYXRes_d                 = 0;
   iYYRes_d                 = 0;
   iYIntensityLevels_d      = 0;

   if (pLayout)
   {
      iNumComponents_d    = pLayout->uComponents;
      MY_PRINT_VAR (iNumComponents_d);

      iKXRes_d            = ENDIAN_SWAP_SHORT (pLayout->usKXRes);
      iKYRes_d            = ENDIAN_SWAP_SHORT (pLayout->usKYRes);
      iKIntensityLevels_d = ENDIAN_SWAP_SHORT (pLayout->usKNumComponents);

      MY_PRINT_VAR (iKXRes_d);
      MY_PRINT_VAR (iKYRes_d);
      MY_PRINT_VAR (iKIntensityLevels_d);

      if (2 <= iNumComponents_d)
      {
         iCXRes_d            = ENDIAN_SWAP_SHORT (pLayout->usCXRes);
         iCYRes_d            = ENDIAN_SWAP_SHORT (pLayout->usCYRes);
         iCIntensityLevels_d = ENDIAN_SWAP_SHORT (pLayout->usCNumComponents);

         MY_PRINT_VAR (iCXRes_d);
         MY_PRINT_VAR (iCYRes_d);
         MY_PRINT_VAR (iCIntensityLevels_d);
      }

      if (3 <= iNumComponents_d)
      {
         iMXRes_d            = ENDIAN_SWAP_SHORT (pLayout->usMXRes);
         iMYRes_d            = ENDIAN_SWAP_SHORT (pLayout->usMYRes);
         iMIntensityLevels_d = ENDIAN_SWAP_SHORT (pLayout->usMNumComponents);

         MY_PRINT_VAR (iMXRes_d);
         MY_PRINT_VAR (iMYRes_d);
         MY_PRINT_VAR (iMIntensityLevels_d);
      }

      if (4 <= iNumComponents_d)
      {
         iYXRes_d            = ENDIAN_SWAP_SHORT (pLayout->usYXRes);
         iYYRes_d            = ENDIAN_SWAP_SHORT (pLayout->usYYRes);
         iYIntensityLevels_d = ENDIAN_SWAP_SHORT (pLayout->usYNumComponents);

         MY_PRINT_VAR (iYXRes_d);
         MY_PRINT_VAR (iYYRes_d);
         MY_PRINT_VAR (iYIntensityLevels_d);
      }

      if (  1   == iNumComponents_d
         && 600 == iKXRes_d
         && 2   == iKIntensityLevels_d
         )
      {
         eScanLineOrder_d = ORDER_600x2;
      }
      else if (  1   == iNumComponents_d
              && 300 == iKXRes_d
              && 4   == iKIntensityLevels_d
              )
      {
         eScanLineOrder_d = ORDER_300x4;
      }
      else if (  1   == iNumComponents_d
              && 300 == iKXRes_d
              && 2   == iKIntensityLevels_d
              )
      {
         eScanLineOrder_d = ORDER_300x2;
      }
      else if (  1   == iNumComponents_d
              && 150 == iKXRes_d
              && 2   == iKIntensityLevels_d
              )
      {
         eScanLineOrder_d = ORDER_150x2;
      }
      else if (  4   == iNumComponents_d
              && 600 == iKXRes_d
              && 2   == iKIntensityLevels_d
              && 300 == iCXRes_d
              && 4   == iCIntensityLevels_d
              && 300 == iMXRes_d
              && 4   == iMIntensityLevels_d
              && 300 == iYXRes_d
              && 4   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_600x2_300x4_300x4_300x4;
      }
      else if (  4   == iNumComponents_d
              && 600 == iKXRes_d
              && 2   == iKIntensityLevels_d
              && 300 == iCXRes_d
              && 2   == iCIntensityLevels_d
              && 300 == iMXRes_d
              && 2   == iMIntensityLevels_d
              && 300 == iYXRes_d
              && 2   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_600x2_300x2_300x2_300x2;
      }
      else if (  4   == iNumComponents_d
              && 300 == iKXRes_d
              && 2   == iKIntensityLevels_d
              && 300 == iCXRes_d
              && 2   == iCIntensityLevels_d
              && 300 == iMXRes_d
              && 2   == iMIntensityLevels_d
              && 300 == iYXRes_d
              && 2   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_300x2_300x2_300x2_300x2;
      }
      else if (  4   == iNumComponents_d
              && 300 == iKXRes_d
              && 4   == iKIntensityLevels_d
              && 300 == iCXRes_d
              && 4   == iCIntensityLevels_d
              && 300 == iMXRes_d
              && 4   == iMIntensityLevels_d
              && 300 == iYXRes_d
              && 4   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_300x4_300x4_300x4_300x4;
      }
      else if (  4   == iNumComponents_d
              && 300 == iKXRes_d
              && 4   == iKIntensityLevels_d
              && 300 == iCXRes_d
              && 3   == iCIntensityLevels_d
              && 300 == iMXRes_d
              && 3   == iMIntensityLevels_d
              && 300 == iYXRes_d
              && 3   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_300x2_300x3_300x3_300x3;
      }
      else if (  4   == iNumComponents_d
              && 300 == iKXRes_d
              && 4   == iKIntensityLevels_d
              && 300 == iCXRes_d
              && 2   == iCIntensityLevels_d
              && 300 == iMXRes_d
              && 2   == iMIntensityLevels_d
              && 300 == iYXRes_d
              && 2   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_300x4_300x2_300x2_300x2;
      }
      else if (  4   == iNumComponents_d
              && 150 == iKXRes_d
              && 2   == iKIntensityLevels_d
              && 150 == iCXRes_d
              && 2   == iCIntensityLevels_d
              && 150 == iMXRes_d
              && 2   == iMIntensityLevels_d
              && 150 == iYXRes_d
              && 2   == iYIntensityLevels_d
           )
      {
         eScanLineOrder_d = ORDER_150x2_150x2_150x2_150x2;
      }
      else
      {
         eScanLineOrder_d = ORDER_UNKNOWN;

         cerr << "Not a resolution format we know about!" << endl;
         assertF (iScanLineOrder_d);
      }
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
ScanLines ::
~ScanLines (void)
{
   if (pMonoBitmap_d)
   {
      delete pMonoBitmap_d;
   }

   if (pColorBitmap_d)
   {
      delete pColorBitmap_d;
   }

   if (pbExtColorScanLine_d)
   {
      free (pbExtColorScanLine_d);
   }

   if (pbKHigh_d)
   {
      free (pbKHigh_d);
   }

   if (pbCHigh_d)
   {
      free (pbCHigh_d);
   }

   if (pbMHigh_d)
   {
      free (pbMHigh_d);
   }

   if (pbYHigh_d)
   {
      free (pbYHigh_d);
   }

   if (pbLastScanline_d)
   {
      free (pbLastScanline_d);
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
SetPageSize (int iForm)
{
   int     iSizeXhmm   = 0;
   int     iSizeYhmm   = 0;
   int     iLeftEdge   = 0;
   int     iRightEdge  = 0;
   int     iTopEdge    = 0;
   int     iBottomEdge = 0;

   if (fInitialized_d)
   {
      cerr << __FUNCTION__ << "Error! Set the size too late!" << endl;
      return;
   }

   switch (iForm)
   {
   case 1: // U.S. Executive
   {
      iCurrentFormSize_d = iForm;
      iSizeXhmm          = 18410;
      iSizeYhmm          = 26670;
      iLeftEdge          =   630;
      iRightEdge         =   630;
      iTopEdge           =   100;
      iBottomEdge        =  1170;
      break;
   }

   case 2: // U.S. Letter
   {
      iCurrentFormSize_d = iForm;
      iSizeXhmm          = 21590;
      iSizeYhmm          = 27940;
      iLeftEdge          =   630;
      iRightEdge         =   630;
      iTopEdge           =   100;
      iBottomEdge        =  1170;
      break;
   }

   default:
      cerr << "Unknown form = " << iForm << "!" << endl;
      return;
   }

   if (  0 != iSizeXhmm
      && 0 != iSizeYhmm
      )
   {
      int   xRes;
      int   yRes;

      // @TBD - account for orientation!

      iSizeXhmm -= (iLeftEdge + iRightEdge);
      iSizeYhmm -= (iTopEdge  + iBottomEdge);

      xRes = iKXRes_d;
      yRes = iKYRes_d;

      cxMono_d = (int)((float)xRes * (float)iSizeXhmm / 2540.0);
      cyMono_d = (int)((float)yRes * (float)iSizeYhmm / 2540.0);

      xRes = omni::max (iCXRes_d, Omni::max (iMXRes_d, iYXRes_d));
      yRes = omni::max (iCYRes_d, Omni::max (iMYRes_d, iYYRes_d));

      cxColor_d = (int)((float)xRes * (float)iSizeXhmm / 2540.0);
      cyColor_d = (int)((float)yRes * (float)iSizeYhmm / 2540.0);

      cerr << __FUNCTION__ << ": Mono size is now (" << cxMono_d << ", " << cyMono_d << ") mono pels" << endl;
      cerr << __FUNCTION__ << ": Color size is now (" << cxColor_d << ", " << cyColor_d << ") mono pels" << endl;

      if (2 == iKIntensityLevels_d)
         iExtMonoBitsPerPel = 1;
      else if (4 == iKIntensityLevels_d)
         iExtMonoBitsPerPel = 8;
      iExtColorBitsPerPel = 8;

      // Double word align it
      iSizeMonoScanLine_d     = (cxMono_d  * NumBits (iKIntensityLevels_d) + 31) / 32 * 4;
      iSizeExtMonoScanLine_d  = (cxMono_d  * iExtMonoBitsPerPel + 31) / 32 * 4;
      iSizeColorScanLine_d    = (cxColor_d * NumBits (iCIntensityLevels_d) + 31) / 32 * 4;
      iSizeExtColorScanLine_d = (cxColor_d * iExtColorBitsPerPel + 31) / 32 * 4;
      MY_PRINT_VAR (iSizeMonoScanLine_d);
      MY_PRINT_VAR (iSizeColorScanLine_d);
      MY_PRINT_VAR (iSizeExtColorScanLine_d);
      MY_PRINT_VAR (iSizeExtMonoScanLine_d);
      MY_PRINT_VAR (iExtMonoBitsPerPel);

      if (0 != iSizeMonoScanLine_d)
      {
         if (pbKHigh_d)
         {
            free (pbKHigh_d);
            pbKHigh_d = NULL;
         }

         if (pbExtMonoScanLine_d)
         {
            free (pbExtMonoScanLine_d);
            pbExtMonoScanLine_d = NULL;
         }

         pbKHigh_d = (PBYTE)malloc (2 * iSizeMonoScanLine_d);
         assertF (pbKHigh_d);

         if (pbKHigh_d)
            pbKLow_d = pbKHigh_d + iSizeMonoScanLine_d;
         else
            pbKLow_d = NULL;

/////////@TBD
/////////if (iExtMonoBitsPerPel != NumBits (iKIntensityLevels_d))
         {
            pbExtMonoScanLine_d = (PBYTE)malloc (iSizeExtMonoScanLine_d);
            assertF (pbExtMonoScanLine_d);
         }
      }

      if (0 != iSizeColorScanLine_d)
      {
         if (pbCHigh_d)
         {
            free (pbCHigh_d);
            pbCHigh_d = NULL;
         }

         pbCHigh_d = (PBYTE)malloc (2 * iSizeColorScanLine_d);
         assertF (pbCHigh_d);

         if (pbCHigh_d)
            pbCLow_d = pbCHigh_d + iSizeColorScanLine_d;
         else
            pbCLow_d = NULL;

         if (pbMHigh_d)
         {
            free (pbMHigh_d);
            pbMHigh_d = NULL;
         }

         pbMHigh_d = (PBYTE)malloc (2 * iSizeColorScanLine_d);
         assertF (pbMHigh_d);

         if (pbMHigh_d)
            pbMLow_d = pbMHigh_d + iSizeColorScanLine_d;
         else
            pbMLow_d = NULL;

         if (pbYHigh_d)
         {
            free (pbYHigh_d);
            pbYHigh_d = NULL;
         }

         pbYHigh_d = (PBYTE)malloc (2 * iSizeColorScanLine_d);
         assertF (pbYHigh_d);

         if (pbYHigh_d)
            pbYLow_d = pbYHigh_d + iSizeColorScanLine_d;
         else
            pbYLow_d = NULL;
      }

      if (0 != iSizeExtColorScanLine_d)
      {
         if (pbExtColorScanLine_d)
         {
            free (pbExtColorScanLine_d);
            pbExtColorScanLine_d = NULL;
         }

         pbExtColorScanLine_d = (PBYTE)malloc (iSizeExtColorScanLine_d);
         assertF (pbExtColorScanLine_d);
      }

      cbLastScanline_d = omni::max (cxMono_d, cxColor_d);
      cbLastScanline_d = (cbLastScanline_d + 7) / 8; // byte align it
      pbLastScanline_d = (PBYTE)malloc (cbLastScanline_d);
   }
   else
   {
      cerr << __FUNCTION__ << ": Error! Did not set the size" << endl;
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
SetCompressionLevel (int iCompressionLevel)
{
   iCompressionLevel_d = iCompressionLevel;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
AddScanLine (int cbScanData, PBYTE pbScanData)
{
   PBYTE   pbTo     = NULL;
   int     iMaxData = 0;

///cerr << "plane #" << iWhichPlane_d << " has " << cbScanData << " bytes" << endl;

   switch (eScanLineOrder_d)
   {
   case ORDER_300x4_300x4_300x4_300x4:
   case ORDER_600x2_300x4_300x4_300x4:
   {
      switch (iWhichPlane_d)
      {
      case 0: pbTo = pbKLow_d;  iMaxData = iSizeMonoScanLine_d;  break;
      case 1: pbTo = pbKHigh_d; iMaxData = iSizeMonoScanLine_d;  break;
      case 2: pbTo = pbCLow_d;  iMaxData = iSizeColorScanLine_d; break;
      case 3: pbTo = pbCHigh_d; iMaxData = iSizeColorScanLine_d; break;
      case 4: pbTo = pbMLow_d;  iMaxData = iSizeColorScanLine_d; break;
      case 5: pbTo = pbMHigh_d; iMaxData = iSizeColorScanLine_d; break;
      case 6: pbTo = pbYLow_d;  iMaxData = iSizeColorScanLine_d; break;
      case 7: pbTo = pbYHigh_d; iMaxData = iSizeColorScanLine_d; break;
      }

      if (7 == iWhichPlane_d)
         iWhichPlane_d = 0;
      else
         iWhichPlane_d++;

      break;
   }

   case ORDER_300x2_300x2_300x2_300x2:
   {
      switch (iWhichPlane_d)
      {
      case 0: pbTo = pbKLow_d; iMaxData = iSizeMonoScanLine_d;  break;
      case 1: pbTo = pbCLow_d; iMaxData = iSizeColorScanLine_d; break;
      case 2: pbTo = pbMLow_d; iMaxData = iSizeColorScanLine_d; break;
      case 3: pbTo = pbYLow_d; iMaxData = iSizeColorScanLine_d; break;
      }

      if (3 == iWhichPlane_d)
         iWhichPlane_d = 0;
      else
         iWhichPlane_d++;

      break;
   }

   case ORDER_300x2:
   {
      pbTo = pbKLow_d;  iMaxData = iSizeMonoScanLine_d;  break;
   }

   default:
      cerr << "Unhandled case = " << (int)eScanLineOrder_d << "!" << endl;
      break;
   }

   if (pbTo)
      DecompressScanLine (pbScanData,
                          cbScanData,
                          pbTo,
                          iMaxData,
                          iCompressionLevel_d);

   if (0 == iWhichPlane_d)
   {
      // We have wrapped... Write the scanlines out
      DumpScanLines ();
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
MoveToY (int iYPosition)
{
   if (iYPosition + iColorScanLinesWritten_d < cyColor_d)
   {
      iColorScanLinesWritten_d += iYPosition;
   }

   if (iYPosition + iMonoScanLinesWritten_d < cyMono_d)
   {
      iMonoScanLinesWritten_d += iYPosition;
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
DumpScanLines (void)
{
///cerr << __FUNCTION__ << ": enter" << endl;

   if (!fInitialized_d)
   {
      cerr << "sizeof (OS2BITMAPFILEHEADER) = " << sizeof (OS2BITMAPFILEHEADER) << endl;
      cerr << "sizeof (OS2BITMAPINFOHEADER) = " << sizeof (OS2BITMAPINFOHEADER) << endl;
      cerr << "sizeof (OS2RGB) = " << sizeof (OS2RGB) << endl;

      if (0 == iCurrentFormSize_d)
         SetPageSize (2);

      // Write the bitmap header out
      fInitialized_d = true;

      if (pMonoBitmap_d)
      {
         delete pMonoBitmap_d;
         pMonoBitmap_d = 0;
      }

      if (0 != iSizeMonoScanLine_d)
      {
         if (1 == iExtMonoBitsPerPel)
         {
            static NEUTRALRGB aMonoColors[] = {
               /* writen out in blue green red order */
               /* K=                                 */
               /* 0  */ { 0xFF, 0xFF, 0xFF },
               /* 1  */ { 0x00, 0x00, 0x00 }
            };

            pMonoBitmap_d = new Bitmap ("k.bmp",
                                        cxMono_d,
                                        cyMono_d,
                                        iExtMonoBitsPerPel,
                                        aMonoColors);
         }
         else if (8 == iExtMonoBitsPerPel)
         {
            static NEUTRALRGB aMonoColors[256] = {
               /* writen out in blue green red order */
               /* K=                                 */
               /* 0  */ { 0xFF, 0xFF, 0xFF },
               /* 1  */ { 0xBF, 0xBF, 0xBF },
               /* 2  */ { 0x7F, 0x7F, 0x7F },
               /* 3  */ { 0x00, 0x00, 0x00 }
            };

            pMonoBitmap_d = new Bitmap ("k.bmp",
                                        cxMono_d,
                                        cyMono_d,
                                        iExtMonoBitsPerPel,
                                        aMonoColors);
         }
      }

      if (pColorBitmap_d)
      {
         delete pColorBitmap_d;
         pColorBitmap_d = 0;
      }

      if (0 != iSizeColorScanLine_d)
      {
         static NEUTRALRGB aColorColors[256] = { // c/{[0-9A-F][0-9A-F]} {[0-9A-F][0-9A-F]} {[0-9A-F][0-9A-F]}/{ 0x#0, 0x#1, 0x#2 },/rm
            /* writen out in blue green red order                                                             */
            /*    C= M=  Y=  0                     1                     2                     3               */
            /*  0 0  0 */ { 0xFF, 0xFF, 0xFF }, { 0xBF, 0xFF, 0xFF }, { 0x7F, 0xFF, 0xFF }, { 0x00, 0xFF, 0xFF },
            /*  4 0  1 */ { 0xFF, 0xBF, 0xFF }, { 0xBF, 0xBF, 0xFF }, { 0x7F, 0xBF, 0xFF }, { 0x00, 0xBF, 0xFF },
            /*  8 0  2 */ { 0xFF, 0x7F, 0xFF }, { 0xBF, 0x7F, 0xFF }, { 0x7F, 0x7F, 0xFF }, { 0x00, 0x7F, 0xFF },
            /* 12 0  3 */ { 0xFF, 0x00, 0xFF }, { 0xBF, 0x00, 0xFF }, { 0x7F, 0x00, 0xFF }, { 0x00, 0x00, 0xFF },
            /* 16 1  0 */ { 0xFF, 0xFF, 0xBF }, { 0xBF, 0xFF, 0xBF }, { 0x7F, 0xFF, 0xBF }, { 0x00, 0xFF, 0xBF },
            /* 20 1  1 */ { 0xFF, 0xBF, 0xBF }, { 0xBF, 0xBF, 0xBF }, { 0x7F, 0xBF, 0xBF }, { 0x00, 0xBF, 0xBF },
            /* 24 1  2 */ { 0xFF, 0x7F, 0xBF }, { 0xBF, 0x7F, 0xBF }, { 0x7F, 0x7F, 0xBF }, { 0x00, 0x7F, 0xBF },
            /* 28 1  3 */ { 0xFF, 0x00, 0xBF }, { 0xBF, 0x00, 0xBF }, { 0x7F, 0x00, 0xBF }, { 0x00, 0x00, 0xBF },
            /* 32 2  0 */ { 0xFF, 0xFF, 0x7F }, { 0xBF, 0xFF, 0x7F }, { 0x7F, 0xFF, 0x7F }, { 0x00, 0xFF, 0x7F },
            /* 36 2  1 */ { 0xFF, 0xBF, 0x7F }, { 0xBF, 0xBF, 0x7F }, { 0x7F, 0xBF, 0x7F }, { 0x00, 0xBF, 0x7F },
            /* 40 2  2 */ { 0xFF, 0x7F, 0x7F }, { 0xBF, 0x7F, 0x7F }, { 0x7F, 0x7F, 0x7F }, { 0x00, 0x7F, 0x7F },
            /* 44 2  3 */ { 0xFF, 0x00, 0x7F }, { 0xBF, 0x00, 0x7F }, { 0x7F, 0x00, 0x7F }, { 0x00, 0x00, 0x7F },
            /* 48 3  0 */ { 0xFF, 0xFF, 0x00 }, { 0xBF, 0xFF, 0x00 }, { 0x7F, 0xFF, 0x00 }, { 0x00, 0xFF, 0x00 },
            /* 52 3  1 */ { 0xFF, 0xBF, 0x00 }, { 0xBF, 0xBF, 0x00 }, { 0x7F, 0xBF, 0x00 }, { 0x00, 0xBF, 0x00 },
            /* 56 3  2 */ { 0xFF, 0x7F, 0x00 }, { 0xBF, 0x7F, 0x00 }, { 0x7F, 0x7F, 0x00 }, { 0x00, 0x7F, 0x00 },
            /* 60 3  3 */ { 0xFF, 0x00, 0x00 }, { 0xBF, 0x00, 0x00 }, { 0x7F, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }
         };

         pColorBitmap_d = new Bitmap ("cmy.bmp",
                                      cxColor_d,
                                      cyColor_d,
                                      iExtColorBitsPerPel,
                                      aColorColors);
      }
   }

   // Write the scan lines out
   switch (eScanLineOrder_d)
   {
   case ORDER_600x2_300x4_300x4_300x4:
   {
      if (pMonoBitmap_d)
      {
         if (pbKHigh_d)
            pMonoBitmap_d->addScanLine (pbKHigh_d, 1);
         if (pbKLow_d)
            pMonoBitmap_d->addScanLine (pbKLow_d, 1);
      }

      if (pColorBitmap_d)
      {
         // Write the cyan, magenta, and yellow planes out
         Xfer600x2_300x4_300x4_300x4 ();
      }
      break;
   }

   case ORDER_300x4_300x4_300x4_300x4:
   {
      if (  pMonoBitmap_d
         && pColorBitmap_d
         )
      {
         // Write the cyan, magenta, and yellow planes out
         Xfer300x4_300x4_300x4_300x4 ();
      }
      break;
   }

   case ORDER_300x2_300x2_300x2_300x2:
   {
      if (  pMonoBitmap_d
         && pColorBitmap_d
         )
      {
         // Write the cyan, magenta, and yellow planes out
         Xfer300x2_300x2_300x2_300x2 ();
      }
      break;
   }

   case ORDER_300x2:
   {
      if (pMonoBitmap_d)
      {
         if (pbKLow_d)
            pMonoBitmap_d->addScanLine (pbKLow_d, 1);
      }
      break;
   }

   default:
      cerr << "Unhandled case = " << (int)eScanLineOrder_d << "!" << endl;
      break;
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
Xfer600x2_300x4_300x4_300x4 (void)
{
   int           iStage;
   unsigned int *puiCHighScanLine;
   unsigned int *puiCLowScanLine;
   unsigned int *puiMHighScanLine;
   unsigned int *puiMLowScanLine;
   unsigned int *puiYHighScanLine;
   unsigned int *puiYLowScanLine;
   int           iBitsCHigh;
   int           iBitsCLow;
   int           iBitsMHigh;
   int           iBitsMLow;
   int           iBitsYHigh;
   int           iBitsYLow;
   unsigned int  uiMaskLine;
   PBYTE         pbOutput;
   BYTE          pel;
   register int  i;

   if (!pbExtColorScanLine_d)
      // Error!
      return;

   // Clear out target scan line
   memset (pbExtColorScanLine_d, 0, iSizeExtColorScanLine_d);

   iStage           = -1;
   i                = 0;
   puiCHighScanLine = (unsigned int *)pbCHigh_d;
   puiCLowScanLine  = (unsigned int *)pbCLow_d;
   puiMHighScanLine = (unsigned int *)pbMHigh_d;
   puiMLowScanLine  = (unsigned int *)pbMLow_d;
   puiYHighScanLine = (unsigned int *)pbYHigh_d;
   puiYLowScanLine  = (unsigned int *)pbYLow_d;
   pbOutput         = pbExtColorScanLine_d;

   while (i < cxColor_d)
   {
      if (-1 == iStage)
      {
         iBitsCHigh = *puiCHighScanLine++;
         iBitsCLow  = *puiCLowScanLine++;
         iBitsMHigh = *puiMHighScanLine++;
         iBitsMLow  = *puiMLowScanLine++;
         iBitsYHigh = *puiYHighScanLine++;
         iBitsYLow  = *puiYLowScanLine++;
         uiMaskLine = 0x00000080L;
         iStage     = 31;
      }

      pel = 0;
      if (iBitsCLow  & uiMaskLine)
         pel++;
      if (iBitsCHigh & uiMaskLine)
         pel += 2;
      pel *= 4;

      if (iBitsMLow  & uiMaskLine)
         pel++;
      if (iBitsMHigh & uiMaskLine)
         pel += 2;
      pel *= 4;

      if (iBitsYLow  & uiMaskLine)
         pel++;
      if (iBitsYHigh & uiMaskLine)
         pel += 2;

      *pbOutput++ = pel;

      /* The input DWORD has its bits in the following format (two WORDs side by side):
      **                            |           |           |           |           |           |           |
      ** iShiftOutput = 07 06 05 04 03 02 01 00 15 14 13 12 11 10 09 08 23 22 21 20 19 18 17 16 31 30 29 28 27 26 25 24
      **                                                                |
      */
      iStage--;
      uiMaskLine >>= 1;
      switch (iStage)
      {
      case 23: uiMaskLine = 0x00008000L; break;
      case 15: uiMaskLine = 0x00800000L; break;
      case  7: uiMaskLine = 0x80000000L; break;
      }

      i++;
   }

   // Done
   pColorBitmap_d->addScanLine (pbExtColorScanLine_d, 1);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
Xfer300x4_300x4_300x4_300x4 (void)
{
   int           iStage;
   unsigned int *puiCHighScanLine;
   unsigned int *puiCLowScanLine;
   unsigned int *puiMHighScanLine;
   unsigned int *puiMLowScanLine;
   unsigned int *puiYHighScanLine;
   unsigned int *puiYLowScanLine;
   unsigned int *puiKHighScanLine;
   unsigned int *puiKLowScanLine;
   unsigned int  iBitsCHigh;
   unsigned int  iBitsCLow;
   unsigned int  iBitsMHigh;
   unsigned int  iBitsMLow;
   unsigned int  iBitsYHigh;
   unsigned int  iBitsYLow;
   unsigned int  iBitsKHigh;
   unsigned int  iBitsKLow;
   unsigned int  uiMaskLine;
   PBYTE         pbCMYOutput;
   PBYTE         pbKOutput;
   BYTE          pel;
   register int  i;

   if (!pbExtColorScanLine_d)
      // Error!
      return;

   // Clear out target scan line
   memset (pbExtColorScanLine_d, 0, iSizeExtColorScanLine_d);

   iStage           = -1;
   i                = 0;
   puiCHighScanLine = (unsigned int *)pbCHigh_d;
   puiCLowScanLine  = (unsigned int *)pbCLow_d;
   puiMHighScanLine = (unsigned int *)pbMHigh_d;
   puiMLowScanLine  = (unsigned int *)pbMLow_d;
   puiYHighScanLine = (unsigned int *)pbYHigh_d;
   puiYLowScanLine  = (unsigned int *)pbYLow_d;
   puiKHighScanLine = (unsigned int *)pbKHigh_d;
   puiKLowScanLine  = (unsigned int *)pbKLow_d;
   pbCMYOutput      = pbExtColorScanLine_d;
   pbKOutput        = pbExtMonoScanLine_d;

   while (i < cxColor_d)
   {
      if (-1 == iStage)
      {
         iBitsCHigh = *puiCHighScanLine++;
         iBitsCLow  = *puiCLowScanLine++;
         iBitsMHigh = *puiMHighScanLine++;
         iBitsMLow  = *puiMLowScanLine++;
         iBitsYHigh = *puiYHighScanLine++;
         iBitsYLow  = *puiYLowScanLine++;
         iBitsKHigh = *puiKHighScanLine++;
         iBitsKLow  = *puiKLowScanLine++;
         uiMaskLine = 0x00000080L;
         iStage     = 31;
      }

      pel = 0;
      if (iBitsCLow  & uiMaskLine)
         pel++;
      if (iBitsCHigh & uiMaskLine)
         pel += 2;
      pel *= 4;

      if (iBitsMLow  & uiMaskLine)
         pel++;
      if (iBitsMHigh & uiMaskLine)
         pel += 2;
      pel *= 4;

      if (iBitsYLow  & uiMaskLine)
         pel++;
      if (iBitsYHigh & uiMaskLine)
         pel += 2;

      *pbCMYOutput++ = pel;

      pel = 0;
      if (iBitsKLow  & uiMaskLine)
         pel++;
      if (iBitsKHigh & uiMaskLine)
         pel += 2;

#if 0
      if (0 == (i & 3))
      {
         *pbKOutput   |= pel << 6;
      }
      else if (1 == (i & 3))
      {
         *pbKOutput   |= pel << 4;
      }
      else if (2 == (i & 3))
      {
         *pbKOutput   |= pel << 2;
      }
      else if (3 == (i & 3))
      {
         *pbKOutput++ |= pel;
      }
#else  // @TEST
      *pbKOutput++ = pel;
#endif

      /* The input DWORD has its bits in the following format (two WORDs side by side):
      **                            |           |           |           |           |           |           |
      ** iShiftOutput = 07 06 05 04 03 02 01 00 15 14 13 12 11 10 09 08 23 22 21 20 19 18 17 16 31 30 29 28 27 26 25 24
      **                                                                |
      */
      iStage--;
      uiMaskLine >>= 1;
      switch (iStage)
      {
      case 23: uiMaskLine = 0x00008000L; break;
      case 15: uiMaskLine = 0x00800000L; break;
      case  7: uiMaskLine = 0x80000000L; break;
      }

      i++;
   }

   // Done
   pColorBitmap_d->addScanLine (pbExtColorScanLine_d, 1);
   pMonoBitmap_d->addScanLine  (pbExtMonoScanLine_d, 1);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
Xfer300x2_300x2_300x2_300x2 (void)
{
   int           iStage;
   unsigned int *puiCLowScanLine;
   unsigned int *puiMLowScanLine;
   unsigned int *puiYLowScanLine;
   unsigned int *puiKLowScanLine;
   int           iBitsCLow;
   int           iBitsMLow;
   int           iBitsYLow;
   int           iBitsKLow;
   unsigned int  uiMaskLine;
   PBYTE         pbOutput;
   BYTE          pel;
   register int  i;

   if (!pbExtMonoScanLine_d)
      // Error!
      return;

   cerr << "Xfer300x2_300x2_300x2_300x2" << endl;

   iStage           = -1;
   i                = 0;
   puiKLowScanLine  = (unsigned int *)pbKLow_d;
   pbOutput         = pbExtMonoScanLine_d;

   // Clear out target scan line
   memset (pbExtMonoScanLine_d, 0, iSizeExtMonoScanLine_d);

   while (i < cxMono_d)
   {
      if (-1 == iStage)
      {
         iBitsKLow  = *puiKLowScanLine++;
         uiMaskLine = 0x00000080L;
         iStage     = 31;
      }

      if (iBitsKLow & uiMaskLine)
      {
         switch (i % 8)
         {
         case 0: *pbOutput |= 0x01; break;
         case 1: *pbOutput |= 0x02; break;
         case 2: *pbOutput |= 0x04; break;
         case 3: *pbOutput |= 0x08; break;
         case 4: *pbOutput |= 0x10; break;
         case 5: *pbOutput |= 0x20; break;
         case 6: *pbOutput |= 0x40; break;
         case 7: *pbOutput |= 0x80; break;
         }
      }
      if (0 == i % 8)
         pbOutput++;

      /* The input DWORD has its bits in the following format (two WORDs side by side):
      **                            |           |           |           |           |           |           |
      ** iShiftOutput = 07 06 05 04 03 02 01 00 15 14 13 12 11 10 09 08 23 22 21 20 19 18 17 16 31 30 29 28 27 26 25 24
      **                                                                |
      */
      iStage--;
      uiMaskLine >>= 1;
      switch (iStage)
      {
      case 23: uiMaskLine = 0x00008000L; break;
      case 15: uiMaskLine = 0x00800000L; break;
      case  7: uiMaskLine = 0x80000000L; break;
      }

      i++;
   }

   pMonoBitmap_d->addScanLine (pbExtMonoScanLine_d, 1);

   if (!pbExtColorScanLine_d)
      // Error!
      return;

   // Clear out target scan line
   memset (pbExtColorScanLine_d, 0, iSizeExtColorScanLine_d);

   iStage           = -1;
   i                = 0;
   puiCLowScanLine  = (unsigned int *)pbCLow_d;
   puiMLowScanLine  = (unsigned int *)pbMLow_d;
   puiYLowScanLine  = (unsigned int *)pbYLow_d;
   pbOutput         = pbExtColorScanLine_d;

   while (i < cxColor_d)
   {
      if (-1 == iStage)
      {
         iBitsCLow  = *puiCLowScanLine++;
         iBitsMLow  = *puiMLowScanLine++;
         iBitsYLow  = *puiYLowScanLine++;
         uiMaskLine = 0x00000080L;
         iStage     = 31;
      }

      pel = 0;
      if (iBitsCLow & uiMaskLine)
         pel++;
      pel *= 4;

      if (iBitsMLow & uiMaskLine)
         pel++;
      pel *= 4;

      if (iBitsYLow & uiMaskLine)
         pel++;

      *pbOutput++ = pel;

      /* The input DWORD has its bits in the following format (two WORDs side by side):
      **                            |           |           |           |           |           |           |
      ** iShiftOutput = 07 06 05 04 03 02 01 00 15 14 13 12 11 10 09 08 23 22 21 20 19 18 17 16 31 30 29 28 27 26 25 24
      **                                                                |
      */
      iStage--;
      uiMaskLine >>= 1;
      switch (iStage)
      {
      case 23: uiMaskLine = 0x00008000L; break;
      case 15: uiMaskLine = 0x00800000L; break;
      case  7: uiMaskLine = 0x80000000L; break;
      }

      i++;
   }

   // Done
   pColorBitmap_d->addScanLine (pbExtColorScanLine_d, 1);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void ScanLines ::
DecompressScanLine (PBYTE pbScanData,
                    int   cbScanData,
                    PBYTE pbTo,
                    int   cbTo,
                    int   iCompressionLevel)
{
///cerr << __FUNCTION__ << ": enter" << endl;

   if (  !pbScanData
      || !pbTo
      || 0 == cbScanData
      || 0 == cbTo
      )
      return;

   PBYTE pbOriginalTo = pbTo;
   int   cbOriginalTo = cbTo;

   memset (pbTo, 0, cbTo);

   switch (iCompressionLevel)
   {
   case 0: // no compression
   {
      memcpy (pbTo, pbScanData, omni::min (cbScanData, cbTo));
      break;
   }

   case 1: // rll compression (HP Mode 1)
   {
      unsigned char cDups,
                    cChar;
      int           i;

      while (  0 < cbScanData
            && 0 < cbTo
            )
      {
         cDups = *pbScanData++; cbScanData--;
         cChar = *pbScanData++; cbScanData--;

         for (i = 0; i <= cDups && 0 < cbTo; i--)
         {
            *pbTo++ = cChar;
            cbTo--;
         }
      }
      break;
   }

   case 2: // tagged image file format encoding (HP Mode 2)
   {
      signed char c;

      while (  0 < cbScanData
            && 0 < cbTo
            )
      {
         c = *pbScanData++;
         cbScanData--;

         if (0 <= c)
         {
            while (  0 <= c
                  && 0 < cbTo
                  )
            {
               *pbTo++ = *pbScanData++;
               cbTo--;
               cbScanData--;
               c--;
            }
         }
         else
         {
            while (  0 >= c
                  && 0 < cbTo
                  )
            {
               *pbTo++ = *pbScanData;
               cbTo--;
               c++;
            }

            pbScanData++;
            cbScanData--;
         }
      }
      break;
   }

   case 3:  // delta compression (HP Mode 3)
   {
      int iBytesToReplace,
          iOffset,
          i;

      memcpy (pbTo, pbLastScanline_d, omni::min (cbLastScanline_d, cbTo));

      while (  0 < cbScanData
            && 0 < cbTo
            )
      {
         iBytesToReplace = ((*pbScanData & 0xE0) >> 5) + 1;
         iOffset         = (*pbScanData & 0x1F);
         pbScanData++;
         cbScanData--;

         if (0x1F == iOffset)
         {
            while (  0xFF == *pbScanData
                  && 0 < cbScanData
                  )
            {
               iOffset += *pbScanData++;
               cbScanData--;
            }
            iOffset += *pbScanData++;
            cbScanData--;
         }

         if (cbTo > iOffset)
         {
            pbTo += iOffset;
            cbTo -= iOffset;
            for (i = 0;
                 (  i < iBytesToReplace
                 && 0 < cbScanData
                 && 0 < cbTo
                 );
                 i++)
            {
               *pbTo++ = *pbScanData++;
               cbTo--;
               cbScanData--;
            }
         }
      }
      break;
   }

   default:
      cerr << "unknown compression level = " << iCompressionLevel << "!" << endl;
      break;
   }

   if (pbLastScanline_d)
      memcpy (pbLastScanline_d,
              pbOriginalTo,
              cbOriginalTo);

///cerr << __FUNCTION__ << ": exit" << endl;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int
NumBits (int iMaxValue)
{
   int iRet = 0;

   cerr << __FUNCTION__ << ": iMaxValue " << iMaxValue << endl;

   if (0 == iMaxValue)
      return 0;

   while (1 < iMaxValue)
   {
      iMaxValue /= 2;
      iRet++;
   }

   cerr << __FUNCTION__ << ": returns " << iRet << endl;

   return iRet;
}
