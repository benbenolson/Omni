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
#ifndef _bitmap_hpp
#define _bitmap_hpp

#include <cstdio>

#include "hppcl3.hpp"

typedef struct _NEUTRALRGB {
   byte bRed;
   byte bGreen;
   byte bBlue;
} NEUTRALRGB, *PNEUTRALRGB;

typedef struct _NEUTRALRGB2 {
   byte bRed;
   byte bGreen;
   byte bBlue;
   byte bOptions;
} NEUTRALRGB2, *PNEUTRALRGB2;

class Bitmap {
public:
   Bitmap (char        *pszFilename,
           int          iCx,
           int          iCy,
           int          iBitCount,
           PNEUTRALRGB  pColors);
   Bitmap (int          iCx,
           int          iCy,
           int          iBitCount,
           PNEUTRALRGB  pColors);
   Bitmap (char        *pszFilename,
           int          iCx,
           int          iCy,
           int          iBitCount,
           PNEUTRALRGB2 pColors);
   Bitmap (int          iCx,
           int          iCy,
           int          iBitCount,
           PNEUTRALRGB2 pColors);
   ~Bitmap ();

   void   addScanLine (PBYTE pbBits, int iScanLines);

private:
   void   sizeFile    ();
   void   writeHeader ();

   FILE           *fp_d;
   int             iCx_d;
   int             iCy_d;
   int             iBitCount_d;
   PNEUTRALRGB     pColors_d;
   int             iSizeColors_d;
   int             iSizeScanLine_d;
   int             iScanLinesWritten_d;
   bool            fFirstTime_d;
};

#endif
