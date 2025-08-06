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
#include "MonoDither.hpp"
#include "Device.hpp"

// Prebuilt Magic table for halftone ordered dithers - Alternate Set

static BYTE GammaTbl [256] = {
     0,  3,    5,   8,  10,  12,  15,  17,  19,  22,  24,  26,  28,  29,  32,  34,
    35,  37,  38,  41,  43,  45,  46,  48,  49,  51,  53,  54,  56,  57,  59,  61,
    62,  64,  65,  67,  69,  71,  73,  74,  75,  77,  79,  80,  82,  84,  86,  87,
    89,  90,  92,  93,  94,  96,  98,  99, 100, 102, 103, 105, 106, 107, 109, 111,
   114, 118, 121, 125, 129, 132, 136, 139, 142, 145, 148, 151, 154, 156, 158, 160,
   160, 161, 162, 163, 164, 164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172,
   173, 173, 174, 175, 175, 176, 177, 177, 178, 179, 179, 180, 181, 182, 182, 183,
   183, 184, 185, 185, 186, 187, 187, 188, 189, 189, 190, 190, 191, 192, 192, 193,
   194, 194, 195, 195, 196, 197, 197, 198, 198, 199, 199, 200, 201, 201, 202, 202,
   203, 203, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210, 211, 211,
   212, 212, 213, 213, 214, 214, 215, 215, 216, 216, 217, 217, 218, 218, 219, 219,
   220, 220, 221, 221, 222, 222, 223, 223, 224, 224, 225, 225, 226, 226, 227, 227,
   228, 228, 229, 229, 230, 230, 230, 231, 231, 232, 232, 233, 233, 234, 234, 235,
   235, 235, 236, 236, 237, 237, 238, 238, 239, 239, 240, 240, 240, 241, 241, 242,
   242, 243, 243, 243, 244, 244, 245, 245, 246, 246, 246, 247, 247, 248, 248, 248,
   249, 249, 250, 250, 251, 251, 251, 252, 252, 253, 253, 253, 254, 254, 255, 255
};

#if 0
static BYTE magicbw[16][16] = {
     { 0xFF, 0x20, 0xd0, 0x30, 0xf2, 0x12, 0xc2, 0x22, 0xfd, 0x1d, 0xcd, 0x2d, 0xf3, 0x13, 0xc3, 0x23 },
     { 0x50, 0xb0, 0x80, 0xa0, 0x42, 0xa2, 0x72, 0x92, 0x4d, 0xad, 0x7d, 0x9d, 0x43, 0xa3, 0x73, 0x93 },
     { 0x40, 0xe0, 0x10, 0xf0, 0x32, 0xd2, 0x02, 0xe2, 0x3d, 0xdd, 0x0d, 0xed, 0x33, 0xd3, 0x03, 0xe3 },
     { 0x90, 0x70, 0xc0, 0x60, 0x82, 0x62, 0xb2, 0x52, 0x8d, 0x6d, 0xbd, 0x5d, 0x83, 0x63, 0xb3, 0x53 },
     { 0xf5, 0x15, 0xc5, 0x25, 0xfb, 0x1b, 0xcb, 0x2b, 0xf8, 0x18, 0xc8, 0x28, 0xfa, 0x1a, 0xca, 0x2a },
     { 0x45, 0xa5, 0x75, 0x95, 0x4b, 0xab, 0x7b, 0x9b, 0x48, 0xa8, 0x78, 0x98, 0x4a, 0xaa, 0x7a, 0x9a },
     { 0x35, 0xd5, 0x05, 0xe5, 0x3b, 0xdb, 0x0b, 0xeb, 0x38, 0xd8, 0x08, 0xe8, 0x3a, 0xda, 0x0a, 0xea },
     { 0x85, 0x65, 0xb5, 0x55, 0x8b, 0x6b, 0xbb, 0x5b, 0x88, 0x68, 0xb8, 0x58, 0x8a, 0x6a, 0xba, 0x5a },
     { 0xf4, 0x14, 0xc4, 0x24, 0xfe, 0x1e, 0xce, 0x2e, 0xf1, 0x11, 0xc1, 0x21, 0xff, 0x1f, 0xcf, 0x2f },
     { 0x44, 0xa4, 0x74, 0x94, 0x4e, 0xae, 0x7e, 0x9e, 0x41, 0xa1, 0x71, 0x91, 0x4f, 0xaf, 0x7f, 0x9f },
     { 0x34, 0xd4, 0x04, 0xe4, 0x3e, 0xde, 0x0e, 0xee, 0x31, 0xd1, 0x01, 0xe1, 0x3f, 0xdf, 0x0f, 0xef },
     { 0x84, 0x64, 0xb4, 0x54, 0x8e, 0x6e, 0xbe, 0x5e, 0x81, 0x61, 0xb1, 0x51, 0x8f, 0x6f, 0xbf, 0x5f },
     { 0xf9, 0x19, 0xc9, 0x29, 0xf7, 0x17, 0xc7, 0x27, 0xfc, 0x1c, 0xcc, 0x2c, 0xf6, 0x16, 0xc6, 0x26 },
     { 0x49, 0xa9, 0x79, 0x99, 0x47, 0xa7, 0x77, 0x97, 0x4c, 0xac, 0x7c, 0x9c, 0x46, 0xa6, 0x76, 0x96 },
     { 0x39, 0xd9, 0x09, 0xe9, 0x37, 0xd7, 0x07, 0xe7, 0x3c, 0xdc, 0x0c, 0xec, 0x36, 0xd6, 0x06, 0xe6 },
     { 0x89, 0x69, 0xb9, 0x59, 0x87, 0x67, 0xb7, 0x57, 0x8c, 0x6c, 0xbc, 0x5c, 0x86, 0x66, 0xb6, 0x56 }
};
#endif

