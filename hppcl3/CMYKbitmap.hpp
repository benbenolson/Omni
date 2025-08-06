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
#ifndef _CMYKbitmap_hpp
#define _CMYKbitmap_hpp

#include <stdio.h>

#include "hppcl3.hpp"

class CMYKBitmap {
public:
   typedef enum {
      CYAN,
      MAGENTA,
      YELLOW,
      BLACK
   } PLANE;

   CMYKBitmap (char        *pszFilename,
               int          iCx,
               int          iCy);
   CMYKBitmap (int          iCx,
               int          iCy);
   ~CMYKBitmap ();

   void   addScanLine (PBYTE pbBits,
                       int   iScanLines,
                       int   iYPos,
                       PLANE eWhichPlane);

private:
   void   sizeFile    ();
   void   writeHeader ();

   FILE           *fp_d;
   char            achFileName_d[512]; // @TBD need max chars in path
   int             iCx_d;
   int             iCy_d;
   int             iBitCount_d;
   int             iSizeScanLineIn_d;
   int             iSizeScanLineOut_d;
   PBYTE           pbScanLine_d;
   bool            fFirstTime_d;
};

#endif
