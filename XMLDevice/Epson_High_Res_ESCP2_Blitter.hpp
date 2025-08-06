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
#ifndef _Epson_High_Res_ESCP2_Blitter
#define _Epson_High_Res_ESCP2_Blitter

#include "Device.hpp"

extern "C" {
   DeviceBlitter *createBlitter (PrintDevice   *pDevice);
   void           deleteBlitter (DeviceBlitter *pBlitter);
};

class Epson_High_Res_ESCP2_Blitter : public DeviceBlitter
{
public:
                         Epson_High_Res_ESCP2_Blitter    (PrintDevice                       *pDevice);
   virtual              ~Epson_High_Res_ESCP2_Blitter    ();

   virtual void          initializeInstance              ();

   virtual bool          rasterize                      (PBYTE                               pbBits,
                                                         PBITMAPINFO2                        pbmi,
                                                         PRECTL                              prectlPageLocation,
                                                         BITBLT_TYPE                         eType);

#ifndef RETAIL
   virtual void          outputSelf                     ();
#endif
   virtual std::string   toString                       (std::ostringstream&                 oss);
   friend std::ostream&  operator<<                     (std::ostream&                       os,
                                                         const Epson_High_Res_ESCP2_Blitter& device);

private:
   bool                  epsonMonoRasterize             (PBYTE                               pbBits,
                                                         PBITMAPINFO2                        pbmi,
                                                         PSIZEL                              psizelPage,
                                                         PRECTL                              prectlPageLocation,
                                                         BITBLT_TYPE                         eType);
   bool                  epsonColorRasterize            (PBYTE                               pbBits,
                                                         PBITMAPINFO2                        pbmi,
                                                         INT                                 icolortech,
                                                         PSIZEL                              psizelPage,
                                                         PRECTL                              prectlPageLocation,
                                                         BITBLT_TYPE                         eType);

   int                   getNumberofPasses              (int                                 iResolution,
                                                         int                                 iXPhysRes);

   void                  epsonSplit2for1                (PBYTE                               pbInputBuffer,
                                                         PBYTE                               pbOutputBuffer,
                                                         int                                 iNumBytes,
                                                         int                                 iBitValue);

   void                  epsonSplit4for1                (PBYTE                               pInputPlane,
                                                         PBYTE                               pOutPlane,
                                                         int                                 iNumBytes,
                                                         int                                 iBitValue);

   void                  epsonSplit8for1                (PBYTE                               pInputPlane,
                                                         PBYTE                               pOutLeftPlane,
                                                         PBYTE                               pOutRghtPlane,
                                                         int                                 iNumBytes,
                                                         int                                 iBitValue);

   int                   GetNextRunDrawLocation         (int                                 iNumBandSize,
                                                         int                                 iHeadSpacing,
                                                         int                                 iBytesPerLine,
                                                         int                                 iLocation,
                                                         int                                 iNumberofPasses);

   void                  InitializePageValues           (int                                 iNumPass);

   bool    fInstanceInitialized_d;

   bool    fGraphicsHaveBeenSent_d;
   int     cbCompress_d;
   PBYTE   pbCompress_d;

   PBYTE   pbOldCPlane;
   PBYTE   pbOldMPlane;
   PBYTE   pbOldYPlane;
   PBYTE   pbOldKPlane;
   PBYTE   pbOldLCPlane;
   PBYTE   pbOldLMPlane;

   long    lCurPageLocation;

   bool    bNotFirstPass;

   // The following values are used for handling the location of the scan lines to
   // be sent down to the device
   int     iMoveRight;
   int     iCurrentHighValue;                      // Current Top line of output value
   int     iCurrentValue;                          // Current line location value
   int     iCurrentLowValue;                       // Current low line location value
   bool    bNotFirstRun;
   long    lSecondPlane_cy;
   long    lFirstPlane_cy;
   long    lFirstBandLocation;
   long    lSecondBandLocation;
   int     iNumPageLines;
   bool    bLastBand;
   int     iShiftValue;
   bool    bDoShiftRight;
   bool    bStartPage;
   int     iRealPos;
   int     iStartPassVal;
   long    lWhiteSpaceIncrement;
   bool    bStillWhite;
};

#endif
