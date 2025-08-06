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
#ifndef _scanline_hpp
#define _scanline_hpp

#include "bitmap.hpp"

class ScanLines {
public:
        ScanLines                    (PBYTE pbData,
                                      int   iLength);
       ~ScanLines                    (void);

   void SetPageSize                  (int   iForm);

   void SetCompressionLevel          (int   iCompressionLevel);

   void AddScanLine                  (int   cbScanData,
                                      PBYTE pbScanData);

   void MoveToY                      (int   iYPosition);

private:
   void DecompressScanLine           (PBYTE    pbScanData,
                                      int      cbScanData,
                                      PBYTE    pbTo,
                                      int      cbMaxScanData,
                                      int      iCompressionLevel);
   void DumpScanLines                (void);

   void Xfer600x2_300x4_300x4_300x4  (void);
   void Xfer300x4_300x4_300x4_300x4  (void);
   void Xfer300x2_300x2_300x2_300x2  (void);

   Bitmap        *pMonoBitmap_d;
   Bitmap        *pColorBitmap_d;

   int            cxMono_d;
   int            cyMono_d;
   int            cxColor_d;
   int            cyColor_d;

   bool           fInitialized_d;
   int            iCurrentFormSize_d;

   int            iSizeMonoScanLine_d;
   int            iMonoScanLinesWritten_d;

   int            iSizeColorScanLine_d;
   int            iColorScanLinesWritten_d;

   int            iSizeExtColorScanLine_d;
   PBYTE          pbExtColorScanLine_d;

   int            iSizeExtMonoScanLine_d;
   PBYTE          pbExtMonoScanLine_d;

   int            iExtMonoBitsPerPel;
   int            iExtColorBitsPerPel;

   int            iScanLineOrder_d;

   int            iCompressionLevel_d;
   int            cbLastScanline_d;
   PBYTE          pbLastScanline_d;

   int            iWhichPlane_d;

   int            iNumComponents_d;

   PBYTE          pbKHigh_d;
   PBYTE          pbKLow_d;
   int            iKXRes_d;
   int            iKYRes_d;
   int            iKIntensityLevels_d;

   PBYTE          pbCHigh_d;
   PBYTE          pbCLow_d;
   int            iCXRes_d;
   int            iCYRes_d;
   int            iCIntensityLevels_d;

   PBYTE          pbMHigh_d;
   PBYTE          pbMLow_d;
   int            iMXRes_d;
   int            iMYRes_d;
   int            iMIntensityLevels_d;

   PBYTE          pbYHigh_d;
   PBYTE          pbYLow_d;
   int            iYXRes_d;
   int            iYYRes_d;
   int            iYIntensityLevels_d;

   typedef enum _ScanLineOrder {
      ORDER_UNKNOWN,
      ORDER_600x2,
      ORDER_300x4,
      ORDER_300x2,
      ORDER_150x2,
      ORDER_600x2_300x4_300x4_300x4,
      ORDER_600x2_300x2_300x2_300x2,
      ORDER_300x2_300x2_300x2_300x2,
      ORDER_300x4_300x4_300x4_300x4,
      ORDER_300x2_300x3_300x3_300x3,
      ORDER_300x4_300x2_300x2_300x2,
      ORDER_150x2_150x2_150x2_150x2
   } ESCANLINEORDER;

   ESCANLINEORDER eScanLineOrder_d;
};

#endif
