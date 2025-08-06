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
#ifndef _GplDitherInstance
#define _GplDitherInstance

#include "defines.hpp"
#include "Device.hpp"

typedef struct {
    int  iPelSize;
    int  iSrcRowSize;
    int  iMapSize;
    int  iDestRowSize;
    bool bMultiBitEnabled;
} PARAMS, *PPARAMS;

typedef struct {
   RGB2 is[1];
} COLR, *PCOLR;

typedef struct {
   BYTE is[16][16];
} *P2ABYTE;

typedef struct _HSVstruct {
   ULONG       ulHue;
   ULONG       ulSaturation;
   ULONG       ulValue;
   BOOL        fIsNotWhite;
   ULONG       ulBlack;
   ULONG       ulBlackWhite;
   BOOL        fValueSat;
   BOOL        fIsGreaterThan0x80;
   ULONG       ulGT80;
   ULONG       ulcolor1;
   ULONG       ulcolor;
   ULONG       ulLE80;
}  HSV, *PHSV;

typedef struct aHSV {
   HSV is[1];
} HSVARRAY, *PHSVARRAY;

typedef struct _MidColor {
   LONG       lRed;
   LONG       lGreen;
   LONG       lBlue;
   LONG       lLRed;
   LONG       lLGreen;
   LONG       lLBlue;
   LONG       lBlack;
} MID_COLOR, *PMID_COLOR;

typedef struct _DELTACOLORstruct {
   LONG        lHue;
   LONG        lSaturation;
   LONG        lValue;
} DELTACOLOR, *PDELTACOLOR;

typedef struct _HSVErrorStruc {
   float flErrColor1;
   float flErrColor2;
   float flErrBlack;
   float flErrWhite;
} HSVERRORSTRUC, *PHSVERRORSTRUC;

typedef struct _HSVValues {
#ifdef DEBUG
   char   *pszColor1;
   char   *pszColor2;
   float   flHue;
   float   flValue;
#endif

   float   flSaturation;
   int     eColor1;
   int     eColor2;
   float   flColor1;
   float   flColor2;
   float   flBlack;
   float   flWhite;

   BYTE    bRed;
   BYTE    bGreen;
   BYTE    bBlue;
} HSVVALUES, *PHSVVALUES;

#define MAX_HSV_CACHE                          4

typedef struct _HSVCache {
   BOOL       afhsvCacheValid[MAX_HSV_CACHE];
   INT        iLastEntry;
   HSVVALUES  ahsvCache[MAX_HSV_CACHE];
} HSVCACHE, *PHSVCACHE;

typedef struct _CMYKErrorStruc {
   float    flKError;
   float    flCError;
   float    flMError;
   float    flYError;
} CMYKERRORSTRUC, *PCMYKERRORSTRUC;

typedef struct {
   BYTE    bR;     // input: RGB
   BYTE    bG;
   BYTE    bB;
   ULONG   lC;     // ouput: CMYK
   ULONG   lM;
   ULONG   lY;
   ULONG   lK;
   ULONG   lLC;
   ULONG   lLM;
   bool    bMult;  // if TRUE, resulting CMYK value will be multiplied by 16
} TOCMYK, *PTOCMYK;

// the order for this enum has to match the order
// in the pszDitherNames in GplDitherInstance.cpp


class GplDitherInstance : public DeviceDither
{
public:
   enum {
      DITHER_UNLISTED                = -1,
      DITHER_LEVEL,
      DITHER_SNAP,
      DITHER_DITHER_4x4,
      DITHER_DITHER_8x8,
      DITHER_STUCKI_DIFFUSION,
      DITHER_STUCKI_BIDIFFUSION,
      DITHER_MAGIC_SQUARES,
      DITHER_ORDERED_SQUARES,
      DITHER_FAST_DIFFUSION,
      DITHER_STEINBERG_DIFFUSION,
      DITHER_SMOOTH_DIFFUSION,
      DITHER_HSV_DIFFUSION,
      DITHER_HSV_BIDIFFUSION,
      DITHER_CMYK_DIFFUSION,
      DITHER_VOID_CLUSTER,
      DITHER_JANIS_STUCKI,
      DITHER_ESTUCKI_DIFFUSION
   };
   enum {
      REQUEST_DEFAULT_REDUCTION      = -1
   };
   enum {
      DITHER_CATAGORY_UNLISTED       = -1,
      DITHER_CATAGORY_MATRIX,
      DITHER_CATAGORY_DIFFUSION,
      DITHER_CATAGORY_HSV_DIFFUSION,
      DITHER_CATAGORY_CMYK_DIFFUSION,
      DITHER_CATAGORY_VOID_CLUSTER,
      DITHER_CATAGORY_NEW_DIFFUSION,
      DITHER_CATAGORY_NEW_MATRIX
   };

                            GplDitherInstance              (Device             *pDevice,
                                                            bool                fDataInRGB,
                                                            int                 iBlackReduction,
                                                            int                 iDitherType,
                                                            int                 iColorTech,
                                                            int                 iNumDitherRows,
                                                            int                 iSrcRowPels,
                                                            int                 iNumDestRowBytes,
                                                            int                 iDestBitsPerPel,
                                                            DeviceGamma        *pGamma);
   virtual                 ~GplDitherInstance              ();