// Diffusion Dithering - Jarvis, Judice, and Ninke

BYTE    jjn[5][5] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 7, 5 },
   { 3, 5, 7, 5, 3 },
   { 1, 3, 5, 3, 1 }
};

// Diffusion Dithering - Stucki

#define mp_jjn  &jjn[2][2]
#define mw_jjn  2
#define mr_jjn  50

BYTE   stu[5][5] = {
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 8, 4 },
    { 2, 4, 8, 4, 2 },
    { 1, 2, 4, 2, 1 }
};

#define mp_stu             &stu[2][2]
#define mw_stu             2
#define mr_stu             42

#define WHITE_PIXEL        255
#define BLACK_PIXEL        0

#undef  CLAMP
#define CLAMP(n, l, h)     ((n) < (l) ? (l) : ((n) > (h) ? (h) : (n)))
#define WEIGHT(i,j)        (*(mp + (i) + (j * ((2 * mw) + 1))))

/*****************************************************************************/
/*                                                                           */
/*  FUNCTION: ConvertSourceScan                                              */
/*                                                                           */
/*  Convert color pels to mono for mono implementation                       */
/*                                                                           */
/*****************************************************************************/

void
ConvertSourceScan (PBYTE      pbScan,
                   LONG       lscan,
                   PIMAGEINFO pimg_infos,
                   ULONG      bWgtAdj,
                   ULONG      rw,
                   ULONG      gw,
                   ULONG      bw)
{
   RGB2  rgb2;
   PBYTE pSrc;
   ULONG i;

   // We don't need to fill tmp buffers past end
   // of original bitmap.
   if (lscan <= pimg_infos->pptlSrc[0].y)
   {
      pSrc = pimg_infos->pbSrcBits + (lscan * pimg_infos->ulSrcBytesPerLine);

      switch (pimg_infos->ulSrcBpp)
      {
      case 4:         // 4 Bit
      {
         break;
      }

      case 8:         // 8 Bit
      {
         for (i = 0; i < (ULONG)pimg_infos->pptlSrc[0].x; i++)
         {
            rgb2 = pimg_infos->pargb2Src[pSrc[0]];

            pbScan[0] = (BYTE)((rgb2.bRed   * rw +
                                rgb2.bGreen * gw +
                                rgb2.bBlue  * bw) / 100);

            if (pbScan[0] + bWgtAdj > 255)
               pbScan[0] = 255;
            else
               pbScan[0] += bWgtAdj;

            pbScan++;
            pSrc++;
         }

         break;
      }

      case 16:        // 16Bit - 5/6/5
      {
         break;
      }

      case 24:        // 24Bit
      {
         for (i = 0; i < (ULONG)pimg_infos->pptlSrc[0].x; i++)
         {
            rgb2.bBlue  = pSrc[0];
            rgb2.bGreen = pSrc[1];
            rgb2.bRed   = pSrc[2];

            // if it is white
            if ((rgb2.bRed+rgb2.bGreen+rgb2.bBlue) == 765)
            {
               pbScan[0] = 255;
            }
            else
            {
                pbScan[0] = (BYTE)((rgb2.bRed   * rw +
                                    rgb2.bGreen * gw +
                                    rgb2.bBlue  * bw) / 100);

                // distribute intensity weighted to black
                pbScan[0] = GammaTbl[pbScan[0]];
            }

            pbScan++;
            pSrc += 3;
         }
         break;
      }

      default:
      {
         break;
      }
      }
   }
}

