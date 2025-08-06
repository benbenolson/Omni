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
#ifndef _MonoDither
#define _MonoDither

#include "defines.hpp"

#include <sstream>
#include <cstdlib>
#include <memory>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define DITHER_DEFAULT_INTENSITY     20

// Adjust to 256 base to convert divisions to shifts
#define DITHER_DEFAULT_RED_WEIGHT    64  // 25
#define DITHER_DEFAULT_GREEN_WEIGHT  153 // 60
#define DITHER_DEFAULT_BLUE_WEIGHT   39  // 15

typedef struct _IMAGE {
   ULONG  xsize;
   ULONG  ysize;
   PBYTE  data;
   LONG   scan;
} IMAGE, *PIMAGE;

/******************************************************************************/
/*    PPOINTL     pptlSrc;            // src left, bottom, width and height.  */
/*    PPOINTL     pptlDst;            // dst left, bottom, width and height.  */
/*    ULONG       ulSrcBytesPerLine;  // src bytes per line                   */
/*    PBYTE       pbSrcBits;          // pointer to src image data            */
/*    ULONG       ulTrgBytesPerLine;  // dst bytes per line                   */
/*    PBYTE       pbTrgBits;          // pointer to dst image data            */
/*    ULONG       ulOptions;          // options                              */
/*    ULONG       ulcClrs;            // count of colors in rgb2 table        */
/*    PRGB2       pargb2;             // rgb2 table                           */
/******************************************************************************/

typedef struct _IMAGEINFOS {        /* imginfo */
   PPOINTL     pptlSrc;
   PPOINTL     pptlDst;
   ULONG       ulSrcBpp;
   ULONG       ulSrcBytesPerLine;
   PBYTE       pbSrcBits;
   ULONG       ulcSrcClrs;
   ULONG       ulSrcClrType;
   PRGB2       pargb2Src;
   ULONG       ulTrgBpp;
   ULONG       ulTrgBytesPerLine;
   PBYTE       pbTrgBits;
   ULONG       ulcTrgClrs;
   ULONG       ulTrgClrType;
   PRGB2       pargb2Trg;
   ULONG       ulOptions;
   ULONG       ulPelSizeCorrection;
} IMAGEINFOS, *PIMAGEINFO;

#define  GDM_NO_DITHER               0x0000   // No dithering in system
#define  GDM_USERDEF_DITHER          0x0001   // Users supplies own Dithering routines
#define  GDM_MATRIX_DITHER           0x0002   // Use System ordered dithering
#define  GDM_ERRORDIF_DITHER         0x0004   // Use System error diffusion dithering
#define  GDM_DITHER_BEGIN            0x0008   // Use System Floyd-Steinberg dithering
#define  GDM_DITHER_END              0x0010   // Use System error propigation dithering
#define  GDM_COLOR_CONVERT           0x0020   // use device's clr mapping functions

/*******************************************************************************************/
/*    ULONG       ulLength;      // length of structure                 - 88               */
/*    ULONG       ulType;        // type of dither info structure       - GDM_MATRIX_DITHER*/
/*    ULONG       fOptions;      // dither info options - DI_MONOINVERT - 00               */
/*    ULONG       ulIntensity;   // RGB Gama Correction Value           - 00               */
/*    BYTE        bRedWt;        // weight of primary color red         - 25               */
/*    BYTE        bGreenWt;      // weight of primary color green       - 60               */
/*    BYTE        bBlueWt;       // weight of primary color blue        - 15               */
/*    BYTE        bPad;          // 4 byte align                        - 00               */
/*    SIZEL       szMatrix;      // halftone pattern size               - 8                */
/*    BYTE        bHalftone[];   // array of halftone patterns          - see 32gdata.c    */
/*******************************************************************************************/

typedef struct _MATRIXDITHERINFO {    /* mtrxdi */
   ULONG       ulLength;
   ULONG       ulType;
   ULONG       fOptions;
   ULONG       ulIntensity;
   BYTE        bRedWt;
   BYTE        bGreenWt;
   BYTE        bBlueWt;
   BYTE        bPad;
   SIZEL       szMatrix;
   BYTE        bHalftone[1];
} MATRIXDITHERINFO, *PMDI;

#define STUCKI_DIF 0x01
#define JJN_DIF    0x02
#define RND_DIF    0x04
#define USER_DIF   0x08

typedef struct _DIFFUSIONDITHERINFO {    /* difdi */
   ULONG       ulLength;
   ULONG       ulType;        // Stucki, user defined, etc.
   ULONG       fOptions;
   ULONG       ulIntensity;
   BYTE        bRedWt;
   BYTE        bGreenWt;
   BYTE        bBlueWt;
   BYTE        bPad;
   PBYTE       pBuffer;
   SIZEL       szFilter;
   BYTE        bFilterArry[1];
} DIFFUSIONDITHERINFO, *PDDI;

extern "C" {
long DiffusionDither                                               (PDDI        pdi,
                                                                    PIMAGEINFO  pimg_infos,
                                                                    PBYTE      *pBuffer,
                                                                    ULONG       ulFlgs);
#ifdef HAVE_BACKWARDS_COMPATIBILITY
long DiffusionDither__FP20_DIFFUSIONDITHERINFOP11_IMAGEINFOSPPUcUi (PDDI        pdi,
                                                                    PIMAGEINFO  pimg_infos,
                                                                    PBYTE      *pBuffer,
                                                                    ULONG       ulFlgs);
#endif
}
#endif