   static DeviceDither     *createDitherInstance           (PSZCRO              pszDitherType,
                                                            Device             *pDevice,
                                                            PSZCRO              pszOptions);

   static std::string      *getCreateHash                  (PSZRO               pszJobProperties);
   static PSZCRO            getIDFromCreateHash            (std::string        *pstringHash);

   void                     ditherRGBtoCMYK                (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbStart);

   bool                     ditherAllPlanesBlank           ();
   bool                     ditherCPlaneBlank              ();
   bool                     ditherMPlaneBlank              ();
   bool                     ditherYPlaneBlank              ();
   bool                     ditherKPlaneBlank              ();
   bool                     ditherLCPlaneBlank             ();
   bool                     ditherLMPlaneBlank             ();

   void                     newFrame                       ();

   BinaryData              *getCPlane                      ();
   BinaryData              *getMPlane                      ();
   BinaryData              *getYPlane                      ();
   BinaryData              *getKPlane                      ();
   BinaryData              *getLCPlane                     ();
   BinaryData              *getLMPlane                     ();

   PSZCRO                   getID                          ();

   static bool              ditherNameValid                (PSZCRO              pszId);
   static bool              ditherCatagoryValid            (PSZCRO              pszId);
   static PSZCRO            getDitherCatagory              (PSZCRO              pszId);

   static Enumeration      *getAllEnumeration              ();

private:
   static int               nameToID                       (PSZCRO              pszId);
   LONG                     GplInitializeRandomNumberTable ();
   int                      GplSeparateColors              (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplLevel                       (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplDitherMatrix                (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource,
                                                            LONG                lSize,
                                                            PRGB2               pMatrix);
   int                      GplColorSquares                (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource,
                                                            BYTE                table[16][16]);
   int                      GplStuckiDiffusion             (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplStuckiBiffusion             (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplEnhancedStuckiDiffusion     (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplSteinbergDiffusion          (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplFastDiffusion               (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplHSVDiffusion                (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplHSVBidiffusion              (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   int                      GplCMYKDiffusion               (PBITMAPINFO2        pbmi2,
                                                            PBYTE               pbSource);
   void                     GplCMYRemoval                  (PBITMAPINFO2        pbmi2);
   LONG                     GplCreateHSVcolorTable         (PBITMAPINFO2        pbmi2);
   void                     SetInitialParameters           (PBITMAPINFO2        pbmi2,
                                                            PPARAMS             pParam);
   bool                     isNotWhite                     (PBYTE               pbBits,
                                                            TOCMYK             *pToCMYK);

   // Class variables
   bool         fDataInRGB_d; // TBD only done for 16&24 bpp
   bool         fModify_d;
   bool         fRemoveBlackColor_d;
   bool         fStartOfPage_d;
   bool         fFirstTime_d;
   float        fKReductionPerc_d;
   int          iDitherType_d;
   int          iColorTech_d;
   int          iSrcRowPels_d;
   int          iNumDitherRows_d;
   int          iNumDestRowBytes_d;
   int          iDestBitsPerPel_d;
   int          iNumColors_d;
   PBYTE        pbRGamma_d;
   PBYTE        pbGGamma_d;
   PBYTE        pbBGamma_d;
   PBYTE        pbKGamma_d;
   MID_COLOR    midPt_d;
   DELTACOLOR   delta_d;
   float        *pfErrRow_d;
   int          iBig_d;
   int          iSeed_d;
   int          iMix_d[57];
   int          lidx1_d;
   int          lidx2_d;
   PHSVARRAY    phsvTable_d;
   PBYTE        pbDest_d;
   PBYTE        pbDestEnd_d;
   PBYTE        pbKBuffer_d;
   PBYTE        pbCBuffer_d;
   PBYTE        pbMBuffer_d;
   PBYTE        pbYBuffer_d;
   PBYTE        pbLCBuffer_d;
   PBYTE        pbLMBuffer_d;
   PBYTE        pbKNextBuffer_d;
   PBYTE        pbCNextBuffer_d;
   PBYTE        pbMNextBuffer_d;
   PBYTE        pbYNextBuffer_d;
   PBYTE        pbLCNextBuffer_d;
   PBYTE        pbLMNextBuffer_d;
   int          iHalftoneSize_d;
   int          iRowCount_d;
   int          iWhiteIndex_d;
   int          iBlackIndex_d;
   bool         fEmptyBlack_d;
   bool         fEmptyCyan_d;
   bool         fEmptyMagenta_d;
   bool         fEmptyYellow_d;
   bool         fEmptyLCyan_d;
   bool         fEmptyLMagenta_d;
   BinaryData  *pbdKPlane_d;
   BinaryData  *pbdCPlane_d;
   BinaryData  *pbdMPlane_d;
   BinaryData  *pbdYPlane_d;
   BinaryData  *pbdLCPlane_d;
   BinaryData  *pbdLMPlane_d;
   PBYTE        pbLightTable_d;
   PBYTE        pbDarkTable_d;
   INT          iNumPens_d;
};

void  GplGenerateGammaCurve         (float        fGammaIn,
                                     int          iBiasIn,
                                     PBYTE        pbGamma);
#endif