#define get_pixel(ip, x, y)    (*(ip.data + (x) + ((y) * ip.xsize)))
#define put_pixel(ip, x, y, v) (*(ip.data + (x) + ((y) * ip.xsize)) = v)

#define get_error(ip, x, y)    (*(PSHORT)(ip.data + ((x)*2) + ((y) * ip.xsize)))
#define put_error(ip, x, y, v) (*(PSHORT)(ip.data + ((x)*2) + ((y) * ip.xsize)) = (v))


/*****************************************************************************/
/*                                                                           */
/*  FUNCTION: put_mono_pixel                                                 */
/*                                                                           */
/*  Add bit for the current pel to the output bitmap                         */
/*                                                                           */
/*****************************************************************************/
void
put_mono_pixel (PIMAGE pop,
                LONG   x,
                LONG   y,
                LONG   v)
{
   PBYTE pDst  = pop->data + (x / 8) + (y * pop->scan);
   BYTE  bMask = 0x80 >> (x % 8);

   if (v)
      *pDst &= ~bMask;
   else
      *pDst |= bMask;
}

LONG r1 = -255;
LONG r2 =  255;

/*****************************************************************************/
/*                                                                           */
/*  FUNCTION: DiffusionDither                                                */
/*                                                                           */
/*  Add bit for the current pel to the output bitmap                         */
/*                                                                           */
/*****************************************************************************/
long
DiffusionDither (PDDI        pdi,
                 PIMAGEINFO  pimg_infos,
                 PBYTE      *pBuffer,
                 ULONG       ulFlgs)
{
#define DWORD_SIZE 8

   LONG    x, y, mx, my;
   LONG    error;
   LONG    u, u1, v, v1, v2;
   IMAGE   InputIMG, OutputIMG, ExtraIMG;

   LONG    mw;
   PBYTE   mp;
   LONG    sum;
   ULONG   rw,gw, bw;
   ULONG   bWgtAdj;

   if (ulFlgs & GDM_DITHER_BEGIN)
   {
      *pBuffer = 0;

      return 1;
   }

   if (ulFlgs & GDM_DITHER_END)
   {
      free (*pBuffer);
      *pBuffer = 0;

      return 1;
   }

   // Check for values passed in by the Driver
   if (  pdi
      && pdi->ulType == GDM_MATRIX_DITHER
      )
   {
      // This is all we need to check for now.
      bWgtAdj = pdi->ulIntensity;
      rw      = pdi->bRedWt;
      gw      = pdi->bGreenWt;
      bw      = pdi->bBlueWt;
   }
   else
   {
      bWgtAdj = DITHER_DEFAULT_INTENSITY;
      rw      = DITHER_DEFAULT_RED_WEIGHT;
      gw      = DITHER_DEFAULT_GREEN_WEIGHT;
      bw      = DITHER_DEFAULT_BLUE_WEIGHT;
   }

   mw  = mw_jjn;
   mp  = mp_jjn;
   sum = mr_jjn;

   // Just for ease of port I am converting into a IMAGE Structs
   InputIMG.xsize = pimg_infos->pptlSrc[0].x;
   InputIMG.ysize = pimg_infos->pptlSrc[0].y;
   InputIMG.scan  = pimg_infos->ulSrcBytesPerLine;
   InputIMG.data  = 0;

   ExtraIMG       = InputIMG;

   // HIGH PRECISION
   ExtraIMG.xsize += ExtraIMG.xsize;

   OutputIMG.xsize = pimg_infos->pptlDst[0].x;
   OutputIMG.ysize = pimg_infos->pptlDst[0].y;
   OutputIMG.scan  = pimg_infos->ulTrgBytesPerLine;
   OutputIMG.data  = pimg_infos->pbTrgBits;

   // Allocate Temporary Storage for Adjusted Scanlines
   if (*pBuffer == 0)
   {
      ExtraIMG.data = *pBuffer = (PBYTE)malloc ((ExtraIMG.xsize * 3)+ DWORD_SIZE);

      if (!ExtraIMG.data)
      {
         DebugOutput::getErrorStream () << "<<<<<<<<<<<<<<<<<<<<<< ERROR >>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
         DebugOutput::getErrorStream () << "   Failed allocation of ExtraIMG.data in DiffusionDither" << std::endl;

         return 0;
      }
      else
      {
         memset (ExtraIMG.data, 0, ExtraIMG.xsize * 3);
      }
   }
   else
   {
      ExtraIMG.data = *pBuffer;
   }

   InputIMG.data = (PBYTE)malloc(InputIMG.xsize);

   if (!InputIMG.data)
   {
      DebugOutput::getErrorStream () <<  "<<<<<<<<<<<<<<<<<<<<<< ERROR >>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
      DebugOutput::getErrorStream () <<  "   Failed allocation of InputIMG.data in DiffusionDither\n" << std::endl;

      return -1;
   }

   // Process image with normal raster as opposed to serpintine
   for (y = InputIMG.ysize -1; y >= 0 ; y--)
   {
      ConvertSourceScan(InputIMG.data, y, pimg_infos, bWgtAdj, rw, gw, bw);

      for (x = 0; x < (LONG)InputIMG.xsize; x++)
      {
         // Get pixel value
         u  = get_pixel(InputIMG, x, 0);

         // HIGH PERCISION
         u1 = u + (SHORT)get_error(ExtraIMG, x, 0);

         // determine actual value and error
         if (u1 > 96)
         {
            v     = WHITE_PIXEL;
            error = CLAMP(u1,u1,WHITE_PIXEL) - v;
         }
         else
         {
            v     = BLACK_PIXEL;
            // calc error from accumulated value
            error = CLAMP(u1,0,u1);
            // use WgtAdj as dot size correction
            error += (pimg_infos->ulPelSizeCorrection );
         }

         put_mono_pixel (&OutputIMG, x, y, v);

         // Now weight neighboring pixels

         for (my = 0 /* -mw */; my <= mw; my++)
         {
            for (mx = -mw; mx <= mw; mx++)
            {
               if ((u = WEIGHT(mx, my)) == 0)
                  continue;

               // HIGH PERCISION
               v  = (SHORT)get_error(ExtraIMG, x + mx, my);

               u1 = (error * u) / sum;
               v1 = v + u1;

               // HIGH PERCISION
               v2 = CLAMP(v1, r1, r2);

               // HIGH PERCISION
               put_error (ExtraIMG, x + mx, my, (SHORT)v2);
            }
         }
      }

      // move scan lines up in tmp buffer
      memcpy (ExtraIMG.data, ExtraIMG.data + ExtraIMG.xsize, ExtraIMG.xsize * 2);
      memset (ExtraIMG.data + (ExtraIMG.xsize * 2), 0, ExtraIMG.xsize);
   }

   free (InputIMG.data);

   return 1;
}

#ifdef HAVE_BACKWARDS_COMPATIBILITY

// Maintain backwards compatibility
long
DiffusionDither__FP20_DIFFUSIONDITHERINFOP11_IMAGEINFOSPPUcUi (PDDI        pdi,
                                                               PIMAGEINFO  pimg_infos,
                                                               PBYTE      *pBuffer,
                                                               ULONG       ulFlgs)
{
   return DiffusionDither (pdi, pimg_infos, pBuffer, ulFlgs);
}

#endif
