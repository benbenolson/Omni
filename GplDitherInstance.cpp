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
#include "Device.hpp"
#include "JobProperties.hpp"

#include <cmath>

#undef  MAX3
#define MAX3(a, b, c)                        ((a) > (b) ? ((a) > (c) ? (a) : (c)) : (b) > (c) ? (b) : (c))
#undef  MIN3
#define MIN3(a, b, c)                        ((a) < (b) ? ((a) < (c) ? (a) : (c)) : (b) < (c) ? (b) : (c))
#undef  MIN
#define MIN(a,b)                             ((a) < (b) ? (a) : (b))

#define A_BIG_NUMBER                         4000000
#define A_SEED                               1618033
#define ERR_EXP                              21
#define RGB_VALUES                           256
#define MAX_RGB_VALUE                        255
#define VOID_CLUSTER_ARRAY_SIZE              64
#define CMYK_FILTER_OVERFLOW                 8
// 4rows * 4colors(RGBK) * 4bytesPerlong * (numberOfSourcePels + boundaryPels)
#define ERROR_FILTER_BUFFER_SIZE(x)          (4 * 4 * 4 * (x + CMYK_FILTER_OVERFLOW))
// 4rows * 7colors(RGB(LR)(LB)(LG)K) * 4bytesPerlong * (numberOfSourcePels + boundaryPels)
#define ERROR_FILTER_BUFFER_SIZE7(x)         (4 * 7 * 4 * (x + CMYK_FILTER_OVERFLOW))
// 8rows * 3colors(RGB) * 4bytesPerlong * (numberOfSourcePels + boundaryPels)
#define ADAPTIVE_FILTER_BUFFER_SIZE(x)       (8 * 3 * 4 * (x + CMYK_FILTER_OVERFLOW))
#define PRECISION                            (8)
#define PRECISION1                           (1 << PRECISION)

#define INVALID_BITMAP                       -1L
#define INVALID_PARAMETER                    INVALID_BITMAP

#define CMYK_CYAN                            0
#define CMYK_MAGENTA                         1
#define CMYK_YELLOW                          2
#define CMYK_BLACK                           3

#define SWAP(x,y)                            { ULONG ulSwap = x; x = y; y = ulSwap; }

#define HIGH                                 3
#define MEDIUM                               2
#define LOW                                  1

#include <cstdio>
#include <cstdlib>

long lEndOfBitPlanes;

extern ULONG ulPrimColors[7];
extern BYTE  aPaintmixer[16][16];
extern BYTE  aOrdered[16][16];
extern RGB2  paHalftone4x4_24[18][4][4];
extern RGB2  paHalftone4x4[18][4][4];
extern RGB2  paHalftone8x8[66][8][8];

bool  GplGammaBuildTable             (int          iDitherType,
                                      PBYTE        pbKGamma,
                                      PBYTE        pbRGamma,
                                      PBYTE        pbGGamma,
                                      PBYTE        pbBGamma,
                                      int          iKBias,
                                      int          iRBias,
                                      int          iGBias,
                                      int          iBBias,
                                      int          iKGamma,
                                      int          iRGamma,
                                      int          iGGamma,
                                      int          iBGamma);
void  ToCMYK                         (PTOCMYK      pToCMYK);
void  ToCMYK6                        (PTOCMYK      pToCMYK, byte *pbLightTable, byte *pbDarkTable);  //@@SIX-COLOR

#define STRING_OF(x) (apszDitherNames[x - DITHER_LEVEL])

static PSZCRO apszDitherNames[] = {
   "DITHER_LEVEL",
   "DITHER_SNAP",
   "DITHER_DITHER_4x4",
   "DITHER_DITHER_8x8",
   "DITHER_STUCKI_DIFFUSION",
   "DITHER_STUCKI_BIDIFFUSION",
   "DITHER_MAGIC_SQUARES",
   "DITHER_ORDERED_SQUARES",
   "DITHER_FAST_DIFFUSION",
   "DITHER_STEINBERG_DIFFUSION",
   "DITHER_SMOOTH_DIFFUSION",
   "DITHER_HSV_DIFFUSION",
   "DITHER_HSV_BIDIFFUSION",
   "DITHER_CMYK_DIFFUSION",
   "DITHER_VOID_CLUSTER",
   "DITHER_JANIS_STUCKI",
   "DITHER_ESTUCKI_DIFFUSION"
};

bool GplDitherInstance::
isNotWhite(PBYTE pbBits, TOCMYK *pToCMYK)
{
    if((*(pbBits+2) & *(pbBits+1) & *pbBits) == 255)
    {
        return false;
    }
    else
    {
        pToCMYK->bR = *(pbBits+2);
        pToCMYK->bG = *(pbBits+1);
        pToCMYK->bB = *pbBits;
        if (!fDataInRGB_d)
           SWAP (pToCMYK->bR, pToCMYK->bB);
        return true;
    }
}


void GplDitherInstance::
SetInitialParameters(PBITMAPINFO2 pbmi2,
                          PPARAMS pParam)
{
    // PelSize in bits
    pParam->iPelSize = (LONG)pbmi2->cPlanes * (LONG)pbmi2->cBitCount;

    // RowSize in bytes
    pParam->iSrcRowSize = ((LONG)pbmi2->cx * (LONG)pbmi2->cBitCount + 31) / 32 * 4;

    // MapSize in bytes
    pParam->iMapSize = pParam->iSrcRowSize * (LONG)pbmi2->cPlanes * (LONG)pbmi2->cy;

    // DestRowSize in bytes
    if(pParam->bMultiBitEnabled)
    {
        pParam->iDestRowSize = ((pbmi2->cx * iDestBitsPerPel_d) + 7) / 8 ;
    }
    else
    {
        pParam->iDestRowSize = (pbmi2->cx + 7) / 8 ;
    }
}



// these dither ids need to be ordered so that
// when the index is used, it will maintain
// an alphabetical order to the dither names

int GplDitherInstance::
nameToID (PSZCRO pszId)
{
   static short int asiEntries[] = {
      DITHER_CMYK_DIFFUSION,
      DITHER_DITHER_4x4,
      DITHER_DITHER_8x8,
      DITHER_ESTUCKI_DIFFUSION,
      DITHER_FAST_DIFFUSION,
      DITHER_HSV_BIDIFFUSION,
      DITHER_HSV_DIFFUSION,
      DITHER_JANIS_STUCKI,
      DITHER_LEVEL,
      DITHER_MAGIC_SQUARES,
      DITHER_ORDERED_SQUARES,
      DITHER_SMOOTH_DIFFUSION,
      DITHER_SNAP,
      DITHER_STEINBERG_DIFFUSION,
      DITHER_STUCKI_BIDIFFUSION,
      DITHER_STUCKI_DIFFUSION,
      DITHER_VOID_CLUSTER
   };

   int iLow  = 0;
   int iMid  = (int)dimof (asiEntries) / 2;
   int iHigh = (int)dimof (asiEntries) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, STRING_OF (asiEntries[iMid]));

      if (0 == iResult)
      {
         return asiEntries[iMid];
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return DITHER_UNLISTED;
}

// these dither ids need to be ordered so that
// when the index is used, it will maintain
// an alphabetical order to the dither names

bool GplDitherInstance::
ditherNameValid (PSZCRO pszId)
{
   static short int asiEntries[] = {
      DITHER_CMYK_DIFFUSION,
      DITHER_DITHER_4x4,
      DITHER_DITHER_8x8,
      DITHER_ESTUCKI_DIFFUSION,
      DITHER_FAST_DIFFUSION,
      DITHER_HSV_BIDIFFUSION,
      DITHER_HSV_DIFFUSION,
      DITHER_JANIS_STUCKI,
      DITHER_LEVEL,
      DITHER_MAGIC_SQUARES,
      DITHER_ORDERED_SQUARES,
      DITHER_SMOOTH_DIFFUSION,
      DITHER_SNAP,
      DITHER_STEINBERG_DIFFUSION,
      DITHER_STUCKI_BIDIFFUSION,
      DITHER_STUCKI_DIFFUSION,
      DITHER_VOID_CLUSTER
   };

   int iLow  = 0;
   int iMid  = (int)dimof (asiEntries) / 2;
   int iHigh = (int)dimof (asiEntries) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, STRING_OF (asiEntries[iMid]));

      if (0 == iResult)
      {
         return true;
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return false;
}

PSZCRO GplDitherInstance::
getDitherCatagory (PSZCRO pszId)
{
   if (  0 == strcmp ("DITHER_LEVEL", pszId)
      || 0 == strcmp ("DITHER_SNAP", pszId)
      || 0 == strcmp ("DITHER_DITHER_4x4", pszId)
      || 0 == strcmp ("DITHER_DITHER_8x8", pszId)
      || 0 == strcmp ("DITHER_MAGIC_SQUARES", pszId)
      || 0 == strcmp ("DITHER_ORDERED_SQUARES", pszId)
      )
      return "DITHER_CATAGORY_MATRIX";
   else if (  0 == strcmp ("DITHER_STUCKI_DIFFUSION", pszId)
           || 0 == strcmp ("DITHER_STUCKI_BIDIFFUSION", pszId)
           || 0 == strcmp ("DITHER_ESTUCKI_DIFFUSION", pszId)
           || 0 == strcmp ("DITHER_FAST_DIFFUSION", pszId)
           || 0 == strcmp ("DITHER_STEINBERG_DIFFUSION", pszId)
           || 0 == strcmp ("DITHER_SMOOTH_DIFFUSION", pszId)
           )
      return "DITHER_CATAGORY_DIFFUSION";
   else if (  0 == strcmp ("DITHER_HSV_DIFFUSION", pszId)
           || 0 == strcmp ("DITHER_HSV_BIDIFFUSION", pszId)
           )
      return "DITHER_CATAGORY_HSV_DIFFUSION";
   else if (0 == strcmp ("DITHER_CMYK_DIFFUSION", pszId))
      return "DITHER_CATAGORY_CMYK_DIFFUSION";
   else if (0 == strcmp ("DITHER_VOID_CLUSTER", pszId))
      return "DITHER_CATAGORY_VOID_CLUSTER";
   else if (0 == strcmp ("DITHER_JANIS_STUCKI", pszId))
      return "DITHER_CATAGORY_NEW_DIFFUSION";

   return "DITHER_CATAGORY_DIFFUSION";
}

bool GplDitherInstance::
ditherCatagoryValid (PSZCRO pszId)
{
   if (  0 == strcmp ("DITHER_CATAGORY_MATRIX", pszId)
      || 0 == strcmp ("DITHER_CATAGORY_DIFFUSION", pszId)
      || 0 == strcmp ("DITHER_CATAGORY_HSV_DIFFUSION", pszId)
      || 0 == strcmp ("DITHER_CATAGORY_CMYK_DIFFUSION", pszId)
      || 0 == strcmp ("DITHER_CATAGORY_VOID_CLUSTER", pszId)
      || 0 == strcmp ("DITHER_CATAGORY_NEW_DIFFUSION", pszId)
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}

DeviceDither * GplDitherInstance::
createDitherInstance (PSZCRO  pszDitherType,
                      Device *pDevice,
                      PSZCRO  pszOptions)
{
   if (!ditherNameValid (pszDitherType))
   {
      // Error!
      return 0;
   }

   bool         fDataInRGB;
   int          iBlackReduction;
   int          iColorTech;
   int          iNumDitherRows;
   int          iSrcRowPels;
   int          iNumDestRowBytes;
   int          iDestBitsPerPel;
   DeviceGamma *pGamma           = 0;
   int          i;

   typedef struct _BoolParmMapping {
      char *pszName;
      bool *pfParm;
   } BOOLPARMMAPPING, *PBOOlPARMMAPPING;
   BOOLPARMMAPPING aBoolMappings[] = {
      { "fDataInRGB=", &fDataInRGB }
   };

   for (i = 0; i < (int)dimof (aBoolMappings); i++)
   {
      char *pszName = aBoolMappings[i].pszName;
      char *pszPos  = strstr (pszOptio   typedef struct _BoolParmMapping {
      const char *pszName;
      bool *pfParm;
   } BOOLPARMMAPPING, *PBOOlPARMMAPPING;
   BOOLPARMMAPPING aBoolMappings[] = {
      { "fDataInRGB=", &fDataInRGB }
      for (i = 0; i < (int)dimof (aBoolMappings); i++)
   {
      const char *pszName = aBoolMappings[i].pszName;
      const char *pszPos  = strstr (pszOptions, pszName);

      if (!pszPos)
         break;

#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ())
      {
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Found " << pszName << " at offset " << (pszPos - pszOptions) << std::endl;
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Looking at " << (pszPos + strlen (pszName)) << std::endl;
      }
#endif

      if (0 == strncasecmp (pszPos + strlen (pszName), "true", 4))
      {
         *aBoolMappings[i].pfParm = true;
      }
      else if (0 == strncasecmp (pszPos + strlen (pszName), "false", 5))
      {
         *aBoolMappings[i].pfParm = false;
      }
      else
      {
         break;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ())
      {
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Its value is " << *aBoolMappings[i].pfParm << std::endl;
      }
#endif
   }

   typedef struct _IntParmMapping {
      const char *pszName;
      int  *piParm;
   } INTPARMMAPPING, *PINTPARMMAPPING;
   INTPARMMAPPING aIntMappings[] = {
      { "iBlackReduction=",  &iBlackReduction  },
      { "iColorTech=",       &iColorTech       },
      { "iNumDitherRows=",   &iNumDitherRows   },
      { "iSrcRowPels=",      &iSrcRowPels      },
      { "iNumDestRowBytes=", &iNumDestRowBytes },
      { "iDestBitsPerPel=",  &iDestBitsPerPel  }
   };
zName);

      if (!pszPos)
         break;

#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ())
      {
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Found " << pszName << " at offset " << (pszPos - pszOptions) << std::endl;
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Looking at " << (pszPos + strlen (pszName)) << std::endl;
      }
#endif

      if (0 == strncasecmp (pszPos + strlen (pszName), "true", 4))
      {
         *aBoolMappings[i].pfParm = true;
      }
      else if (0 == strncasecmp (pszPos + strlen (pszName), "false", 5))
      {
         *aBoolMappings[i].pfParm = false;
      }
      else
      {
         break;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ())
      {
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Its value is " << *aBoolMappings[i].pfParm << std::endl;
      }
#endif
   }

   typedef struct _IntParmMapping {
      const char *pszName;
      int  *piParm;
   } INTPARMMAPPING, *PINTPARMMAPPING;
   INTPARMMAPPING aIntMappings[] = {
      { "iBlackReduction=",  &iBlackReduction  },
      { "iColorTech=",       &iColorTech       },
      { "iNumDitherRows=",   &iNumDitherRows   },
      { "iSrcRowPels=",      &iSrcRowPels      },
      { "iNumDestRowBytes=", &iNumDestRowBytes },
      { "iDestBitsPerPel=",  &iDestBitsPerPel  }
   };
Found " << pszName << " at offset " << pszPos - pszOptions << std::endl;
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Looking at " << pszPos + strlen (pszName) << std::endl;
      }
#endif

      if (0 == sscanf (pszPos + strlen (pszName),
                       "%d",
                       aIntMappings[i].   };

   for (i = 0; i < (int)dimof (aIntMappings); i++)
   {
      const char *pszName = aIntMappings[i].pszName;
      const char *pszPos  = strstr (pszOptions, pszName);

      if (!pszPos)
         break;

#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ())
      {
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Found " << pszName << " at offset " << (pszPos - pszOptions) << std::endl;
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Looking at " << (pszPos + strlen (pszName)) << std::endl;
      }
#endif

      if (0 == sscanf (pszPos + strlen (pszName), "%d", aIntMappings[i].piParm))
         break;

#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ())
      {
         DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Its value is " << *aIntMappings[i].piParm << std::endl;
      }
#endif
   }
INE__ << "!" << std::endl;
   }
#endif

   if (i != (int)dimof (aIntMappings))
      // Failure!  Every element was supposed to be on the options line.
      return 0;

   pGamma = pDevice->getCurrentGamma ();

   return new GplDitherInstance (pDevice,
                                 fDataInRGB,
                                 iBlackReduction,
                                 nameToID (pszDitherType),
                                 iColorTech,
                                 iNumDitherRows,
                                 iSrcRowPels,
                                 iNumDestRowBytes,
                                 iDestBitsPerPel,
                                 pGamma);
}

std::string * GplDitherInstance::
getCreateHash (PSZRO pszJobProperties)
{
   std::string *pstringValue = 0;
   std::string *pstringRet   = 0;

   pstringValue = DeviceDither::getDitherValue (pszJobProperties);

   if (pstringValue)
   {
      int id = nameToID (pstringValue->c_str ());

      if (DITHER_UNLISTED != id)
      {
         std::ostringstream oss;

         oss << "DDI1_"
             << id;

         pstringRet = new std::string (oss.str ());
      }

      delete pstringValue;
   }

   return pstringRet;
}

PSZCRO GplDitherInstance::
getIDFromCreateHash (std::string *pstringHash)
{
   PSZRO pszCreateHash = 0;
   int   indexDither   = -1;

   if (!pstringHash)
   {
      return 0;
   }

   pszCreateHash = pstringHash->c_str ();

   if (0 == strncmp (pszCreateHash, "DDI1_", 5))
   {
      if (  1           == sscanf (pszCreateHash, "DDI1_%d", &indexDither)
         && 0           <= indexDither
         && indexDither <  (int)dimof (apszDitherNames)
         )
      {
         return apszDitherNames[indexDither];
      }
   }

   return 0;
}

GplDitherInstance::
GplDitherInstance (Device      *pDevice,
                   bool         fDataInRGB,
                   int          iBlackReduction,
                   int          iDitherType,
                   int          iColorTech,
                   int          iNumDitherRows,
                   int          iSrcRowPels,
                   int          iNumDestRowBytes,
                   int          iDestBitsPerPel,
                   DeviceGamma *pGamma)
   : DeviceDither (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   fDataInRGB_d        = fDataInRGB;
   fModify_d           = true;
   fRemoveBlackColor_d = true;
   fStartOfPage_d      = true;
   fFirstTime_d        = true;

   iDitherType_d       = iDitherType;
   iColorTech_d        = iColorTech;
   iSrcRowPels_d       = iSrcRowPels;
   iNumDitherRows_d    = iNumDitherRows;
   iRowCount_d         = 0;

   fEmptyBlack_d       = true;
   fEmptyCyan_d        = true;
   fEmptyMagenta_d     = true;
   fEmptyYellow_d      = true;

   if (  REQUEST_DEFAULT_REDUCTION == iBlackReduction
      || 0 > iBlackReduction
      || 100 < iBlackReduction
      )
   {
      fKReductionPerc_d = 0.0;
   }
   else
   {
      fKReductionPerc_d = ((float)iBlackReduction)/100.0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ())
   {
      DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Applying gammas CMYK "
                                     << pGamma->getCGamma()
                                     << " " << pGamma->getMGamma()
                                     << " " << pGamma->getYGamma()
                                     << " " << pGamma->getKGamma()
                                     << std::endl;;
   }
#endif

   pbKGamma_d = (PBYTE)calloc (1, RGB_VALUES);
   pbRGamma_d = (PBYTE)calloc (1, RGB_VALUES);
   pbGGamma_d = (PBYTE)calloc (1, RGB_VALUES);
   pbBGamma_d = (PBYTE)calloc (1, RGB_VALUES);

   GplGammaBuildTable (iDitherType,
                       pbKGamma_d,
                       pbRGamma_d,
                       pbGGamma_d,
                       pbBGamma_d,
                       (!pGamma ?  0 : pGamma->getKBias ()),
                       (!pGamma ?  0 : pGamma->getCBias ()),
                       (!pGamma ?  0 : pGamma->getMBias ()),
                       (!pGamma ?  0 : pGamma->getYBias ()),
                       (!pGamma ? 10 : pGamma->getKGamma ()),
                       (!pGamma ? 10 : pGamma->getCGamma ()),
                       (!pGamma ? 10 : pGamma->getMGamma ()),
                       (!pGamma ? 10 : pGamma->getYGamma ()));

   midPt_d.lRed    = (int)((ULONG)((char)pbRGamma_d[127]) << ERR_EXP);
   midPt_d.lGreen  = (int)((ULONG)((char)pbGGamma_d[127]) << ERR_EXP);
   midPt_d.lBlue   = (int)((ULONG)((char)pbBGamma_d[127]) << ERR_EXP);
   midPt_d.lLRed   = (int)((ULONG)((char)pbRGamma_d[192]) << ERR_EXP);
   midPt_d.lLGreen = (int)((ULONG)((char)pbGGamma_d[192]) << ERR_EXP);
   midPt_d.lLBlue  = (int)((ULONG)((char)pbBGamma_d[192]) << ERR_EXP);
   midPt_d.lBlack  = (int)((ULONG)((char)MAX3 (pbRGamma_d[127],
                                               pbGGamma_d[127],
                                               pbBGamma_d[127]) << ERR_EXP));

   pfErrRow_d       = 0;
   pbKBuffer_d      = 0;
   pbCBuffer_d      = 0;
   pbMBuffer_d      = 0;
   pbYBuffer_d      = 0;
   pbLCBuffer_d     = 0;
   pbLMBuffer_d     = 0;
   pbCNextBuffer_d  = 0;
   pbMNextBuffer_d  = 0;
   pbYNextBuffer_d  = 0;
   pbLCNextBuffer_d = 0;
   pbLMNextBuffer_d = 0;
   phsvTable_d      = 0;

   if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
   {
       iNumPens_d = 6;
   }
   else
   {
       iNumPens_d = 4;
   }

   switch (iDitherType_d)
   {
       case DITHER_LEVEL:
       case DITHER_DITHER_4x4:
       case DITHER_DITHER_8x8:
          break;

       case DITHER_VOID_CLUSTER:
       {
          int iSize = VOID_CLUSTER_ARRAY_SIZE
                      * VOID_CLUSTER_ARRAY_SIZE
//                      * sizeof (ULONG)
                      * sizeof (float)
                      * 2;
          pfErrRow_d = (float *)malloc(iSize);
          break;
       }

       case DITHER_STUCKI_DIFFUSION:
       case DITHER_ESTUCKI_DIFFUSION:
       {
          int    iSize;

          iSize = (iNumPens_d * 4 * sizeof (float) * (iSrcRowPels_d + CMYK_FILTER_OVERFLOW));

          int    i     = iSize / sizeof (float);
          float *pf    = NULL;
          pfErrRow_d = (float *)malloc (iSize);
          for (pf = (float *)pfErrRow_d; 0 < i; i--)
             *pf++ = 0.0;
          midPt_d.lRed    = pbRGamma_d[127];
          midPt_d.lGreen  = pbGGamma_d[127];
          midPt_d.lBlue   = pbBGamma_d[127];
          midPt_d.lLRed   = pbRGamma_d[192];
          midPt_d.lLGreen = pbGGamma_d[192];
          midPt_d.lLBlue  = pbBGamma_d[192];
          midPt_d.lBlack  = MAX3 (pbRGamma_d[127], pbGGamma_d[127], pbBGamma_d[127]);
          break;
       }

       case DITHER_STUCKI_BIDIFFUSION:
       case DITHER_STEINBERG_DIFFUSION:
       {
          int iSize;
            // Num of planes * num of rows * size of value * number of pels
          iSize = (iNumPens_d * 2 * sizeof (float) * (iSrcRowPels_d + CMYK_FILTER_OVERFLOW));

          int    i     = iSize / sizeof (float);
          float *pf    = NULL;
          pfErrRow_d = (float *)malloc (iSize);

          for (pf = (float *)pfErrRow_d; 0 < i; i--)
             *pf++ = 0.0;

          break;
       }

       case DITHER_FAST_DIFFUSION:
       {
          int iSize = ERROR_FILTER_BUFFER_SIZE (iSrcRowPels_d);
          pfErrRow_d = (float *)malloc(iSize);

          iBig_d  = A_BIG_NUMBER;
          iSeed_d = A_SEED;

          GplInitializeRandomNumberTable ();
          break;
       }

       case DITHER_HSV_DIFFUSION:
       case DITHER_HSV_BIDIFFUSION:
       {
#define HSV_NONE_ERR               1        /* No error diffusion           */
#define HSV_FLST_ERR               2        /* Floyd & Steinberg       1975 */
#define HSV_JJN_ERR                3        /* Jarvis, Judice, & Ninke 1976 */
#define HSV_STU_ERR                4        /* Stucki                  1981 */
#define HSV_STAR_ERR               5        /* Stevenson & Arce        1985 */

#define NUM_HSV_NONE_ERR_NEIGHBORS 0
#define NUM_HSV_FLST_ERR_NEIGHBORS 1
#define NUM_HSV_JJN_ERR_NEIGHBORS  2
#define NUM_HSV_STU_ERR_NEIGHBORS  2
#define NUM_HSV_STAR_ERR_NEIGHBORS 3

#define NUM_HSV_NONE_ERR_ROWS      1
#define NUM_HSV_FLST_ERR_ROWS      2
#define NUM_HSV_JJN_ERR_ROWS       3
#define NUM_HSV_STU_ERR_ROWS       3
#define NUM_HSV_STAR_ERR_ROWS      4

/* Change this line only and not the following
*/
#define HSV_WHICH_ERR              STU

/* Intermediate macros
*/
#define HSV_ERR_ALGORITHM2(x)     HSV_ERR_ALGORITHM1(x)
#define HSV_ERR_ALGORITHM1(x)     HSV_##x##_ERR
#define NUM_HSV_ERR_NEIGHBORS2(x) NUM_HSV_ERR_NEIGHBORS1(x)
#define NUM_HSV_ERR_NEIGHBORS1(x) NUM_HSV_##x##_ERR_NEIGHBORS
#define NUM_HSV_ERR_ROWS2(x)      NUM_HSV_ERR_ROWS1(x)
#define NUM_HSV_ERR_ROWS1(x)      NUM_HSV_##x##_ERR_ROWS

/* Final target macros used elsewhere.
** Ex: HSV_ERR_ALGORITHM     -> HSV_xxx_ERR
**     NUM_HSV_ERR_NEIGHBORS -> NUM_HSV_xxx_ERR_NEIGHBORS
**     NUM_HSV_ERR_ROWS      -> NUM_HSV_xxx_ERR_ROWS
** where xxx is (FLST, STU, or STAR)
*/
#define HSV_ERR_ALGORITHM         HSV_ERR_ALGORITHM2(HSV_WHICH_ERR)
#define NUM_HSV_ERR_NEIGHBORS     NUM_HSV_ERR_NEIGHBORS2(HSV_WHICH_ERR)
#define NUM_HSV_ERR_ROWS          NUM_HSV_ERR_ROWS2(HSV_WHICH_ERR)

          int iSize = ( ( NUM_HSV_ERR_NEIGHBORS   // <-pad->
                        + iSrcRowPels_d           // cx
                        + NUM_HSV_ERR_NEIGHBORS   // <-pad->
                        )
                      * NUM_HSV_ERR_ROWS          // cy
                      )
                    * sizeof (HSVERRORSTRUC)
                    ;
          pfErrRow_d = (float *)malloc(iSize);
          break;
       }

       case DITHER_CMYK_DIFFUSION:
       {
          int iSize = ( ( NUM_HSV_ERR_NEIGHBORS   // <-pad->
                        + iSrcRowPels_d           // cx
                        + NUM_HSV_ERR_NEIGHBORS   // <-pad->
                        )
                      * NUM_HSV_ERR_ROWS          // cy
                      )
                    * sizeof (HSVERRORSTRUC)
                    ;
          pfErrRow_d = (float *)malloc(iSize);
          break;
       }
   }

   if (0 == iDestBitsPerPel)
      iDestBitsPerPel_d = 1;
   else
      iDestBitsPerPel_d = iDestBitsPerPel;

   iNumDestRowBytes_d = ((iSrcRowPels * iDestBitsPerPel_d) + 7) / 8;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": iNumDitherRows_d = " << iNumDitherRows_d << ", iNumDestRowBytes_d  = " << iNumDestRowBytes_d << std::endl;
#endif

   int iBlockSize = iNumDitherRows_d * iNumDestRowBytes_d;
   int iSize      = 0;

   if (DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d)
   {
      iSize           = 4 * iBlockSize;
      pbDest_d        = (PBYTE)calloc (1, iSize);

      if (!pbDest_d)
      {
          DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": CMYK Dither Memory allocation failure " << std::endl;
      }

      pbKBuffer_d     = pbDest_d;
      pbKNextBuffer_d = pbKBuffer_d + iNumDestRowBytes_d;
      pbCBuffer_d     = pbKBuffer_d + iBlockSize;
      pbCNextBuffer_d = pbCBuffer_d + iNumDestRowBytes_d;
      pbMBuffer_d     = pbCBuffer_d + iBlockSize;
      pbMNextBuffer_d = pbMBuffer_d + iNumDestRowBytes_d;
      pbYBuffer_d     = pbMBuffer_d + iBlockSize;
      pbYNextBuffer_d = pbYBuffer_d + iNumDestRowBytes_d;

   }
   else if(DevicePrintMode::COLOR_TECH_CMY == iColorTech_d)
   {
      iSize           = 3 * iBlockSize;
      pbDest_d        = (PBYTE)calloc (1, iSize);

      if (!pbDest_d)
      {
          DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": CMY Dither Memory allocation failure " << std::endl;
      }

      pbKBuffer_d     = 0;
      pbKNextBuffer_d = 0;
      pbCBuffer_d     = pbDest_d;
      pbCNextBuffer_d = pbCBuffer_d + iNumDestRowBytes_d;
      pbMBuffer_d     = pbCBuffer_d + iBlockSize;
      pbMNextBuffer_d = pbMBuffer_d + iNumDestRowBytes_d;
      pbYBuffer_d     = pbMBuffer_d + iBlockSize;
      pbYNextBuffer_d = pbYBuffer_d + iNumDestRowBytes_d;
   }
   else if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
   {
      iSize           = 6 * iBlockSize;
      pbDest_d        = (PBYTE)calloc (1, iSize);

      if (!pbDest_d)
      {
          DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": CcMmYK Dither Memory allocation failure " << std::endl;
      }

      memset (pbDest_d, 0, iSize);

      pbKBuffer_d      = pbDest_d;
      pbKNextBuffer_d  = pbKBuffer_d  + iNumDestRowBytes_d;
      pbCBuffer_d      = pbKBuffer_d  + iBlockSize;
      pbCNextBuffer_d  = pbCBuffer_d  + iNumDestRowBytes_d;
      pbMBuffer_d      = pbCBuffer_d  + iBlockSize;
      pbMNextBuffer_d  = pbMBuffer_d  + iNumDestRowBytes_d;
      pbYBuffer_d      = pbMBuffer_d  + iBlockSize;
      pbYNextBuffer_d  = pbYBuffer_d  + iNumDestRowBytes_d;
      pbLCBuffer_d     = pbYBuffer_d  + iBlockSize;
      pbLCNextBuffer_d = pbLCBuffer_d + iNumDestRowBytes_d;
      pbLMBuffer_d     = pbLCBuffer_d + iBlockSize;
      pbLMNextBuffer_d = pbLMBuffer_d + iNumDestRowBytes_d;

   }

   pbDestEnd_d = pbDest_d + iSize - 1;   // end of Dest for mem overwrite check

   pbdKPlane_d  = new BinaryData (pbKBuffer_d, iNumDitherRows_d * iNumDestRowBytes_d);
   pbdCPlane_d  = new BinaryData (pbCBuffer_d, iNumDitherRows_d * iNumDestRowBytes_d);
   pbdMPlane_d  = new BinaryData (pbMBuffer_d, iNumDitherRows_d * iNumDestRowBytes_d);
   pbdYPlane_d  = new BinaryData (pbYBuffer_d, iNumDitherRows_d * iNumDestRowBytes_d);
   pbdLCPlane_d = 0;
   pbdLMPlane_d = 0;

   pbLightTable_d = new byte[256];
   pbDarkTable_d  = new byte[256];

   if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
   {
       int i, iValue;;
       float fPercent = .1;
       float ftmp;
       for(i = 0 ; i < 256 ; i++)
       {
//         if(!(i % 9) && (i != 0))
//             fPercent = fPercent+.038;
         if(!(i % 6) && (i != 0))
             fPercent = fPercent+.022;          // Saturation Values -> down is greater
         iValue = (int)((float)i*fPercent);
         if(iValue > i)
             iValue = i;
         pbDarkTable_d[i] =(byte) iValue;
       }

//       for (i = 0 ; i < 256 ; i++)
//       {
//           fPercent = (float)i/255.0;
//           iValue = (int)((float)i * (1.0 - fPercent));
//           pbLightTable_d[i]= (byte) iValue;
//       }
       fPercent = 1.0;
       ftmp = .05;
       for(i = 0 ; i < 256 ; i ++)
       {
           if(!(i % 8) && (i != 0))
           {
               fPercent = fPercent - ftmp;
//               if(ftmp > .03)
//               if(ftmp > .022)
//               if(ftmp > .010)
               if(ftmp > 0.0)   // <== As we drop this value, we raise the amount of light
                   ftmp -=.005;	 // for brightness and intensity at the high end of the spectrum
           }
           iValue = (int)((float)i * fPercent);
           pbLightTable_d[i] = (byte) iValue;
       }


       pbdLCPlane_d = new BinaryData (pbLCBuffer_d, iNumDitherRows_d * iNumDestRowBytes_d);
       pbdLMPlane_d = new BinaryData (pbLMBuffer_d, iNumDitherRows_d * iNumDestRowBytes_d);
   }

   // Clean up
   delete pGamma;
}

GplDitherInstance::
~GplDitherInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pbKGamma_d)
   {
      free (pbKGamma_d);
      pbKGamma_d = 0;
   }
   if (pbRGamma_d)
   {
      free (pbRGamma_d);
      pbRGamma_d = 0;
   }
   if (pbGGamma_d)
   {
      free (pbGGamma_d);
      pbGGamma_d = 0;
   }
   if (pbBGamma_d)
   {
      free (pbBGamma_d);
      pbBGamma_d = 0;
   }
   if (pfErrRow_d)
   {
      free (pfErrRow_d);
      pfErrRow_d = 0;
   }
   if (phsvTable_d)
   {
      free (phsvTable_d);
      phsvTable_d = 0;
   }
   if (pbDest_d)
   {
      free (pbDest_d);
      pbDest_d = 0;
   }
   if (pbdKPlane_d)
   {
      delete pbdKPlane_d;
      pbdKPlane_d = 0;
   }
   if (pbdCPlane_d)
   {
      delete pbdCPlane_d;
      pbdCPlane_d = 0;
   }
   if (pbdMPlane_d)
   {
      delete pbdMPlane_d;
      pbdMPlane_d = 0;
   }
   if (pbdYPlane_d)
   {
      delete pbdYPlane_d;
      pbdYPlane_d = 0;
   }
   if (pbdLCPlane_d)
   {
      delete pbdLCPlane_d;
      pbdLCPlane_d = 0;
   }
   if (pbdLMPlane_d)
   {
      delete pbdLMPlane_d;
      pbdLMPlane_d = 0;
   }
   if (pbLightTable_d)
   {
       delete[] pbLightTable_d;
       pbLightTable_d = 0;
   }
   if (pbDarkTable_d)
   {
       delete[] pbDarkTable_d;
       pbDarkTable_d = 0;
   }
}

PSZCRO GplDitherInstance::
getID ()
{
   return apszDitherNames[iDitherType_d];
}

class GplDitherEnumerator : public Enumeration
{
public:
   GplDitherEnumerator (int cDithers, PSZCRO aDithers[])
   {
      iDither_d  = 0;
      cDithers_d = cDithers;
      aDithers_d = aDithers;
   }

   virtual bool hasMoreElements ()
   {
      if (iDither_d < cDithers_d)
         return true;
      else
         return false;
   }

   virtual void *nextElement ()
   {
      if (iDither_d > cDithers_d - 1)
         return 0;

      std::ostringstream oss;

      oss << JOBPROP_DITHER
          << "="
          << aDithers_d[iDither_d++];

      return (void *)new JobProperties (oss.str ().c_str ());
   }

private:
   int     iDither_d;
   int     cDithers_d;
   PSZCRO *aDithers_d;
};

Enumeration * GplDitherInstance::
getAllEnumeration ()
{
   return new GplDitherEnumerator (dimof (apszDitherNames), apszDitherNames);
}

void GplDitherInstance::
ditherRGBtoCMYK (PBITMAPINFO2 pbmi2, PBYTE pbStart)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (fFirstTime_d)
   {
      iNumColors_d = 1 << pbmi2->cBitCount;

      if (16 > pbmi2->cBitCount)
      {
         LONG  i,
               j,
               k,
               numColorGroups,
               numTableColors,
               midpt,
               midptLight;
         PRGB2 pRGB;

         numTableColors = 1 << pbmi2->cBitCount;
         midpt = numTableColors / 2 - 1;
         midptLight = numTableColors - numTableColors / 4 - 1;
         numColorGroups = 256 / numTableColors;

         midPt_d.lRed    = pbRGamma_d[midpt];
         midPt_d.lGreen  = pbGGamma_d[midpt];
         midPt_d.lBlue   = pbBGamma_d[midpt];
         midPt_d.lLRed   = pbRGamma_d[midptLight];
         midPt_d.lLGreen = pbGGamma_d[midptLight];
         midPt_d.lLBlue  = pbBGamma_d[midptLight];
         midPt_d.lBlack  = pbGGamma_d[midpt];

         /*------------------------------------------------*/
         /* Replace the color table value with a gamma     */
         /* adjusted value                                 */
         /*------------------------------------------------*/
         pRGB = pbmi2->argbColor;
         for (j = 0; j < numColorGroups; j++)
         {
            k = j * numTableColors;

            for (i = 0; i < numTableColors; i++)
            {
               pRGB[i+k].bRed   = pbRGamma_d[pRGB[i].bRed];
               pRGB[i+k].bGreen = pbGGamma_d[pRGB[i].bGreen];
               pRGB[i+k].bBlue  = pbBGamma_d[pRGB[i].bBlue];

               // Black, uses unused fcOptions for storage.
               // Create it if it is needed or not.
               pRGB[i].fcOptions = MAX3 (pRGB[i].bRed,
                                         pRGB[i].bGreen,
                                         pRGB[i].bBlue);

               pRGB[i+k].fcOptions = pbGGamma_d[pRGB[i].fcOptions];
            }
         }

         /*--------------------------------*/
         /* Create Black for color table   */
         /*--------------------------------*/
         if (  DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d
            && (  DITHER_STUCKI_DIFFUSION    == iDitherType_d
               || DITHER_STEINBERG_DIFFUSION == iDitherType_d
               || DITHER_FAST_DIFFUSION      == iDitherType_d
               || DITHER_DITHER_4x4          == iDitherType_d
               || DITHER_DITHER_8x8          == iDitherType_d
               || DITHER_STUCKI_BIDIFFUSION  == iDitherType_d
               || DITHER_ESTUCKI_DIFFUSION   == iDitherType_d
               )
            )
         {
            ULONG ulTemp;
            int   i;

            for (i = 0; i < 256; i++)
            {
               ulTemp = (MAX_RGB_VALUE - pbmi2->argbColor[i].fcOptions);

               // Lighten each color by removing % of darkness
               // New !!
               pbmi2->argbColor[i].fcOptions += (byte)((double)ulTemp * fKReductionPerc_d);
               pbmi2->argbColor[i].bRed      += ulTemp;
               pbmi2->argbColor[i].bGreen    += ulTemp;
               pbmi2->argbColor[i].bBlue     += ulTemp;
            }

            midPt_d.lRed    = (LONG)((ULONG)pbmi2->argbColor[127].bRed      << ERR_EXP);
            midPt_d.lGreen  = (LONG)((ULONG)pbmi2->argbColor[127].bGreen    << ERR_EXP);
            midPt_d.lBlue   = (LONG)((ULONG)pbmi2->argbColor[127].bBlue     << ERR_EXP);
            midPt_d.lLRed   = (LONG)((ULONG)pbmi2->argbColor[192].bRed      << ERR_EXP);
            midPt_d.lLGreen = (LONG)((ULONG)pbmi2->argbColor[192].bGreen    << ERR_EXP);
            midPt_d.lLBlue  = (LONG)((ULONG)pbmi2->argbColor[192].bBlue     << ERR_EXP);
            midPt_d.lBlack  = (LONG)((ULONG)pbmi2->argbColor[127].fcOptions << ERR_EXP);

            midPt_d.lRed    = (LONG)((ULONG)pbmi2->argbColor[64].bRed       << ERR_EXP);
            midPt_d.lGreen  = (LONG)((ULONG)pbmi2->argbColor[64].bGreen     << ERR_EXP);
            midPt_d.lBlue   = (LONG)((ULONG)pbmi2->argbColor[64].bBlue      << ERR_EXP);
         }
      }

      // If we are using "Magic Squares" or "Ordered Squares" dithers
      // we need to setup HSV tables
      if (  DITHER_MAGIC_SQUARES   == iDitherType_d
         || DITHER_ORDERED_SQUARES == iDitherType_d
         )
      {
         GplCreateHSVcolorTable (pbmi2);
      }
   }

   if (!fStartOfPage_d)
   {
      size_t stSize = (size_t)(iNumDestRowBytes_d * (LONG)pbmi2->cy);

      memset (pbCBuffer_d, 0, stSize);
      memset (pbMBuffer_d, 0, stSize);
      memset (pbYBuffer_d, 0, stSize);

      if ((iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)||(iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK))
         memset (pbKBuffer_d, 0, stSize);

      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
      {
          memset (pbLCBuffer_d, 0, stSize);
          memset (pbLMBuffer_d, 0, stSize);
      }
   }

   GplSeparateColors (pbmi2, pbStart);

   fStartOfPage_d = false;
   fFirstTime_d   = false;
}

bool GplDitherInstance::
ditherAllPlanesBlank ()
{
///if (fEmptyBlack_d)
///   DebugOutput::getErrorStream () << " ";
///else
///   DebugOutput::getErrorStream () << "K";
///if (fEmptyCyan_d)
///   DebugOutput::getErrorStream () << " ";
///else
///   DebugOutput::getErrorStream () << "C";
///if (fEmptyMagenta_d)
///   DebugOutput::getErrorStream () << " ";
///else
///   DebugOutput::getErrorStream () << "M";
///if (fEmptyYellow_d)
///   DebugOutput::getErrorStream () << " ";
///else
///   DebugOutput::getErrorStream () << "Y";
///DebugOutput::getErrorStream () << std::endl;

  if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
  {
      return    fEmptyBlack_d
             && fEmptyCyan_d
             && fEmptyMagenta_d
             && fEmptyYellow_d
             && fEmptyLCyan_d
             && fEmptyLMagenta_d;
  }
  else
  {
     return    fEmptyBlack_d
             && fEmptyCyan_d
             && fEmptyMagenta_d
             && fEmptyYellow_d;
  }
}

bool GplDitherInstance::
ditherCPlaneBlank ()
{
   return fEmptyCyan_d;
}

bool GplDitherInstance::
ditherMPlaneBlank ()
{
   return fEmptyMagenta_d;
}

bool GplDitherInstance::
ditherYPlaneBlank ()
{
   return fEmptyYellow_d;
}

bool GplDitherInstance::
ditherKPlaneBlank ()
{
   return fEmptyBlack_d;
}

bool GplDitherInstance::
ditherLCPlaneBlank ()
{
   return fEmptyLCyan_d;
}

bool GplDitherInstance::
ditherLMPlaneBlank ()
{
   return fEmptyLMagenta_d;
}

void GplDitherInstance::
newFrame ()
{
   fStartOfPage_d = true;
}

BinaryData * GplDitherInstance::
getCPlane ()
{
   return pbdCPlane_d;
}

BinaryData * GplDitherInstance::
getMPlane ()
{
   return pbdMPlane_d;
}

BinaryData * GplDitherInstance::
getYPlane ()
{
   return pbdYPlane_d;
}

BinaryData * GplDitherInstance::
getKPlane ()
{
   return pbdKPlane_d;
}

BinaryData * GplDitherInstance::
getLCPlane ()
{
   return pbdLCPlane_d;
}

BinaryData * GplDitherInstance::
getLMPlane ()
{
   return pbdLMPlane_d;
}

/****************************************************************************/
/* PROCEDURE NAME : GplSeparateColors                                       */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    :                                                         */
/*                                                                          */
/*   This function performs RGB to CMYK conversion of each pel in a         */
/* color bitmap.  This function can handle 1, 4, 8, 16, or 24 bpp           */
/* bitmap formats.  NOTE: it has only been tested on 8 and 24.              */
/*                                                                          */
/* CONTROL FLOW:                                                            */
/*                                                                          */
/*  [1] Initialize "bEmpty" flags to true indicating CMYK buffers are       */
/*      assumed empty until dithering determines otherwise.                 */
/*  [2] Validate input bitmap size (based on planes/bitcount).              */
/*  [3] Call the specific dithering algorithm caller requested.             */
/*  [4] Upon return will have CMYK buffers with dithered                    */
/*      output to be printed.                                               */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplSeparateColors (PBITMAPINFO2 pbmi2, PBYTE pbSource)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   fEmptyBlack_d    = true;
   fEmptyCyan_d     = true;
   fEmptyMagenta_d  = true;
   fEmptyYellow_d   = true;
   fEmptyLCyan_d    = true;
   fEmptyLMagenta_d = true;

   int iNewLength = iNumDestRowBytes_d * pbmi2->cy;
   pbdKPlane_d->setLength (iNewLength);
   pbdCPlane_d->setLength (iNewLength);
   pbdMPlane_d->setLength (iNewLength);
   pbdYPlane_d->setLength (iNewLength);

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbdLCPlane_d->setLength (iNewLength);
       pbdLMPlane_d->setLength (iNewLength);
   }

   int iPelSize = (LONG)pbmi2->cPlanes * (LONG)pbmi2->cBitCount;

   switch (iDitherType_d)
   {
   case DITHER_MAGIC_SQUARES:
   {
      return GplColorSquares (pbmi2, pbSource, aPaintmixer);
   }

   case DITHER_ORDERED_SQUARES:
   {
      return GplColorSquares (pbmi2, pbSource, aOrdered);
   }

   case DITHER_DITHER_4x4:
   {
      if (iPelSize < 16)
         return GplDitherMatrix (pbmi2, pbSource, 4, (PRGB2)paHalftone4x4);
      else
         return GplDitherMatrix (pbmi2, pbSource, 4, (PRGB2)paHalftone4x4_24);
   }

   case DITHER_DITHER_8x8:
   {
      return GplDitherMatrix (pbmi2, pbSource, 8, (PRGB2)paHalftone8x8);
   }

   case DITHER_STUCKI_DIFFUSION:
   {
      return GplStuckiDiffusion (pbmi2, pbSource);
   }
   case DITHER_ESTUCKI_DIFFUSION:
   {
      return GplEnhancedStuckiDiffusion (pbmi2, pbSource);
   }

   case DITHER_STEINBERG_DIFFUSION:
   {
      return GplSteinbergDiffusion (pbmi2, pbSource);
   }

   case DITHER_STUCKI_BIDIFFUSION:
   {
      return GplStuckiBiffusion (pbmi2, pbSource);
   }

   case DITHER_FAST_DIFFUSION:
   {
      return GplFastDiffusion (pbmi2, pbSource);
   }

   case DITHER_HSV_DIFFUSION:
   {
      return GplHSVDiffusion (pbmi2, pbSource);
   }

   case DITHER_HSV_BIDIFFUSION:
   {
      return GplHSVBidiffusion (pbmi2, pbSource);
   }

   case DITHER_CMYK_DIFFUSION:
   {
      return GplCMYKDiffusion (pbmi2, pbSource);
   }

///// jk1115
///case DITHER_VOID_CLUSTER:
///{
///   return GplVoidCluster (pbmi2, pbSource);
///}

   case DITHER_LEVEL:
   default:
      return GplLevel (pbmi2, pbSource);
   }
}

/* color request -  left to right */
static const BYTE pbProcessList[9][5] = {
   {CMYK_CYAN,    (BYTE)-1,     (BYTE)-1,     (BYTE)-1,    (BYTE)-1},
   {CMYK_MAGENTA, (BYTE)-1,     (BYTE)-1,     (BYTE)-1,    (BYTE)-1},
   {CMYK_YELLOW,  (BYTE)-1,     (BYTE)-1,     (BYTE)-1,    (BYTE)-1},
   {CMYK_BLACK,   (BYTE)-1,     (BYTE)-1,     (BYTE)-1,    (BYTE)-1},
   {CMYK_CYAN,    CMYK_MAGENTA, CMYK_YELLOW,  (BYTE)-1,    (BYTE)-1},
   {CMYK_BLACK,   CMYK_CYAN,    CMYK_MAGENTA, CMYK_YELLOW, (BYTE)-1},
   {CMYK_BLACK,   CMYK_YELLOW,  CMYK_MAGENTA, CMYK_CYAN,   (BYTE)-1},
   {CMYK_YELLOW,  CMYK_MAGENTA, CMYK_CYAN,    (BYTE)-1,    (BYTE)-1},
   {CMYK_CYAN,    CMYK_MAGENTA, CMYK_YELLOW,  CMYK_BLACK,  (BYTE)-1}
};

int GplDitherInstance::
GplLevel (PBITMAPINFO2 pbmi2, PBYTE pbSource)
{
// The following 2 macros control bitmap scan indexing

#define INC                i++; pbSrc++;                \
                           if ((i & 7) == 0)            \
                              pbDest++;

#define TESTROWEND         if (i >= (LONG)pbmi2->cx)               \
                           {                                       \
                              i = 0;                               \
                              pbSrc      = pbSrcNext;              \
                              pbDest     = pbDestNext;             \
                              pbDestNext = pbDest + params.iDestRowSize;  \
                              pbSrcNext  = pbSrc  + params.iSrcRowSize;   \
                              if ((ULONG)pbSrc >= (ULONG)pbMapEnd) \
                                 break;                            \
                           }

// The following 2 macros set print bits

#define SETBIT(p)          *pbDest |= (BYTE)(0x80 >> (i & 7)); \
                           fEmpty##p##_d = false;

#define SNAP(bVal,c,p)     if (bVal<= midpt.c)   \
                           {                     \
                              SETBIT (p)         \
                           }

// The following 6 macros are invoked indirectly as required; see below
//
// The next 4 macros perform 8 bit pel bitmap scans

#define ECMY4(c, p)        while((ULONG)pbSrc < (ULONG)pbMapEnd)                       \
                           {                                                           \
                              SNAP (pColor->is[(LONG)((*pbSrc & 0xf0) >> 4)].c, c, p)  \
                              i++;                                                     \
                              TESTROWEND                                               \
                              SNAP (pColor->is[(LONG)(*pbSrc & 0x0f)].c, c, p)         \
                              INC                                                      \
                              TESTROWEND                                               \
                           }

#define EBLACK4            while ((ULONG)pbSrc < (ULONG)pbMapEnd)                                 \
                           {                                                                      \
                              if (  pColor->is[(LONG)((*pbSrc & 0xf0)>>4)].bRed <= midpt.bRed     \
                                 && pColor->is[(LONG)((*pbSrc & 0xf0)>>4)].bGreen <= midpt.bGreen \
                                 && pColor->is[(LONG)((*pbSrc & 0xf0)>>4)].bBlue <= midpt.bBlue   \
                                 )                                                                \
                               {                                                                  \
                                 SETBIT (Black)             \
                                 if (modify)                \
                                    *pbSrc = iWhiteIndex_d; \
                               }                            \
                               i++;                         \
                               TESTROWEND                   \
                               if (  pColor->is[(LONG)(*pbSrc & 0x0f)].bRed <= midpt.bRed     \
                                  && pColor->is[(LONG)(*pbSrc & 0x0f)].bGreen <= midpt.bGreen \
                                  && pColor->is[(LONG)(*pbSrc & 0x0f)].bBlue <= midpt.bBlue   \
                                  )                                                           \
                               {                                                              \
                                  SETBIT (Black)             \
                                  if (modify)                \
                                     *pbSrc = iWhiteIndex_d; \
                               }                             \
                               INC                           \
                               TESTROWEND                    \
                           }

#define ECMY8(c, p)        while ((ULONG)pbSrc < (ULONG)pbMapEnd)        \
                           {                                             \
                              SNAP (pColor->is[(LONG)(*pbSrc)].c, c, p)  \
                              INC                                        \
                              TESTROWEND                                 \
                           }

// EBLACK8 and EBLACK24 require that all three pColors to be less than
// the respective mid point. If the "distroy" boolean is set,
// the source bitmap pel index value will be changed to point
// to white, an RGB pColor in which all pColor values are greater than
// the mid point.

#define EBLACK8            while ((ULONG)pbSrc < (ULONG)pbMapEnd)                     \
                           {                                                          \
                              if (  pColor->is[(LONG)(*pbSrc)].bRed <= midpt.bRed     \
                                 && pColor->is[(LONG)(*pbSrc)].bGreen <= midpt.bGreen \
                                 && pColor->is[(LONG)(*pbSrc)].bBlue <= midpt.bBlue   \
                                 )                                                    \
                              {                                                       \
                                 SETBIT (Black)             \
                                 if (modify)                \
                                    *pbSrc = iWhiteIndex_d; \
                              }                             \
                              INC                           \
                              TESTROWEND                    \
                           }

// The following 2 bitmaps perform 16 bit pel bitmap scans

#define ECMY16(src, c, p)  SNAP (src, c, p)

#define EBLACK16           if (  (*pbSrc & 0xf8) <= midpt.bRed                                                  \
                              && (((*pbSrc & 7) << 5) | ((*(pbSrc+1) & 0xE0) >> 3) ) <= midpt.bGreen            \
                              && ((*(pbSrc+1) & 0x1f) << 3) <= midpt.bBlue                                      \
                              )                                                                                 \
                           {                                                                                    \
                              SETBIT (Black)                                                                    \
                              if (modify)                                                                       \
                              {                                                                                 \
                                 *pbSrc     = (maxColor.bRed & 0xf8) | (maxColor.bGreen >> 5);                  \
                                 *(pbSrc+1) = ((maxColor.bGreen & 0x1c) << 3) | ((maxColor.bBlue & 0xf8) >> 3); \
                              }                                                                                 \
                           }

// The following 2 bitmaps perform 24 bit pel bitmap scans

#define ECMY24(b, c, p)    SNAP (*(pbSrc + b), c, p)

#define EBLACK24           if (  *(pbSrc+2) <= midpt.bRed      \
                              && *(pbSrc+1) <= midpt.bGreen    \
                              && *(pbSrc) <= midpt.bBlue       \
                              )                                \
                           {                                   \
                              SETBIT (Black)                   \
                              if (modify)                      \
                              {                                \
                                 *(pbSrc+2) = maxColor.bRed;   \
                                 *(pbSrc+1) = maxColor.bGreen; \
                                 *(pbSrc)   = maxColor.bBlue;  \
                              }                                \
                           }

// macro GENERATE_CMY_COLOR
//
// This macro generates specific code for each pColor
// except Black  which depends on the values of all 3 pColors.
//
// For a pColor and pel size
//    loop through the bitmap
//       if a pel pColor value < the midpoint value
//          set the corresponding print bit
//          Note that conversion to CMY in implicit

#define GENERATE_CMY_COLOR(c, p)                           \
         switch (params.iPelSize)                                 \
         {                                                 \
            case 1:                                        \
               savepbDest = pbDest;                        \
               if (pColor->is[0].c <= midpt.c)             \
                  while ((ULONG)pbSrc < (ULONG)pbMapEnd)   \
                  {                                        \
                     *pbDest = ~(*pbSrc);                  \
                     i+=8; pbSrc++; pbDest++;              \
                     TESTROWEND                            \
                  }                                        \
               pbDest = savepbDest;                        \
               pbDestNext = savepbDest + params.iDestRowSize;     \
               pbSrc = pbSource;                           \
               pbSrcNext = pbSource + params.iSrcRowSize;         \
               i = 0;                                      \
               if (pColor->is[1].c <= midpt.c)             \
                  while ((ULONG)pbSrc < (ULONG)pbMapEnd)   \
                  {                                        \
                     *pbDest |= (*pbSrc);                  \
                     i+=8; pbSrc++; pbDest++;              \
                     TESTROWEND                            \
                  }                                        \
               break;                                      \
            case 2:                                        \
               while ((ULONG)pbSrc < (ULONG)pbMapEnd)                   \
               {                                                        \
                  SNAP (pColor->is[(LONG)(*pbSrc & 0xC0) >> 6].c, c, p) \
                  i++;                                                  \
                  TESTROWEND                                            \
                  SNAP (pColor->is[(LONG)(*pbSrc & 0x30) >> 4].c, c, p) \
                  i++;                                                  \
                  TESTROWEND                                            \
                  SNAP (pColor->is[(LONG)(*pbSrc & 0xC) >> 2].c, c, p)  \
                  i++;                                                  \
                  TESTROWEND                                            \
                  SNAP (pColor->is[(LONG)(*pbSrc & 3)].c, c, p)         \
                  INC                                      \
                  TESTROWEND                               \
               }                                           \
               break;                                      \
            case 4:                                        \
               BLACK4                                      \
               CMY4                                        \
               break;                                      \
            case 8:                                        \
               BLACK8                                      \
               CMY8                                        \
               break;                                      \
            case 16:                                       \
               while ((ULONG)pbSrc < (ULONG)pbMapEnd)      \
               {                                           \
                  BLACK16                                  \
                  CMY16                                    \
                  pbSrc+=2;                                \
                  i++;                                     \
                  TESTROWEND                               \
               }                                           \
                break;                                     \
            case 24:                                       \
               while ((ULONG)pbSrc < (ULONG)pbMapEnd)      \
               {                                           \
                  BLACK24                                  \
                  CMY24                                    \
                  pbSrc+=3;                                \
                  i++;                                     \
                  if ((i & 7) == 0)                        \
                     pbDest++;                             \
                  TESTROWEND                               \
               }                                           \
               break;                                      \
            default:                                       \
              return INVALID_BITMAP;                       \
        }

   LONG     i, j, n;
   PBYTE    pbMapEnd, pbSrcNext, pbDestNext, pbDest, pbSrc, savepbDest;
   RGB2     midpt    = {127,
                        127,
                        127,
                        127};

   RGB2     maxColor = {0,0,0,0};
   RGB2     minColor = {MAX_RGB_VALUE, MAX_RGB_VALUE, MAX_RGB_VALUE, MAX_RGB_VALUE };
   PCOLR    pColor   = 0;
   BOOL     modify   = fModify_d;
   PARAMS   params;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   params.bMultiBitEnabled = false;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

   if (params.iMapSize == 0)
      return INVALID_BITMAP;

   // For pels less than 16 bits,
   //    the pColor table must be provided at the end of BITMAPINFO2
   //    When the pColor table changes, scan all pColor values for
   //       a white index to use if distroy is set and for some snap types
   //       and if the minimum pColor values are requested to be used for snapping
   //          find a Black  index
   if (16 > params.iPelSize)
   {
      // 4 bit pel bitmaps may be reformatted to 8 bit pels by JPEG coversion to BMP, therefore,
      // the number of used pColors may be different than the plane pel bit count,
      // so create a bMask for the pColor index

      pColor = (PCOLR)&pbmi2->argbColor[0];

      if (fFirstTime_d)
      {
         for (j = 0; j < iNumColors_d; j++)
         {
            if (  pColor->is[j].bRed > maxColor.bRed
               && pColor->is[j].bGreen > maxColor.bGreen
               && pColor->is[j].bBlue > maxColor.bBlue)
            {
               maxColor.bRed = pColor->is[j].bRed;
               maxColor.bGreen = pColor->is[j].bGreen;
               maxColor.bBlue = pColor->is[j].bBlue;
               iWhiteIndex_d = j;
            }
            if (  pColor->is[j].bRed < minColor.bRed
               && pColor->is[j].bGreen < minColor.bGreen
               && pColor->is[j].bBlue < minColor.bBlue)
            {
               minColor.bRed = pColor->is[j].bRed;
               minColor.bGreen = pColor->is[j].bGreen;
               minColor.bBlue = pColor->is[j].bBlue;
               iBlackIndex_d = j;
            }
         }
      }
      else
      {
         maxColor.bRed = pColor->is[iWhiteIndex_d].bRed;
         maxColor.bGreen = pColor->is[iWhiteIndex_d].bGreen;
         maxColor.bBlue = pColor->is[iWhiteIndex_d].bBlue;

         minColor.bRed = pColor->is[iBlackIndex_d].bRed;
         minColor.bGreen = pColor->is[iBlackIndex_d].bGreen;
         minColor.bBlue = pColor->is[iBlackIndex_d].bBlue;
      }
   }
   else
   {
      maxColor.bRed = maxColor.bGreen = maxColor.bBlue = MAX_RGB_VALUE;
      minColor.bRed = minColor.bGreen = minColor.bBlue = 1;
   }

   // @TBD missing from port
   midpt.bRed   = (BYTE)((ULONG)maxColor.bRed   / 2);
   midpt.bGreen = (BYTE)((ULONG)maxColor.bGreen / 2);
   midpt.bBlue  = (BYTE)((ULONG)maxColor.bBlue  / 2);

   // @TBD convert/rewrite
   pbDest     = pbDest_d;
   pbDestNext = pbDest_d + params.iDestRowSize;

   // for each pColor requested to be separated out of the bitmap
   //    set the source bitmap index pointer
   //    clear the pbDestination buffer for the current pColor to be output
   //    for the current pColor
   //       and pel size
   //          and for each pel
   //             set a print bit if the source bitmap value is less that a midpoint
   //             value for the specific pColor
   n = 0;
   while (pbProcessList[(ULONG)iColorTech_d][n] != (BYTE)-1)
   {
      pbSrc = pbSource;
      pbSrcNext = pbSource + params.iSrcRowSize;
      memset (pbDest, 0, (size_t)(params.iDestRowSize*(LONG)pbmi2->cy));
      i = 0;

      switch (pbProcessList[(ULONG)iColorTech_d][n])
      {
      case CMYK_BLACK:
      {
         #define BLACK4 EBLACK4
         #define BLACK8 EBLACK8
         #define BLACK16 EBLACK16
         #define BLACK24 EBLACK24
         #define CMY4
         #define CMY8
         #define CMY16
         #define CMY24
         // bGreen is arbitrarily chosen here since it is not used
         GENERATE_CMY_COLOR (bGreen, Black)
         #undef BLACK4
         #undef BLACK8
         #undef BLACK16
         #undef BLACK24
         #define BLACK4
         #define BLACK8
         #define BLACK16
         #define BLACK24
         #undef CMY4
         #undef CMY8
         #undef CMY16
         #undef CMY24
         break;
      }

      case CMYK_CYAN:
      {
         #define CMY4 ECMY4 (bRed, Cyan)
         #define CMY8 ECMY8 (bRed, Cyan)
         #define CMY16 ECMY16 ((*(pbSrc) & 0xf8), bBlue, Yellow)
         #define CMY24 ECMY24 (0, bRed, Cyan)
         GENERATE_CMY_COLOR (bRed, Cyan)
         #undef CMY4
         #undef CMY8
         #undef CMY16
         #undef CMY24
         break;
      }

      case CMYK_MAGENTA:
      {
         #define CMY4 ECMY4 (bGreen, Magenta)
         #define CMY8 ECMY8 (bGreen, Magenta)
         #define CMY16 ECMY16 ((((*pbSrc & 7) << 5) | ((*(pbSrc+1) & 0xE0) >> 3)), bBlue, Yellow)
         #define CMY24 ECMY24 (1, bGreen, Magenta)
         GENERATE_CMY_COLOR (bGreen, Magenta)
         #undef CMY4
         #undef CMY8
         #undef CMY16
         #undef CMY24
         break;
      }

      case CMYK_YELLOW:
      {
         #define CMY4 ECMY4 (bBlue, Yellow)
         #define CMY8 ECMY8 (bBlue, Yellow)
         #define CMY16 ECMY16 (((*(pbSrc+1) & 0x1f) << 3), bBlue, Yellow)
         #define CMY24 ECMY24 (2, bBlue, Yellow)
         GENERATE_CMY_COLOR (bBlue, Yellow)
         break;
      }
      }
      n++;
   }

   return 0;

#undef BLACK24
#undef BLACK16
#undef BLACK8
#undef BLACK4
#undef CMY24
#undef CMY8
#undef CMY4
#undef ECMY4
#undef ECMY8
#undef CMY16
#undef ECMY24
#undef EBLACK4
#undef EBLACK8
#undef EBLACK16
#undef EBLACK24
#undef SNAP
#undef SETBIT
#undef INC
#undef GENERATE_CMY_COLOR
#undef TESTROWEND
}
/****************************************************************************/
/* PROCEDURE NAME : GplDitherMatrix                                         */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    : Is a modified copy of GplSeparateColors                 */
/*                  that performs a matrix dither                           */
/*                                                                          */
/* This function provides an intensity dither or pHalftone?x?.              */
/* It uses ls2b surface coordinates to select quadrant and the pel          */
/* pColor value to select the level. Uses a different table for each pColor.*/
/*                                                                          */
/*           Dithering pattern levels for lDitherType = 0x10                */
/*            ---------     ---------    ---------                          */
/* bRed       | | | | |     | | | | |    |*| | | |                          */
/*            ---------     ---------    ---------   o o o                  */
/*            | | | | |     | |*| | |    | | | | |                          */
/*            ---------     ---------    ---------                          */
/*            | | | | |     | | | | |    | | |*| |                          */
/*            ---------     ---------    ---------                          */
/*            | | | | |     | | | | |    | | | | |                          */
/*            ---------     ---------    ---------                          */
/*                                                                          */
/*                                                                          */
/* bGreen     ---------     ---------    ---------                          */
/*            | | | | |     | | | | |    | | | | |   o o o                  */
/*            ---------     ---------    ---------                          */
/*            | | | | |     | | |*| |    | |*| | |                          */
/*            ---------     ---------    ---------                          */
/*            | | | | |     | | | | |    | | | | |                          */
/*            ---------     ---------    ---------                          */
/*            | | | | |     | | | | |    | | |*| |                          */
/*            ---------     ---------    ---------                          */
/*                                                                          */
/* bBlue       ---------    ---------    ---------                          */
/*             | | | | |    | | | | |    | | | | |   o o o                  */
/*             ---------    ---------    ---------                          */
/*             | | | | |    | | | | |    | | |*| |                          */
/*             ---------    ---------    ---------                          */
/*             | | | | |    | |*| | |    | | | | |                          */
/*             ---------    ---------    ---------                          */
/*             | | | | |    | | | | |    |*| | | |                          */
/*             ---------    ---------    ---------                          */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG : June, 1999 Svetlana Holavina                   */
/*--------------------------------------------------------------------------*/
/* There are improvement of CMYK colors by RGB to CMYK conversion function  */
/* ToCMYK (&tToCMYK). If we put this function before SNAP() work then own   */
/* DITHER() conversion (it is included in SNAP macros) will use             */
/* already better colors for halftoning by paHalftone tables.               */
/****************************************************************************/
int GplDitherInstance::
GplDitherMatrix (PBITMAPINFO2 pbmi2,
                 PBYTE        pbSource,
                 LONG         lHalftoneSize,
                 PRGB2        pMatrix)
{
#define INC(ival)                     i++;                 \
                                      pbSrc += ival;       \
                                      if ((i & 7) == 0)    \
                                      {                    \
                                         pbCDest++;        \
                                         pbMDest++;        \
                                         pbYDest++;        \
                                         pbKDest++;        \
                                      }

#define TESTROWEND                    if (i >= (LONG)pbmi2->cx)                 \
                                      {                                         \
                                         i=0;                                   \
                                         pbCDest = pbCDestNext;                 \
                                         pbCDestNext = pbCDest + params.iDestRowSize;  \
                                         pbMDest = pbMDestNext;                 \
                                         pbMDestNext = pbMDest + params.iDestRowSize;  \
                                         pbYDest = pbYDestNext;                 \
                                         pbYDestNext = pbYDest + params.iDestRowSize;  \
                                         pbKDest = pbKDestNext;                 \
                                         pbKDestNext = pbKDest + params.iDestRowSize;  \
                                         pbSrc = pbSrcNext;                     \
                                         pbSrcNext = pbSrc + params.iSrcRowSize;       \
                                         y++;                                   \
                                         if ((ULONG)pbSrc >= (ULONG)pbMapEnd)   \
                                            break;                              \
                                      }

#define SETBIT(p,pdest)               *pdest |= (BYTE)(0x80 >> (i & 7));  \
                                      fEmpty##p##_d = false;

#define DITHER(bVal,c)                pTable->is[ lditherGrid             \
                                                  * ((bVal+1)/levelSize)  \
                                                + lHalftoneSize           \
                                                  * (y & ltableMask)      \
                                                + (i & ltableMask)].c

#define SNAP(bVal,c,e,pdest)          if (DITHER (bVal, c) == (BYTE)1)    \
                                      {                                   \
                                         SETBIT (e, pdest)                \
                                      }

#define LOW_BIT_CONV(imask, ishift)   tToCMYK.bMult = false;            \
                                      tToCMYK.bR    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bRed;   \
                                      tToCMYK.bG    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bGreen; \
                                      tToCMYK.bB    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bBlue;  \
                                      ToCMYK (&tToCMYK);

   LONG     i, j, y;
   ULONG    levelSize, lditherGrid, ltableMask;
   PBYTE    pbMapEnd, pbSrcNext, pbSrc;
   PBYTE    pbKDestNext, pbKDest, pbCDestNext, pbCDest, pbMDestNext, pbMDest, pbYDestNext, pbYDest;
   PBYTE    pbLMDest = 0, pbLMDestNext = 0, pbLCDest = 0, pbLCDestNext = 0;
   RGB2     maxColor = {0,0,0,0};
   PCOLR    pColor   = 0;
   PCOLR    pTable;

   TOCMYK   tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};
   PARAMS   params;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (  lHalftoneSize == 0
      || (lHalftoneSize != 4 && lHalftoneSize != 8)
      )
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   pTable      = (PCOLR)pMatrix;  // both are really PRGB2
   lditherGrid = lHalftoneSize * lHalftoneSize;
   ltableMask  = lHalftoneSize - 1;
   levelSize   = iNumColors_d <= 256 ? iNumColors_d/lditherGrid : 256/lditherGrid;
   if ((LONG)levelSize <= 0)
      return INVALID_PARAMETER;

   params.bMultiBitEnabled = false;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd     = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

   // For pels less than 16 bits,
   //    the pColor table must be provided at the end of BITMAPINFO2
   //    When the pColor table changes, scan all pColor values for
   //       a white index to use if destroy is set and for some snap types
   if (16 > params.iPelSize)
   {
      // 4 bit pel bitmaps may be reformatted to 8 bit pels by JPEG coversion to BMP, therefore,
      // the number of used pColors may be different than the plane pel bit count,
      // so create a bMask for the pColor index
      pColor = (PCOLR)&pbmi2->argbColor[0];

      if (fFirstTime_d)
      {
         for (j = 0; j < iNumColors_d; j++)
         {
            if (  pColor->is[j].bBlue >= maxColor.bBlue
               && pColor->is[j].bGreen >= maxColor.bGreen
               && pColor->is[j].bRed >= maxColor.bRed
               )
            {
               maxColor.bRed   = pColor->is[j].bRed;
               maxColor.bGreen = pColor->is[j].bGreen;
               maxColor.bBlue  = pColor->is[j].bBlue;
               iWhiteIndex_d = j;
            }
         }
      }
   }
   else
   {
      maxColor.bRed = maxColor.bGreen = maxColor.bBlue = MAX_RGB_VALUE;
   }

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLMDest = pbLMBuffer_d;
       pbLCDest = pbLCBuffer_d;
       pbLMDestNext = pbLMNextBuffer_d;
       pbLCDestNext = pbLCNextBuffer_d;
   }

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   y = iRowCount_d;

   i = 0;

   switch (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0x80, 7)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x40,6)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x20,5)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x10,4)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x80,3)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x40,2)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x20,1)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x01,0)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         INC(1)
         TESTROWEND
      }
      break;
   }

   case 2:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xC0,6)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x30,4)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
//@@TBD         SNAP (pColor->is[(LONG)(*pbSrc & 0x30)>>4].bBlue, bBlue, Yellow,  pbYDest)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0C,2)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
//@@TBD         SNAP (pColor->is[(LONG)(*pbSrc & 0xC)>>2].bBlue, bBlue, Yellow,  pbYDest)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x03,0)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         INC(1)
         TESTROWEND
      }
      break;
   }

   case 4:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xF0,4)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0F,0)
         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
         INC(1)
         TESTROWEND
      }
      break;
   }

#endif
   case 8:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
//            ulTempR =pColor->is[(ULONG)*pbSrc].bRed;
//            ulTempG =pColor->is[(ULONG)*pbSrc].bGreen;
//            ulTempB =pColor->is[(ULONG)*pbSrc].bBlue;
//            ulTempK =pColor->is[(ULONG)*pbSrc].fcOptions;

            // Convert from RGBK -> RGB
//            tToCMYK.bR = 255 - MIN (255, ((255 - ulTempR) * ulTempK / 255 + (255 - ulTempK)));
//            tToCMYK.bG = 255 - MIN (255, ((255 - ulTempG) * ulTempK / 255 + (255 - ulTempK)));
//            tToCMYK.bB = 255 - MIN (255, ((255 - ulTempB) * ulTempK / 255 + (255 - ulTempK)));

            tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
            tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
            tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], bRed,      Cyan,     pbCDest)
            SNAP (pbGGamma_d[tToCMYK.lM], bGreen,    Magenta,  pbMDest)
            SNAP (pbBGamma_d[tToCMYK.lY], bBlue,     Yellow,   pbYDest)
            SNAP (pbBGamma_d[tToCMYK.lK], fcOptions, Black,    pbKDest)
            INC(1)
            TESTROWEND
         }
      }
      else
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = pColor->is[(LONG)(*pbSrc)].bRed;
            tToCMYK.bB = pColor->is[(LONG)(*pbSrc)].bGreen;
            tToCMYK.bG = pColor->is[(LONG)(*pbSrc)].bBlue;

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,     pbCDest)
            SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta,  pbMDest)
            SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,   pbYDest)
            INC(1)
            TESTROWEND
         }
      }
      break;
   }

   case 16:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         tToCMYK.bR = (*pbSrc & 0xf8);
         tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
         tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

         if (!fDataInRGB_d)
            SWAP (tToCMYK.bR, tToCMYK.bB);

         ToCMYK(&tToCMYK);

         SNAP (pbRGamma_d[tToCMYK.lC], bRed,   Cyan,    pbCDest)
         SNAP (pbGGamma_d[tToCMYK.lM], bGreen, Magenta, pbMDest)
         SNAP (pbBGamma_d[tToCMYK.lY], bBlue,  Yellow,  pbYDest)

         INC(2)

         TESTROWEND
      }
      break;
   }

   case 24:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK(&tToCMYK);
            }
            else
            {
               // Turn Black Off
               tToCMYK.lK = tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], bRed,      Cyan,    pbCDest)
            SNAP (pbGGamma_d[tToCMYK.lM], bGreen,    Magenta, pbMDest)
            SNAP (pbBGamma_d[tToCMYK.lY], bBlue,     Yellow,  pbYDest)
            SNAP (pbKGamma_d[tToCMYK.lK], fcOptions, Black,   pbKDest)

            INC(3)

            TESTROWEND
         }
      }
      else
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
             if(isNotWhite(pbSrc, &tToCMYK))
             {
                 ToCMYK(&tToCMYK);
             }
             else
             {
                // Turn Black Off
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
             }

             SNAP (pbRGamma_d[tToCMYK.lC], bRed,      Cyan,    pbCDest)
             SNAP (pbGGamma_d[tToCMYK.lM], bGreen,    Magenta, pbMDest)
             SNAP (pbBGamma_d[tToCMYK.lY], bBlue,     Yellow,  pbYDest)

            INC(3)
            TESTROWEND
         }
      }
      else
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
             if(isNotWhite(pbSrc, &tToCMYK))
             {
                 ToCMYK6 (&tToCMYK, pbLightTable_d, pbDarkTable_d);
             }
             else
             {
                // Turn Black Off
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lLC = tToCMYK.lLM = tToCMYK.lK = 0;
             }

             SNAP (pbRGamma_d[tToCMYK.lC], bRed,      Cyan,    pbCDest)
             SNAP (pbGGamma_d[tToCMYK.lM], bGreen,    Magenta, pbMDest)
             SNAP (pbBGamma_d[tToCMYK.lY], bBlue,     Yellow,  pbYDest)
             SNAP (pbRGamma_d[tToCMYK.lLC],bRed,      LCyan,   pbLCDest)
             SNAP (pbGGamma_d[tToCMYK.lLM],bGreen,    LMagenta,pbLMDest)
             SNAP (pbKGamma_d[tToCMYK.lK], fcOptions, Black,   pbKDest)

            INC(3)
            TESTROWEND
         }
      }

      break;
   }

   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = y;

   return 0;

#undef SNAP
#undef SETBIT
#undef INC
#undef TESTROWEND
#undef LOW_BIT_CONV
}

/****************************************************************************/
/* MACRO NAME     : ConvertRGBToHSVDthr       for 1 to 8 bit pels           */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:     iRGBIndex, pos, ulc                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
/*
   ConvertRGBToHSVDthr       for 1 to 8 bit pels
                             Utilizies magic-squares tables
   Inputs:  iRGBIndex                 index into argbColor table
            (PPOINTL)pos
            ulRGBIndex

   Outputs: ulc                        primary color

*/
#define ConvertRGBToHSVDthr(pArray,ahsvTable,iRGBIndex,pos,ulc) \
   ulc = CLR_WHITE;                                             \
   phsv = (PHSV)&ahsvTable->is[ iRGBIndex ];                    \
   if (phsv->fIsNotWhite)                                       \
   {                                                            \
      ulRetClr = pArray->is[(pos.x & 0x0f)][(pos.y & 0x0f)];    \
      if (ulRetClr <= phsv->ulBlack)                            \
         ulc = CLR_BLACK;                                       \
      else if( phsv->fValueSat )                                \
         if (ulRetClr > phsv->ulBlackWhite)                     \
            if (phsv->fIsGreaterThan0x80)                       \
               if (ulRetClr <= phsv->ulGT80)                    \
                  ulc = phsv->ulcolor1;                         \
               else                                             \
                  ulc = phsv->ulcolor;                          \
            else                                                \
               if (ulRetClr <= phsv->ulLE80)                    \
                  ulc = phsv->ulcolor;                          \
               else                                             \
                  ulc = phsv->ulcolor1;                         \
   }

/****************************************************************************/
/* MACRO NAME     : ConvertRGBToCMYKDthr  for 16 and 24 bit pels            */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:    ulRed, ulGreen, ulBlue, pos, delta, ulc                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
/*
   ConvertRGBToCMYKDthr       for 16 and 24 bit pels
                              Performs magic-squares dither
   Inputs:  delta.lSaturation     -   (MAX_RGB_VALUE * S(-100..100))/100   (DELTACOLOR)
            delta.lValue              (256 * V(-100..100))/100
            delta.lHue                ((H/60)<<8) + (256 * (H(-180..180) - H*60))/60
            (PPOINTL)pos
            ulRed, ulGreen ulBlue

   Outputs: ulc                        primary color
  Black = MAX_RGB_VALUE - MAX3( R, G, B)
  Saturation = 256*(max - min)/max
  White = Black + (MAX_RGB_VALUE - Saturation) * max / 256
  i.e. White = Black + min

*/
#define ConvertRGBToCMYKDthr(pArray,ulRed,ulGreen,ulBlue,pos,delta,ulc) \
   ulRetClr = pArray->is[pos.x & 0xf][pos.y & 0xf];   \
   ulValue = MAX3 (ulRed, ulGreen, ulBlue);           \
   ulNewValue = ulValue - delta.lValue;               \
   if ((LONG)ulNewValue > MAX_RGB_VALUE)              \
      ulNewValue = MAX_RGB_VALUE;                     \
   else if((LONG)ulNewValue < 0)                      \
      ulNewValue = 0;                                 \
   ulBlack = PRECISION1 - 1 - ulNewValue;             \
   ulc = CLR_WHITE;                                   \
   if (ulRetClr <= ulBlack)                           \
   {                                                  \
      if (  ulRed != MAX_RGB_VALUE                    \
         || ulGreen != MAX_RGB_VALUE                  \
         || ulBlue != MAX_RGB_VALUE                   \
         )                                            \
         ulc = CLR_BLACK;                             \
   }                                                  \
   else if (ulValue)                                  \
   {                                                  \
      ulMin = MIN3 (ulRed, ulGreen, ulBlue);          \
      ulDelta = ulValue - ulMin;                      \
      ulSaturation = (ulDelta << PRECISION) / ulValue + delta.lSaturation; \
      if ((LONG)ulSaturation < 0)                     \
         ulSaturation = 0;                            \
      else if ((LONG)ulSaturation > PRECISION1)       \
         ulSaturation = PRECISION1;                   \
      ulWhite = ulBlack + (((PRECISION1 - ulSaturation) * ulNewValue) >> PRECISION); \
      if (ulRetClr > ulWhite)                         \
      {                                               \
         ulHue = 0;                                   \
         if (ulSaturation != 0 || ulRed != MAX_RGB_VALUE || ulGreen != MAX_RGB_VALUE || ulBlue != MAX_RGB_VALUE) \
         {                                            \
            if (ulDelta)                              \
            {                                         \
               if (ulValue == ulRed)                  \
                  if (ulMin == ulGreen)               \
                     ulHue = (5<<PRECISION) + ((ulValue - ulBlue) << PRECISION)  / ulDelta; \
                  else                                \
                     ulHue = (1<<PRECISION) - ((ulValue - ulGreen) << PRECISION) / ulDelta; \
               else if (ulValue == ulGreen)           \
                  if (ulMin == ulBlue)                \
                     ulHue = (1<<PRECISION) + ((ulValue - ulRed) << PRECISION)   / ulDelta; \
                  else                                \
                     ulHue = (3<<PRECISION) - ((ulValue - ulBlue) << PRECISION)  / ulDelta; \
               else if (ulValue == ulBlue)            \
                  if (ulMin == ulRed)                 \
                     ulHue = (3<<PRECISION) + ((ulValue - ulGreen) << PRECISION) / ulDelta; \
                  else                                \
                     ulHue = (5<<PRECISION) - ((ulValue - ulRed) << PRECISION)   / ulDelta; \
               ulDelta = ulHue >> PRECISION;          \
               ulHue &= 0xff;                         \
               if (ulHue > 0x80)                      \
                  if (ulRetClr <= (ulWhite + (((MAX_RGB_VALUE - ulWhite) * ulHue) >> PRECISION))) \
                     ulc = ulPrimColors[ulDelta + 1]; \
                  else                                \
                     ulc = ulPrimColors[ulDelta];     \
               else                                   \
                  if (ulRetClr <= (ulWhite + (((MAX_RGB_VALUE - ulWhite) * (0x100 - ulHue)) >> PRECISION))) \
                     ulc = ulPrimColors[ulDelta];     \
                  else                                \
                     ulc = ulPrimColors[ulDelta + 1]; \
            }                                         \
         }                                            \
      }                                               \
   }

/****************************************************************************/
/* PROCEDURE NAME : GplColorSquares                                         */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:     PDITHEREQUEST  pReq                                      */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplColorSquares (PBITMAPINFO2 pbmi2,
                 PBYTE        pbSource,
                 BYTE         table[16][16])
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

#define SETBIT(pdest,p) *pdest |= (BYTE)(0x80 >> (pos.x & 0x07));  \
                        fEmpty##p##_d = false;

#define SETBYTE(p,s)   *p |= (BYTE)*s;

#define INC            pos.x++;                                 \
                       if((pos.x & 0x07) == 0)                  \
                       {                                        \
                          pbKDest++;                            \
                          pbCDest++;                            \
                          pbMDest++;                            \
                          pbYDest++;                            \
                       }                                        \
                       if (pos.x >= (LONG)pbmi2->cx)            \
                       {                                        \
                          pbKDest     = pbKDestNext;            \
                          pbKDestNext = pbKDest + params.iDestRowSize; \
                          pbCDest     = pbCDestNext;            \
                          pbCDestNext = pbCDest + params.iDestRowSize; \
                          pbMDest     = pbMDestNext;            \
                          pbMDestNext = pbMDest + params.iDestRowSize; \
                          pbYDest     = pbYDestNext;            \
                          pbYDestNext = pbYDest + params.iDestRowSize; \
                          pbSrc       = pbSrcNext;              \
                          pbSrcNext = pbSrc + params.iSrcRowSize;      \
                          pos.y++;                              \
                          pos.x=0;                              \
                       }

#define INCBYTE        pos.x += 8;                                \
                       pbKDest++;                                 \
                       pbCDest++;                                 \
                       pbMDest++;                                 \
                       pbYDest++;                                 \
                       if (pos.x >= (LONG)pbmi2->cx)              \
                       {                                          \
                          pbKDest     = pbKDestNext;              \
                          pbKDestNext = pbKDest + params.iDestRowSize;   \
                          pbCDest     = pbCDestNext;              \
                          pbCDestNext = pbKDest + params.iDestRowSize;   \
                          pbMDest     = pbMDestNext;              \
                          pbMDestNext = pbCDest + params.iDestRowSize;   \
                          pbYDest     = pbYDestNext;              \
                          pbYDestNext = pbMDest + params.iDestRowSize;   \
                          pbSrc       = pbSrcNext;                \
                          pbSrcNext   = pbSrc + params.iSrcRowSize;      \
                          pos.y++;                                \
                          pos.x=0;                                \
                       }

#define SETPEL         ConvertRGBToCMYKDthr (pArray, ulRed, ulGreen, ulBlue, pos, delta_d, ulc) \
                       switch (ulc)                           \
                       {                                      \
                       case CLR_BLACK:                        \
                          if (fBlackFlag)                     \
                          {                                   \
                             SETBIT (pbKDest, Black)          \
                          }                                   \
                          else                                \
                          {                                   \
                             SETBIT (pbCDest, Cyan)           \
                             SETBIT (pbMDest, Magenta)        \
                             SETBIT (pbYDest, Yellow)         \
                          }                                   \
                          break;                              \
                       case CLR_RED:                          \
                          SETBIT (pbMDest, Magenta)           \
                          SETBIT (pbYDest, Yellow)            \
                          break;                              \
                       case CLR_GREEN:                        \
                          SETBIT (pbCDest, Cyan)              \
                          SETBIT (pbYDest, Yellow)            \
                          break;                              \
                       case CLR_BLUE:                         \
                          SETBIT (pbCDest, Cyan)              \
                          SETBIT (pbMDest, Magenta)           \
                          break;                              \
                       case CLR_CYAN:                         \
                          SETBIT (pbCDest, Cyan)              \
                          break;                              \
                       case CLR_PINK:                         \
                          SETBIT (pbMDest, Magenta)           \
                          break;                              \
                       case CLR_YELLOW:                       \
                          SETBIT (pbYDest, Yellow)            \
                       }

#define SETPEL8(idx)   ConvertRGBToHSVDthr (pArray, phsvTable, idx, pos, ulc) \
                       switch (ulc)                           \
                       {                                      \
                       case CLR_BLACK:                        \
                          if (fBlackFlag)                     \
                          {                                   \
                             SETBIT (pbKDest, Black)          \
                          }                                   \
                          else                                \
                          {                                   \
                             SETBIT (pbCDest, Cyan)           \
                             SETBIT (pbMDest, Magenta)        \
                             SETBIT (pbYDest, Yellow)         \
                          }                                   \
                          break;                              \
                       case CLR_RED:                          \
                          SETBIT (pbMDest, Magenta)           \
                          SETBIT (pbYDest, Yellow)            \
                          break;                              \
                       case CLR_GREEN:                        \
                          SETBIT (pbCDest, Cyan)              \
                          SETBIT (pbYDest, Yellow)            \
                          break;                              \
                       case CLR_BLUE:                         \
                          SETBIT (pbCDest, Cyan)              \
                          SETBIT (pbMDest, Magenta)           \
                          break;                              \
                       case CLR_CYAN:                         \
                          SETBIT (pbCDest, Cyan)              \
                          break;                              \
                       case CLR_PINK:                         \
                          SETBIT (pbMDest, Magenta)           \
                          break;                              \
                       case CLR_YELLOW:                       \
                          SETBIT (pbYDest, Yellow)            \
                       }

   LONG      i, indey;
   PBYTE     pbMapEnd, pbSrcNext, pbSrc;
   PBYTE     pbKDestNext, pbKDest, pbCDestNext, pbCDest, pbMDestNext, pbMDest, pbYDestNext, pbYDest;
   POINTL    pos;
   BOOL      fBlackFlag = false;
// ConvertRGBtoCMYDthr declarations
   P2ABYTE   pArray;
   int       ulc;
   ULONG     ulRed = 0, ulGreen = 0, ulBlue = 0;
   ULONG     ulValue;
   ULONG     ulMin;
   ULONG     ulHue;
   ULONG     ulSaturation;
   ULONG     ulBlack, ulWhite;
   ULONG     ulRetClr;
   ULONG     ulDelta;
   ULONG     ulNewValue;
   PHSV      phsv;
   PHSVARRAY phsvTable = 0;
   TOCMYK    tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};
   PARAMS    params;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (fFirstTime_d)
   {
      if (256 >= iNumColors_d)
      {
         phsvTable_d = (PHSVARRAY)calloc (1, 256 * sizeof (HSV));
      }
   }

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   params.bMultiBitEnabled = true;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd     = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

   pArray = (P2ABYTE)table;

   indey = (LONG)pbmi2->cy * params.iDestRowSize;

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   pos.x = 0;
   pos.y = iRowCount_d;

   switch (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:                       // Foreground and background are solid pColor - no dithering
   {
      BYTE bXor;

      bXor = 0xff;
      for (i = 0; i < 2; i++)
      {
         pos.x = 0;
         pos.y = iRowCount_d;

         if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
         {
            pbKDest     = pbDest_d;
            pbKDestNext = pbKDest + params.iDestRowSize;
            pbCDest     = pbKDest + indey;
         }
         else
            pbCDest = pbDest_d;

         pbCDestNext = pbCDest + params.iDestRowSize;

         pbMDest     = pbCDest + indey;
         pbMDestNext = pbMDest + params.iDestRowSize;

         pbYDest     = pbMDest + indey;
         pbYDestNext = pbYDest + params.iDestRowSize;

         pbSrc     = pbSource;
         pbSrcNext = pbSource + params.iSrcRowSize;

         ConvertRGBToHSVDthr (pArray, phsvTable_d, i, pos, ulc)

         switch (ulc)
         {
         case CLR_BLACK:
         {
            if (fBlackFlag)
            {
               fEmptyBlack_d = false;
            }
            else
            {
               fEmptyCyan_d    = false;
               fEmptyMagenta_d = false;
               fEmptyYellow_d  = false;
            }

            if (fBlackFlag)
            {
               while ((ULONG)pbSrc < (ULONG)pbMapEnd)
               {
                  *pbKDest |= (*pbSrc ^ bXor);
                  INCBYTE
               }
            }
            else
            {
               while ((ULONG)pbSrc < (ULONG)pbMapEnd)
               {
                  *pbCDest |= (*pbSrc ^ bXor);
                  *pbMDest |= (*pbSrc ^ bXor);
                  *pbYDest |= (*pbSrc ^ bXor);
                  INCBYTE
               }
            }
            break;
         }
         case CLR_RED:
         {
            fEmptyMagenta_d = false;
            fEmptyYellow_d = false;
            while ((ULONG)pbSrc < (ULONG)pbMapEnd)
            {
               *pbMDest |= (*pbSrc ^ bXor);
               *pbYDest |= (*pbSrc ^ bXor);
               INCBYTE
            }
            break;
         }
         case CLR_GREEN:
         {
            fEmptyCyan_d = false;
            fEmptyYellow_d = false;
            while ((ULONG)pbSrc < (ULONG)pbMapEnd)
            {
               *pbCDest |= (*pbSrc ^ bXor);
               *pbYDest |= (*pbSrc ^ bXor);
               INCBYTE
            }
            break;
         }
         case CLR_BLUE:
         {
            fEmptyCyan_d = false;
            fEmptyMagenta_d = false;
            while ((ULONG)pbSrc < (ULONG)pbMapEnd)
            {
               *pbCDest |= (*pbSrc ^ bXor);
               *pbMDest |= (*pbSrc ^ bXor);
               INCBYTE
            }
            break;
         }
         case CLR_CYAN:
         {
            fEmptyCyan_d = false;
            while ((ULONG)pbSrc < (ULONG)pbMapEnd)
            {
               *pbCDest |= (*pbSrc ^ bXor);
               INCBYTE
            }
            break;
         }
         case CLR_PINK:
         {
            fEmptyMagenta_d = false;
            while ((ULONG)pbSrc < (ULONG)pbMapEnd)
            {
               *pbMDest |= (*pbSrc ^ bXor);
               INCBYTE
            }
            break;
         }
         case CLR_YELLOW:
         {
            fEmptyYellow_d = false;
            while ((ULONG)pbSrc < (ULONG)pbMapEnd)
            {
               *pbYDest |= (*pbSrc ^ bXor);
               INCBYTE
            }
         }
         }  // CLR_WHITE=none

         bXor = 0;
      }
      break;
   }

   case 2:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         switch (pos.x & 3)
         {
         case 0:
            i = (*pbSrc & 0xC0) >> 6;
            break;
         case 1:
            i = (*pbSrc & 0x30) >> 4;
            break;
         case 2:
            i = (*pbSrc & 0xC) >> 2;
            break;
         case 3:
            i = (*pbSrc & 3);
            pbSrc++;
         }
         SETPEL8 (i)
         INC
      }
      break;
   }

   case 4:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         if (pos.x & 1)
         {
            i = *pbSrc & 0x0f;
            pbSrc++;
         }
         else
         {
            i = (*pbSrc & 0xf0) >> 4;
         }
         SETPEL8 (i)
         INC
      }
      break;
   }
#endif

   case 8:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         i = *pbSrc;
         SETPEL8 (i)

         pbSrc++;
         INC
      }
      break;
   }

   case 16:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
        tToCMYK.bR = *pbSrc & 0xf8;
        tToCMYK.bG = ((*pbSrc & 7) << 5) & ((*(pbSrc+1) & 0xE0) >> 3);
        tToCMYK.bB = (*(pbSrc+1) & 0x1f) << 3;

        if (!fDataInRGB_d)
           SWAP (tToCMYK.bR, tToCMYK.bB);

        ToCMYK(&tToCMYK);

        SETPEL
        pbSrc+=2;
        INC
      }
      break;
   }

   case 24:
   {
      i = params.iPelSize / 8;
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {

         if(isNotWhite(pbSrc, &tToCMYK))
         {
             ToCMYK(&tToCMYK);
         }
         else
         {
            // Turn Black Off
            tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
         }

         if (pbRGamma_d) ulRed   = pbRGamma_d[tToCMYK.lC];
         if (pbGGamma_d) ulGreen = pbGGamma_d[tToCMYK.lM];
         if (pbBGamma_d) ulBlue  = pbBGamma_d[tToCMYK.lY];

         SETPEL
         pbSrc += i;
         INC
      }
      break;
   }

   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = pos.y;

   return 0;

#undef SETBIT
#undef SETPEL
#undef SETPEL8
#undef INC
}

/****************************************************************************/
/* PROCEDURE NAME : GplStuckiDiffusion    Stucki 32 bit                     */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/*    Stucki error diffusion :                                              */
/*          k = pColorTableValue at current [x][y] point +                  */
/*                                  LastPointErr1 + vtemp[x][y]             */
/*          if( k < midpt.c )                                               */
/*             print CMYK [x][y] point for this RGB c, pColor.              */
/*          error = midpt.c - k                                             */
/*          vtemp[x+1][y]   = 8/42 * error                                  */
/*          vtemp[x][y+1]   = 8/42 * error                                  */
/*                                                                          */
/*          vtemp[x+1][y+1] = 4/42 * error   :                              */
/*          vtemp[x-1][y+1] = 4/42 * error                                  */
/*          vtemp[x+2][y]   = 4/42 * error                                  */
/*          vtemp[x][y+2]   = 4/42 * error                                  */
/*                                                                          */
/*          vtemp[x-2][y+1] = 2/42 * error                                  */
/*          vtemp[x+2][y+1] = 2/42 * error                                  */
/*          vtemp[x+1][y+2] = 2/42 * error                                  */
/*          vtemp[x-1][y+2] = 2/42 * error                                  */
/*                                                                          */
/*          vtemp[x+2][y+2] = 1/42 * error                                  */
/*          vtemp[x-2][y+2] = 1/42 * error                                  */
/*                                                                          */
/*    use x++ on even row passes                                            */
/*                                                                          */
/*    Option: use x-- and reverse indexes on steps 4, 5, and 7 on odd       */
/*    row passes iLastPointErr is the error to be added to this point       */
/*    from the last point during the current pass iCurRow is the            */
/*    errors for this row computed from the last row.                       */
/*                                                                          */
/*    A diffusion error buffer must have been allocated and initialized to  */
/*    zero on first entry.  The error buffer will contain intermediate      */
/*    values between bands and must be deallocated at page end.             */
/*    The error buffer must be 6 times (row pel length (cx) plus 4),        */
/*    in longs; 3 rows per CMY pColor.                                      */
/*                                                                          */
/*    NOTE: CMYK_ALL or CMY_ALL must be selected.                           */
/*                                                                          */
/*    if( R != MAX_RGB_VALUE || G != MAX_RGB_VALUE || B != MAX_RGB_VALUE )  */
/*       Black = MAX_RGB_VALUE - max(R, G, B)                               */
/*       Black Plane Value = max(R, G, B)                                   */
/*                                                                          */
/*    Saturation = 256 * (max-min) / max                                    */
/*    if either R, G, or B are zero then                                    */
/*       100% saturation exists i.e. MAX_RGB_VALUE * (max - 0) / max =      */
/*                                                   MAX_RGB_VALUE = 100%   */
/*                                                                          */
/*    White = Black + (256 - Saturation) * max / 256                        */
/*    i.e. White = MAX_RGB_VALUE + min - max                                */
/*                                                                          */
/* PARAMETERS:      PDITHEREQUEST  pReq                                     */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplStuckiDiffusion (PBITMAPINFO2 pbmi2,
                    PBYTE        pbSource)
{

#define INC(iSrcInc)     pbSrc += iSrcInc;           \
                         if (1 == iDestBitsPerPel_d) \
                         {                           \
                           if ((i & 7) == 7)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         else                     \
                         {                        \
                           if ((i & 3) == 3)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         i++;


#define TESTROWEND       if (i >= (LONG)pbmi2->cx)                                  \
                         {                                                          \
                            i = 0;                                                  \
                            pbCDest     = pbCDestNext;                              \
                            pbCDestNext = pbCDest + iNumDestRowBytes_d;             \
                            pbMDest     = pbMDestNext;                              \
                            pbMDestNext = pbMDest + iNumDestRowBytes_d;             \
                            pbYDest     = pbYDestNext;                              \
                            pbYDestNext = pbYDest + iNumDestRowBytes_d;             \
                            pbKDest     = pbKDestNext;                              \
                            pbKDestNext = pbKDest + iNumDestRowBytes_d;             \
                            if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK) \
                            {                                                       \
                               pbLCDest     = pbLCDestNext;                         \
                               pbLCDestNext = pbLCDest + iNumDestRowBytes_d;        \
                               pbLMDest     = pbLMDestNext;                         \
                               pbLMDestNext = pbLMDest + iNumDestRowBytes_d;        \
                            }                                                       \
                            pbSrc       = pbSrcNext;               \
                            pbSrcNext   = pbSrc + params.iSrcRowSize;     \
                            y++;                                   \
                            if (y  == (long)pbmi2->cy)   \
                               break;                              \
                            SETUP_ERROR_ROWS;                      \
                         }

#define SETUP_ERROR_ROWS n = (y % 3);                                   \
                         if (n == 0)                                    \
                         {                                              \
                            piRedErrRow2 = piRow2;                      \
                            piRedErrRow1 = piRow1;                      \
                            piRedCurRow  = piRow0;                      \
                         }                                              \
                         else if (n == 1)                               \
                         {                                              \
                            piRedErrRow2 = piRow0;                      \
                            piRedErrRow1 = piRow2;                      \
                            piRedCurRow  = piRow1;                      \
                         }                                              \
                         else                                           \
                         {                                              \
                            piRedErrRow2 = piRow1;                      \
                            piRedErrRow1 = piRow0;                      \
                            piRedCurRow  = piRow2;                      \
                         }                                              \
                         piGreenErrRow2 = piRedErrRow2   + iErrRowSize; \
                         piGreenErrRow1 = piRedErrRow1   + iErrRowSize; \
                         piGreenCurRow  = piRedCurRow    + iErrRowSize; \
                         piBlueErrRow2  = piGreenErrRow2 + iErrRowSize; \
                         piBlueErrRow1  = piGreenErrRow1 + iErrRowSize; \
                         piBlueCurRow   = piGreenCurRow  + iErrRowSize; \
                         piKErrRow2     = piBlueErrRow2  + iErrRowSize; \
                         piKErrRow1     = piBlueErrRow1  + iErrRowSize; \
                         piKCurRow      = piBlueCurRow   + iErrRowSize; \
                         if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)      \
                         {                                                            \
                            piLCErrRow2    = piKErrRow2     + iErrRowSize;            \
                            piLCErrRow1    = piKErrRow1     + iErrRowSize;            \
                            piLCCurRow     = piKCurRow      + iErrRowSize;            \
                            piLMErrRow2    = piLCErrRow2    + iErrRowSize;            \
                            piLMErrRow1    = piLCErrRow1    + iErrRowSize;            \
                            piLMCurRow     = piLCCurRow     + iErrRowSize;            \
                         }                                                            \
                         {                                                            \
                            float *pf = piRedErrRow2 - (CMYK_FILTER_OVERFLOW / 2);    \
                            int    ii;                                                \
                            for (ii = 0; ii < iNumPens_d * iErrRowSize; ii++)           \
                            {                                                         \
                               *pf++ = 0.0;                                           \
                            }                                                         \
                         }

#define SETBIT(p,pdest)  *pdest |= (BYTE)(0x80 >> (i & 7));    \
                         fEmpty##p##_d = false;

#define SETINTENSITYBIT(p, pdest, iIntensity)  \
     *pdest |= (BYTE)((BYTE)abMask[iIntensity] >> ((i & 3) *2)); \
                         fEmpty##p##_d = false;

// piCurRow1 index is 4 bytes ahead of the current relative position.
// iLastPointErr is initially THIS point brought along from the last point's
// SNAP diffusion.

// The CurRow1 begins with the third entry since Stucki requires two pels in
// back and two lines below and two pels in fron. ilast1 and ilast2 are used
// to store the errors for the next two pel.  ErrRow11 is always the next row.
// Three buffers are rotated in order to add the CurErr(+1) to the curent row.
// iErr -> iLastPointErr1&2 -> piErrRow1&2, piCurRow -> iErr -> E+ERR_EXP

#define SNAP(bVal,c,p,pdest,pierr1,pierr2,picur,ilast1,ilast2)                                   \
                           iErr = (float)bVal + *picur + ilast1;                                 \
                           if (1 == iDestBitsPerPel_d)                                           \
                           {                                                                     \
                              if (iErr <= midPt_d.c)                                             \
                              {                                                                  \
                                 SETBIT (p, pdest)                                                     \
                              }                                                                  \
                              else                                                               \
                              {                                                                  \
                                 iErr -= 255.0;                                                    \
                              }                                                                  \
                           }                                                                     \
                           else                                                                  \
                           {                                                                     \
                              if (iErr >= 127.0 )                                                 \
                              {                                                                  \
                                 iErr -= 255.0;                                                    \
                                 iErr = iErr*.95;                                                \
                              }                                                                  \
                              else                                                               \
                              {                                                                  \
                                 if (iErr <= 0.0)                                                \
                                 {                                                               \
                                    iIntensity = HIGH;                                              \
                                 }                                                               \
                                 else if (iErr <= 50.0)                                          \
                                 {                                                               \
                                    iIntensity = MEDIUM;                                              \
                                 }                                                               \
                                 else if (iErr < 128.0)                                          \
                                 {                                                               \
                                    iIntensity = LOW;                                              \
                                 }                                                               \
                                 SETINTENSITYBIT(p, pdest, iIntensity)                           \
                                 iErr = iErr*.95;                                                \
                              }                                                                  \
                           }                                                                     \
                           if(iErr > 255.0)                                                      \
                             iErr = 255.0;                                                       \
                           else                                                                  \
                             if(iErr < -255.0)                                                   \
                               iErr = -255.0;                                                    \
                           if(iErr > -4.0 && iErr < 4.0)                                         \
                             iErr = 0.0;                                                         \
                                                                                                 \
                           iFrac = iErr/42.0;                                                    \
                           i2 = iFrac * 2.0;                                                     \
                           i4 = iFrac * 4.0;                                                     \
                           i8 = iFrac * 8.0;                                                     \
                                                                                                 \
                           ilast1 = ilast2 + i8;                                                 \
                           ilast2 = i4;                                                          \
                                                                                                 \
                           *(pierr1 - 2) += i2;                                                  \
                           *(pierr1 - 1) += i4;                                                  \
                           *(pierr1    ) += i8;                                                  \
                           *(pierr1 + 1) += i4;                                                  \
                           *(pierr1 + 2) += i2;                                                  \
                                                                                                 \
                           *(pierr2 - 2) += iFrac;                                               \
                           *(pierr2 - 1) += i2;                                                  \
                           *(pierr2    ) += i4;                                                  \
                           *(pierr2 + 1) += i2;                                                  \
                           *(pierr2 + 2) += iFrac;                                               \
                                                                                                 \
                           pierr2++;                                                             \
                           pierr1++;                                                             \
                           picur++;

#define SNAP6DARK(bVal,c,p,pdest,pierr1,pierr2,picur,ilast1,ilast2)                     \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (iErr >= 127.0 )                                    \
                           {                                                      \
                              iErr -= 255.0;                                      \
                           }                                                      \
                           else                                                   \
                           {                                                      \
                              if (iErr <= 0.0)                                    \
                              {                                                   \
                                 iIntensity = HIGH;                                  \
                              }                                                   \
                              else if (iErr <= 75.0)                              \
                              {                                                   \
                                 iIntensity = MEDIUM;                                  \
                              }                                                   \
                              else if (iErr < 128.0)                              \
                              {                                                   \
                                 iIntensity = LOW;                                  \
                              }                                                   \
                              SETINTENSITYBIT(p, pdest, iIntensity)               \
                           }                                                      \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define SNAP6YELLOW(bVal,c,p,pdest,pierr1,pierr2,picur,ilast1,ilast2)                     \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (iErr >= 127.0 )                                    \
                           {                                                      \
                              iErr -= 255.0;                                      \
                           }                                                      \
                           else                                                   \
                           {                                                      \
                              if (iErr <= -20.0)                                    \
                              {                                                   \
                                 iIntensity = HIGH;                                  \
                              }                                                   \
                              else if (iErr <= 50.0)                              \
                              {                                                   \
                                 iIntensity = MEDIUM;                                  \
                              }                                                   \
                              else if (iErr < 128.0)                              \
                              {                                                   \
                                 iIntensity = LOW;                                  \
                              }                                                   \
                              SETINTENSITYBIT(p, pdest, iIntensity)               \
                           }                                                      \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define SNAP6LIGHT(bVal,c, name, pdest, pierr1,pierr2,picur,ilast1,ilast2)                \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (iErr >= 127.0 )                                       \
                           {                                                      \
                              iErr -= 255;                                        \
                           }                                                      \
                           else                                                   \
                           {                                                      \
                              if (iErr <= -10.0)                                 \
                              {                                                   \
                                 iIntensity = HIGH;                                  \
                              }                                                   \
                              else if (iErr <= 65.0)                             \
                              {                                                   \
                                 iIntensity = MEDIUM;                                  \
                              }                                                   \
                              else if (iErr <= 128.0)                             \
                              {                                                   \
                                 iIntensity = LOW;                                  \
                              }                                                   \
                              SETINTENSITYBIT(name, pdest, iIntensity)            \
                           }                                                      \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define LOW_BIT_CONV(imask, ishift)   tToCMYK.bMult = false;            \
                                      tToCMYK.bR    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bRed;   \
                                      tToCMYK.bG    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bGreen; \
                                      tToCMYK.bB    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bBlue;  \
                                      ToCMYK (&tToCMYK);

   INT      i, n, y;
   float    i2, i4, i8;
   float    iRedLast1   = 0,  iRedLast2   = 0;
   float    iGreenLast1 = 0,  iGreenLast2 = 0;
   float    iBlueLast1  = 0,  iBlueLast2  = 0;
   float    iKLast1     = 0,  iKLast2     = 0;
   float    iLCLast1    = 0,  iLCLast2     = 0;  //@@SIX-COLOR
   float    iLMLast1    = 0,  iLMLast2     = 0;  //@@SIX-COLOR
   float    iErr,
            iFrac;
   INT      iErrRowSize,
            iIntensity    = 0;
   PCOLR    pColor;                  // Used for Color Table lookup
   float   *piRow0,         *piRow1,          *piRow2;
   float   *piRedCurRow,    *piRedErrRow1,    *piRedErrRow2;
   float   *piGreenCurRow,  *piGreenErrRow1,  *piGreenErrRow2;
   float   *piBlueCurRow,   *piBlueErrRow1,   *piBlueErrRow2;
   float   *piKCurRow,      *piKErrRow1,      *piKErrRow2;
   float   *piLCCurRow = 0, *piLCErrRow1 = 0, *piLCErrRow2 = 0;  //@@SIX-COLOR
   float   *piLMCurRow = 0, *piLMErrRow1 = 0, *piLMErrRow2 = 0;  //@@SIX-COLOR
   PBYTE    pbMapEnd,
            pbSrcNext,        pbSrc;
   PBYTE    pbKDestNext,      pbKDest,
            pbCDestNext,      pbCDest,
            pbMDestNext,      pbMDest,
            pbLCDestNext = 0, pbLCDest = 0,       //@@SIX-COLOR
            pbLMDestNext = 0, pbLMDest = 0,       //@@SIX-COLOR
            pbYDestNext,      pbYDest;
   BYTE     abMask[]= {0x00,0x40,0x80,0xC0};      // mask for multi-bit per pel
   TOCMYK   tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};
   PARAMS   params;


#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   midPt_d.lBlack = 127;

   params.bMultiBitEnabled = true;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);


#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": iSrcRowSize = " << params.iSrcRowSize << ", iDestRowSize = " << params.iDestRowSize << std::endl;
#endif

   pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLCDest = pbLCBuffer_d;
       pbLMDest = pbLMBuffer_d;
       pbLCDestNext = pbLCNextBuffer_d;
       pbLMDestNext = pbLMNextBuffer_d;
   }

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   piRow0 = (float *)pfErrRow_d + CMYK_FILTER_OVERFLOW / 2;

   // ErrRowSize in pels = (pels Per Row  +  boundry Overflow)
   iErrRowSize = ((ULONG)pbmi2->cx + CMYK_FILTER_OVERFLOW);
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": iErrRowSize = " << iErrRowSize << std::endl;
#endif

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       piRow1 = piRow0 + iNumPens_d * iErrRowSize;
       piRow2 = piRow1 + iNumPens_d * iErrRowSize;
   }
   else
   {
       // first buffer + 4colorsPerBufferGroup * rowSizeInPels (Dword increment)
       piRow1 = piRow0 + iNumPens_d * iErrRowSize;
       piRow2 = piRow1 + iNumPens_d * iErrRowSize;
   }

   y = iRowCount_d;
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": y = " << y << std::endl;
#endif

   SETUP_ERROR_ROWS;

   i = 0;
   switch (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:
   {
      //-----------------------------
      // STUCKI: 1-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0x80, 7)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x40, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x20, 5)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x10, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x08, 3)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x04, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x02, 1)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x01, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
   case 2:
   {
      //-----------------------------
      // STUCKI: 2-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xC0, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x30, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0C, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x03, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
   case 4:
   {
      //-----------------------------
      // STUCKI: 4-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xF0, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0F, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
#endif
   case 8:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 8-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
//            ulTempR = pColor->is[(ULONG)*pbSrc].bRed;
//            ulTempG = pColor->is[(ULONG)*pbSrc].bGreen;
//            ulTempB = pColor->is[(ULONG)*pbSrc].bBlue;
//            ulTempK = pColor->is[(ULONG)*pbSrc].fcOptions;

            // Convert from RGBK -> RGB
//            tToCMYK.bR = 255 - MIN (255, ((255 - ulTempR) * ulTempK / 255 + (255 - ulTempK)));
//            tToCMYK.bG = 255 - MIN (255, ((255 - ulTempG) * ulTempK / 255 + (255 - ulTempK)));
//            tToCMYK.bB = 255 - MIN (255, ((255 - ulTempB) * ulTempK / 255 + (255 - ulTempK)));
            tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
            tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
            tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP (pbBGamma_d[tToCMYK.lK], lBlack, Black,    pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(1)
            TESTROWEND
         }
      }
      else
      {
         //-----------------------------
         // STUCKI: 8-bit:CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
            tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
            tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

            INC(1)
            TESTROWEND
         }
      }
      break;
   }

   case 16:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 16-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = (*pbSrc & 0xf8);
            tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
            tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

            if (!fDataInRGB_d)
               SWAP (tToCMYK.bR,tToCMYK.bB);

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP (pbBGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(2)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
         //-----------------------------
         // STUCKI: 16-bit:CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = (*pbSrc & 0xf8);
            tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
            tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

            if (!fDataInRGB_d)
               SWAP (tToCMYK.bR, tToCMYK.bB);

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

            INC(2)
            TESTROWEND
         }
      }
      break;
   }
   case 24:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
      {
         //-----------------------------
         // STUCKI: 24-bit:CcMmYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {

            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK6 (&tToCMYK, pbLightTable_d, pbDarkTable_d);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lLC = tToCMYK.lM = tToCMYK.lLM = tToCMYK.lY = tToCMYK.lK = 0;
            }

            SNAP6DARK   (pbRGamma_d[tToCMYK.lC],  lRed,   Cyan,    pbCDest,  piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP6DARK   (pbRGamma_d[tToCMYK.lM],  lGreen, Magenta, pbMDest,  piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP6LIGHT  (pbRGamma_d[tToCMYK.lLC], lRed,   LCyan,   pbLCDest, piLCErrRow1,    piLCErrRow2,    piLCCurRow,    iLCLast1,    iLCLast2)
            SNAP6LIGHT  (pbGGamma_d[tToCMYK.lLM], lGreen, LMagenta,pbLMDest, piLMErrRow1,    piLMErrRow2,    piLMCurRow,    iLMLast1,    iLMLast2)
            SNAP6YELLOW (pbBGamma_d[tToCMYK.lY],  lBlue,  Yellow,  pbYDest,  piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP6DARK   (pbKGamma_d[tToCMYK.lK],  lBlack, Black,   pbKDest,  piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(3)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 24-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK (&tToCMYK);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lK = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(3)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
         //-----------------------------
         // STUCKI: 24-bit: CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {

            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK (&tToCMYK);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

            INC(3)
            TESTROWEND
         }
      }
      break;
   }

   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = y;

   if (  (  (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
         || (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
         )
      && fRemoveBlackColor_d
      )
   {
      GplCMYRemoval (pbmi2);
   }

   return 0;

#undef SNAP
#undef SETBIT
#undef INC
#undef TESTROWEND
#undef SETUP_ERROR_ROWS

#undef SNAP6LIGHT
#undef SNAP6DARK
#undef SNAP6YELLOW
#undef SETINTENSITYBIT
#undef LOW_BIT_CONV
}

/****************************************************************************/
/* PROCEDURE NAME : GplEnhancedStuckiDiffusion    Stucki 32 bit             */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/*    Stucki error diffusion :                                              */
/*          k = pColorTableValue at current [x][y] point +                  */
/*                                  LastPointErr1 + vtemp[x][y]             */
/*          if( k < midpt.c )                                               */
/*             print CMYK [x][y] point for this RGB c, pColor.              */
/*          error = midpt.c - k                                             */
/*          vtemp[x+1][y]   = 8/42 * error                                  */
/*          vtemp[x][y+1]   = 8/42 * error                                  */
/*                                                                          */
/*          vtemp[x+1][y+1] = 4/42 * error   :                              */
/*          vtemp[x-1][y+1] = 4/42 * error                                  */
/*          vtemp[x+2][y]   = 4/42 * error                                  */
/*          vtemp[x][y+2]   = 4/42 * error                                  */
/*                                                                          */
/*          vtemp[x-2][y+1] = 2/42 * error                                  */
/*          vtemp[x+2][y+1] = 2/42 * error                                  */
/*          vtemp[x+1][y+2] = 2/42 * error                                  */
/*          vtemp[x-1][y+2] = 2/42 * error                                  */
/*                                                                          */
/*          vtemp[x+2][y+2] = 1/42 * error                                  */
/*          vtemp[x-2][y+2] = 1/42 * error                                  */
/*                                                                          */
/*    use x++ on even row passes                                            */
/*                                                                          */
/*    Option: use x-- and reverse indexes on steps 4, 5, and 7 on odd       */
/*    row passes iLastPointErr is the error to be added to this point       */
/*    from the last point during the current pass iCurRow is the            */
/*    errors for this row computed from the last row.                       */
/*                                                                          */
/*    A diffusion error buffer must have been allocated and initialized to  */
/*    zero on first entry.  The error buffer will contain intermediate      */
/*    values between bands and must be deallocated at page end.             */
/*    The error buffer must be 6 times (row pel length (cx) plus 4),        */
/*    in longs; 3 rows per CMY pColor.                                      */
/*                                                                          */
/*    NOTE: CMYK_ALL or CMY_ALL must be selected.                           */
/*                                                                          */
/*    if( R != MAX_RGB_VALUE || G != MAX_RGB_VALUE || B != MAX_RGB_VALUE )  */
/*       Black = MAX_RGB_VALUE - max(R, G, B)                               */
/*       Black Plane Value = max(R, G, B)                                   */
/*                                                                          */
/*    Saturation = 256 * (max-min) / max                                    */
/*    if either R, G, or B are zero then                                    */
/*       100% saturation exists i.e. MAX_RGB_VALUE * (max - 0) / max =      */
/*                                                   MAX_RGB_VALUE = 100%   */
/*                                                                          */
/*    White = Black + (256 - Saturation) * max / 256                        */
/*    i.e. White = MAX_RGB_VALUE + min - max                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplEnhancedStuckiDiffusion (PBITMAPINFO2 pbmi2,
                    PBYTE        pbSource)
{

#define INC(iSrcInc)     pbSrc += iSrcInc;           \
                         if (1 == iDestBitsPerPel_d) \
                         {                           \
                           if ((i & 7) == 7)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         else                     \
                         {                        \
                           if ((i & 3) == 3)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         i++;


#define TESTROWEND       if (i >= (LONG)pbmi2->cx)                                      \
                         {                                                              \
                            i = 0;                                                      \
                            pbCDest     = pbCDestNext;                                  \
                            pbCDestNext = pbCDest + iNumDestRowBytes_d;                 \
                            pbMDest     = pbMDestNext;                                  \
                            pbMDestNext = pbMDest + iNumDestRowBytes_d;                 \
                            pbYDest     = pbYDestNext;                                  \
                            pbYDestNext = pbYDest + iNumDestRowBytes_d;                 \
                            pbKDest     = pbKDestNext;                                  \
                            pbKDestNext = pbKDest + iNumDestRowBytes_d;                 \
                            if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)     \
                            {                                                           \
                               pbLCDest     = pbLCDestNext;                             \
                               pbLCDestNext = pbLCDest + iNumDestRowBytes_d;            \
                               pbLMDest     = pbLMDestNext;                             \
                               pbLMDestNext = pbLMDest + iNumDestRowBytes_d;            \
                            }                                                           \
                            pbSrc       = pbSrcNext;                                    \
                            pbSrcNext   = pbSrc + params.iSrcRowSize;                   \
                            y++;                                                        \
                            if (y  == (long)pbmi2->cy)                                  \
                               break;                                                   \
                            SETUP_ERROR_ROWS;                                           \
                         }

#define SETUP_ERROR_ROWS n = (y % 3);                                                   \
                         if (n == 0)                                                    \
                         {                                                              \
                            piRedErrRow2 = piRow2;                                      \
                            piRedErrRow1 = piRow1;                                      \
                            piRedCurRow  = piRow0;                                      \
                         }                                                              \
                         else if (n == 1)                                               \
                         {                                                              \
                            piRedErrRow2 = piRow0;                                      \
                            piRedErrRow1 = piRow2;                                      \
                            piRedCurRow  = piRow1;                                      \
                         }                                                              \
                         else                                                           \
                         {                                                              \
                            piRedErrRow2 = piRow1;                                      \
                            piRedErrRow1 = piRow0;                                      \
                            piRedCurRow  = piRow2;                                      \
                         }                                                              \
                         piGreenErrRow2 = piRedErrRow2   + iErrRowSize;                 \
                         piGreenErrRow1 = piRedErrRow1   + iErrRowSize;                 \
                         piGreenCurRow  = piRedCurRow    + iErrRowSize;                 \
                         piBlueErrRow2  = piGreenErrRow2 + iErrRowSize;                 \
                         piBlueErrRow1  = piGreenErrRow1 + iErrRowSize;                 \
                         piBlueCurRow   = piGreenCurRow  + iErrRowSize;                 \
                         piKErrRow2     = piBlueErrRow2  + iErrRowSize;                 \
                         piKErrRow1     = piBlueErrRow1  + iErrRowSize;                 \
                         piKCurRow      = piBlueCurRow   + iErrRowSize;                 \
                         if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)        \
                         {                                                              \
                             piLCErrRow2    = piKErrRow2     + iErrRowSize;             \
                             piLCErrRow1    = piKErrRow1     + iErrRowSize;             \
                             piLCCurRow     = piKCurRow      + iErrRowSize;             \
                             piLMErrRow2    = piLCErrRow2    + iErrRowSize;             \
                             piLMErrRow1    = piLCErrRow1    + iErrRowSize;             \
                             piLMCurRow     = piLCCurRow     + iErrRowSize;             \
                         }                                                              \
                         {                                                              \
                            float *pf = piRedErrRow2 - (CMYK_FILTER_OVERFLOW / 2);      \
                            int    ii;                                                  \
                            int    iNoise = 0;                                          \
                            for (ii = 0; ii <  iErrRowSize-(CMYK_FILTER_OVERFLOW / 2); ii++) \
                            {                                                                \
                                 *pf++ = afVal[iNoise++ % 10];                               \
                            }                                                                \
                         }                                                                   \
                         {                                                                   \
                            float *pf = piGreenErrRow2 - (CMYK_FILTER_OVERFLOW / 2);         \
                            int    ii;                                                       \
                            int    iNoise = 3;                                               \
                            for (ii = 0; ii <  iErrRowSize-(CMYK_FILTER_OVERFLOW / 2); ii++) \
                            {                                                                \
                                 *pf++ = afVal[iNoise++ % 10];                               \
                            }                                                                \
                         }                                                                   \
                         {                                                                   \
                            float *pf = piBlueErrRow2 - (CMYK_FILTER_OVERFLOW / 2);          \
                            int    ii;                                                       \
                            int    iNoise = 6;                                               \
                            for (ii = 0; ii <  iErrRowSize-(CMYK_FILTER_OVERFLOW / 2); ii++) \
                            {                                                                \
                                 *pf++ = afVal[iNoise++ % 10];                               \
                            }                                                                \
                         }                                                                   \
                         {                                                                   \
                            float *pf = piKErrRow2 - (CMYK_FILTER_OVERFLOW / 2);             \
                            int    ii;                                                       \
                            int    iNoise = 0;                                               \
                            for (ii = 0; ii <  iErrRowSize-(CMYK_FILTER_OVERFLOW / 2); ii++) \
                            {                                                                \
                                 *pf++ = afVal[iNoise++ % 10];                               \
                            }                                                                \
                         }                                                                   \
                         if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)             \
                         {                                                                   \
                             {                                                               \
                                float *pf = piLCErrRow2 - (CMYK_FILTER_OVERFLOW / 2);        \
                                int    ii;                                                   \
                                int    iNoise = 5;                                           \
                                for (ii = 0; ii <  iErrRowSize-(CMYK_FILTER_OVERFLOW / 2); ii++) \
                                {                                                                \
                                     *pf++ = afVal[iNoise++ % 10];                               \
                                }                                                                \
                             }                                                                   \
                             {                                                                   \
                                float *pf = piLMErrRow2 - (CMYK_FILTER_OVERFLOW / 2);            \
                                int    ii;                                                       \
                                int    iNoise = 7;                                               \
                                for (ii = 0; ii <  iErrRowSize-(CMYK_FILTER_OVERFLOW / 2); ii++) \
                                {                                                                \
                                    *pf++ = afVal[iNoise++ % 10];                                \
                                }                                                                \
                             }                                                                   \
                         }

#define SETBIT(p,pdest)  *pdest |= (BYTE)(0x80 >> (i & 7));    \
                         fEmpty##p##_d = false;

#define SETINTENSITYBIT(p, pdest, iIntensity)  \
     *pdest |= (BYTE)((BYTE)abMask[iIntensity] >> ((i & 3) *2)); \
                         fEmpty##p##_d = false;

// piCurRow1 index is 4 bytes ahead of the current relative position.
// iLastPointErr is initially THIS point brought along from the last point's
// SNAP diffusion.

// The CurRow1 begins with the third entry since Stucki requires two pels in
// back and two lines below and two pels in fron. ilast1 and ilast2 are used
// to store the errors for the next two pel.  ErrRow11 is always the next row.
// Three buffers are rotated in order to add the CurErr(+1) to the curent row.
// iErr -> iLastPointErr1&2 -> piErrRow1&2, piCurRow -> iErr -> E+ERR_EXP

#define SNAP(bVal,c,p,pdest,pierr1,pierr2,picur,ilast1,ilast2)                       \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (1 == iDestBitsPerPel_d)                               \
                           {                                                         \
                              if (iErr <= 127.0)                                     \
                              {                                                      \
                                 SETBIT (p, pdest)                                   \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                              }                                                      \
                           }                                                         \
                           else                                                      \
                           {                                                         \
                              if (iErr >= 127.0 )                                    \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                                 iErr = iErr*.95;                                    \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 if (iErr <= 0.0)                                    \
                                 {                                                   \
                                    iIntensity = HIGH;                               \
                                 }                                                   \
                                 else if (iErr <= 50.0)                              \
                                 {                                                   \
                                    iIntensity = MEDIUM;                             \
                                 }                                                   \
                                 else if (iErr < 128.0)                              \
                                 {                                                   \
                                    iIntensity = LOW;                                \
                                 }                                                   \
                                 SETINTENSITYBIT(p, pdest, iIntensity)               \
                                 iErr = iErr*.95;                                    \
                              }                                                      \
                           }                                                         \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define SNAP6DARK(bVal,c,name,pdest,pierr1,pierr2,picur,ilast1,ilast2)               \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (1 == iDestBitsPerPel_d)                               \
                           {                                                         \
                              if (iErr <= 127.0)                                     \
                              {                                                      \
                                 SETBIT (name, pdest)                                \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                              }                                                      \
                           }                                                         \
                           else                                                      \
                           {                                                         \
                              if (iErr >= 127.0 )                                    \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 if (iErr <= 0.0)                                    \
                                 {                                                   \
                                    iIntensity = HIGH;                               \
                                 }                                                   \
                                 else if (iErr <= 75.0)                              \
                                 {                                                   \
                                    iIntensity = MEDIUM;                             \
                                 }                                                   \
                                 else if (iErr < 128.0)                              \
                                 {                                                   \
                                    iIntensity = LOW;                                \
                                 }                                                   \
                                 SETINTENSITYBIT(name, pdest, iIntensity)            \
                              }                                                      \
                           }                                                         \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define SNAP6YELLOW(bVal,c,name,pdest,pierr1,pierr2,picur,ilast1,ilast2)             \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (1 == iDestBitsPerPel_d)                               \
                           {                                                         \
                              if (iErr <= 127.0)                                     \
                              {                                                      \
                                 SETBIT (name, pdest)                                \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                              }                                                      \
                           }                                                         \
                           else                                                      \
                           {                                                         \
                              if (iErr >= 127.0 )                                    \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 if (iErr <= -20.0)                                  \
                                 {                                                   \
                                    iIntensity = HIGH;                               \
                                 }                                                   \
                                 else if (iErr <= 50.0)                              \
                                 {                                                   \
                                    iIntensity = MEDIUM;                             \
                                 }                                                   \
                                 else if (iErr < 128.0)                              \
                                 {                                                   \
                                    iIntensity = LOW;                                \
                                 }                                                   \
                                 SETINTENSITYBIT(name, pdest, iIntensity)            \
                              }                                                      \
                           }                                                         \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define SNAP6LIGHT(bVal,c, name, pdest, pierr1,pierr2,picur,ilast1,ilast2)           \
                           iErr = (float)bVal + *picur + ilast1;                     \
                           if (1 == iDestBitsPerPel_d)                               \
                           {                                                         \
                              if (iErr <= 127.0)                                     \
                              {                                                      \
                                 SETBIT (name, pdest)                                \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 iErr -= 255.0;                                      \
                              }                                                      \
                           }                                                         \
                           else                                                      \
                           {                                                         \
                              if (iErr >= 127.0 )                                    \
                              {                                                      \
                                 iErr -= 255;                                        \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 if (iErr <= -10.0)                                  \
                                 {                                                   \
                                    iIntensity = HIGH;                               \
                                 }                                                   \
                                 else if (iErr <= 65.0)                              \
                                 {                                                   \
                                    iIntensity = MEDIUM;                             \
                                 }                                                   \
                                 else if (iErr <= 128.0)                             \
                                 {                                                   \
                                    iIntensity = LOW;                                \
                                 }                                                   \
                                 SETINTENSITYBIT(name, pdest, iIntensity)            \
                              }                                                      \
                           }                                                         \
                           if(iErr > 255.0)                                          \
                             iErr = 255.0;                                           \
                           else                                                      \
                             if(iErr < -255.0)                                       \
                               iErr = -255.0;                                        \
                           if(iErr > -4.0 && iErr < 4.0)                             \
                             iErr = 0.0;                                             \
                                                                                     \
                           iFrac = iErr/42.0;                                        \
                           i2 = iFrac * 2.0;                                         \
                           i4 = iFrac * 4.0;                                         \
                           i8 = iFrac * 8.0;                                         \
                                                                                     \
                           ilast1 = ilast2 + i8;                                     \
                           ilast2 = i4;                                              \
                                                                                     \
                           *(pierr1 - 2) += i2;                                      \
                           *(pierr1 - 1) += i4;                                      \
                           *(pierr1    ) += i8;                                      \
                           *(pierr1 + 1) += i4;                                      \
                           *(pierr1 + 2) += i2;                                      \
                                                                                     \
                           *(pierr2 - 2) += iFrac;                                   \
                           *(pierr2 - 1) += i2;                                      \
                           *(pierr2    ) += i4;                                      \
                           *(pierr2 + 1) += i2;                                      \
                           *(pierr2 + 2) += iFrac;                                   \
                                                                                     \
                           pierr2++;                                                 \
                           pierr1++;                                                 \
                           picur++;

#define LOW_BIT_CONV(imask, ishift)   tToCMYK.bMult = false;            \
                                      tToCMYK.bR    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bRed;   \
                                      tToCMYK.bG    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bGreen; \
                                      tToCMYK.bB    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bBlue;  \
                                      ToCMYK (&tToCMYK);

   INT      i, n, y;
   float    i2, i4, i8;
   float    iRedLast1   = 0, iRedLast2   = 0;
   float    iGreenLast1 = 0, iGreenLast2 = 0;
   float    iBlueLast1  = 0, iBlueLast2  = 0;
   float    iKLast1     = 0, iKLast2     = 0;
   float    iLCLast1     = 0, iLCLast2     = 0;  //@@SIX-COLOR
   float    iLMLast1     = 0, iLMLast2     = 0;  //@@SIX-COLOR
   float    iErr,
            iFrac;
   INT      iErrRowSize,
            iIntensity    = 0;
   PCOLR    pColor;
   float   *piRow0,         *piRow1,          *piRow2;
   float   *piRedCurRow,    *piRedErrRow1,    *piRedErrRow2;
   float   *piGreenCurRow,  *piGreenErrRow1,  *piGreenErrRow2;
   float   *piBlueCurRow,   *piBlueErrRow1,   *piBlueErrRow2;
   float   *piKCurRow,      *piKErrRow1,      *piKErrRow2;
   float   *piLCCurRow = 0, *piLCErrRow1 = 0, *piLCErrRow2 = 0;  //@@SIX-COLOR
   float   *piLMCurRow = 0, *piLMErrRow1 = 0, *piLMErrRow2 = 0;  //@@SIX-COLOR
   PBYTE    pbMapEnd,
            pbSrcNext,        pbSrc;
   PBYTE    pbKDestNext,      pbKDest,
            pbCDestNext,      pbCDest,
            pbMDestNext,      pbMDest,
            pbLCDestNext = 0, pbLCDest = 0,       //@@SIX-COLOR
            pbLMDestNext = 0, pbLMDest = 0,       //@@SIX-COLOR
            pbYDestNext,      pbYDest;
   BYTE     abMask[]= {0x00,0x40,0x80,0xC0};
//   BYTE     abDotMask[] = {0xCC, 0x33};
   TOCMYK   tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};
   float    afVal[] = {5.0, -1.0, 4.0, -2.0, 3.0, -3.0, 2.0, -4.0, 1.0, -5.0};
   PARAMS   params;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   midPt_d.lBlack = 127;

   params.bMultiBitEnabled = true;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": iSrcRowSize = " << params.iSrcRowSize << ", iDestRowSize = " << params.iDestRowSize << std::endl;
#endif

   pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLCDest = pbLCBuffer_d;
       pbLMDest = pbLMBuffer_d;
       pbLCDestNext = pbLCNextBuffer_d;
       pbLMDestNext = pbLMNextBuffer_d;
   }

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   piRow0 = (float *)pfErrRow_d + CMYK_FILTER_OVERFLOW / 2;

   // ErrRowSize in pels = (pels Per Row  +  boundry Overflow)
   iErrRowSize = ((ULONG)pbmi2->cx + CMYK_FILTER_OVERFLOW);
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": iErrRowSize = " << iErrRowSize << std::endl;
#endif

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       // first buffer + 6colorsPerBufferGroup * rowSizeInPels (Dword increment)
       piRow1 = piRow0 + 6 * iErrRowSize;
       piRow2 = piRow1 + 6 * iErrRowSize;
   }
   else
   {
       // first buffer + 4colorsPerBufferGroup * rowSizeInPels (Dword increment)
       piRow1 = piRow0 + 4 * iErrRowSize;
       piRow2 = piRow1 + 4 * iErrRowSize;
   }

   y = iRowCount_d;
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": y = " << y << std::endl;
#endif

   SETUP_ERROR_ROWS;

   i = 0;
   switch (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:
   {
      //-----------------------------
      // STUCKI: 1-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0x80, 7)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x40, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x20, 5)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x10, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x08, 3)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x04, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x02, 1)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x01, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
   case 2:
   {
      //-----------------------------
      // STUCKI: 2-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xC0, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x30, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0C, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x03, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
   case 4:
   {
      //-----------------------------
      // STUCKI: 4-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xF0, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0F, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
#endif
   case 8:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 8-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
//            ulTempR = pColor->is[(ULONG)*pbSrc].bRed;
//            ulTempG = pColor->is[(ULONG)*pbSrc].bGreen;
//            ulTempB = pColor->is[(ULONG)*pbSrc].bBlue;
//            ulTempK = pColor->is[(ULONG)*pbSrc].fcOptions;

            // Convert from RGBK -> RGB
//            tToCMYK.bR = 255 - MIN (255, ((255 - ulTempR) * ulTempK / 255 + (255 - ulTempK)));
//            tToCMYK.bG = 255 - MIN (255, ((255 - ulTempG) * ulTempK / 255 + (255 - ulTempK)));
//            tToCMYK.bB = 255 - MIN (255, ((255 - ulTempB) * ulTempK / 255 + (255 - ulTempK)));

            tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
            tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
            tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP (pbBGamma_d[tToCMYK.lK], lBlack, Black,    pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(1)
            TESTROWEND
         }
      }
      else
      {
         //-----------------------------
         // STUCKI: 8-bit:CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
            tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
            tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

            INC(1)
            TESTROWEND
         }
      }
      break;
   }

   case 16:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 16-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = (*pbSrc & 0xf8);
            tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
            tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

            if (!fDataInRGB_d)
               SWAP (tToCMYK.bR,tToCMYK.bB);

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP (pbBGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(2)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
         //-----------------------------
         // STUCKI: 16-bit:CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = (*pbSrc & 0xf8);
            tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
            tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

            if (!fDataInRGB_d)
               SWAP (tToCMYK.bR, tToCMYK.bG);

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

            INC(2)
            TESTROWEND
         }
      }
      break;
   }

   case 24:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
      {
         //-----------------------------
         // STUCKI: 24-bit:CcMmYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {

            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK6 (&tToCMYK, pbLightTable_d, pbDarkTable_d);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lLC = tToCMYK.lM = tToCMYK.lLM = tToCMYK.lY = tToCMYK.lK = 0;
            }

            SNAP6DARK   (pbRGamma_d[tToCMYK.lC],  lRed,   Cyan,    pbCDest,  piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP6DARK   (pbRGamma_d[tToCMYK.lM],  lGreen, Magenta, pbMDest,  piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP6LIGHT  (pbRGamma_d[tToCMYK.lLC], lRed,   LCyan,   pbLCDest, piLCErrRow1,    piLCErrRow2,    piLCCurRow,    iLCLast1,    iLCLast2)
            SNAP6LIGHT  (pbGGamma_d[tToCMYK.lLM], lGreen, LMagenta,pbLMDest, piLMErrRow1,    piLMErrRow2,    piLMCurRow,    iLMLast1,    iLMLast2)
            SNAP6YELLOW (pbBGamma_d[tToCMYK.lY],  lBlue,  Yellow,  pbYDest,  piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP6DARK   (pbKGamma_d[tToCMYK.lK],  lBlack, Black,   pbKDest,  piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(3)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 24-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK (&tToCMYK);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lK = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
            SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

            INC(3)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
         //-----------------------------
         // STUCKI: 24-bit: CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {

            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK (&tToCMYK);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

            INC(3)
            TESTROWEND
         }
      }
      break;
   }
   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = y;

   if (  (  (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
         || (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
         )
      && fRemoveBlackColor_d
      )
   {
      GplCMYRemoval (pbmi2);
   }

   return 0;

#undef SNAP
#undef SETBIT
#undef INC
#undef TESTROWEND
#undef SETUP_ERROR_ROWS

#undef SNAP6YELLOW
#undef SNAP6LIGHT
#undef SNAP6DARK
#undef SETINTENSITYBIT
#undef LOW_BIT_CONV
}

/****************************************************************************/
/* PROCEDURE NAME : GplStuckiBiffusion    Stucki BiDirectional              */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PDITHEREQUEST  pReq                                     */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplStuckiBiffusion (PBITMAPINFO2 pbmi2,
                    PBYTE        pbSource)
{
/*
      Stucki error diffusion bidirectional :
            k = pColorTableValue at current [x][y] point + LastPointErr1 + vtemp[x][y]
            if( k < midpt.c )
               print CMYK [x][y] point for this RGB c, pColor.
            error = midpt.c - k
            vtemp[x+1][y]  = 8/42 * error
            vtemp[x-1][y-1] = 4/42 * error
            vtemp[x][y-1]   = 8/42 * error
            vtemp[x+1][y-1] = 4/42 * error   : ( remaining error )
            vtemp[x+2][y]  = 4/42 * error
            vtemp[x-2][y-1] = 2/42 * error
            vtemp[x][y-2]   = 4/42 * error
            vtemp[x+2][y-2] = 1/42 * error
            vtemp[x+2][y-1] = 2/42 * error
            vtemp[x+1][y-2] = 2/42 * error
            vtemp[x-1][y-2] = 2/42 * error
            vtemp[x-2][y-2] = 1/42 * error

      use x++ on even row passes
      Option: use x-- and reverse indexes on steps 4, 5, and 7 on odd row passes
      iLastPointErr is the error to be added to this point from the last point
         during the current pass
      iCurRow is the errors for this row computed from the last row.

      A diffusion error buffer must have been allocated and initialized to
      zero on first entry.  The error buffer will contain intermediate values
      between bands and must be deallocated at page end. The error buffer
      must be 6 times (row pel length (cx) plus 4), in longs; 3 rows per CMY pColor.
*/

#define INC(iVal)       if ((y & 1) == 0)         \
                        {                         \
                           i++;                   \
                           pbSrc += iVal;         \
                           if ((i & 7) == 0)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                           }                      \
                        }                         \
                        else                      \
                        {                         \
                           pbSrc -= iVal ;        \
                           if ((i & 7) == 0)      \
                           {                      \
                              pbCDest--;          \
                              pbMDest--;          \
                              pbYDest--;          \
                           }                      \
                           i--;                   \
                        }

#define TESTROWEND      if (  i >= (LONG)pbmi2->cx                   \
                           || i <= 0                                 \
                           )                                         \
                        {                                            \
                           y++;                                      \
                           n = (y%3);                                \
                           if ((y & 1) == 0)                         \
                           {                                         \
                              i = 0;                                 \
                              pbCDest     = pbCDestNext;             \
                              pbCDestNext = pbCDest +  params.iDestRowSize; \
                              pbMDest     = pbMDestNext;             \
                              pbMDestNext = pbMDest +  params.iDestRowSize; \
                              pbYDest     = pbYDestNext;             \
                              pbYDestNext = pbYDest +  params.iDestRowSize; \
                              pbSrc       = pbSrcNext;               \
                              pbSrcNext   = pbSrc + params.iSrcRowSize;     \
                              if ((ULONG)pbSrc >= (ULONG)pbMapEnd)   \
                                 break;                              \
                              if (n  == 0)                           \
                              {                                      \
                                 piRedErrRow2 = piRow2;              \
                                 piRedErrRow1 = piRow1;              \
                                 piRedCurRow  = piRow0;              \
                              }                                      \
                              else if (n == 1)                       \
                              {                                      \
                                 piRedErrRow2 = piRow0;              \
                                 piRedErrRow1 = piRow2;              \
                                 piRedCurRow  = piRow1;              \
                              }                                      \
                              else                                   \
                              {                                      \
                                 piRedErrRow2 = piRow1;              \
                                 piRedErrRow1 = piRow0;              \
                                 piRedCurRow  = piRow2;              \
                              }                                      \
                              piGreenErrRow2 = piRedErrRow2   + iErrRowSize; \
                              piGreenErrRow1 = piRedErrRow1   + iErrRowSize; \
                              piGreenCurRow  = piRedCurRow    + iErrRowSize; \
                              piBlueErrRow2  = piGreenErrRow2 + iErrRowSize; \
                              piBlueErrRow1  = piGreenErrRow1 + iErrRowSize; \
                              piBlueCurRow   = piGreenCurRow  + iErrRowSize; \
                              memset ((PBYTE)(piRedErrRow2 - (CMYK_FILTER_OVERFLOW / 2)), 0, (3 * 4 * iErrRowSize)); \
                           }                                         \
                           else                                      \
                           {                                         \
                              i = (LONG)pbmi2->cx - 1;               \
                              pbCDestNext =  pbCDestNext + params.iDestRowSize; \
                              pbCDest = pbCDestNext - 1;             \
                              pbMDestNext =  pbMDestNext + params.iDestRowSize; \
                              pbMDest = pbMDestNext - 1;             \
                              pbYDestNext =  pbYDestNext + params.iDestRowSize; \
                              pbYDest = pbYDestNext - 1;             \
                              pbSrc = pbSrcNext + iSrcDataEnd;       \
                              pbSrcNext = pbSrcNext + params.iSrcRowSize;   \
                              if ((ULONG)pbSrc >= (ULONG)pbMapEnd )  \
                                 break;                              \
                              if (n == 0)                            \
                              {                                      \
                                 piRedErrRow2 = piRow2 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                                 piRedErrRow1 = piRow1 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                                 piRedCurRow  = piRow0 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                              }                                      \
                              else if (n == 1)                       \
                              {                                      \
                                 piRedErrRow2 = piRow0 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                                 piRedErrRow1 = piRow2 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                                 piRedCurRow  = piRow1 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                              }                                      \
                              else                                   \
                              {                                      \
                                 piRedErrRow2 = piRow1 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                                 piRedErrRow1 = piRow0 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                                 piRedCurRow  = piRow2 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1; \
                              }                                      \
                              piGreenErrRow2  = piRedErrRow2   + iErrRowSize;   \
                              piGreenErrRow1  = piRedErrRow1   + iErrRowSize;   \
                              piGreenCurRow   = piRedCurRow    + iErrRowSize;   \
                              piBlueErrRow2   = piGreenErrRow2 + iErrRowSize;   \
                              piBlueErrRow1   = piGreenErrRow1 + iErrRowSize;   \
                              piBlueCurRow    = piGreenCurRow  + iErrRowSize;   \
                              memset ((PBYTE)(piRedErrRow2 - iErrRowSize + (CMYK_FILTER_OVERFLOW / 2) + 1), 0, (3 * 4 * iErrRowSize)); \
                           }                                         \
                        }

#define SETBIT(p,pdest) *pdest |= (BYTE)(0x80 >> (i & 7)); \
                        fEmpty##p##_d = false;

#define SETINTENSITYBIT(p, pdest, iIntensity)  \
     *pdest |= (BYTE)((BYTE)abMask[iIntensity] >> ((i & 3) *2)); \
                         fEmpty##p##_d = false;

// piCurRow1 index is 4 bytes ahead of the current relative position. iLastPointErr is initially THIS point
// brought along from the last point's SNAP diffusion.

// The CurRow1 begins with the third entry since Stuchi requires two pels in back and
// two lines below and two pels in fron. ilast1 and ilast2 are  used
// to store the errors for the next two pel.  ErrRow11 is always the next row.  Three buffers are rotated
// in order to add the CurErr(+1) to the curent row.
// iErr -> iLastPointErr1&2 -> piErrRow1&2, piCurRow -> iErr -> E+ERR_EXP

#define SNAP(bVal,c,p,pdest,pierr1,pierr2,picur,ilast1,ilast2)     \
                        iErr = (LONG)((ULONG)bVal << ERR_EXP) + *picur + ilast1; \
                        if (1 == iDestBitsPerPel_d)                \
                        {                                          \
                            if (iErr <= midPt_d.c)                 \
                            {                                      \
                               SETBIT (p, pdest)                   \
                            }                                      \
                            else                                   \
                            {                                      \
                               iErr -= (0xff << ERR_EXP);          \
                            }                                      \
                        }                                          \
                        else                                       \
                        {                                          \
                            if (iErr <= -20.0)                     \
                            {                                      \
                               iIntensity = HIGH;                  \
                            }                                      \
                            else if (iErr <= 50.0)                 \
                            {                                      \
                               iIntensity = MEDIUM;                \
                            }                                      \
                            else if (iErr < 128.0)                 \
                            {                                      \
                               iIntensity = LOW;                   \
                            }                                      \
                            SETINTENSITYBIT(p, pdest, iIntensity)  \
                        }                                          \
                        iFrac = iErr/42;                           \
                        i2 = (iFrac << 1);                         \
                        i4 = (iFrac << 2);                         \
                        i8 = (iFrac << 3);                         \
                        i1 = ((i8 << 1) + (i4 << 2) + (i2 << 2));  \
                        i1a = ((iErr - i1) >> 1);                  \
                        *(pierr2 - 2) += i1a;                      \
                        *(pierr2 + 2) += (iErr - i1 - i1a);        \
                        ilast1 = ilast2 + i8;                      \
                        *pierr1       += i8;                       \
                        *(pierr1 - 1) += i4;                       \
                        *(pierr1 + 1) += i4;                       \
                        *pierr2       += i4;                       \
                        ilast2 = i4;                               \
                        *(pierr1 - 2) += i2;                       \
                        *(pierr1 + 2) += i2;                       \
                        *(pierr2 - 1) += i2;                       \
                        *(pierr2 + 1) += i2;                       \
                        if ((y & 1) == 0)                          \
                        {                                          \
                           pierr2++;                               \
                           pierr1++;                               \
                           picur++;                                \
                        }                                          \
                        else                                       \
                        {                                          \
                           pierr2--;                               \
                           pierr1--;                               \
                           picur--;                                \
                        }

#define LOW_BIT_CONV(imask, ishift)   tToCMYK.bMult = false;            \
                                      tToCMYK.bR    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bRed;   \
                                      tToCMYK.bG    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bGreen; \
                                      tToCMYK.bB    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bBlue;  \
                                      ToCMYK (&tToCMYK);

   INT   i, n, y;

   INT   i1, i1a, i2, i4, i8;
   INT   iIntensity = 0;

   INT   iRedLast1 = 0;
   INT   iGreenLast1 = 0;
   INT   iBlueLast1 = 0;
   INT   iKLast1 = 0;
   INT   iLCLast1 = 0;
   INT   iLMLast1 = 0;

   INT   iRedLast2 = 0;
   INT   iGreenLast2 = 0;
   INT   iBlueLast2 = 0;
   INT   iKLast2 = 0;
   INT   iLCLast2 = 0;
   INT   iLMLast2 = 0;

   INT   iErr, iFrac;

   INT   iErrRowSize,
         iSrcDataEnd;

   PCOLR pColor;

   PLONG piRedErrRow1,    piRedCurRow,  piRow0,  piRow1, piRow2;
   PLONG piGreenErrRow1 = 0,  piGreenCurRow = 0;
   PLONG piBlueErrRow1 = 0,   piBlueCurRow = 0;
   PLONG piKErrRow1 = 0,   piKCurRow = 0;
   PLONG piLCErrRow1 = 0,   piLCCurRow = 0;
   PLONG piLMErrRow1 = 0 ,   piLMCurRow = 0;

   PLONG piRedErrRow2 = 0;
   PLONG piGreenErrRow2 = 0;
   PLONG piBlueErrRow2 = 0;
   PLONG piKErrRow2 = 0;
   PLONG piLMErrRow2 = 0;
   PLONG piLCErrRow2 = 0;

   PBYTE pbMapEnd, pbSrc, pbSrcNext;
   PBYTE pbKDestNext, pbKDest, pbCDestNext, pbCDest, pbMDestNext,
         pbMDest, pbYDestNext, pbYDest;
   PBYTE pbLCDest = 0 , pbLCDestNext = 0;                  //ROUTINE NEEDS COMPLETION FOR SIX COLOR
   PBYTE pbLMDest = 0, pbLMDestNext = 0;
   BYTE     abMask[]= {0x00,0x40,0x80,0xC0};
   TOCMYK   tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};

   PARAMS params;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (!pfErrRow_d)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   params.bMultiBitEnabled = false;

   SetInitialParameters(pbmi2, &params);

   // RowSize in bytes
   //TBD:  iSrcDataEnd is INVALID for pel sizes less than 8 bits
   iSrcDataEnd = ((((LONG)pbmi2->cx - 1) * (LONG)pbmi2->cBitCount)/8) ;

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize); // This points to a Pel after the last valid pel

   pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLCDest = pbLCBuffer_d;
       pbLMDest = pbLMBuffer_d;
       pbLCDestNext = pbLCNextBuffer_d;
       pbLMDestNext = pbLMNextBuffer_d;
   }

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   y = iRowCount_d;
   n = y % 3;

   piRow0 = (PLONG)pfErrRow_d + CMYK_FILTER_OVERFLOW / 2;

   // ErrRowSize in pels = (pels Per Row  +  boundry Overflow)
   iErrRowSize = ((ULONG)pbmi2->cx + CMYK_FILTER_OVERFLOW) ;

   // first buffer + 3colorsPerBufferGroup * rowSizeInPels (Dword increment)
   piRow1 = piRow0 + 3 * iErrRowSize;
   piRow2 = piRow1 + 3 * iErrRowSize;

   // @TBD - fix!
   if ((y & 1) == 0)
   {
      i = 0;
      pbCDestNext = pbCDest +  params.iDestRowSize;
      pbMDestNext = pbMDest +  params.iDestRowSize;
      pbYDestNext = pbYDest +  params.iDestRowSize;
      pbSrc = pbSource;
      pbSrcNext = pbSource + params.iSrcRowSize;
      if (n == 0)
      {
         piRedErrRow2 = piRow2;
         piRedErrRow1 = piRow1;
         piRedCurRow  = piRow0;
      }
      else if (n == 1)
      {
         piRedErrRow2 = piRow0;
         piRedErrRow1 = piRow2;
         piRedCurRow  = piRow1;
      }
      else
      {
         piRedErrRow2 = piRow1;
         piRedErrRow1 = piRow0;
         piRedCurRow  = piRow2;
      }

      piGreenErrRow2  = piRedErrRow2 + iErrRowSize;
      piGreenErrRow1  = piRedErrRow1 + iErrRowSize;
      piGreenCurRow   = piRedCurRow  + iErrRowSize;

      piBlueErrRow2   = piGreenErrRow2 + iErrRowSize;
      piBlueErrRow1   = piGreenErrRow1 + iErrRowSize;
      piBlueCurRow    = piGreenCurRow  + iErrRowSize;

      // 3rowsOneforEachColor * 4bytesPerPel(i.e. a Long) * rowSizeInPels
      memset ((PBYTE)(piRedErrRow2 - (CMYK_FILTER_OVERFLOW /2)), 0, (size_t)(3 * 4 * iErrRowSize));
   }
   else
   {
      i = (LONG)pbmi2->cx - 1;
      pbCDestNext =  pbCDest + params.iDestRowSize;
      pbCDest = pbCDestNext - 1;
      pbMDestNext =  pbMDest + params.iDestRowSize;
      pbMDest = pbMDestNext - 1;
      pbYDestNext =  pbYDest + params.iDestRowSize;
      pbYDest = pbYDestNext - 1;
      pbSrcNext = pbSource + params.iSrcRowSize;
      pbSrc = pbSource + iSrcDataEnd;
      if (n == 0)
      {
         piRedErrRow2 = piRow2 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
         piRedErrRow1 = piRow1 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
         piRedCurRow  = piRow0 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
      }
      else if (n == 1)
      {
         piRedErrRow2 = piRow0 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
         piRedErrRow1 = piRow2 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
         piRedCurRow  = piRow1 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
      }
      else
      {
         piRedErrRow2 = piRow1 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
         piRedErrRow1 = piRow0 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
         piRedCurRow  = piRow2 + iErrRowSize - CMYK_FILTER_OVERFLOW - 1;
      }
      piGreenErrRow2  = piRedErrRow2   + iErrRowSize;
      piGreenErrRow1  = piRedErrRow1   + iErrRowSize;
      piGreenCurRow   = piRedCurRow    + iErrRowSize;
      piBlueErrRow2   = piGreenErrRow2 + iErrRowSize;
      piBlueErrRow1   = piGreenErrRow1 + iErrRowSize;
      piBlueCurRow    = piGreenCurRow  + iErrRowSize;
      memset ((PBYTE)(piRedErrRow2 - iErrRowSize + (CMYK_FILTER_OVERFLOW /2) + 1), 0, (3 * 4 * iErrRowSize));
   }

   switch (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0x80, 7)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x40, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x20, 5)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x10, 4)
         SNAP (pbRGamma_d[tToCMYK.lC]], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM]], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY]], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x8, 3)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x4, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x2, 1)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x1, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }

   case 2:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xc0, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM]. bGreen, lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x30, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0xC, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x3, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }

   case 4:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xf0, 4)
         SNAP (pbRGamma_d[tToCMYK.lC],  lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM],  lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY],  lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0f, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }
#endif

   case 8:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {

         tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
         tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
         tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

         if (!fDataInRGB_d)
            SWAP (tToCMYK.bR, tToCMYK.bB);

         ToCMYK(&tToCMYK);

         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
         INC(1)
         TESTROWEND
      }
      break;
   }

   case 16:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         tToCMYK.bR = (*pbSrc & 0xf8);
         tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
         tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

         if (!fDataInRGB_d)
            SWAP (tToCMYK.bR, tToCMYK.bB);

         ToCMYK(&tToCMYK);

         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2) \
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

         INC(2)
         TESTROWEND
      }
      break;
   }

   case 24:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
          while ((ULONG)pbSrc < (ULONG)pbMapEnd)
          {
             if(isNotWhite(pbSrc, &tToCMYK))
             {
                 ToCMYK (&tToCMYK);
             }
             else
             {
                 tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
             }

             SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
             SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
             SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)

             INC(3)
             TESTROWEND
          }
      }
      else
      {
          if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)    //@@TBD
          {
              while ((ULONG)pbSrc < (ULONG)pbMapEnd)
              {

                 if(isNotWhite(pbSrc, &tToCMYK))
                 {
                     ToCMYK (&tToCMYK);
                 }
                 else
                 {
                     tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lK = 0;
                 }

                 SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
                 SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
                 SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
                 SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)

                 INC(3)
                 TESTROWEND
              }
          }
          else
          {
              if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)    //@@TBD
              {
                  while ((ULONG)pbSrc < (ULONG)pbMapEnd)
                  {

                     if(isNotWhite(pbSrc, &tToCMYK))
                     {
                         ToCMYK6 (&tToCMYK, pbLightTable_d, pbDarkTable_d);
                     }
                     else
                     {
                         tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lK = tToCMYK.lLC = tToCMYK.lLM = 0;
                     }

                     SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedErrRow2,   piRedCurRow,   iRedLast1,   iRedLast2)
                     SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenErrRow2, piGreenCurRow, iGreenLast1, iGreenLast2)
                     SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueErrRow2,  piBlueCurRow,  iBlueLast1,  iBlueLast2)
                     SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow1,     piKErrRow2,     piKCurRow,     iKLast1,     iKLast2)
                     SNAP (pbRGamma_d[tToCMYK.lLC], lRed,   LCyan,   pbLCDest, piLCErrRow1,  piLCErrRow2,    piLCCurRow,    iLCLast1,    iLCLast2)
                     SNAP (pbGGamma_d[tToCMYK.lLM], lGreen, LMagenta,pbLMDest, piLMErrRow1,  piLMErrRow2,    piLMCurRow,    iLMLast1,    iLMLast2)

                     INC(3)
                     TESTROWEND
                  }
              }
          }
      }
      break;
   }

   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = y;

   if (  ((iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
          || (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK))
      && fRemoveBlackColor_d
      )
   {
      GplCMYRemoval (pbmi2);
   }

   return 0;

#undef SNAP
#undef SETBIT
#undef INC
#undef TESTROWEND
#undef LOW_BIT_CONV
}

/****************************************************************************/
/* PROCEDURE NAME : SteinbergDiffusion    Floyd-Steinberg                   */
/* DATE WRITTEN   : 8/1/94                                                  */
/* DESCRIPTION    :                                                         */
/*                                                                          */
/*    Floyd-Steinberg error diffusion :                                     */
/*                                                                          */
/*    1. k = pClrTableValue at [x][y] point + iLastPointErr + piCurRow[x][y]*/
/*                                                                          */
/*    2. if( k <= midpt.c )                                                 */
/*          print CMYK [x][y] point for this RGB c, pColor.                 */
/*                                                                          */
/*    3. iError =  k - MAX_RGB_VALUE                                        */
/*                                                                          */
/*    // compute error for next point                                       */
/*    4. iLastPointErr[x+1][y]   = 7/16 * iError                            */
/*                                                                          */
/*    5. piErrRow[x-1][y-1] = 3/16 * iError                                 */
/*                                                                          */
/*    6. piErrRow[x][y-1]   = 5/16 * iError                                 */
/*                                                                          */
/*    7. piErrRow[x+1][y-1] = 1/16 * iError   : ( remaining error )         */
/*                                                                          */
/*    use x++ on even row passes                                            */
/*                                                                          */
/*    OPTION: use x-- and reverse indexes on steps 4, 5, and 7 on           */
/*            odd row passes iLastPointErr is the error value computed      */
/*            from the last point for this point piErrRow is an array       */
/*            to be added to the the next line.                             */
/*                                                                          */
/*    A diffusion error buffer must have been allocated and initialized     */
/*    to zero on first entry.  The error buffer will contain                */
/*    intermediate values between bands and must be deallocated at          */
/*    page end.                                                             */
/*    The error buffer must be 6 times (row pel length (cx) plus 2),        */
/*    in (for now) longs; 2 rows per CMY pColor.                            */
/*                                                                          */
/*    NOTE: KCMY_ALL or CMY_ALL must be selected.                           */
/*                                                                          */
/*     piCurRow index is 4 bytes ahead of the current relative position.    */
/*     iLastPointErr is initially THIS point brought along from the last    */
/*     point's SNAP diffusion.                                              */
/*                                                                          */
/*     The CurRow begins with the second entry since Floyd-Standberg        */
/*     requires one pel in back and one line below and one pel in front     */
/*     and below.                                                           */
/*                                                                          */
/*     LastPointErr is a single variable that is used to store the error    */
/*     for the next pel.  ErrRow is always the next row.                    */
/*     Two buffers are toggled in order to add the CurErr(+1) to the        */
/*     current row.                                                         */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplSteinbergDiffusion (PBITMAPINFO2 pbmi2,
                       PBYTE        pbSource)
{
#define INC(iSrcInc)     pbSrc += iSrcInc;           \
                         if (1 == iDestBitsPerPel_d) \
                         {                           \
                           if ((i & 7) == 7)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         else                     \
                         {                        \
                           if ((i & 3) == 3)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         i++;

#define TESTROWEND       if (i >= (LONG)pbmi2->cx)                                  \
                         {                                                          \
                            i = 0;                                                  \
                            pbCDest     = pbCDestNext;                              \
                            pbCDestNext = pbCDest + iNumDestRowBytes_d;             \
                            pbMDest     = pbMDestNext;                              \
                            pbMDestNext = pbMDest + iNumDestRowBytes_d;             \
                            pbYDest     = pbYDestNext;                              \
                            pbYDestNext = pbYDest + iNumDestRowBytes_d;             \
                            pbKDest     = pbKDestNext;                              \
                            pbKDestNext = pbKDest + iNumDestRowBytes_d;             \
                            if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK) \
                            {                                                       \
                               pbLCDest     = pbLCDestNext;                         \
                               pbLCDestNext = pbLCDest + iNumDestRowBytes_d;        \
                               pbLMDest     = pbLMDestNext;                         \
                               pbLMDestNext = pbLMDest + iNumDestRowBytes_d;        \
                            }                                                       \
                            pbSrc       = pbSrcNext;                                \
                            pbSrcNext   = pbSrc + params.iSrcRowSize;                      \
                            y++;                                                    \
                            if (y  == (long)pbmi2->cy)                              \
                               break;                                               \
                            SETUP_ERROR_ROWS;                                       \
                         }

#define SETUP_ERROR_ROWS if (y & 1)                                               \
                         {                                                        \
                            piRedErrRow  = piEvenRow;                             \
                            piRedCurRow  = piOddRow;                              \
                         }                                                        \
                         else                                                     \
                         {                                                        \
                            piRedErrRow  = piOddRow;                              \
                            piRedCurRow  = piEvenRow;                             \
                         }                                                        \
                         piGreenErrRow  = piRedErrRow    + iErrRowSize;           \
                         piGreenCurRow  = piRedCurRow    + iErrRowSize;           \
                         piBlueErrRow   = piGreenErrRow  + iErrRowSize;           \
                         piBlueCurRow   = piGreenCurRow  + iErrRowSize;           \
                         piKErrRow      = piBlueErrRow   + iErrRowSize;           \
                         piKCurRow      = piBlueCurRow   + iErrRowSize;           \
                         if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)  \
                         {                                                        \
                            piLCErrRow     = piKErrRow      + iErrRowSize;        \
                            piLCCurRow     = piKCurRow      + iErrRowSize;        \
                            piLMErrRow     = piLCErrRow     + iErrRowSize;        \
                            piLMCurRow     = piLCCurRow     + iErrRowSize;        \
                         }                                                        \
                         {                                                        \
                            float *pf = piRedErrRow - (CMYK_FILTER_OVERFLOW / 2); \
                            int    ii;                                            \
                            for (ii = 0; ii < iNumPens_d * iErrRowSize; ii++)       \
                            {                                                     \
                               *pf++ = 0.0;                                       \
                            }                                                     \
                         }

#define SETBIT(p,pdest)  *pdest |= (BYTE)(0x80 >> (i & 7));                       \
                         fEmpty##p##_d = false;

#define SETINTENSITYBIT(p, pdest, iIntensity)                                     \
     *pdest |= (BYTE)((BYTE)abMask[iIntensity] >> ((i & 3) *2));                  \
                         fEmpty##p##_d = false;


#define SNAP(bVal,c,p,pdest,pierr,picur,ilast)                                    \
                           iErr = (float)bVal + *picur + ilast;                   \
                           if (1 == iDestBitsPerPel_d)                            \
                           {                                                      \
                              if (iErr <= midPt_d.c)                              \
                              {                                                   \
                                 SETBIT (p, pdest)                                \
                              }                                                   \
                              else                                                \
                              {                                                   \
                                 iErr -= 255.0;                                   \
                              }                                                   \
                           }                                                      \
                           else                                                   \
                           {                                                      \
                              if (iErr >= 127.0 )                                 \
                              {                                                   \
                                 iErr -= 255.0;                                   \
                              }                                                   \
                              else                                                \
                              {                                                   \
                                 if (iErr <= 0.0)                                 \
                                 {                                                \
                                    iIntensity = 3;                               \
                                 }                                                \
                                 else if (iErr <= 50.0)                           \
                                 {                                                \
                                    iIntensity = 2;                               \
                                 }                                                \
                                 else if (iErr < 128.0)                           \
                                 {                                                \
                                    iIntensity = 1;                               \
                                 }                                                \
                                 SETINTENSITYBIT(p, pdest, iIntensity)            \
                              }                                                   \
                           }                                                      \
                                                                                  \
                           iFrac = iErr/16.0;                                     \
                                                                                  \
                           ilast = iFrac * 7.0;                                   \
                                                                                  \
                           *(pierr - 1) += iFrac * 3.0;                           \
                           *(pierr    ) += iFrac * 5.0;                           \
                           *(pierr + 1) += iFrac;                                 \
                                                                                  \
                           pierr++;                                               \
                           picur++;


#define SNAP6DARK(bVal,c,p,pdest,pierr,picur,ilast)                               \
                           iErr = (float)bVal + *picur + ilast;                   \
                           if (1 == iDestBitsPerPel_d)                            \
                           {                                                      \
                              if (iErr <= midPt_d.c)                              \
                              {                                                   \
                                 SETBIT (p, pdest)                                \
                              }                                                   \
                              else                                                \
                              {                                                   \
                                 iErr -= 255.0;                                   \
                              }                                                   \
                           }                                                      \
                           else                                                   \
                           {                                                      \
                              if (iErr >= 127.0 )                                 \
                              {                                                   \
                                 iErr -= 255.0;                                   \
                                 iErr = iErr*.95;                                 \
                              }                                                   \
                              else                                                \
                              {                                                   \
                                 if (iErr <= 0.0)                                 \
                                 {                                                \
                                    iIntensity = 3;                               \
                                 }                                                \
                                 else if (iErr <= 50.0)                           \
                                 {                                                \
                                    iIntensity = 2;                               \
                                 }                                                \
                                 else if (iErr < 128.0)                           \
                                 {                                                \
                                    iIntensity = 1;                               \
                                 }                                                \
                                 SETINTENSITYBIT(p, pdest, iIntensity)            \
                                 iErr += iErr * .95;                              \
                              }                                                   \
                           }                                                      \
                           if(iErr > 255.0)                                       \
                             iErr = 255.0;                                        \
                           else                                                   \
                             if(iErr < -255.0)                                    \
                               iErr = -255.0;                                     \
                           if(iErr > -7.0 && iErr < 7.0)                          \
                             iErr = 0.0;                                          \
                                                                                  \
                             iFrac = iErr/80;                                     \
                             i3 = iFrac * 15;                                     \
                             i5 = iFrac * 25;                                     \
                                                                                  \
                             ilast = iFrac * 35;                                  \
                                                                                  \
                             *(pierr - 1) += i3;                                  \
                             *(pierr    ) += i5;                                  \
                             *(pierr + 1) += iFrac;                               \
                                                                                  \
                             pierr++;                                             \
                             picur++;                                             \
                                                                                  \

#define SNAP6LIGHT(bVal,c, name, pdest, pierr,picur,ilast)                        \
                           iErr = (float)bVal + *picur + ilast;                   \
                           if (1 == iDestBitsPerPel_d)                            \
                           {                                                      \
                              if (iErr <= midPt_d.c)                              \
                              {                                                   \
                                 SETBIT (name, pdest)                             \
                              }                                                   \
                              else                                                \
                              {                                                   \
                                 iErr -= 255.0;                                   \
                              }                                                   \
                           }                                                      \
                           else                                                   \
                           {                                                      \
                              if (iErr >= 127.0 )                                 \
                              {                                                   \
                                 iErr -= 255;                                     \
                                 iErr = iErr *.95;                                \
                              }                                                   \
                              else                                                \
                              {                                                   \
                                 if (iErr <= -10.0)                               \
                                 {                                                \
                                    iIntensity = 3;                               \
                                 }                                                \
                                 else if (iErr <= 65.0)                           \
                                 {                                                \
                                    iIntensity = 2;                               \
                                 }                                                \
                                 else if (iErr <= 128.0)                          \
                                 {                                                \
                                    iIntensity = 1;                               \
                                 }                                                \
                                 SETINTENSITYBIT(name, pdest, iIntensity)         \
                                 iErr = iErr *.95;                               \
                              }                                                   \
                           }                                                      \
                           if(iErr > 255.0)                                       \
                             iErr = 255.0;                                        \
                           else                                                   \
                             if(iErr < -255.0)                                    \
                               iErr = -255.0;                                     \
                           if(iErr > -8.0 && iErr < 8.0)                          \
                             iErr = 0.0;                                          \
                                                                                  \
                             iFrac = iErr/80.0;                                   \
                             i3 = iFrac * 15.0;                                   \
                             i5 = iFrac * 25.0;                                   \
                                                                                  \
                             ilast = iFrac * 35.0;                                \
                                                                                  \
                             *(pierr - 1) += i3;                                  \
                             *(pierr    ) += i5;                                  \
                             *(pierr + 1) += iFrac;                               \
                                                                                  \
                             pierr++;                                             \
                             picur++;

#define LOW_BIT_CONV(imask, ishift)   tToCMYK.bMult = false;            \
                                      tToCMYK.bR    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bRed;   \
                                      tToCMYK.bG    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bGreen; \
                                      tToCMYK.bB    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bBlue;  \
                                      ToCMYK (&tToCMYK);


   INT      i, y = 0;
   float    iFrac;

   float      i3, i5;

   float      iRedLast   = 0.0;
   float      iGreenLast = 0.0;
   float      iBlueLast  = 0.0;
   float      iKLast     = 0.0;
   float      iLMLast    = 0.0;
   float      iLCLast    = 0.0;

   float      iErr;

   INT      iErrRowSize,
            iIntensity = 0;

   PCOLR    pColor;

   float    *piOddRow,  *piEvenRow;

   float    *piRedErrRow,    *piRedCurRow;
   float    *piGreenErrRow,  *piGreenCurRow;
   float    *piBlueErrRow,   *piBlueCurRow;
   float    *piKErrRow,      *piKCurRow;
   float    *piLCErrRow = 0, *piLCCurRow = 0;
   float    *piLMErrRow = 0, *piLMCurRow = 0;
   PBYTE    pbMapEnd,
            pbSrc,            pbSrcNext;
   PBYTE    pbKDestNext,      pbKDest,
            pbCDestNext,      pbCDest,
            pbMDestNext,      pbMDest,
            pbYDestNext,      pbYDest,
            pbLMDestNext = 0, pbLMDest = 0,
            pbLCDestNext = 0, pbLCDest = 0;
   BYTE     abMask[]= {0x00,0x40,0x80,0xC0};

   ULONG    ulTempR, ulTempG, ulTempB, ulTempK;

   TOCMYK   tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};
   PARAMS   params;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   params.bMultiBitEnabled = true;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize); // This points to a Pel after the last valid pel

   pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLCDest     = pbLCBuffer_d;
       pbLMDest     = pbLMBuffer_d;
       pbLCDestNext = pbLCNextBuffer_d;
       pbLMDestNext = pbLMNextBuffer_d;
   }

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   // ErrRowSize in pels = (pels Per Row  +  boundry Overflow)
   iErrRowSize = ((ULONG)pbmi2->cx + CMYK_FILTER_OVERFLOW);

   piEvenRow  = (float *)pfErrRow_d + CMYK_FILTER_OVERFLOW / 2;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       // first buffer + 6colorsPerBufferGroup * rowSize
       piOddRow = piEvenRow + iNumPens_d * iErrRowSize;
   }
   else
   {
       // first buffer + 4colorsPerBufferGroup * rowSize
       piOddRow = piEvenRow + iNumPens_d * iErrRowSize;
   }

   SETUP_ERROR_ROWS;
   y = iRowCount_d;


   i = 0;

   switch (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:
   {
      //-----------------------------
      //  1-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0x80, 7)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x40, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x20, 5)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x10, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x08, 3)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x04, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x02, 1)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x01, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         INC(1)
         TESTROWEND
      }
      break;
   }
   case 2:
   {
      //-----------------------------
      //  2-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xC0, 6)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x30, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0C, 2)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x03, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         INC(1)
         TESTROWEND
      }
      break;
   }
   case 4:
   {
      //-----------------------------
      //  4-bit:
      //-----------------------------
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         LOW_BIT_CONV(0xF0, 4)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         i++;
         TESTROWEND
         LOW_BIT_CONV(0x0F, 0)
         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
         INC(1)
         TESTROWEND
      }
      break;
   }
#endif
   case 8:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         //  8-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            ulTempR = pColor->is[(ULONG)*pbSrc].bRed;
            ulTempG = pColor->is[(ULONG)*pbSrc].bGreen;
            ulTempB = pColor->is[(ULONG)*pbSrc].bBlue;
            ulTempK = pColor->is[(ULONG)*pbSrc].fcOptions;

            // Convert from RGBK -> RGB
            tToCMYK.bR = 255 - MIN (255, ((255 - ulTempR) * ulTempK / 255 + (255 - ulTempK)));
            tToCMYK.bG = 255 - MIN (255, ((255 - ulTempG) * ulTempK / 255 + (255 - ulTempK)));
            tToCMYK.bB = 255 - MIN (255, ((255 - ulTempB) * ulTempK / 255 + (255 - ulTempK)));

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
            SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow,    piKCurRow,     iKLast  )

            INC(1)
            TESTROWEND
         }
      }
      else
      {
         //-----------------------------
         //  8-bit:CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = pColor->is[(ULONG)*pbSrc].bRed;
            tToCMYK.bG = pColor->is[(ULONG)*pbSrc].bGreen;
            tToCMYK.bB = pColor->is[(ULONG)*pbSrc].bBlue;

            if (!fDataInRGB_d)
               SWAP (tToCMYK.lC, tToCMYK.lY);

            ToCMYK(&tToCMYK);
            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)

            INC(1)
            TESTROWEND
         }
      }
      break;
   }

   case 16:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         //  16-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = (*pbSrc & 0xf8);
            tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
            tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

            if (!fDataInRGB_d)
               SWAP (tToCMYK.bR, tToCMYK.bB);

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)
            SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow,    piKCurRow,     iKLast  )

            INC(2)
            TESTROWEND
         }
      }
      else
      {
         //-----------------------------
         //  16-bit:CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            tToCMYK.bR = (*pbSrc & 0xf8);
            tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
            tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

            if (!fDataInRGB_d)
               SWAP (tToCMYK.bR, tToCMYK.bB);

            ToCMYK(&tToCMYK);

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,  piRedCurRow,   iRedLast)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow,piGreenCurRow, iGreenLast)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow,  iBlueLast)

            INC(2)
            TESTROWEND
         }
      }
      break;
   }

   case 24:
   {
      if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
      {
         //-----------------------------
         // STUCKI: 24-bit:CcMmYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {

            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK6 (&tToCMYK, pbLightTable_d, pbDarkTable_d);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lLC = tToCMYK.lM = tToCMYK.lLM = tToCMYK.lY = tToCMYK.lK = 0;
            }

            SNAP6DARK   (pbRGamma_d[tToCMYK.lC],  lRed,   Cyan,    pbCDest,  piRedErrRow,   piRedCurRow,   iRedLast)
            SNAP6DARK   (pbRGamma_d[tToCMYK.lM],  lGreen, Magenta, pbMDest,  piGreenErrRow, piGreenCurRow, iGreenLast)
            SNAP6LIGHT  (pbRGamma_d[tToCMYK.lLC], lRed,   LCyan,   pbLCDest, piLCErrRow,    piLCCurRow,    iLCLast)
            SNAP6LIGHT  (pbGGamma_d[tToCMYK.lLM], lGreen, LMagenta,pbLMDest, piLMErrRow,    piLMCurRow,    iLMLast)
            SNAP6LIGHT  (pbBGamma_d[tToCMYK.lY],  lBlue,  Yellow,  pbYDest,  piBlueErrRow,  piBlueCurRow,  iBlueLast)
            SNAP6DARK   (pbKGamma_d[tToCMYK.lK],  lBlack, Black,   pbKDest,  piKErrRow,     piKCurRow,     iKLast)

            INC(3)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      {
         //-----------------------------
         // STUCKI: 24-bit:CMYK
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {


            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK (&tToCMYK);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lK = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow,   piRedCurRow,   iRedLast)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow,  piBlueCurRow,  iBlueLast)
            SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piKErrRow,     piKCurRow,     iKLast)

            INC(3)
            TESTROWEND
         }
      }
      else if (iColorTech_d == DevicePrintMode::COLOR_TECH_CMY)
      {
         //-----------------------------
         // STUCKI: 24-bit: CMY
         //-----------------------------
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {

            if(isNotWhite(pbSrc, &tToCMYK))
            {
                ToCMYK (&tToCMYK);
            }
            else
            {
                tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
            }

            SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow, piRedCurRow, iRedLast)
            SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow, piGreenCurRow, iGreenLast)
            SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow, piBlueCurRow, iBlueLast)

            INC(3)
            TESTROWEND
         }
      }
      break;
   }

   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = y;

   if (((iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)||(iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK))
      && fRemoveBlackColor_d
      )
   {
      GplCMYRemoval (pbmi2);
   }

   return 0;

#undef SNAP
#undef SETBIT
#undef INC
#undef TESTROWEND
#undef SETUP_ERROR_ROWS


#undef TESTROWEND6
#undef SNAP6LIGHT
#undef SNAP6DARK
#undef SETINTENSITYBIT
#undef LOW_BIT_CONV
}

/****************************************************************************/
/* PROCEDURE NAME : GET_RANDOM_COEFFICIENT                                  */
/* DATE WRITTEN   : 12-28-94                                                */
/* DESCRIPTION    :                                                         */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                                                                          */
/*                                                                          */
/* RETURN VALUES:   Success: NO_ERROR                  0                    */
/* Get a random coefficient between Low  and .Hi and keep mixing            */
/* the random number table.  low = 34    hi=66                              */
/* GplInitializeRandomNumberTable must have been previously invoked.        */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
#define GET_RANDOM_COEFFICIENT(idx1,idx2,lmix,ltemp,lbig,lRandNum)  \
   (idx1)++;                        \
   (idx2)++;                        \
   if (idx1 >= 56)                  \
      idx1 = 1;                     \
   if (idx2 >= 56)                  \
      idx2=1;                       \
   ltemp = lmix[idx1] - lmix[idx2]; \
   if (ltemp < 0)                   \
      ltemp += lbig;                \
   lmix[idx1] = ltemp;              \
   lRandNum =  34 + (ltemp & 0x1f);

/****************************************************************************/
/* PROCEDURE NAME : GplFastDiffusion                                        */
/* DATE WRITTEN   : 12/23/94                                                */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PDITHEREQUEST  pReq                                     */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplFastDiffusion (PBITMAPINFO2 pbmi2,
                  PBYTE        pbSource)
{
/*
      Error diffusion :
            k = sum of the errors for the current pel
            if( k <= 127<<Exp )
               Set Output Bit
               error = k
            else
               error = k - MAX_RGB_VALUE<<Exp
            vtemp[x+1][y] = coeff1 * error / 100
            vtemp[x][y-1] = error - vtemp[x+1][y]

      use x++ on even row passes
      Option: use x-- and reverse indexes on steps 4, 5, and 7 on odd row passes
      iLastPointErr is the error to be added to this point from the last point
         during the current pass
      iCurRow is the errors for this row computed from the last row.

      A diffusion error buffer must have been allocated and initialized to
      zero on first entry.  The error buffer will contain intermediate values
      between bands and must be deallocated at page end. The error buffer
      must be 6 times (row pel length (cx) plus 4), in longs; 3 rows per CMY pColor.
      NOTE: KCMY_ALL or CMY_ALL must be selected.
*/

#define INC(iSrcInc)    i++;                     \
                         pbSrc += iSrcInc;        \
                         if (1 == iDestBitsPerPel_d) \
                         {                           \
                           if ((i & 7) == 7)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }                        \
                         else                     \
                         {                        \
                           if ((i & 3) == 3)      \
                           {                      \
                              pbCDest++;          \
                              pbMDest++;          \
                              pbYDest++;          \
                              pbKDest++;          \
                              pbLCDest++;         \
                              pbLMDest++;         \
                           }                      \
                         }

#define TESTROWEND         if (i >= (LONG)pbmi2->cx)                                 \
                           {                                                         \
                              i = 0;                                                 \
                              pbCDest     = pbCDestNext;                             \
                              pbCDestNext = pbCDest +  params.iDestRowSize;          \
                              pbMDest     = pbMDestNext;                             \
                              pbMDestNext = pbMDest +  params.iDestRowSize;          \
                              pbYDest     = pbYDestNext;                             \
                              pbYDestNext = pbYDest +  params.iDestRowSize;          \
                              pbKDest     = pbKDestNext;                             \
                              pbKDestNext = pbKDest +  params.iDestRowSize;          \
                              if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)\
                              {                                                      \
                                 pbLCDest     = pbLCDestNext;                        \
                                 pbLCDestNext = pbLCDest + params.iDestRowSize;      \
                                 pbLMDest     = pbLMDestNext;                        \
                                 pbLMDestNext = pbLMDest + params.iDestRowSize;      \
                              }                                                      \
                              pbSrc       = pbSrcNext;                               \
                              pbSrcNext   = pbSrc + params.iSrcRowSize;              \
                              y++;                                                   \
                              if ((ULONG)pbSrc >= (ULONG)pbMapEnd)                   \
                                 break;                                              \
                              if ((y & 1) == 0)                                      \
                              {                                                      \
                                 piRedErrRow1 = piRow1;                              \
                                 piRedCurRow  = piRow0;                              \
                              }                                                      \
                              else                                                   \
                              {                                                      \
                                 piRedErrRow1 = piRow0;                              \
                                 piRedCurRow  = piRow1;                              \
                              }                                                      \
                              piGreenErrRow1 = piRedErrRow1   + iErrRowSize;         \
                              piGreenCurRow  = piRedCurRow    + iErrRowSize;         \
                              piBlueErrRow1  = piGreenErrRow1 + iErrRowSize;         \
                              piBlueCurRow   = piGreenCurRow  + iErrRowSize;         \
                              piBlackErrRow1 = piBlueErrRow1  + iErrRowSize;         \
                              piBlackCurRow  = piBlueCurRow   + iErrRowSize;         \
                              if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)\
                              {                                                      \
                                 piLCErrRow1     = piBlackErrRow1 + iErrRowSize;      \
                                 piLCCurRow     = piBlackCurRow  + iErrRowSize;      \
                                 piLMErrRow1     = piLCErrRow1     + iErrRowSize;      \
                                 piLMCurRow     = piLCCurRow     + iErrRowSize;      \
                              }                                                      \
                              memset ((PBYTE)(piRedErrRow1 - (CMYK_FILTER_OVERFLOW / 2)), 0, (size_t)(iNumPens_d * 4 * iErrRowSize)); \
                           }

#define SETBIT(p,pdest)  *pdest |= (BYTE)(0x80 >> (i & 7));    \
                         fEmpty##p##_d = false;

#define SETINTENSITYBIT(p, pdest, iIntensity)  \
     *pdest |= (BYTE)((BYTE)abMask[iIntensity] >> ((i & 3) *2)); \
                         fEmpty##p##_d = false;

// piCurRow1 index is 4 bytes ahead of the current relative position. iLastPointErr is initially THIS point
// brought along from the last point's SNAP diffusion.
#if 0
#define SNAP(bVal,c,p,pdest,pierr1,picur,ilast1,fcoeff1)             \
                           iErr = (LONG)((ULONG)bVal<<RND_EXP) + *picur + ilast1;  \
                           if( iErr <= (LONG)0x7f<<RND_EXP)          \
                           {                                         \
                              SETBIT (p, pdest)                      \
                           }                                         \
                           else                                      \
                              iErr -= (LONG)0xff<<RND_EXP;           \
                           ilast1 = (LONG)((float)iErr * fcoeff1);   \
                           *pierr1 += (LONG)((float)iErr - ilast1);  \
                           pierr1++;                                 \
                           picur++;

#endif


#define SNAP(bVal,c,p,pdest,pierr1,picur,ilast1,lcoeff1)            \
                          iErr = (LONG)((int)((ULONG)bVal << ERR_EXP)) + *picur + ilast1; \
                          if (1 == iDestBitsPerPel_d)                     \
                          {                                               \
                              if (iErr <= midPt_d.c)                      \
                              {                                           \
                                 SETBIT (p, pdest)                        \
                              }                                           \
                              else                                        \
                              {                                           \
                                  iErr -= (0xff << ERR_EXP);              \
                              }                                           \
                          }                                               \
                          else                                            \
                          {                                               \
                              if (iErr >= 127 )                         \
                              {                                           \
                                 iErr -= (0xff << ERR_EXP);               \
                              }                                           \
                              else                                        \
                              {                                           \
                                 if (iErr <= 0)                         \
                                 {                                        \
                                    iIntensity = HIGH;                    \
                                 }                                        \
                                 else if (iErr <= (50 << ERR_EXP))      \
                                 {                                        \
                                    iIntensity = MEDIUM;                  \
                                 }                                        \
                                 else if (iErr < (128 << ERR_EXP))      \
                                 {                                        \
                                    iIntensity = LOW;                     \
                                 }                                        \
                                 SETINTENSITYBIT(p, pdest, iIntensity)    \
                              }                                           \
                          }                                               \
                          ilast1   = (iErr * (int)lcoeff1) / 100;         \
                          *pierr1 += (iErr - ilast1);                     \
                          pierr1++;                                       \
                          picur++;

#define LOW_BIT_CONV(imask, ishift)   tToCMYK.bMult = false;            \
                                      tToCMYK.bR    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bRed;   \
                                      tToCMYK.bG    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bGreen; \
                                      tToCMYK.bB    = pColor->is[(LONG)(*pbSrc & imask)>>ishift].bBlue;  \
                                      ToCMYK (&tToCMYK);

   INT      i, y;

   INT      iRedLast1   = 0;
   INT      iGreenLast1 = 0;
   INT      iBlueLast1  = 0;
   INT      iBlackLast1 = 0;
   INT      iLCLast1    = 0;
   INT      iLMLast1    = 0;
   INT      iErr;
   INT      iErrRowSize;
   PCOLR    pColor;
   PLONG    piRedErrRow1,
            piRedCurRow,
            piRow0,
            piRow1;
   PLONG    piGreenErrRow1,
            piGreenCurRow;
   PLONG    piLCErrRow1 = 0,
            piLCCurRow  = 0;
   PLONG    piLMErrRow1 = 0,
            piLMCurRow  = 0;
   PLONG    piBlueErrRow1,
            piBlueCurRow,
            piBlackErrRow1,
            piBlackCurRow;
   PBYTE    pbMapEnd,
            pbSrc,
            pbSrcNext;
   PBYTE    pbKDestNext,
            pbKDest,
            pbCDestNext,
            pbCDest,
            pbLCDestNext = 0,
            pbLCDest     = 0,
            pbMDestNext,
            pbMDest,
            pbLMDestNext = 0,
            pbLMDest     = 0,
            pbYDestNext,
            pbYDest;
   PLONG    pidx1,
            pidx2;
   long     lbig;
   PLONG    plmix;
   long     ltemp,
            lcoeff1;
   BYTE     abMask[]= {0x00,0x40,0x80,0xC0};      // mask for multi-bit per pel
   INT      iIntensity = 0;
   PARAMS   params;
   TOCMYK   tToCMYK = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   params.bMultiBitEnabled = true;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

   pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;
   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLCDest = pbLCBuffer_d;
       pbLMDest = pbLMBuffer_d;
   }

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       pbLCDestNext = pbLCNextBuffer_d;
       pbLMDestNext = pbLMNextBuffer_d;
   }


   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   piRow0  = (PLONG)pfErrRow_d + CMYK_FILTER_OVERFLOW / 2;

   // ErrRowSize in pels = (pels Per Row  +  boundry Overflow)
   iErrRowSize = ((ULONG)pbmi2->cx + CMYK_FILTER_OVERFLOW);

   // first buffer + 4colorsPerBufferGroup * rowSizeInPels (4byte/Dword increment)
   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       piRow1 = piRow0 + iNumPens_d * iErrRowSize;
   }
   else
   {
       piRow1 = piRow0 + iNumPens_d * iErrRowSize;
   }


   y = iRowCount_d;

   if ((y & 1) == 0)
   {
      piRedErrRow1 = piRow1;
      piRedCurRow  = piRow0;
   }
   else
   {
      piRedErrRow1 = piRow0;
      piRedCurRow  = piRow1;
   }

   piGreenErrRow1  = piRedErrRow1   + iErrRowSize;
   piGreenCurRow   = piRedCurRow    + iErrRowSize;

   piBlueErrRow1   = piGreenErrRow1 + iErrRowSize;
   piBlueCurRow    = piGreenCurRow  + iErrRowSize;

   piBlackErrRow1  = piBlueErrRow1  + iErrRowSize;
   piBlackCurRow   = piBlueCurRow   + iErrRowSize;

   if (iColorTech_d == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       piLCErrRow1     = piBlackErrRow1 + iErrRowSize;
       piLCCurRow      = piBlackCurRow  + iErrRowSize;
       piLMErrRow1     = piLCErrRow1     + iErrRowSize;
       piLMCurRow      = piLCCurRow     + iErrRowSize;
       memset ((PBYTE)(piRedErrRow1 - (CMYK_FILTER_OVERFLOW / 2)), 0, (size_t)(6 * 4 * iErrRowSize));
   }
   else // 3rowsforColors * 4bytesPerPel(i.e. a Long) * rowSizeInPels
   {
       memset ((PBYTE)(piRedErrRow1 - (CMYK_FILTER_OVERFLOW / 2)), 0, (size_t)(4 * 4 * iErrRowSize));
   }

   i = 0;

   pidx1 = (PLONG)&lidx1_d;
   pidx2 = (PLONG)&lidx2_d;
   lbig  = iBig_d;
   plmix = (PLONG)&iMix_d[0];

   switch  (params.iPelSize)
   {
#ifdef ENABLE124
   case 1:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

         SNAP (pColor->is[(LONG)(*pbSrc & 0x80)>>7].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x80)>>7].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x80)>>7].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x40)>>6].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x40)>>6].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x40)>>6].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x20)>>5].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x20)>>5].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x20)>>5].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x10)>>4].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x10)>>4].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x10)>>4].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x8)>>3].bRed,    lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x8)>>3].bGreen,  lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x8)>>3].bBlue,   lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x4)>>2].bRed,    lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x4)>>2].bGreen,  lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x4)>>2].bBlue,   lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x2)>>1].bRed,    lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x2)>>1].bGreen,  lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x2)>>1].bBlue,   lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 1)].bRed,         lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 1)].bGreen,       lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 1)].bBlue,        lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         INC(1)
         TESTROWEND
      }
      break;
   }

   case 2:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

         SNAP (pColor->is[(LONG)(*pbSrc & 0xC0)>>6].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0xC0)>>6].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0xC0)>>6].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0x30)>>4].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x30)>>4].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0x30)>>4].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 0xC)>>2].bRed,    lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0xC)>>2].bGreen,  lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 0xC)>>2].bBlue,   lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(LONG)(*pbSrc & 3)].bRed,         lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 3)].bGreen,       lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(LONG)(*pbSrc & 3)].bBlue,        lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)

         INC(1)
         TESTROWEND
      }
      break;
   }

   case 4:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

         SNAP (pColor->is[(ULONG)((*pbSrc & 0xf0)>>4)].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(ULONG)((*pbSrc & 0xf0)>>4)].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(ULONG)((*pbSrc & 0xf0)>>4)].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         i++;
         TESTROWEND
         SNAP (pColor->is[(ULONG)(*pbSrc & 0x0f)].bRed,        lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pColor->is[(ULONG)(*pbSrc & 0x0f)].bGreen,      lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pColor->is[(ULONG)(*pbSrc & 0x0f)].bBlue,       lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
         INC(1)
         TESTROWEND
      }
      break;
   }

#endif
   case 8:
   {
      if (DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d)
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

            SNAP (pColor->is[(ULONG)*pbSrc].bRed,      lRed,    Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
            SNAP (pColor->is[(ULONG)*pbSrc].bGreen,    lGreen,  Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
            SNAP (pColor->is[(ULONG)*pbSrc].bBlue,     lBlue,   Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
            SNAP (pColor->is[(ULONG)*pbSrc].fcOptions, lBlack,  Black,    pbKDest, piBlackErrRow1, piBlackCurRow, iBlackLast1, lcoeff1)
            INC(1)
            TESTROWEND
         }
      }
      else
      {
         while ((ULONG)pbSrc < (ULONG)pbMapEnd)
         {
            GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

            SNAP (pColor->is[(ULONG)*pbSrc].bRed,   lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
            SNAP (pColor->is[(ULONG)*pbSrc].bGreen, lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
            SNAP (pColor->is[(ULONG)*pbSrc].bBlue,  lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
            INC(1)
            TESTROWEND
         }
      }
      break;
   }

   case 16:
   {
      while ((ULONG)pbSrc < (ULONG)pbMapEnd)
      {
         GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

         tToCMYK.bR = (*pbSrc & 0xf8);
         tToCMYK.bG = (((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3));
         tToCMYK.bB = ((*(pbSrc+1) & 0x1f) << 5);

         if (!fDataInRGB_d)
            SWAP (tToCMYK.bR, tToCMYK.bB);

         ToCMYK(&tToCMYK);

         SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
         SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
         SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)

         INC(2)
         TESTROWEND
      }
      break;
   }

   case 24:
   {
      if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
      {
          while ((ULONG)pbSrc < (ULONG)pbMapEnd)
          {

             GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)


             if(isNotWhite(pbSrc, &tToCMYK))
             {
                 ToCMYK6 (&tToCMYK, pbLightTable_d, pbDarkTable_d);
             }
             else
             {
                 tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lLC = tToCMYK.lLM = tToCMYK.lK = 0;
             }

             SNAP (pbRGamma_d[tToCMYK.lC],  lRed,    Cyan,     pbCDest,  piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
             SNAP (pbGGamma_d[tToCMYK.lM],  lGreen,  Magenta,  pbMDest,  piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
             SNAP (pbBGamma_d[tToCMYK.lY],  lBlue,   Yellow,   pbYDest,  piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
             SNAP (pbKGamma_d[tToCMYK.lK],  lBlack,  Black,    pbKDest,  piBlackErrRow1, piBlackCurRow, iBlackLast1, lcoeff1)
             SNAP (pbRGamma_d[tToCMYK.lLM], lGreen, LMagenta, pbLMDest, piLMErrRow1,    piLCCurRow,    iLMLast1,    lcoeff1)
             SNAP (pbGGamma_d[tToCMYK.lLC], lRed,   LCyan,    pbLCDest, piLCErrRow1,    piLMCurRow,    iLCLast1,    lcoeff1)

             INC(3);

             TESTROWEND
          }
      }
      else
      {
          if (DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d)
          {
             while ((ULONG)pbSrc < (ULONG)pbMapEnd)
             {
                GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

                if(isNotWhite(pbSrc, &tToCMYK))
                {
                    ToCMYK(&tToCMYK);
                }
                else
                {
                    tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = tToCMYK.lK = 0;
                }

                SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,    pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
                SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta, pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
                SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,  pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)
                SNAP (pbKGamma_d[tToCMYK.lK], lBlack, Black,   pbKDest, piBlackErrRow1, piBlackCurRow, iBlackLast1, lcoeff1)

                INC(3);

                TESTROWEND
             }
          }
          else
          {
             while ((ULONG)pbSrc < (ULONG)pbMapEnd)
             {
                GET_RANDOM_COEFFICIENT ((*pidx1), (*pidx2), plmix, ltemp, lbig, lcoeff1)

                if(isNotWhite(pbSrc, &tToCMYK))
                {
                    ToCMYK(&tToCMYK);
                }
                else
                {
                    tToCMYK.lC = tToCMYK.lM = tToCMYK.lY = 0;
                }

                SNAP (pbRGamma_d[tToCMYK.lC], lRed,   Cyan,     pbCDest, piRedErrRow1,   piRedCurRow,   iRedLast1,   lcoeff1)
                SNAP (pbGGamma_d[tToCMYK.lM], lGreen, Magenta,  pbMDest, piGreenErrRow1, piGreenCurRow, iGreenLast1, lcoeff1)
                SNAP (pbBGamma_d[tToCMYK.lY], lBlue,  Yellow,   pbYDest, piBlueErrRow1,  piBlueCurRow,  iBlueLast1,  lcoeff1)

                INC(3);

                TESTROWEND
             }
          }
      }
      break;
   }

   default:
      return INVALID_BITMAP;
   }

   iRowCount_d = y;

   if (  (iColorTech_d == DevicePrintMode::COLOR_TECH_CMYK)
      && fRemoveBlackColor_d
      )
   {
      GplCMYRemoval (pbmi2);
   }

   return 0;

#undef SNAP
#undef SETBIT
#undef INC
#undef INC1
#undef TESTROWEND
}

INT vBlackReduction     = 75;    //  75 %
INT vSaturationIncrease = 125;   // 125 %

#ifdef DESKJET_TEST_SEPARATE_HSV_CMYRGB

/****************************************************************************/
/* PROCEDURE NAME : GetHSVValues                                            */
/* AUTHOR         : Mark Hamzy                                              */
/* DATE WRITTEN   : 04/16/98                                                */
/* DESCRIPTION    :                                                         */
/*                                                                          */
/* This routine will take an RGB value and convert it into its HSV (Hue,    */
/* Saturation, and Value) value.  This will then return the percentage of   */
/* Color1, Color2, Black and White.                                         */
/* The colors will be either K(black)W(white)R(red)G(green)B(blue)C(cyan)   */
/* M(magenta) or Y(yellow).                                                 */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PRGB           prgb      - input RGB value              */
/*                  PBYTE          pbRGamma  - input Red gamma table        */
/*                  PBYTE          pbGGamma  - input Green gamma table      */
/*                  PBYTE          pbBGamma  - input Blue gamma table       */
/*                  PHSVVALUES     pRet      - output return value          */
/*                  PHSVCACHE      phsvCache - i/o cache of HSV values      */
/*                                                                          */
/* RETURN VALUES:   void                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
void
GetHSVValues (PRGB       prgb,
              PBYTE      pbRGamma,
              PBYTE      pbGGamma,
              PBYTE      pbBGamma,
              PHSVVALUES pRet,
              PHSVCACHE  phsvCache)
{
   BYTE         bRed;
   BYTE         bGreen;
   BYTE         bBlue;
   INT          iMax;
   INT          iMin;
   ULONG        ulValue;
   float        flSaturation;
   float        flHue;
   float        flDelta;
// float        flGrey;
   float        flBlackPercent;
   float        flBlack;
   float        flWhite;
   float        flColor;
   float        c1;
   float        c2;
   PHSVVALUES   pCacheUpdate;
   register INT i;

   bRed   = prgb->bRed;
   bGreen = prgb->bGreen;
   bBlue  = prgb->bBlue;

   if (pbRGamma) bRed   = pbRGamma[bRed];
   if (pbGGamma) bGreen = pbGGamma[bGreen];
   if (pbBGamma) bBlue  = pbBGamma[bBlue];

   // Is it in the cache?
   for (i = 0; i < MAX_HSV_CACHE; i++)
   {
      if (phsvCache->afhsvCacheValid[i])
      {
         if (  bRed   == phsvCache->ahsvCache[i].bRed
            && bGreen == phsvCache->ahsvCache[i].bGreen
            && bBlue  == phsvCache->ahsvCache[i].bBlue
            )
         {
            memcpy (pRet, &phsvCache->ahsvCache[i], sizeof (*pRet));
            return;
         }
      }
   }

   iMax = MAX3 (bRed, bGreen, bBlue);
   iMin = MIN3 (bRed, bGreen, bBlue);

   ulValue = iMax;

   if (0 != iMax)
      flSaturation = ((float)iMax - (float)iMin) / (float)iMax;
   else
      flSaturation = 0.0;

   if (iMax == iMin)
   {
      flHue = 0.0;
   }
   else
   {
      flDelta = iMax - iMin;

      if (bRed == iMax)
         flHue = (bGreen - bBlue) / flDelta;
      else if (bGreen == iMax)
         flHue = 2.0 + ((bBlue- bRed) / flDelta);
      else
         flHue = 4.0 + ((bRed- bGreen) / flDelta);

      flHue = flHue * 60.0;

      if (flHue < 0.0)
         flHue = flHue + 360.0;
   }

///flGrey = 1.0 - flSaturation;

   flBlackPercent = (255.0 - (float)ulValue) / 255.0;

   flBlack = flBlackPercent * 100.0;

#undef COOP_TEST
//#define COOP_TEST 1
#ifdef COOP_TEST
   flBlack = flBlack * ((float)vBlackReduction / 100.0);
#endif

   flColor = (100.0 - flBlack) * flSaturation;
   flWhite = 100.0 - (flBlack + flColor);

   if (flHue < 60.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Red";
      pRet->pszColor2 = "Yellow";
#endif
      pRet->eColor1   = CLR_RED;
      pRet->eColor2   = CLR_YELLOW;
      c2              = (flHue / 60.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 120.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Yellow";
      pRet->pszColor2 = "Green";
#endif
      pRet->eColor1   = CLR_YELLOW;
      pRet->eColor2   = CLR_GREEN;
      c2              = ((flHue - 60.0) / 60.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 180.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Green";
      pRet->pszColor2 = "Cyan";
#endif
      pRet->eColor1   = CLR_GREEN;
      pRet->eColor2   = CLR_CYAN;
      c2              = ((flHue - 120.0) / 60.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 240.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Cyan";
      pRet->pszColor2 = "Blue";
#endif
      pRet->eColor1   = CLR_CYAN;
      pRet->eColor2   = CLR_BLUE;
      c2              = ((flHue - 180.0) / 60.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 300.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Blue";
      pRet->pszColor2 = "Magenta";
#endif
      pRet->eColor1   = CLR_BLUE;
      pRet->eColor2   = CLR_MAGENTA;
      c2              = ((flHue - 240.0) / 60.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 360.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Magenta";
      pRet->pszColor2 = "Red";
#endif
      pRet->eColor1   = CLR_MAGENTA;
      pRet->eColor2   = CLR_RED;
      c2              = ((flHue - 300.0) / 60.0) * 100.0;
      c1              = 100.0 - c2;
   }

   c2 = c2 * flColor / 100.0;
   c1 = flColor - c2;


#ifdef COOP_TEST
   flSaturation *= ((float)vSaturationIncrease / 100.0);
#endif

   // Done! Update the return structure
#ifdef DEBUG
   pRet->flHue        = flHue;
   pRet->flValue      = (float)ulValue;
#endif

   pRet->flSaturation = flSaturation;
   pRet->flColor1     = c1;
   pRet->flColor2     = c2;
   pRet->flBlack      = flBlack;
   pRet->flWhite      = flWhite;

   pRet->bRed         = bRed;
   pRet->bGreen       = bGreen;
   pRet->bBlue        = bBlue;

   // Update the cache
   // First, find an empty cache entry
   pCacheUpdate = NULL;
   for (i = 0; i < MAX_HSV_CACHE; i++)
   {
      if (!phsvCache->afhsvCacheValid[i])
      {
         phsvCache->afhsvCacheValid[i] = true;
         pCacheUpdate = &phsvCache->ahsvCache[i];
         break;
      }
   }

   // If not, then use the least recently used one
   if (MAX_HSV_CACHE == i)
   {
      phsvCache->afhsvCacheValid[phsvCache->iLastEntry] = true;
      pCacheUpdate = &phsvCache->ahsvCache[phsvCache->iLastEntry];

      phsvCache->iLastEntry++;
      if (MAX_HSV_CACHE == phsvCache->iLastEntry)
         phsvCache->iLastEntry = 0;
   }

   if (pCacheUpdate)
   {
      memcpy (pCacheUpdate, pRet, sizeof (*pRet));
   }
}

#else

/****************************************************************************/
/* PROCEDURE NAME : GetHSVValues                                            */
/* AUTHOR         : Mark Hamzy                                              */
/* DATE WRITTEN   : 04/16/98                                                */
/* DESCRIPTION    :                                                         */
/*                                                                          */
/* This routine will take an RGB value and convert it into its HSV (Hue,    */
/* Saturation, and Value) value.  This will then return the percentage of   */
/* Color1, Color2, Black and White.                                         */
/* The colors will be either K(black)W(white)C(cyan)M(magenta) or Y(yellow) */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PRGB           prgb      - input RGB value              */
/*                  PBYTE          pbRGamma  - input Red gamma table        */
/*                  PBYTE          pbGGamma  - input Green gamma table      */
/*                  PBYTE          pbBGamma  - input Blue gamma table       */
/*                  PHSVVALUES     pRet      - output return value          */
/*                  PHSVCACHE      phsvCache - i/o cache of HSV values      */
/*                                                                          */
/* RETURN VALUES:   void                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
void
GetHSVValues (PRGB       prgb,
              PBYTE      pbRGamma,
              PBYTE      pbGGamma,
              PBYTE      pbBGamma,
              PHSVVALUES pRet,
              PHSVCACHE  phsvCache)
{
   BYTE         bRed;
   BYTE         bGreen;
   BYTE         bBlue;
   INT          iMax;
   INT          iMin;
   ULONG        ulValue;
   float        flSaturation;
   float        flHue;
   float        flDelta;
// float        flGrey;
   float        flBlackPercent;
   float        flBlack;
   float        flWhite;
   float        flColor;
   float        c1;
   float        c2 = 0;
   PHSVVALUES   pCacheUpdate;
   register INT i;

   bRed   = prgb->bRed;
   bGreen = prgb->bGreen;
   bBlue  = prgb->bBlue;

   if (pbRGamma) bRed   = pbRGamma[bRed];
   if (pbGGamma) bGreen = pbGGamma[bGreen];
   if (pbBGamma) bBlue  = pbBGamma[bBlue];

   // Is it in the cache?
   for (i = 0; i < MAX_HSV_CACHE; i++)
   {
      if (phsvCache->afhsvCacheValid[i])
      {
         if (  bRed   == phsvCache->ahsvCache[i].bRed
            && bGreen == phsvCache->ahsvCache[i].bGreen
            && bBlue  == phsvCache->ahsvCache[i].bBlue
            )
         {
            memcpy (pRet, &phsvCache->ahsvCache[i], sizeof (*pRet));
            return;
         }
      }
   }

   iMax = MAX3 (bRed, bGreen, bBlue);
   iMin = MIN3 (bRed, bGreen, bBlue);

   ulValue = iMax;

   if (0 != iMax)
      flSaturation = ((float)iMax - (float)iMin) / (float)iMax;
   else
      flSaturation = 0.0;

   if (iMax == iMin)
   {
      flHue = 0.0;
   }
   else
   {
      flDelta = iMax - iMin;

      if (bRed == iMax)
         flHue = (bGreen - bBlue) / flDelta;
      else if (bGreen == iMax)
         flHue = 2.0 + ((bBlue- bRed) / flDelta);
      else
         flHue = 4.0 + ((bRed- bGreen) / flDelta);

      flHue = flHue * 60.0;

      if (flHue < 0.0)
         flHue = flHue + 360.0;

      // Normalize it for Yellow at the top (or hue = 0)
      if (flHue < 60.0)
         flHue = 300.0 + flHue;
      else
         flHue -= 60.0;
   }

///flGrey = 1.0 - flSaturation;

   flBlackPercent = (255.0 - (float)ulValue) / 255.0;


   flBlack = flBlackPercent * 100.0;

#undef COOP_TEST
//#define COOP_TEST 1
#ifdef COOP_TEST
   flBlack = flBlack * ((float)vBlackReduction / 100.0);
#endif

   flColor = (100.0 - flBlack) * flSaturation;
   flWhite = 100.0 - (flBlack + flColor);

   if (flHue < 120.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Yellow";
      pRet->pszColor2 = "Cyan";
#endif
      pRet->eColor1   = CLR_YELLOW;
      pRet->eColor2   = CLR_CYAN;
      c2              = (flHue / 120.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 240.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Cyan";
      pRet->pszColor2 = "Magenta";
#endif
      pRet->eColor1   = CLR_CYAN;
      pRet->eColor2   = CLR_MAGENTA;
      c2              = ((flHue - 120.0) / 120.0) * 100.0;
      c1              = 100.0 - c2;
   }
   else if (flHue < 360.0)
   {
#ifdef DEBUG
      pRet->pszColor1 = "Magenta";
      pRet->pszColor2 = "Yellow";
#endif
      pRet->eColor1   = CLR_MAGENTA;
      pRet->eColor2   = CLR_YELLOW;
      c2              = ((flHue - 240.0) / 120.0) * 100.0;
      c1              = 100.0 - c2;
   }

   c2 = c2 * flColor / 100.0;
   c1 = flColor - c2;


#ifdef COOP_TEST
   flSaturation *= ((float)vSaturationIncrease / 100.0);
#endif

   // Done! Update the return structure
#ifdef DEBUG
   pRet->flHue        = flHue;
   pRet->flValue      = (float)ulValue;
#endif

   pRet->flSaturation = 100.0 * flSaturation;
   pRet->flColor1     = c1;
   pRet->flColor2     = c2;
   pRet->flBlack      = flBlack;
   pRet->flWhite      = flWhite;

   pRet->bRed         = bRed;
   pRet->bGreen       = bGreen;
   pRet->bBlue        = bBlue;

   // Update the cache
   // First, find an empty cache entry
   pCacheUpdate = NULL;
   for (i = 0; i < MAX_HSV_CACHE; i++)
   {
      if (!phsvCache->afhsvCacheValid[i])
      {
         phsvCache->afhsvCacheValid[i] = true;
         pCacheUpdate = &phsvCache->ahsvCache[i];
         break;
      }
   }

   // If not, then use the least recently used one
   if (MAX_HSV_CACHE == i)
   {
      phsvCache->afhsvCacheValid[phsvCache->iLastEntry] = true;
      pCacheUpdate = &phsvCache->ahsvCache[phsvCache->iLastEntry];

      phsvCache->iLastEntry++;
      if (MAX_HSV_CACHE == phsvCache->iLastEntry)
         phsvCache->iLastEntry = 0;
   }

   if (pCacheUpdate)
   {
      memcpy (pCacheUpdate, pRet, sizeof (*pRet));
   }
}

#endif


/****************************************************************************/
/* PROCEDURE NAME : GplHSVDiffusion  new error diffusion                    */
/* AUTHOR         : Mark Hamzy (code)                                       */
/* DATE WRITTEN   : 04/16/98                                                */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PDITHEREQUEST  pReq                                     */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplHSVDiffusion (PBITMAPINFO2 pbmi2,
                 PBYTE        pbSource)
{
#define INC          i++;                   \
                     if ((i & 7) == 0)      \
                     {                      \
                        pbCDest++;          \
                        pbMDest++;          \
                        pbYDest++;          \
                        pbKDest++;          \
                     }

#define TESTROWEND   if (i >= (LONG)pbmi2->cx)                                    \
                     {                                                            \
                        i = 0;                                                    \
                        pbCDest     = pbCDestNext;                                \
                        pbMDest     = pbMDestNext;                                \
                        pbYDest     = pbYDestNext;                                \
                        pbKDest     = pbKDestNext;                                \
                        pbCDestNext = pbCDest + iDestBitsPerPel_d * params.iDestRowSize; \
                        pbMDestNext = pbMDest + iDestBitsPerPel_d * params.iDestRowSize; \
                        pbYDestNext = pbYDest + iDestBitsPerPel_d * params.iDestRowSize; \
                        pbKDestNext = pbKDest + iDestBitsPerPel_d * params.iDestRowSize; \
                        pbSrc       = pbSrcNext;                                  \
                        pbSrcNext   = pbSrc + params.iSrcRowSize;                        \
                        y++;                                                      \
                        CLAMP_ROWS                                                \
                        if ((ULONG)pbSrc >= (ULONG)pbMapEnd)                      \
                           break;                                                 \
                     }

#if HSV_ERR_ALGORITHM == HSV_NONE_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                          \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;           \
                             t++)                                                 \
                        {                                                         \
                           pesStartRow1[t].flErrColor1 = 0.0;                     \
                           pesStartRow1[t].flErrColor2 = 0.0;                     \
                           pesStartRow1[t].flErrBlack  = 0.0;                     \
                           pesStartRow1[t].flErrWhite  = 0.0;                     \
                        }                                                         \
                        pesRow1 = pesStartRow1;
#endif

#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                   \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow2 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));                                    \
                        pesRow1 = pesStartRow1;                                               \
                        pesRow2 = pesStartRow2;
#endif

#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                   \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                           pesStartRow2[t].flErrColor1 = CLAMP (pesStartRow3[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                           pesStartRow2[t].flErrColor2 = CLAMP (pesStartRow3[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                           pesStartRow2[t].flErrBlack = CLAMP (pesStartRow3[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                           pesStartRow2[t].flErrWhite = CLAMP (pesStartRow3[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow3 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));                                    \
                        pesRow1 = pesStartRow1;                                               \
                        pesRow2 = pesStartRow2;                                               \
                        pesRow3 = pesStartRow3;
#endif

#if HSV_ERR_ALGORITHM == HSV_STU_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                   \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                           pesStartRow2[t].flErrColor1 = CLAMP (pesStartRow3[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                           pesStartRow2[t].flErrColor2 = CLAMP (pesStartRow3[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                           pesStartRow2[t].flErrBlack = CLAMP (pesStartRow3[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                           pesStartRow2[t].flErrWhite = CLAMP (pesStartRow3[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow3 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));                                    \
                        pesRow1 = pesStartRow1;                                               \
                        pesRow2 = pesStartRow2;                                               \
                        pesRow3 = pesStartRow3;
#endif

#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                   \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                           pesStartRow2[t].flErrColor1 = CLAMP (pesStartRow3[t].flErrColor1); \
                           pesStartRow3[t].flErrColor1 = CLAMP (pesStartRow4[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                           pesStartRow2[t].flErrColor2 = CLAMP (pesStartRow3[t].flErrColor2); \
                           pesStartRow3[t].flErrColor2 = CLAMP (pesStartRow4[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                           pesStartRow2[t].flErrBlack = CLAMP (pesStartRow3[t].flErrBlack);   \
                           pesStartRow3[t].flErrBlack = CLAMP (pesStartRow4[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                           pesStartRow2[t].flErrWhite = CLAMP (pesStartRow3[t].flErrWhite);   \
                           pesStartRow3[t].flErrWhite = CLAMP (pesStartRow4[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow4 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));                                       \
                        pesRow1 = pesStartRow1;                                               \
                        pesRow2 = pesStartRow2;                                               \
                        pesRow3 = pesStartRow3;                                               \
                        pesRow4 = pesStartRow4;
#endif

#define SETBIT(color, pdest, x) *pdest |= (BYTE)(0x80 >> (x & 7));     \
                                fEmpty##color##_d = false;

#define SETINTENSEBIT(color, pdest, cb, x, i) *pdest        |= (BYTE)(((i & 1) ? 0x80 : 0x00) >> (x & 7)); \
                                              *(pdest + cb) |= (BYTE)(((i & 2) ? 0x80 : 0x00) >> (x & 7)); \
                                              if (0 < i)                                                   \
                                                 fEmpty##color##_d = false;

#if HSV_ERR_ALGORITHM == HSV_NONE_ERR
#define ERROR_DIFFUSE(elm) (pesRow1 + 0)->elm = 0.0;
#endif

#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 16.0; \
                           flFrac3 = flFrac1 * 3.0;       \
                           flFrac5 = flFrac1 * 5.0;       \
                           flFrac7 = flFrac1 * 7.0;       \
                           (pesRow1 + 1)->elm += flFrac7; \
                                                          \
                           (pesRow2 - 1)->elm += flFrac3; \
                           (pesRow2 + 0)->elm += flFrac5; \
                           (pesRow2 + 1)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 48.0; \
                           flFrac3 = flFrac1 * 3.0;       \
                           flFrac5 = flFrac1 * 5.0;       \
                           flFrac7 = flFrac1 * 7.0;       \
                           (pesRow1 + 1)->elm += flFrac7; \
                           (pesRow1 + 2)->elm += flFrac5; \
                                                          \
                           (pesRow2 - 2)->elm += flFrac3; \
                           (pesRow2 - 1)->elm += flFrac5; \
                           (pesRow2 + 0)->elm += flFrac7; \
                           (pesRow2 + 1)->elm += flFrac5; \
                           (pesRow2 + 2)->elm += flFrac3; \
                                                          \
                           (pesRow3 - 2)->elm += flFrac1; \
                           (pesRow3 - 1)->elm += flFrac3; \
                           (pesRow3 + 0)->elm += flFrac5; \
                           (pesRow3 + 1)->elm += flFrac3; \
                           (pesRow3 + 2)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_STU_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 42.0; \
                           flFrac2 = flFrac1 * 2.0;       \
                           flFrac4 = flFrac1 * 4.0;       \
                           flFrac8 = flFrac1 * 8.0;       \
                           (pesRow1 + 1)->elm += flFrac8; \
                           (pesRow1 + 2)->elm += flFrac4; \
                                                          \
                           (pesRow2 - 2)->elm += flFrac2; \
                           (pesRow2 - 1)->elm += flFrac4; \
                           (pesRow2 + 0)->elm += flFrac8; \
                           (pesRow2 + 1)->elm += flFrac4; \
                           (pesRow2 + 2)->elm += flFrac2; \
                                                          \
                           (pesRow3 - 2)->elm += flFrac1; \
                           (pesRow3 - 1)->elm += flFrac2; \
                           (pesRow3 + 0)->elm += flFrac4; \
                           (pesRow3 + 1)->elm += flFrac2; \
                           (pesRow3 + 2)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
#define ERROR_DIFFUSE(elm) flFrac1  = pesRow1->elm / 200.0;      \
                           flFrac12 = flFrac1 * 12.0;            \
                           (pesRow1 + 2)->elm += flFrac1 * 32.0; \
                                                                 \
                           (pesRow2 - 3)->elm += flFrac12;       \
                           (pesRow2 - 1)->elm += flFrac1 * 26.0; \
                           (pesRow2 + 1)->elm += flFrac1 * 30.0; \
                           (pesRow2 + 3)->elm += flFrac1 * 16.0; \
                                                                 \
                           (pesRow3 - 2)->elm += flFrac12;       \
                           (pesRow3 + 0)->elm += flFrac1 * 26.0; \
                           (pesRow3 + 2)->elm += flFrac12;       \
                                                                 \
                           (pesRow4 - 3)->elm += flFrac1 *  5.0; \
                           (pesRow4 - 1)->elm += flFrac12;       \
                           (pesRow4 + 1)->elm += flFrac12;       \
                           (pesRow4 + 3)->elm += flFrac1 *  5.0;
#endif

#undef  CLAMP
#define CLAMP(elm)         ((0.0 > elm) ? 0.0 : ((100.0 < elm) ? 100.0 : elm))

   INT          i, y, t;


   PBYTE        pbMapEnd,
                pbSrc,
                pbSrcNext;
   PBYTE        pbKDestNext,
                pbKDest,
                pbCDestNext,
                pbCDest,
                pbMDestNext,
                pbMDest,
                pbYDestNext,
                pbYDest;
   ULONG        ulWidth         = iSrcRowPels_d;
   PARAMS       params;

   PHSVERRORSTRUC  pesRoot;
   PHSVERRORSTRUC  pesRow1, pesStartRow1;
   PHSVERRORSTRUC  pesRow2, pesStartRow2;
   PHSVERRORSTRUC  pesRow3, pesStartRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   PHSVERRORSTRUC  pesRow4, pesStartRow4;
#endif
   HSVVALUES       hsv;
   HSVCACHE        hsvCache;
   INT             eType;
   float           flHiVal;
#if HSV_ERR_ALGORITHM != HSV_NONE_ERR
   float           flFrac1;
#endif
#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
   float           flFrac3;
   float           flFrac5;
   float           flFrac7;
#endif
#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
   float           flFrac3;
   float           flFrac5;
   float           flFrac7;
#endif
#if HSV_ERR_ALGORITHM == HSV_STU_ERR
   float           flFrac2;
   float           flFrac4;
   float           flFrac8;
#endif
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   float           flFrac12;
#endif
   PCOLR           pColor          = 0;
   RGB             rgb;
   PRGB            pRGB            = &rgb;
   INT             iIntensityLevel = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   memset (&hsvCache, 0, sizeof (hsvCache));

   params.bMultiBitEnabled = false;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

   if (params.iPelSize < 16)
      pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   y = iRowCount_d;
   i = 0;

   pesRoot      = (PHSVERRORSTRUC)pfErrRow_d;
   pesStartRow1 = NUM_HSV_ERR_NEIGHBORS + pesRoot;
   pesStartRow2 = pesStartRow1 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
   pesStartRow3 = pesStartRow2 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   pesStartRow4 = pesStartRow3 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
#endif

   pesRow1 = pesStartRow1;
   pesRow2 = pesStartRow2;
   pesRow3 = pesStartRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   pesRow4 = pesStartRow4;
#endif

   while ((ULONG)pbSrc < (ULONG)pbMapEnd)
   {
      if (24 == params.iPelSize)
      {
         if (fDataInRGB_d)
         {
            pRGB = (PRGB)pbSrc;
         }
         else
         {
            rgb.bRed   = *(pbSrc+2);
            rgb.bGreen = *(pbSrc+1);
            rgb.bBlue  = *pbSrc;
         }

         pbSrc += sizeof (RGB);
      }
      else if (16 == params.iPelSize)
      {
         rgb.bRed   = *pbSrc & 0xf8;
         rgb.bGreen = ((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3);
         rgb.bBlue  = (*(pbSrc+1) & 0x1f) << 5;

         if (!fDataInRGB_d)
         {
            BYTE bSwap = rgb.bRed;
            rgb.bRed   = rgb.bGreen;
            rgb.bGreen = bSwap;
         }

         pbSrc += 2;
      }
      else if (8 == params.iPelSize)
      {
         rgb.bRed   = pColor->is[*pbSrc].bRed;
         rgb.bGreen = pColor->is[*pbSrc].bGreen;
         rgb.bBlue  = pColor->is[*pbSrc].bBlue;

         pbSrc++;
      }
#ifdef ENABLE124
      else if (4 == params.iPelSize)
      {
         if (0 == (i & 1))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0xF0) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0xF0) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0xF0) >> 4].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x0F)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x0F)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x0F)].bBlue;

            pbSrc++;
         }
      }
      else if (2 == params.iPelSize)
      {
         if (0 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0xC0) >> 6].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0xC0) >> 6].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0xC0) >> 6].bBlue;
         }
         else if (1 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x30) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x30) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x30) >> 4].bBlue;
         }
         else if (2 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x0C) >> 2].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x0C) >> 2].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x0C) >> 2].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x03)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x03)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x03)].bBlue;

            pbSrc++;
         }
      }
      else if (1 == params.iPelSize)
      {
         if (0 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x80) >> 7].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x80) >> 7].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x80) >> 7].bBlue;
         }
         else if (1 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x40) >> 6].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x40) >> 6].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x40) >> 6].bBlue;
         }
         else if (2 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x20) >> 5].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x20) >> 5].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x20) >> 5].bBlue;
         }
         else if (3 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x10) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x10) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x10) >> 4].bBlue;
         }
         else if (4 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x08) >> 3].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x08) >> 3].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x08) >> 3].bBlue;
         }
         else if (5 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x04) >> 2].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x04) >> 2].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x04) >> 2].bBlue;
         }
         else if (6 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x02) >> 1].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x02) >> 1].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x02) >> 1].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x01)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x01)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x01)].bBlue;

            pbSrc++;
         }
      }
#endif
      else
         return INVALID_BITMAP;

      // This returns the % of Color1, Color2, Black and White
      // The colors will be either K(black)W(white)R(red)G(green)
      // B(blue)C(cyan)M(magenta) or Y(yellow)
      GetHSVValues (pRGB,
                    pbRGamma_d,
                    pbGGamma_d,
                    pbBGamma_d,
                    &hsv,
                    &hsvCache);

      // Now add the percentages to the error storage
      pesRow1->flErrColor1 += hsv.flColor1;
      pesRow1->flErrColor2 += hsv.flColor2;
      pesRow1->flErrBlack  += hsv.flBlack;
      pesRow1->flErrWhite  += hsv.flWhite;

      // Now get the major color
      flHiVal = pesRow1->flErrColor1;
      eType   = hsv.eColor1;

      if (flHiVal < pesRow1->flErrColor2) { flHiVal = pesRow1->flErrColor2; eType = hsv.eColor2; }
      if (flHiVal < pesRow1->flErrBlack)  { flHiVal = pesRow1->flErrBlack;  eType = CLR_BLACK;   }
      if (flHiVal < pesRow1->flErrWhite)  { flHiVal = pesRow1->flErrWhite;  eType = CLR_WHITE;   }

      if (1 == iDestBitsPerPel_d)
      {
         // Not used but what the heck
         iIntensityLevel = 3;
      }
      else if (2 == iDestBitsPerPel_d)
      {
         if (CLR_BLACK == eType)
         {
            if (33.3 > hsv.flBlack)
               iIntensityLevel = 1;
            else if (66.6 > hsv.flBlack)
               iIntensityLevel = 2;
            else
               iIntensityLevel = 3;
         }
         else if (CLR_WHITE == eType)
         {
            // Not used but what the heck
            iIntensityLevel = 3;
         }
         else
         {
            if (33.3 > hsv.flSaturation)
               iIntensityLevel = 1;
            else if (66.6 > hsv.flSaturation)
               iIntensityLevel = 2;
            else
               iIntensityLevel = 3;
         }
      }

      // Now write the Major color to the CMYK Planes
      switch (eType)
      {
      case CLR_RED:
      {
         if (1 == iDestBitsPerPel_d)
         {
            SETBIT        (Magenta,  pbMDest, i);
            SETBIT        (Yellow,   pbYDest, i);
         }
         else
         {
            SETINTENSEBIT (Magenta,  pbMDest, params.iDestRowSize, i, iIntensityLevel);
            SETINTENSEBIT (Yellow,   pbYDest, params.iDestRowSize, i, iIntensityLevel);
         }
         break;
      }

      case CLR_GREEN:
      {
         if (1 == iDestBitsPerPel_d)
         {
            SETBIT        (Cyan,     pbCDest, i);
            SETBIT        (Yellow,   pbYDest, i);
         }
         else
         {
            SETINTENSEBIT (Cyan,     pbCDest, params.iDestRowSize, i, iIntensityLevel);
            SETINTENSEBIT (Yellow,   pbYDest, params.iDestRowSize, i, iIntensityLevel);
         }
         break;
      }

      case CLR_BLUE:
      {
         if (1 == iDestBitsPerPel_d)
         {
            SETBIT        (Cyan,     pbCDest, i);
            SETBIT        (Magenta,  pbMDest, i);
         }
         else
         {
            SETINTENSEBIT (Cyan,     pbCDest, params.iDestRowSize, i, iIntensityLevel);
            SETINTENSEBIT (Magenta,  pbMDest, params.iDestRowSize, i, iIntensityLevel);
         }
         break;
      }

      case CLR_CYAN:
      {
         if (1 == iDestBitsPerPel_d)
         {
            SETBIT        (Cyan,     pbCDest, i);
         }
         else
         {
            SETINTENSEBIT (Cyan,     pbCDest, params.iDestRowSize, i, iIntensityLevel);
         }
         break;
      }

      case CLR_MAGENTA:
      {
         if (1 == iDestBitsPerPel_d)
         {
            SETBIT        (Magenta,  pbMDest, i);
         }
         else
         {
            SETINTENSEBIT (Magenta,  pbMDest, params.iDestRowSize, i, iIntensityLevel);
         }
         break;
      }

      case CLR_YELLOW:
      {
         if (1 == iDestBitsPerPel_d)
         {
            SETBIT        (Yellow,   pbYDest, i);
         }
         else
         {
            SETINTENSEBIT (Yellow,   pbYDest, params.iDestRowSize, i, iIntensityLevel);
         }
         break;
      }

      case CLR_WHITE:
      {
         break;
      }

      case CLR_BLACK:
      {
         if (1 == iDestBitsPerPel_d)
         {
            if (DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d)
            {
               SETBIT        (Black,    pbKDest, i);
            }
            else
            {
               SETBIT        (Cyan,     pbCDest, i);
               SETBIT        (Magenta,  pbMDest, i);
               SETBIT        (Yellow,   pbYDest, i);
            }
         }
         else
         {
            if (DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d)
            {
///////////////// At certain intensity levels, switch from pure to composite black
///////////////if (  1 == iIntensityLevel
///////////////   || 2 == iIntensityLevel
///////////////   )
///////////////{
///////////////   // Draw the black pels as composite black
///////////////   SETINTENSEBIT (Cyan,     pbKDest, iDestRowSize, i, iIntensityLevel);
///////////////   SETINTENSEBIT (Magenta,  pbKDest, iDestRowSize, i, iIntensityLevel);
///////////////   SETINTENSEBIT (Yellow,   pbKDest, iDestRowSize, i, iIntensityLevel);
///////////////}
///////////////else
///////////////{
                  // Draw the black pels as pure black
                  SETINTENSEBIT (Black,    pbKDest, params.iDestRowSize, i, iIntensityLevel);
///////////////}
            }
            else
            {
               SETINTENSEBIT (Cyan,     pbCDest, params.iDestRowSize, i, iIntensityLevel);
               SETINTENSEBIT (Magenta,  pbMDest, params.iDestRowSize, i, iIntensityLevel);
               SETINTENSEBIT (Yellow,   pbYDest, params.iDestRowSize, i, iIntensityLevel);
            }
         }
         break;
      }
      }

      if (flHiVal == pesRow1->flErrColor1) pesRow1->flErrColor1 *= -1.0;
      if (flHiVal == pesRow1->flErrColor2) pesRow1->flErrColor2 *= -1.0;
      if (flHiVal == pesRow1->flErrBlack)  pesRow1->flErrBlack  *= -1.0;
      if (flHiVal == pesRow1->flErrWhite)  pesRow1->flErrWhite  *= -1.0;

      // Take each error value and multply by above tables to spread error
      // Calculate the error for each error value.
      ERROR_DIFFUSE (flErrColor1);
      ERROR_DIFFUSE (flErrColor2);
      ERROR_DIFFUSE (flErrBlack);
      ERROR_DIFFUSE (flErrWhite);

      pesRow1++;
      pesRow2++;
      pesRow3++;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
      pesRow4++;
#endif

      INC

      TESTROWEND
   }

   iRowCount_d = y;

   return 0;

#undef INC
#undef TESTROWEND
#undef CLAMP_ROWS
#undef SETBIT
#undef SETINTENSEBIT
#undef ERROR_DIFFUSE
#undef CLAMP
}

/****************************************************************************/
/* PROCEDURE NAME : GplHSVBidiffusion new error diffusion                   */
/* AUTHOR         : Mark Hamzy                                              */
/* DATE WRITTEN   : 04/16/98                                                */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PDITHEREQUEST  pReq                                     */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplHSVBidiffusion (PBITMAPINFO2 pbmi2,
                   PBYTE        pbSource)
{
#define INC          if (0 == (y & 1))      \
                     {                      \
                        i++;                \
                        if (0 == (i & 7))   \
                        {                   \
                           pbCDest++;       \
                           pbMDest++;       \
                           pbYDest++;       \
                           pbKDest++;       \
                        }                   \
                                            \
                        pesRow1++;          \
                        pesRow2++;          \
                        pesRow3++;          \
                     }                      \
                     else                   \
                     {                      \
                        if (0 == (i & 7))   \
                        {                   \
                           pbCDest--;       \
                           pbMDest--;       \
                           pbYDest--;       \
                           pbKDest--;       \
                        }                   \
                        i--;                \
                                            \
                        pesRow1--;          \
                        pesRow2--;          \
                        pesRow3--;          \
                     }

#define TESTROWEND   if (  i >= (LONG)pbmi2->cx                          \
                        || 0 >= i                                        \
                        )                                                \
                     {                                                   \
                        y++;                                             \
                        if (0 == (y & 1))                                \
                        {                                                \
                           i = 0;                                        \
                           pbCDest     = pbCDestNext;                    \
                           pbCDestNext = pbCDest + params.iDestRowSize;         \
                           pbMDest     = pbMDestNext;                    \
                           pbMDestNext = pbMDest + params.iDestRowSize;         \
                           pbYDest     = pbYDestNext;                    \
                           pbYDestNext = pbYDest + params.iDestRowSize;         \
                           pbKDest     = pbKDestNext;                    \
                           pbKDestNext = pbKDest + params.iDestRowSize;         \
                           pbSrc       = pbSrcNext;                      \
                           pbSrcNext   = pbSrcNext + params.iSrcRowSize;        \
                        }                                                \
                        else                                             \
                        {                                                \
                           i = (LONG)pbmi2->cx - 1;                      \
                           pbCDest     = pbCDestNext - 1;                \
                           pbCDestNext = pbCDestNext + params.iDestRowSize;     \
                           pbMDest     = pbMDestNext - 1;                \
                           pbMDestNext = pbMDestNext + params.iDestRowSize;     \
                           pbYDest     = pbYDestNext - 1;                \
                           pbYDestNext = pbYDestNext + params.iDestRowSize;     \
                           pbKDest     = pbKDestNext - 1;                \
                           pbKDestNext = pbKDestNext + params.iDestRowSize;     \
                           pbSrc       = pbSrcNext - (LONG)pbmi2->cBitCount / 8; \
                           pbSrcNext   = pbSrcNext + params.iSrcRowSize;        \
                        }                                                \
                        CLAMP_ROWS                                       \
                        if ((ULONG)pbSrc >= (ULONG)pbMapEnd)             \
                           break;                                        \
                     }

#if HSV_ERR_ALGORITHM == HSV_NONE_ERR
#define CLAMP_ROWS      if (0 == (y & 1))                                                     \
                        {                                                                     \
                           pesRow1 = pesStartRow1;                                            \
                        }                                                                     \
                        else                                                                  \
                        {                                                                     \
                           pesRow1 = pesEndRow1;                                              \
                        }                                                                     \
                        for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                   \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = 0.0;                                 \
                           pesStartRow1[t].flErrColor2 = 0.0;                                 \
                           pesStartRow1[t].flErrBlack  = 0.0;                                 \
                           pesStartRow1[t].flErrWhite  = 0.0;                                 \
                        }
#endif

#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
#define CLAMP_ROWS      if (0 == (y & 1))                                                     \
                        {                                                                     \
                           pesRow1 = pesStartRow1;                                            \
                           pesRow2 = pesStartRow2;                                            \
                        }                                                                     \
                        else                                                                  \
                        {                                                                     \
                           pesRow1 = pesEndRow1;                                              \
                           pesRow2 = pesEndRow2;                                              \
                        }                                                                     \
                        for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                       \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow2 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));
#endif

#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
#define CLAMP_ROWS      if (0 == (y & 1))                                                     \
                        {                                                                     \
                           pesRow1 = pesStartRow1;                                            \
                           pesRow2 = pesStartRow2;                                            \
                           pesRow3 = pesStartRow3;                                            \
                        }                                                                     \
                        else                                                                  \
                        {                                                                     \
                           pesRow1 = pesEndRow1;                                              \
                           pesRow2 = pesEndRow2;                                              \
                           pesRow3 = pesEndRow3;                                              \
                        }                                                                     \
                        for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                       \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                           pesStartRow2[t].flErrColor1 = CLAMP (pesStartRow3[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                           pesStartRow2[t].flErrColor2 = CLAMP (pesStartRow3[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                           pesStartRow2[t].flErrBlack = CLAMP (pesStartRow3[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                           pesStartRow2[t].flErrWhite = CLAMP (pesStartRow3[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow3 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));
#endif

#if HSV_ERR_ALGORITHM == HSV_STU_ERR
#define CLAMP_ROWS      if (0 == (y & 1))                                                     \
                        {                                                                     \
                           pesRow1 = pesStartRow1;                                            \
                           pesRow2 = pesStartRow2;                                            \
                           pesRow3 = pesStartRow3;                                            \
                        }                                                                     \
                        else                                                                  \
                        {                                                                     \
                           pesRow1 = pesEndRow1;                                              \
                           pesRow2 = pesEndRow2;                                              \
                           pesRow3 = pesEndRow3;                                              \
                        }                                                                     \
                        for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                       \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                           pesStartRow2[t].flErrColor1 = CLAMP (pesStartRow3[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                           pesStartRow2[t].flErrColor2 = CLAMP (pesStartRow3[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                           pesStartRow2[t].flErrBlack = CLAMP (pesStartRow3[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                           pesStartRow2[t].flErrWhite = CLAMP (pesStartRow3[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow3 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));
#endif

#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
#define CLAMP_ROWS      if (0 == (y & 1))                                                     \
                        {                                                                     \
                           pesRow1 = pesStartRow1;                                            \
                           pesRow2 = pesStartRow2;                                            \
                           pesRow3 = pesStartRow3;                                            \
                           pesRow4 = pesStartRow4;                                            \
                        }                                                                     \
                        else                                                                  \
                        {                                                                     \
                           pesRow1 = pesEndRow1;                                              \
                           pesRow2 = pesEndRow2;                                              \
                           pesRow3 = pesEndRow3;                                              \
                           pesRow4 = pesEndRow4;                                              \
                        }                                                                     \
                        for (t = -NUM_HSV_ERR_NEIGHBORS;                                      \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                       \
                             t++)                                                             \
                        {                                                                     \
                           pesStartRow1[t].flErrColor1 = CLAMP (pesStartRow2[t].flErrColor1); \
                           pesStartRow2[t].flErrColor1 = CLAMP (pesStartRow3[t].flErrColor1); \
                           pesStartRow3[t].flErrColor1 = CLAMP (pesStartRow4[t].flErrColor1); \
                                                                                              \
                           pesStartRow1[t].flErrColor2 = CLAMP (pesStartRow2[t].flErrColor2); \
                           pesStartRow2[t].flErrColor2 = CLAMP (pesStartRow3[t].flErrColor2); \
                           pesStartRow3[t].flErrColor2 = CLAMP (pesStartRow4[t].flErrColor2); \
                                                                                              \
                           pesStartRow1[t].flErrBlack = CLAMP (pesStartRow2[t].flErrBlack);   \
                           pesStartRow2[t].flErrBlack = CLAMP (pesStartRow3[t].flErrBlack);   \
                           pesStartRow3[t].flErrBlack = CLAMP (pesStartRow4[t].flErrBlack);   \
                                                                                              \
                           pesStartRow1[t].flErrWhite = CLAMP (pesStartRow2[t].flErrWhite);   \
                           pesStartRow2[t].flErrWhite = CLAMP (pesStartRow3[t].flErrWhite);   \
                           pesStartRow3[t].flErrWhite = CLAMP (pesStartRow4[t].flErrWhite);   \
                        }                                                                     \
                        memset (pesStartRow4 - NUM_HSV_ERR_NEIGHBORS,                         \
                                0,                                                            \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                         \
                                * sizeof (HSVERRORSTRUC));
#endif

#define SETBIT(color, pdest, x) *pdest |= (BYTE)(0x80 >> (x & 7));     \
                                fEmpty##color##_d = false;

#if HSV_ERR_ALGORITHM == HSV_NONE_ERR
#define ERROR_DIFFUSE(elm) (pesRow1 + 0)->elm = 0.0;
#endif

#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 16.0; \
                           flFrac3 = flFrac1 * 3.0;       \
                           flFrac5 = flFrac1 * 5.0;       \
                           flFrac7 = flFrac1 * 7.0;       \
                           (pesRow1 + 1)->elm += flFrac7; \
                                                          \
                           (pesRow2 - 1)->elm += flFrac3; \
                           (pesRow2 + 0)->elm += flFrac5; \
                           (pesRow2 + 1)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 48.0; \
                           flFrac3 = flFrac1 * 3.0;       \
                           flFrac5 = flFrac1 * 5.0;       \
                           flFrac7 = flFrac1 * 7.0;       \
                           (pesRow1 + 1)->elm += flFrac7; \
                           (pesRow1 + 2)->elm += flFrac5; \
                                                          \
                           (pesRow2 - 2)->elm += flFrac3; \
                           (pesRow2 - 1)->elm += flFrac5; \
                           (pesRow2 + 0)->elm += flFrac7; \
                           (pesRow2 + 1)->elm += flFrac5; \
                           (pesRow2 + 2)->elm += flFrac3; \
                                                          \
                           (pesRow3 - 2)->elm += flFrac1; \
                           (pesRow3 - 1)->elm += flFrac3; \
                           (pesRow3 + 0)->elm += flFrac5; \
                           (pesRow3 + 1)->elm += flFrac3; \
                           (pesRow3 + 2)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_STU_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 42.0; \
                           flFrac2 = flFrac1 * 2.0;       \
                           flFrac4 = flFrac1 * 4.0;       \
                           flFrac8 = flFrac1 * 8.0;       \
                           (pesRow1 + 1)->elm += flFrac8; \
                           (pesRow1 + 2)->elm += flFrac4; \
                                                          \
                           (pesRow2 - 2)->elm += flFrac2; \
                           (pesRow2 - 1)->elm += flFrac4; \
                           (pesRow2 + 0)->elm += flFrac8; \
                           (pesRow2 + 1)->elm += flFrac4; \
                           (pesRow2 + 2)->elm += flFrac2; \
                                                          \
                           (pesRow3 - 2)->elm += flFrac1; \
                           (pesRow3 - 1)->elm += flFrac2; \
                           (pesRow3 + 0)->elm += flFrac4; \
                           (pesRow3 + 1)->elm += flFrac2; \
                           (pesRow3 + 2)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
#define ERROR_DIFFUSE(elm) flFrac1  = pesRow1->elm / 200.0;      \
                           flFrac12 = flFrac1 * 12.0;            \
                           (pesRow1 + 2)->elm += flFrac1 * 32.0; \
                                                                 \
                           (pesRow2 - 3)->elm += flFrac12;       \
                           (pesRow2 - 1)->elm += flFrac1 * 26.0; \
                           (pesRow2 + 1)->elm += flFrac1 * 30.0; \
                           (pesRow2 + 3)->elm += flFrac1 * 16.0; \
                                                                 \
                           (pesRow3 - 2)->elm += flFrac12;       \
                           (pesRow3 + 0)->elm += flFrac1 * 26.0; \
                           (pesRow3 + 2)->elm += flFrac12;       \
                                                                 \
                           (pesRow4 - 3)->elm += flFrac1 *  5.0; \
                           (pesRow4 - 1)->elm += flFrac12;       \
                           (pesRow4 + 1)->elm += flFrac12;       \
                           (pesRow4 + 3)->elm += flFrac1 *  5.0;
#endif

#undef  CLAMP
#define CLAMP(elm)         ((0.0 > elm) ? 0.0 : ((100.0 < elm) ? 100.0 : elm))

   INT          i, y, t;

   PBYTE        pbMapEnd,
                pbSrc,
                pbSrcNext;
   PBYTE        pbKDestNext,
                pbKDest,
                pbCDestNext,
                pbCDest,
                pbMDestNext,
                pbMDest,
                pbYDestNext,
                pbYDest;
   ULONG        ulWidth         = iSrcRowPels_d;
   PARAMS       params;

   PHSVERRORSTRUC  pesRoot;
   PHSVERRORSTRUC  pesRow1, pesStartRow1, pesEndRow1;
   PHSVERRORSTRUC  pesRow2, pesStartRow2, pesEndRow2;
   PHSVERRORSTRUC  pesRow3, pesStartRow3, pesEndRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   PHSVERRORSTRUC  pesRow4, pesStartRow4, pesEndRow4;
#endif
   HSVVALUES       hsv;
   HSVCACHE        hsvCache;
   INT             eType;
   float           flHiVal;
#if HSV_ERR_ALGORITHM != HSV_NONE_ERR
   float           flFrac1;
#endif
#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
   float           flFrac3;
   float           flFrac5;
   float           flFrac7;
#endif
#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
   float           flFrac3;
   float           flFrac5;
   float           flFrac7;
#endif
#if HSV_ERR_ALGORITHM == HSV_STU_ERR
   float           flFrac2;
   float           flFrac4;
   float           flFrac8;
#endif
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   float        flFrac12;
#endif
   PCOLR           pColor          = 0;
   RGB             rgb;
   PRGB            pRGB            = &rgb;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   memset (&hsvCache, 0, sizeof (hsvCache));

   y = iRowCount_d;
   i = 0;

   params.bMultiBitEnabled = false;

   SetInitialParameters(pbmi2, &params);

   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)params.iMapSize);

   if (params.iPelSize < 16)
      pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   pbSrc     = pbSource;
   pbSrcNext = pbSource + params.iSrcRowSize;

   // @TBD - fix
   if (1 == (y & 1))
   {
      pbCDest = pbCDestNext - 1;
      pbMDest = pbMDestNext - 1;
      pbYDest = pbYDestNext - 1;
      pbKDest = pbKDestNext - 1;
   }

   pbSrc       = pbSource;
   pbSrcNext   = pbSource + params.iSrcRowSize;
   if (1 == (y & 1))
      pbSrc = pbSrcNext - (LONG)pbmi2->cBitCount / 8;

   pesRoot      = (PHSVERRORSTRUC)pfErrRow_d;
   pesStartRow1 = 2 + pesRoot;
   pesEndRow1   =     pesRoot +     (ulWidth + 4) - 2;
   pesStartRow2 = 2 + pesRoot +     (ulWidth + 4);
   pesEndRow2   =     pesRoot + 2 * (ulWidth + 4) - 2;
   pesStartRow3 = 2 + pesRoot + 2 * (ulWidth + 4);
   pesEndRow3   =     pesRoot + 3 * (ulWidth + 4) - 2;

   pesRoot      = (PHSVERRORSTRUC)pfErrRow_d;
   pesStartRow1 = NUM_HSV_ERR_NEIGHBORS + pesRoot;
   pesEndRow1   = pesRoot + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS) - NUM_HSV_ERR_NEIGHBORS;
   pesStartRow2 = pesStartRow1 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
   pesEndRow2   = pesEndRow1 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
   pesStartRow3 = pesStartRow2 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
   pesEndRow3   = pesEndRow2 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   pesStartRow4 = pesStartRow3 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
   pesEndRow4   = pesEndRow3 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
#endif

   if (0 == (y & 1))
   {
      pesRow1 = pesStartRow1;
      pesRow2 = pesStartRow2;
      pesRow3 = pesStartRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
      pesRow4 = pesStartRow4;
#endif
   }
   else
   {
      pesRow1 = pesEndRow1;
      pesRow2 = pesEndRow2;
      pesRow3 = pesEndRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
      pesRow4 = pesEndRow4;
#endif
   }

   while ((ULONG)pbSrc < (ULONG)pbMapEnd)
   {
      if (24 == params.iPelSize)
      {
         if (fDataInRGB_d)
         {
            pRGB = (PRGB)pbSrc;
         }
         else
         {
            rgb.bRed   = *(pbSrc+2);
            rgb.bGreen = *(pbSrc+1);
            rgb.bBlue  = *pbSrc;
         }

         if (0 == (y & 1))
            pbSrc += sizeof (RGB);
         else
            pbSrc -= sizeof (RGB);
      }
      else if (16 == params.iPelSize)
      {
         rgb.bRed   = *pbSrc & 0xf8;
         rgb.bGreen = ((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3);
         rgb.bBlue  = (*(pbSrc+1) & 0x1f) << 5;

         if (!fDataInRGB_d)
         {
            BYTE bSwap = rgb.bRed;
            rgb.bRed   = rgb.bGreen;
            rgb.bGreen = bSwap;
         }

         pbSrc += 2;
      }
      else if (8 == params.iPelSize)
      {
         rgb.bRed   = pColor->is[*pbSrc].bRed;
         rgb.bGreen = pColor->is[*pbSrc].bGreen;
         rgb.bBlue  = pColor->is[*pbSrc].bBlue;

         pbSrc++;
      }
#ifdef ENABLE124
      else if (4 == params.iPelSize)
      {
         if (0 == (i & 1))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0xF0) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0xF0) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0xF0) >> 4].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x0F)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x0F)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x0F)].bBlue;

            pbSrc++;
         }
      }
      else if (2 == params.iPelSize)
      {
         if (0 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0xC0) >> 6].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0xC0) >> 6].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0xC0) >> 6].bBlue;
         }
         else if (1 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x30) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x30) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x30) >> 4].bBlue;
         }
         else if (2 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x0C) >> 2].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x0C) >> 2].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x0C) >> 2].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x03)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x03)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x03)].bBlue;

            pbSrc++;
         }
      }
      else if (1 == params.iPelSize)
      {
         if (0 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x80) >> 7].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x80) >> 7].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x80) >> 7].bBlue;
         }
         else if (1 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x40) >> 6].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x40) >> 6].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x40) >> 6].bBlue;
         }
         else if (2 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x20) >> 5].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x20) >> 5].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x20) >> 5].bBlue;
         }
         else if (3 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x10) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x10) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x10) >> 4].bBlue;
         }
         else if (4 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x08) >> 3].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x08) >> 3].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x08) >> 3].bBlue;
         }
         else if (5 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x04) >> 2].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x04) >> 2].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x04) >> 2].bBlue;
         }
         else if (6 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x02) >> 1].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x02) >> 1].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x02) >> 1].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x01)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x01)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x01)].bBlue;

            pbSrc++;
         }
      }
#endif
      else
         return INVALID_BITMAP;

      // This returns the % of Color1, Color2, Black and White
      // The colors will be either K(black)W(white)R(red)G(green)
      // B(blue)C(cyan)M(magenta) or Y(yellow)
      GetHSVValues (pRGB,
                    pbRGamma_d,
                    pbGGamma_d,
                    pbBGamma_d,
                    &hsv,
                    &hsvCache);

      // Now add the percentages to the error storage
      pesRow1->flErrColor1 += hsv.flColor1;
      pesRow1->flErrColor2 += hsv.flColor2;
      pesRow1->flErrBlack  += hsv.flBlack;
      pesRow1->flErrWhite  += hsv.flWhite;

      // Now get the major color
      flHiVal = pesRow1->flErrColor1;
      eType   = hsv.eColor1;

      if (flHiVal < pesRow1->flErrColor2) { flHiVal = pesRow1->flErrColor2; eType = hsv.eColor2; }
      if (flHiVal < pesRow1->flErrBlack)  { flHiVal = pesRow1->flErrBlack;  eType = CLR_BLACK;   }
      if (flHiVal < pesRow1->flErrWhite)  { flHiVal = pesRow1->flErrWhite;  eType = CLR_WHITE;   }

      // Now write the Major color to the CMYK Planes
      switch (eType)
      {
      case CLR_RED:
      {
         SETBIT (Magenta,  pbMDest, i);
         SETBIT (Yellow,   pbYDest, i);
         break;
      }

      case CLR_GREEN:
      {
         SETBIT (Cyan,     pbCDest, i);
         SETBIT (Yellow,   pbYDest, i);
         break;
      }

      case CLR_BLUE:
      {
         SETBIT (Cyan,     pbCDest, i);
         SETBIT (Magenta,  pbMDest, i);
         break;
      }

      case CLR_CYAN:
      {
         SETBIT (Cyan,     pbCDest, i);
         break;
      }

      case CLR_MAGENTA:
      {
         SETBIT (Magenta,  pbMDest, i);
         break;
      }

      case CLR_YELLOW:
      {
         SETBIT (Yellow,   pbYDest, i);
         break;
      }

      case CLR_WHITE:
      {
         break;
      }

      case CLR_BLACK:
      {
         if (DevicePrintMode::COLOR_TECH_CMYK == iColorTech_d)
         {
            SETBIT (Black,    pbKDest, i);
         }
         else
         {
            SETBIT (Cyan,     pbCDest, i);
            SETBIT (Magenta,  pbMDest, i);
            SETBIT (Yellow,   pbYDest, i);
         }
         break;
      }
      }

      if (flHiVal == pesRow1->flErrColor1) pesRow1->flErrColor1 *= -1.0;
      if (flHiVal == pesRow1->flErrColor2) pesRow1->flErrColor2 *= -1.0;
      if (flHiVal == pesRow1->flErrBlack)  pesRow1->flErrBlack  *= -1.0;
      if (flHiVal == pesRow1->flErrWhite)  pesRow1->flErrWhite  *= -1.0;

      // Take each error value and multply by above tables to spread error
      // Calculate the error for each error value.
      ERROR_DIFFUSE (flErrColor1);
      ERROR_DIFFUSE (flErrColor2);
      ERROR_DIFFUSE (flErrBlack);
      ERROR_DIFFUSE (flErrWhite);

      INC

      TESTROWEND
   }

   iRowCount_d = y;

   return 0;

#undef INC
#undef TESTROWEND
#undef CLAMP_ROWS
#undef SETBIT
#undef ERROR_DIFFUSE
#undef CLAMP
}

/****************************************************************************/
/* PROCEDURE NAME : GplCMYKDiffusion  new error diffusion                   */
/* AUTHOR         : Mark Hamzy                                              */
/* DATE WRITTEN   : 04/16/98                                                */
/* DESCRIPTION    : See Below                                               */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:      PDITHEREQUEST  pReq                                     */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int GplDitherInstance::
GplCMYKDiffusion (PBITMAPINFO2 pbmi2,
                  PBYTE        pbSource)
{
#define INC          i++;                   \
                     if ((i & 7) == 0)      \
                     {                      \
                        pbCDest++;          \
                        pbMDest++;          \
                        pbYDest++;          \
                        pbKDest++;          \
                     }

#define TESTROWEND   if (i >= (LONG)pbmi2->cx)                                    \
                     {                                                            \
                        i = 0;                                                    \
                        pbCDest     = pbCDestNext;                                \
                        pbMDest     = pbMDestNext;                                \
                        pbYDest     = pbYDestNext;                                \
                        pbKDest     = pbKDestNext;                                \
                        pbCDestNext = pbCDest + iDestBitsPerPel_d * iDestRowSize; \
                        pbMDestNext = pbMDest + iDestBitsPerPel_d * iDestRowSize; \
                        pbYDestNext = pbYDest + iDestBitsPerPel_d * iDestRowSize; \
                        pbKDestNext = pbKDest + iDestBitsPerPel_d * iDestRowSize; \
                        pbSrc       = pbSrcNext;                                  \
                        pbSrcNext   = pbSrc + iSrcRowSize;                        \
                        y++;                                                      \
                        CLAMP_ROWS                                                \
                        if ((ULONG)pbSrc >= (ULONG)pbMapEnd)                      \
                           break;                                                 \
                     }

#if HSV_ERR_ALGORITHM == HSV_NONE_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                              \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;               \
                             t++)                                                     \
                        {                                                             \
                           pesStartRow1[t].flKError = 0.0;                            \
                           pesStartRow1[t].flCError = 0.0;                            \
                           pesStartRow1[t].flMError = 0.0;                            \
                           pesStartRow1[t].flYError = 0.0;                            \
                        }                                                             \
                        pesRow1 = pesStartRow1;
#endif

#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                 \
                             t++)                                                       \
                        {                                                               \
                           pesStartRow1[t].flKError = CLAMP (pesStartRow2[t].flKError); \
                                                                                        \
                           pesStartRow1[t].flCError = CLAMP (pesStartRow2[t].flCError); \
                                                                                        \
                           pesStartRow1[t].flMError = CLAMP (pesStartRow2[t].flMError); \
                                                                                        \
                           pesStartRow1[t].flYError = CLAMP (pesStartRow2[t].flYError); \
                        }                                                               \
                        memset (pesStartRow2 - NUM_HSV_ERR_NEIGHBORS,                   \
                                0,                                                      \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                   \
                                * sizeof (CMYKERRORSTRUC));                             \
                        pesRow1 = pesStartRow1;                                         \
                        pesRow2 = pesStartRow2;
#endif

#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                 \
                             t++)                                                       \
                        {                                                               \
                           pesStartRow1[t].flKError = CLAMP (pesStartRow2[t].flKError); \
                           pesStartRow2[t].flKError = CLAMP (pesStartRow3[t].flKError); \
                                                                                        \
                           pesStartRow1[t].flCError = CLAMP (pesStartRow2[t].flCError); \
                           pesStartRow2[t].flCError = CLAMP (pesStartRow3[t].flCError); \
                                                                                        \
                           pesStartRow1[t].flMError = CLAMP (pesStartRow2[t].flMError); \
                           pesStartRow2[t].flMError = CLAMP (pesStartRow3[t].flMError); \
                                                                                        \
                           pesStartRow1[t].flYError = CLAMP (pesStartRow2[t].flYError); \
                           pesStartRow2[t].flYError = CLAMP (pesStartRow3[t].flYError); \
                        }                                                               \
                        memset (pesStartRow3 - NUM_HSV_ERR_NEIGHBORS,                   \
                                0,                                                      \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                   \
                                * sizeof (CMYKERRORSTRUC));                             \
                        pesRow1 = pesStartRow1;                                         \
                        pesRow2 = pesStartRow2;                                         \
                        pesRow3 = pesStartRow3;
#endif

#if HSV_ERR_ALGORITHM == HSV_STU_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                 \
                             t++)                                                       \
                        {                                                               \
                           pesStartRow1[t].flKError = CLAMP (pesStartRow2[t].flKError); \
                           pesStartRow2[t].flKError = CLAMP (pesStartRow3[t].flKError); \
                                                                                        \
                           pesStartRow1[t].flCError = CLAMP (pesStartRow2[t].flCError); \
                           pesStartRow2[t].flCError = CLAMP (pesStartRow3[t].flCError); \
                                                                                        \
                           pesStartRow1[t].flMError = CLAMP (pesStartRow2[t].flMError); \
                           pesStartRow2[t].flMError = CLAMP (pesStartRow3[t].flMError); \
                                                                                        \
                           pesStartRow1[t].flYError = CLAMP (pesStartRow2[t].flYError); \
                           pesStartRow2[t].flYError = CLAMP (pesStartRow3[t].flYError); \
                        }                                                               \
                        memset (pesStartRow3 - NUM_HSV_ERR_NEIGHBORS,                   \
                                0,                                                      \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                   \
                                * sizeof (CMYKERRORSTRUC));                             \
                        pesRow1 = pesStartRow1;                                         \
                        pesRow2 = pesStartRow2;                                         \
                        pesRow3 = pesStartRow3;
#endif

#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
#define CLAMP_ROWS      for (t = -NUM_HSV_ERR_NEIGHBORS;                                \
                             t < iSrcRowPels_d + NUM_HSV_ERR_NEIGHBORS;                 \
                             t++)                                                       \
                        {                                                               \
                           pesStartRow1[t].flKError = CLAMP (pesStartRow2[t].flKError); \
                           pesStartRow2[t].flKError = CLAMP (pesStartRow3[t].flKError); \
                           pesStartRow3[t].flKError = CLAMP (pesStartRow4[t].flKError); \
                                                                                        \
                           pesStartRow1[t].flCError = CLAMP (pesStartRow2[t].flCError); \
                           pesStartRow2[t].flCError = CLAMP (pesStartRow3[t].flCError); \
                           pesStartRow3[t].flCError = CLAMP (pesStartRow4[t].flCError); \
                                                                                        \
                           pesStartRow1[t].flMError = CLAMP (pesStartRow2[t].flMError); \
                           pesStartRow2[t].flMError = CLAMP (pesStartRow3[t].flMError); \
                           pesStartRow3[t].flMError = CLAMP (pesStartRow4[t].flMError); \
                                                                                        \
                           pesStartRow1[t].flYError = CLAMP (pesStartRow2[t].flYError); \
                           pesStartRow2[t].flYError = CLAMP (pesStartRow3[t].flYError); \
                           pesStartRow3[t].flYError = CLAMP (pesStartRow4[t].flYError); \
                        }                                                               \
                        memset (pesStartRow4 - NUM_HSV_ERR_NEIGHBORS,                   \
                                0,                                                      \
                                (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS)                   \
                                * sizeof (CMYKERRORSTRUC));                             \
                        pesRow1 = pesStartRow1;                                         \
                        pesRow2 = pesStartRow2;                                         \
                        pesRow3 = pesStartRow3;                                         \
                        pesRow4 = pesStartRow4;
#endif

#if 0
/* Ranges:
** 0     0     /   0
** 1   1 -  85 /  43
** 2  86 - 170 / 128
** 3 171 - 255 / 213
*/
#define DRAWCOLOR(v, e, color, pdest, cb, x) if (0 < v)                                   \
                                             {                                            \
                                                if (85 >= v)                              \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 1) \
                                                   fEmpty##color##_d = false;             \
                                                   e = v - 43;                            \
                                                }                                         \
                                                else if (170 >= v)                        \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 2) \
                                                   fEmpty##color##_d = false;             \
                                                   e = v - 128;                           \
                                                }                                         \
                                                else                                      \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 3) \
                                                   fEmpty##color##_d = false;             \
                                                   e = MIN (v, 255) - 213;                \
                                                }                                         \
                                             }                                            \
                                             else if (0 > v)                              \
                                             {                                            \
                                                e = 0;                                    \
                                             }
#elif 0
/* Ranges:
** 0   0 -   5 /   0
** 1   6 - 127 / 127
** 3 128 - 255 / 255
*/
#define DRAWCOLOR(v, e, color, pdest, cb, x) if (5 < v)                                   \
                                             {                                            \
                                                if (127 >= v)                             \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 1) \
                                                   fEmpty##color##_d = false;             \
                                                   e = -(127 - v);                        \
                                                }                                         \
                                                else                                      \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 3) \
                                                   fEmpty##color##_d = false;             \
                                                   e = -(255 - v);                        \
                                                }                                         \
                                             }                                            \
                                             else if (0 > v)                              \
                                             {                                            \
                                                e = 0;                                    \
                                             }
#elif 1
/* Ranges:
** 0     0     /   0
** 1   1 - 127 / 127
** 3 128 - 255 / 255
*/
#define DRAWCOLOR(v, e, color, pdest, cb, x) if (0 < v)                                   \
                                             {                                            \
                                                if (127 >= v)                             \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 1) \
                                                   fEmpty##color##_d = false;             \
                                                   e = -(127 - v);                        \
                                                }                                         \
                                                else                                      \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 3) \
                                                   fEmpty##color##_d = false;             \
                                                   e = -(255 - v);                        \
                                                }                                         \
                                             }                                            \
                                             else if (0 > v)                              \
                                             {                                            \
                                                e = 0;                                    \
                                             }
#elif 0
/* Ranges:
** 0     0     /   0
** 1   1 - 127 /  64
** 3 128 - 255 / 192
*/
#define DRAWCOLOR(v, e, color, pdest, cb, x) if (0 < v)                                   \
                                             {                                            \
                                                if (127 >= v)                             \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 1) \
                                                   fEmpty##color##_d = false;             \
                                                   e = v - 64;                            \
                                                }                                         \
                                                else                                      \
                                                {                                         \
                                                   SETINTENSEBIT (color, pdest, cb, x, 3) \
                                                   fEmpty##color##_d = false;             \
                                                   e = v - 192;                           \
                                                }                                         \
                                             }                                            \
                                             else if (0 > v)                              \
                                             {                                            \
                                                e = 0;                                    \
                                             }
#endif

#define SETBIT(color, pdest, x) *pdest |= (BYTE)(0x80 >> (x & 7));                        \
                                fEmpty##color##_d = false;

#define SETINTENSEBIT(color, pdest, cb, x, i) *pdest        |= (BYTE)(((i & 1) ? 0x80 : 0x00) >> (x & 7)); \
                                              *(pdest + cb) |= (BYTE)(((i & 2) ? 0x80 : 0x00) >> (x & 7));

#define UNSETINTENSEBIT(color, pdest, cb, x) *pdest        &= ~(BYTE)(0x80 >> (x & 7)); \
                                             *(pdest + cb) &= ~(BYTE)(0x80 >> (x & 7)); \
                                             fEmpty##color##_d = false;

#if HSV_ERR_ALGORITHM == HSV_NONE_ERR
#define ERROR_DIFFUSE(elm) (pesRow1 + 0)->elm = 0.0;
#endif

#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 16.0; \
                           flFrac3 = flFrac1 * 3.0;       \
                           flFrac5 = flFrac1 * 5.0;       \
                           flFrac7 = flFrac1 * 7.0;       \
                           (pesRow1 + 1)->elm += flFrac7; \
                                                          \
                           (pesRow2 - 1)->elm += flFrac3; \
                           (pesRow2 + 0)->elm += flFrac5; \
                           (pesRow2 + 1)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 48.0; \
                           flFrac3 = flFrac1 * 3.0;       \
                           flFrac5 = flFrac1 * 5.0;       \
                           flFrac7 = flFrac1 * 7.0;       \
                           (pesRow1 + 1)->elm += flFrac7; \
                           (pesRow1 + 2)->elm += flFrac5; \
                                                          \
                           (pesRow2 - 2)->elm += flFrac3; \
                           (pesRow2 - 1)->elm += flFrac5; \
                           (pesRow2 + 0)->elm += flFrac7; \
                           (pesRow2 + 1)->elm += flFrac5; \
                           (pesRow2 + 2)->elm += flFrac3; \
                                                          \
                           (pesRow3 - 2)->elm += flFrac1; \
                           (pesRow3 - 1)->elm += flFrac3; \
                           (pesRow3 + 0)->elm += flFrac5; \
                           (pesRow3 + 1)->elm += flFrac3; \
                           (pesRow3 + 2)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_STU_ERR
#define ERROR_DIFFUSE(elm) flFrac1 = pesRow1->elm / 42.0; \
                           flFrac2 = flFrac1 * 2.0;       \
                           flFrac4 = flFrac1 * 4.0;       \
                           flFrac8 = flFrac1 * 8.0;       \
                           (pesRow1 + 1)->elm += flFrac8; \
                           (pesRow1 + 2)->elm += flFrac4; \
                                                          \
                           (pesRow2 - 2)->elm += flFrac2; \
                           (pesRow2 - 1)->elm += flFrac4; \
                           (pesRow2 + 0)->elm += flFrac8; \
                           (pesRow2 + 1)->elm += flFrac4; \
                           (pesRow2 + 2)->elm += flFrac2; \
                                                          \
                           (pesRow3 - 2)->elm += flFrac1; \
                           (pesRow3 - 1)->elm += flFrac2; \
                           (pesRow3 + 0)->elm += flFrac4; \
                           (pesRow3 + 1)->elm += flFrac2; \
                           (pesRow3 + 2)->elm += flFrac1;
#endif

#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
#define ERROR_DIFFUSE(elm) flFrac1  = pesRow1->elm / 200.0;      \
                           flFrac12 = flFrac1 * 12.0;            \
                           (pesRow1 + 2)->elm += flFrac1 * 32.0; \
                                                                 \
                           (pesRow2 - 3)->elm += flFrac12;       \
                           (pesRow2 - 1)->elm += flFrac1 * 26.0; \
                           (pesRow2 + 1)->elm += flFrac1 * 30.0; \
                           (pesRow2 + 3)->elm += flFrac1 * 16.0; \
                                                                 \
                           (pesRow3 - 2)->elm += flFrac12;       \
                           (pesRow3 + 0)->elm += flFrac1 * 26.0; \
                           (pesRow3 + 2)->elm += flFrac12;       \
                                                                 \
                           (pesRow4 - 3)->elm += flFrac1 *  5.0; \
                           (pesRow4 - 1)->elm += flFrac12;       \
                           (pesRow4 + 1)->elm += flFrac12;       \
                           (pesRow4 + 3)->elm += flFrac1 *  5.0;
#endif

#undef  CLAMP
#define CLAMP(elm)         ((-255.0 > elm) ? -255.0 : ((255.0 < elm) ? 255.0 : elm))

   INT                i, y, t;

   INT                iMapSize,
                      iPelSize,
                      iSrcRowSize,
                      iDestRowSize;

   PBYTE              pbMapEnd,
                      pbSrc,
                      pbSrcNext;
   PBYTE              pbKDestNext,
                      pbKDest,
                      pbCDestNext,
                      pbCDest,
                      pbMDestNext,
                      pbMDest,
                      pbYDestNext,
                      pbYDest;
   ULONG              ulWidth         = iSrcRowPels_d;

   PCMYKERRORSTRUC    pesRoot;
   PCMYKERRORSTRUC    pesRow1, pesStartRow1;
   PCMYKERRORSTRUC    pesRow2, pesStartRow2;
   PCMYKERRORSTRUC    pesRow3, pesStartRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   PCMYKERRORSTRUC    pesRow4, pesStartRow4;
#endif
#if HSV_ERR_ALGORITHM != HSV_NONE_ERR
   float              flFrac1;
#endif
#if HSV_ERR_ALGORITHM == HSV_FLST_ERR
   float              flFrac3;
   float              flFrac5;
   float              flFrac7;
#endif
#if HSV_ERR_ALGORITHM == HSV_JJN_ERR
   float              flFrac3;
   float              flFrac5;
   float              flFrac7;
#endif
#if HSV_ERR_ALGORITHM == HSV_STU_ERR
   float              flFrac2;
   float              flFrac4;
   float              flFrac8;
#endif
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   float              flFrac12;
#endif
   PCOLR              pColor   = 0;
   RGB                rgb;
   float              flTempK,
                      flTempC,
                      flTempM,
                      flTempY;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

   if (pfErrRow_d == 0)
      return INVALID_PARAMETER;

   if (pbmi2->cy > iNumDitherRows_d)
      return INVALID_PARAMETER;

   // PelSize in bits
   iPelSize = (LONG)pbmi2->cPlanes * (LONG)pbmi2->cBitCount;

   // RowSize in bytes
   iSrcRowSize = ((LONG)pbmi2->cx * (LONG)pbmi2->cBitCount + 31)/32 * 4;

   // MapSize in bytes
   iMapSize = iSrcRowSize * (LONG)pbmi2->cPlanes * (LONG)pbmi2->cy;
   pbMapEnd = (PBYTE)((ULONG)pbSource + (ULONG)iMapSize);

   // DestRowSize in bytes
   iDestRowSize = ((LONG)pbmi2->cx + 7)/8;

   if (iPelSize < 16)
      pColor = (PCOLR)&pbmi2->argbColor[0];

   pbKDest = pbKBuffer_d;
   pbCDest = pbCBuffer_d;
   pbMDest = pbMBuffer_d;
   pbYDest = pbYBuffer_d;

   pbKDestNext = pbKNextBuffer_d;
   pbCDestNext = pbCNextBuffer_d;
   pbMDestNext = pbMNextBuffer_d;
   pbYDestNext = pbYNextBuffer_d;

   pbSrc     = pbSource;
   pbSrcNext = pbSource + iSrcRowSize;

   y = iRowCount_d;
   i = 0;

   pesRoot      = (PCMYKERRORSTRUC)pfErrRow_d;
   pesStartRow1 = NUM_HSV_ERR_NEIGHBORS + pesRoot;
   pesStartRow2 = pesStartRow1 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
   pesStartRow3 = pesStartRow2 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   pesStartRow4 = pesStartRow3 + (ulWidth + 2 * NUM_HSV_ERR_NEIGHBORS);
#endif

   pesRow1 = pesStartRow1;
   pesRow2 = pesStartRow2;
   pesRow3 = pesStartRow3;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
   pesRow4 = pesStartRow4;
#endif

   while ((ULONG)pbSrc < (ULONG)pbMapEnd)
   {
      if (24 == iPelSize)
      {
         if (fDataInRGB_d)
         {
            rgb = *(PRGB)pbSrc;
         }
         else
         {
            rgb.bRed   = *(pbSrc+2);
            rgb.bGreen = *(pbSrc+1);
            rgb.bBlue  = *pbSrc;
         }

         pbSrc += sizeof (RGB);
      }
      else if (16 == iPelSize)
      {
         rgb.bRed   = *pbSrc & 0xf8;
         rgb.bGreen = ((*pbSrc & 0x07) << 5) & ((*(pbSrc+1) & 0xE0) >> 3);
         rgb.bBlue  = (*(pbSrc+1) & 0x1f) << 5;

         if (!fDataInRGB_d)
         {
            BYTE bSwap = rgb.bRed;
            rgb.bRed   = rgb.bGreen;
            rgb.bGreen = bSwap;
         }

         pbSrc += 2;
      }
      else if (8 == iPelSize)
      {
         rgb.bRed   = pColor->is[*pbSrc].bRed;
         rgb.bGreen = pColor->is[*pbSrc].bGreen;
         rgb.bBlue  = pColor->is[*pbSrc].bBlue;

         pbSrc++;
      }
#ifdef ENABLE124
      else if (4 == iPelSize)
      {
         if (0 == (i & 1))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0xF0) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0xF0) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0xF0) >> 4].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x0F)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x0F)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x0F)].bBlue;

            pbSrc++;
         }
      }
      else if (2 == iPelSize)
      {
         if (0 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0xC0) >> 6].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0xC0) >> 6].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0xC0) >> 6].bBlue;
         }
         else if (1 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x30) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x30) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x30) >> 4].bBlue;
         }
         else if (2 == (i & 3))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x0C) >> 2].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x0C) >> 2].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x0C) >> 2].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x03)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x03)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x03)].bBlue;

            pbSrc++;
         }
      }
      else if (1 == iPelSize)
      {
         if (0 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x80) >> 7].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x80) >> 7].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x80) >> 7].bBlue;
         }
         else if (1 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x40) >> 6].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x40) >> 6].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x40) >> 6].bBlue;
         }
         else if (2 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x20) >> 5].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x20) >> 5].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x20) >> 5].bBlue;
         }
         else if (3 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x10) >> 4].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x10) >> 4].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x10) >> 4].bBlue;
         }
         else if (4 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x08) >> 3].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x08) >> 3].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x08) >> 3].bBlue;
         }
         else if (5 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x04) >> 2].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x04) >> 2].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x04) >> 2].bBlue;
         }
         else if (6 == (i & 7))
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x02) >> 1].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x02) >> 1].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x02) >> 1].bBlue;
         }
         else
         {
            rgb.bRed   = pColor->is[(*pbSrc & 0x01)].bRed;
            rgb.bGreen = pColor->is[(*pbSrc & 0x01)].bGreen;
            rgb.bBlue  = pColor->is[(*pbSrc & 0x01)].bBlue;

            pbSrc++;
         }
      }
#endif
      else
         return INVALID_BITMAP;


      // Gamma correct the values
      if (pbRGamma_d) rgb.bRed   = pbRGamma_d[0xff & ~rgb.bRed];
      if (pbGGamma_d) rgb.bGreen = pbGGamma_d[0xff & ~rgb.bGreen];
      if (pbBGamma_d) rgb.bBlue  = pbBGamma_d[0xff & ~rgb.bBlue];

      // Convert from RGB to CMYK

      // Black is the minimum of the three
      flTempK = MIN3 ((255 - rgb.bRed),
                      (255 - rgb.bGreen),
                      (255 - rgb.bBlue));

      if (255.0 == flTempK)
      {
         // Avoid divide by zero.
         // (R, G, B) = (0, 0, 0) ->  K(black) = 255, no color (C, M, Y) = (0, 0, 0).
         flTempC = 0;
         flTempM = 0;
         flTempY = 0;
      }
      else
      {
         flTempC = 255.0 * (255.0 - (float)rgb.bRed   - flTempK) / (255.0 - flTempK);
         flTempM = 255.0 * (255.0 - (float)rgb.bGreen - flTempK) / (255.0 - flTempK);
         flTempY = 255.0 * (255.0 - (float)rgb.bBlue  - flTempK) / (255.0 - flTempK);
      }

      /* Add in the accumulated errors
      */
      flTempK += pesRow1->flKError;
      flTempC += pesRow1->flCError;
      flTempM += pesRow1->flMError;
      flTempY += pesRow1->flYError;

      /* Draw the bits
      */
      DRAWCOLOR (flTempK, pesRow1->flKError, Black,    pbKDest, iDestRowSize, i)
      DRAWCOLOR (flTempC, pesRow1->flCError, Cyan,     pbCDest, iDestRowSize, i)
      DRAWCOLOR (flTempM, pesRow1->flMError, Magenta,  pbMDest, iDestRowSize, i)
      DRAWCOLOR (flTempY, pesRow1->flYError, Yellow,   pbYDest, iDestRowSize, i)

      ERROR_DIFFUSE (flKError);
      ERROR_DIFFUSE (flCError);
      ERROR_DIFFUSE (flMError);
      ERROR_DIFFUSE (flYError);

#if 0 // @TEST
      // Only print CMY planes
//////UNSETINTENSEBIT (Black,   pbKDest, iDestRowSize, i);
#endif
#if 0 // @TEST
      // Only print the K plane
//////UNSETINTENSEBIT (Cyan,    pbCDest, iDestRowSize, i);
//////UNSETINTENSEBIT (Magenta, pbMDest, iDestRowSize, i);
//////UNSETINTENSEBIT (Yellow,  pbYDest, iDestRowSize, i);
#endif

      pesRow1++;
      pesRow2++;
      pesRow3++;
#if HSV_ERR_ALGORITHM == HSV_STAR_ERR
      pesRow4++;
#endif

      INC

      TESTROWEND
   }

   iRowCount_d = y;

   return 0;

#undef INC
#undef TESTROWEND
#undef CLAMP_ROWS
#undef SETBIT
#undef SETINTENSEBIT
#undef ERROR_DIFFUSE
#undef CLAMP
}

void GplDitherInstance::
GplCMYRemoval (PBITMAPINFO2 pbmi2)
{
    //-------------------------------------------------------
    //     CMY Removal if Co-Incident with Dithered-Black
    //-------------------------------------------------------
    int      iXPos,
             iYPos,
             iDestPelsPerByte;
    //int      i,                        // debug
    //         iXByteCount;              // debug
    //long     lBitsRemoved = 0;         // debug
    BYTE     bComposite;
    PBYTE    pbKDestNext, pbKDest,
             pbCDestNext, pbCDest,
             pbMDestNext, pbMDest,
             pbLCDestNext = 0, pbLCDest = 0,
             pbLMDestNext = 0, pbLMDest = 0,
             pbYDestNext, pbYDest;

#ifndef RETAIL
    if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ()" << std::endl;
#endif

    switch (iDestBitsPerPel_d)
    {
    case 8 :
        iDestPelsPerByte = 1;
        break;
    case 4 :
        iDestPelsPerByte = 2;
        break;
    case 2 :
        iDestPelsPerByte = 4;
        break;
    case 1 :
        iDestPelsPerByte = 8;
        break;
    default:
        DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ***** ERROR Invalid CYM bits per pel***** " << std::endl;
        return;
    }

    // set each plane to empty - in the loop below if we leave any
    // bits in the plane we will reset to false
    fEmptyCyan_d    = true;
    fEmptyMagenta_d = true;
    fEmptyYellow_d  = true;
    fEmptyLMagenta_d = true;
    fEmptyLCyan_d    = true;

#ifndef RETAIL
    if (DebugOutput::shouldOutputGplDitherInstance ())
    {
       DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": cBitCount=" << pbmi2->cBitCount << std::endl;
       DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": pbmi2->cy=" << pbmi2->cy << " pbmi2->cx=" << pbmi2->cx <<  std::endl;
       DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": pbmi2->cPlanes=" << pbmi2->cPlanes <<  std::endl;
    }
#endif

    pbKDest = pbKBuffer_d;
    pbCDest = pbCBuffer_d;
    pbMDest = pbMBuffer_d;
    pbYDest = pbYBuffer_d;

    if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
    {
        pbLCDest = pbLCBuffer_d;
        pbLMDest = pbLMBuffer_d;
    }

    pbKDestNext = pbKNextBuffer_d;
    pbCDestNext = pbCNextBuffer_d;
    pbMDestNext = pbMNextBuffer_d;
    pbYDestNext = pbYNextBuffer_d;

    if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
    {
        pbLCDestNext = pbLCNextBuffer_d;
        pbLMDestNext = pbLMNextBuffer_d;
    }

    // for number of rows
    for (iYPos = 0; iYPos < pbmi2->cy; iYPos++)
    {
       // iXByteCount = 0;   // byte count for debug
       iXPos = 0;         // beggining of line

#ifndef RETAIL
       if (DebugOutput::shouldOutputGplDitherInstance ()) DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": Processing Row " << iYPos << " of " << pbmi2->cy << std::endl;
#endif

       // loop through the destbits containing the src number of pels
       while (iXPos < (LONG)pbmi2->cx)
       {
           // firewall - check for ptr passed end of allocated mem.
           // assumes y is the last plane.
           if (pbYDest > pbDestEnd_d)
           {
               DebugOutput::getErrorStream () << "GplDitherInstance:" << __FUNCTION__ << ": ** ERROR PASSED END OF BUFFER **" <<std::endl;
               break;
           }
          // @TBD We might want to try only removing large composite black dots for var dot printers.
          // @TBD Also, try only removing large runs of composite black.
          bComposite = (*pbCDest & *pbMDest & *pbYDest);
          if (bComposite)
          {
              // Let color show by removing composite black and converting
              // composite black to pure black.
              *pbKDest |= bComposite;
              *pbCDest ^= bComposite;
              *pbMDest ^= bComposite;
              *pbYDest ^= bComposite;

              //@@SIX-COLOR
              if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
              {
                  *pbLCDest &= ~bComposite;
                  *pbLMDest &= ~bComposite;
              }

              fEmptyBlack_d  = false;    // make sure the black plane is not set to empty

              // debug count bits removed
              // for (i = 0; i < 8; i++) {
              //    if (bComposite & 0x01 << i) {
              //       lBitsRemoved++;
              //    }
              //  }
          }

          if (*pbCDest != 0)
             fEmptyCyan_d = false;
          if (*pbMDest != 0)
             fEmptyMagenta_d = false;
          if (*pbYDest != 0)
             fEmptyYellow_d = false;

          if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
         {
              if (*pbLCDest != 0)
                 fEmptyLCyan_d = false;
              if (*pbLMDest != 0)
                 fEmptyLMagenta_d = false;
         }

          iXPos += iDestPelsPerByte;  // inc by number of pels processed in a byte
          // iXByteCount++;              // debug
          pbKDest++;
          pbCDest++;
          pbMDest++;
          pbYDest++;
          if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
          {
              pbLCDest++;
              pbLMDest++;
          }

       }
       // DebugOutput::getErrorStream () << " GplCMYRemoval ()" << " Row End " << (iXPos) << " of " << pbmi2->cx <<std::endl;

       pbKDest     = pbKDestNext;
       pbKDestNext = pbKDest + iNumDestRowBytes_d;
       pbCDest     = pbCDestNext;
       pbCDestNext = pbCDest + iNumDestRowBytes_d;
       pbMDest     = pbMDestNext;
       pbMDestNext = pbMDest + iNumDestRowBytes_d;
       pbYDest     = pbYDestNext;
       pbYDestNext = pbYDest + iNumDestRowBytes_d;

       if (DevicePrintMode::COLOR_TECH_CcMmYK == iColorTech_d)
       {
           pbLCDest     = pbLCDestNext;
           pbLCDestNext = pbLCDest + iNumDestRowBytes_d;
           pbLMDest     = pbLMDestNext;
           pbLMDestNext = pbLMDest + iNumDestRowBytes_d;
       }
    }
    //   DebugOutput::getErrorStream () << " GplCMYRemovel () CMY lBitsRemoved =" << lBitsRemoved  << std::endl;
}

void
DumpGammaTable (PSZCRO pszTitle,
                PBYTE  pbGamma)
{
   DebugOutput::getErrorStream () << pszTitle;
   for (int i = 0; i < RGB_VALUES; i++)
   {
      if (0 == (i % 8))
      {
         DebugOutput::getErrorStream () << std::endl;
      }
      DebugOutput::getErrorStream () << " ";
      DebugOutput::getErrorStream ().width (3);
      DebugOutput::getErrorStream () << (int)pbGamma[i];
   }
   DebugOutput::getErrorStream ().width ();
   DebugOutput::getErrorStream () << std::endl;
}

/*************************************************************************/
/* PROCEDURE NAME : GplGammaBuildTable                                   */
/* DESCRIPTION    :  See below                                           */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/* Change History Log:                                                   */
/* jk1115 gamma table for DITHER_VOID_CLUSTER and DITHER_JANIS_STUCKI    */
/*        methods                                                        */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*************************************************************************/
bool
GplGammaBuildTable (int   iDitherType,
                    PBYTE pbKGamma,
                    PBYTE pbRGamma,
                    PBYTE pbGGamma,
                    PBYTE pbBGamma,
                    int   iKBias,
                    int   iRBias,
                    int   iGBias,
                    int   iBBias,
                    int   iKGamma,
                    int   iRGamma,
                    int   iGGamma,
                    int   iBGamma)
{

   // Red Gamma Curve
   GplGenerateGammaCurve (iRGamma, iRBias, pbRGamma);
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DumpGammaTable ("Red:", pbRGamma);
#endif

   // Green Gamma Curve
   GplGenerateGammaCurve (iGGamma, iGBias, pbGGamma);
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DumpGammaTable ("Green:", pbGGamma);
#endif

   // Blue Gamma Curve
   GplGenerateGammaCurve (iBGamma, iBBias, pbBGamma);
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplDitherInstance ()) DumpGammaTable ("Blue:", pbBGamma);
#endif

   // Black Gamma Curve
   if (pbKGamma)
   {
      GplGenerateGammaCurve (iKGamma, iKBias, pbKGamma);
#ifndef RETAIL
      if (DebugOutput::shouldOutputGplDitherInstance ()) DumpGammaTable ("Black:", pbKGamma);
#endif
   }

   return true;
}

/*************************************************************************/
/* PROCEDURE NAME : GplGenerateGammaCurve                                */
/* DATE WRITTEN   : 10/26/94                                             */
/* DESCRIPTION    :  See below                                           */
/*                                                                       */
/*   for each input color = 0..255:                                      */
/*                                                                       */
/*       a) newColor = 255 * (color/255)**(1/gamma)                      */
/*                                                                       */
/*          or rewritten as a Power of 2:                                */
/*                                                                       */
/*       b) newColor = 255 * 2**(1/gamma}*log2(color/255)                */
/*                                                                       */
/*          log base 2 can be rewritten with natural logs                */
/*                                                                       */
/*          log2(color/255) = ln(color/255)/ln(2)                        */
/*                                                                       */
/*       c) newcolor = 255 * 2**((1/gamma)( ln(color/255)/ln(2) ))       */
/*                                                                       */
/*          We can simplify log2 calculation by using a lookup table     */
/*          for all input RGB colors:  0 < color <= 255                  */
/*                                                                       */
/*       d) newcolor = 255 * 2**( faLog2Table[color]/gamma )             */
/*                                                                       */
/*          the result of faLog2Table[color]/gamma will be a             */
/*          real number with a int portion and fract portion.            */
/*                                                                       */
/*          (faLog2Table[color]/gamma = negative real number)            */
/*                                                                       */
/*          Using laws of exponents we can rewrite the equation as       */
/*          follows:                                                     */
/*                                                                       */
/*       e)  newcolor = 255 * ( 2**IntPart * 2**FractPart )              */
/*                                                                       */
/*           where: 2**Fraction is a table lookup;                       */
/*                  0 <= iFractionIndex  <= 99                           */
/*                                                                       */
/*           NOTE: both integer and fractional parts are negative        */
/*                 since the faLog2Table[] consists of all negative      */
/*                 entries.                                              */
/*                                                                       */
/*       f) So the final equation looks like this:                       */
/*                                                                       */
/*          newcolor = 255 *  1 / (2**IntPart) * 1 / (2**FractPart)      */
/*                                                                       */
/*       g) newcolor = 255 * 1 / (2**IntPart) * 1 / faExp2FractionTable[]*/
/*                                                                       */
/*       NOTE: actually table starts at 1 giving us gamma even when      */
/*             not asked for.  We need to correct this - MFR.            */
/*             (i.e. when gamma=1.0 input color=127 output=128)          */
/*                                                                       */
/* RATIONALE:                                                            */
/*                                                                       */
/*  Log base 2 tables of x/255 where 0 < x < 256;  the table is used     */
/*  because log functions are not included in the subsystem              */
/*  library, DDE4NBS.  The C-Set compiler could generate the             */
/*  simple 387/486/Pentium floating point log instructions:              */
/*                                                                       */
/*  But, The PowerPC 601 does not have Log or Exp instructions.          */
/*                                                                       */
/*  NOTE: Sufficient accuracy is achieved with just 4 terms.             */
/*                                                                       */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/* Change History Log:                                                   */
/*-----------------------------------------------------------------------*/
/*************************************************************************/

float faLog2Table[256] = {
-7.9943500, -6.9943500, -6.4093900, -5.9943500, -5.6724300, -5.40939000, -5.1870000, -4.9943500,
-4.8244300, -4.6724300, -4.5349200, -4.4093900, -4.2939100, -4.18700000, -4.0874600, -3.9943500,
-3.9068900, -3.8244300, -3.7464300, -3.6724300, -3.6020400, -3.53492000, -3.4707900, -3.4093900,
-3.3505000, -3.2939100, -3.2394700, -3.1870000, -3.1363700, -3.08746000, -3.0401600, -2.9943500,
-2.9499600, -2.9068900, -2.8650700, -2.8244300, -2.7849000, -2.74643000, -2.7089500, -2.6724300,
-2.6368000, -2.6020400, -2.5680900, -2.5349200, -2.5025000, -2.47079000, -2.4397600, -2.4093900,
-2.3796400, -2.3505000, -2.3219300, -2.2939100, -2.2664300, -2.23947000, -2.2129900, -2.1870000,
-2.1614600, -2.1363700, -2.1117100, -2.0874600, -2.0636200, -2.04016000, -2.0170700, -1.9943500,
-1.9719900, -1.9499600, -1.9282600, -1.9068900, -1.8858300, -1.86507000, -1.8446100, -1.8244300,
-1.8045300, -1.7849000, -1.7655300, -1.7464300, -1.7275700, -1.70895000, -1.6905700, -1.6724300,
-1.6545000, -1.6368000, -1.6193100, -1.6020400, -1.5849600, -1.56809000, -1.5514100, -1.5349200,
-1.5186200, -1.5025000, -1.4865600, -1.4707900, -1.4551900, -1.43976000, -1.4245000, -1.4093900,
-1.3944400, -1.3796400, -1.3650000, -1.3505000, -1.3361400, -1.32193000, -1.3078500, -1.2939100,
-1.2801100, -1.2664300, -1.2528900, -1.2394700, -1.2261700, -1.21299000, -1.1999400, -1.1870000,
-1.1741700, -1.1614600, -1.1488600, -1.1363700, -1.1239900, -1.11171000, -1.0995400, -1.0874600,
-1.0754900, -1.0636200, -1.0518400, -1.0401600, -1.0285700, -1.01707000, -1.0056700, -0.9943530,
-0.9831260, -0.9719860, -0.9609300, -0.9499590, -0.9390710, -0.92826400, -0.9175380, -0.9068910,
-0.8963210, -0.8858290, -0.8754120, -0.8650700, -0.8548020, -0.84460600, -0.8344820, -0.8244280,
-0.8144440, -0.8045290, -0.7946810, -0.7849000, -0.7751850, -0.76553500, -0.7559490, -0.7464260,
-0.7369660, -0.7275670, -0.7182290, -0.7089510, -0.6997330, -0.69057300, -0.6814700, -0.6724250,
-0.6634370, -0.6545030, -0.6456250, -0.6368010, -0.6280310, -0.61931400, -0.6106490, -0.6020360,
-0.5934740, -0.5849620, -0.5765010, -0.5680890, -0.5597250, -0.55141000, -0.5431420, -0.5349220,
-0.5267480, -0.5186200, -0.5105380, -0.5025000, -0.4945080, -0.48655900, -0.4786540, -0.4707910,
-0.4629720, -0.4551950, -0.4474590, -0.4397650, -0.4321110, -0.42449800, -0.4169250, -0.4093910,
-0.4018960, -0.3944410, -0.3870230, -0.3796440, -0.3723020, -0.36499700, -0.3577290, -0.3504970,
-0.3433020, -0.3361420, -0.3290170, -0.3219280, -0.3148730, -0.30785300, -0.3008660, -0.2939140,
-0.2869940, -0.2801080, -0.2732540, -0.2664330, -0.2596440, -0.25288600, -0.2461610, -0.2394660,
-0.2328020, -0.2261690, -0.2195660, -0.2129940, -0.2064510, -0.19993800, -0.1934540, -0.1869990,
-0.1805720, -0.1741740, -0.1678050, -0.1614630, -0.1551500, -0.14886300, -0.1426040, -0.1363720,
-0.1301670, -0.1239890, -0.1178360, -0.1117100, -0.1056100, -0.09953570, -0.0934866, -0.0874628,
-0.0814641, -0.0754902, -0.0695409, -0.0636161, -0.0577155, -0.05183890, -0.0459862, -0.0401571,
-0.0343515, -0.0285692, -0.0228099, -0.0170735, -0.0113599, -0.00566875, 0 };

#define FRACTION_TABLE_SIZE 256
float faExp2FractionTable[FRACTION_TABLE_SIZE] = {
1.00000, 1.00271, 1.00543, 1.00816, 1.01089, 1.01363, 1.01638, 1.01913, 1.02190,
1.02467, 1.02745, 1.03023, 1.03302, 1.03583, 1.03863, 1.04145, 1.04427, 1.04711,
1.04994, 1.05279, 1.05565, 1.05851, 1.06138, 1.06425, 1.06714, 1.07003, 1.07293,
1.07584, 1.07876, 1.08169, 1.08462, 1.08756, 1.09051, 1.09346, 1.09643, 1.09940,
1.10238, 1.10537, 1.10837, 1.11137, 1.11439, 1.11741, 1.12044, 1.12348, 1.12652,
1.12958, 1.13264, 1.13571, 1.13879, 1.14188, 1.14497, 1.14808, 1.15119, 1.15431,
1.15744, 1.16058, 1.16372, 1.16688, 1.17004, 1.17322, 1.17640, 1.17959, 1.18278,
1.18599, 1.18921, 1.19243, 1.19566, 1.19891, 1.20216, 1.20542, 1.20868, 1.21196,
1.21525, 1.21854, 1.22185, 1.22516, 1.22848, 1.23181, 1.23515, 1.23850, 1.24186,
1.24522, 1.24860, 1.25199, 1.25538, 1.25878, 1.26220, 1.26562, 1.26905, 1.27249,
1.27594, 1.27940, 1.28287, 1.28635, 1.28984, 1.29333, 1.29684, 1.30036, 1.30388,
1.30742, 1.31096, 1.31452, 1.31808, 1.32165, 1.32524, 1.32883, 1.33243, 1.33605,
1.33967, 1.34330, 1.34694, 1.35059, 1.35426, 1.35793, 1.36161, 1.36530, 1.36900,
1.37271, 1.37644, 1.38017, 1.38391, 1.38766, 1.39142, 1.39520, 1.39898, 1.40277,
1.40658, 1.41039, 1.41421, 1.41805, 1.42189, 1.42575, 1.42961, 1.43349, 1.43738,
1.44127, 1.44518, 1.44910, 1.45303, 1.45697, 1.46092, 1.46488, 1.46885, 1.47283,
1.47683, 1.48083, 1.48485, 1.48887, 1.49291, 1.49696, 1.50101, 1.50508, 1.50916,
1.51326, 1.51736, 1.52147, 1.52560, 1.52973, 1.53388, 1.53804, 1.54221, 1.54639,
1.55058, 1.55479, 1.55900, 1.56323, 1.56747, 1.57172, 1.57598, 1.58025, 1.58454,
1.58883, 1.59314, 1.59746, 1.60179, 1.60614, 1.61049, 1.61486, 1.61924, 1.62363,
1.62803, 1.63244, 1.63687, 1.64131, 1.64576, 1.65022, 1.65469, 1.65918, 1.66368,
1.66819, 1.67271, 1.67725, 1.68179, 1.68635, 1.69092, 1.69551, 1.70011, 1.70472,
1.70934, 1.71397, 1.71862, 1.72328, 1.72795, 1.73264, 1.73733, 1.74204, 1.74677,
1.75150, 1.75625, 1.76101, 1.76579, 1.77058, 1.77538, 1.78019, 1.78502, 1.78986,
1.79471, 1.79957, 1.80445, 1.80935, 1.81425, 1.81917, 1.82410, 1.82905, 1.83401,
1.83898, 1.84397, 1.84897, 1.85398, 1.85901, 1.86405, 1.86910, 1.87417, 1.87925,
1.88434, 1.88945, 1.89458, 1.89971, 1.90486, 1.91003, 1.91521, 1.92040, 1.92561,
1.93083, 1.93606, 1.94131, 1.94657, 1.95185, 1.95714, 1.96245, 1.96777, 1.97311,
1.97846, 1.98382, 1.98920, 1.99459 };

// In the following, it was found that -.5 is an order-magnatude better than .5
void
GplGenerateGammaCurve (float fGammaIn, int iBiasIn, PBYTE pbGamma)
{
   float fGamma;
   int   iBias;
   int   i;
   float fy, f2exp = 0;
   int   ic, iIntegerPart, iFractionIndex;

   if (0.0 == fGammaIn)
   {
      // Map to identity
      for (i = 0; i < 256; i++)
         pbGamma[i] = i;

      return;
   }

   fGamma = fGammaIn / 10.0;
   iBias  = iBiasIn;

   // Gamma: Check/correct for upper/lower limits
   if (fGamma < 0.1)
      fGamma = 0.1;

   // Bias: Check/correct for upper/lower limits
   if (iBias < 0)
      iBias = 0;
   else if (iBias > 255)
      iBias = 255;

   for (i = 0; i < 256; i++)
   {
      fy = - faLog2Table[i] / fGamma;

      iIntegerPart = (int)fy;

      if (iIntegerPart < 0)
         iIntegerPart = - iIntegerPart;

      iFractionIndex = (int)(-0.5 + (fy - iIntegerPart) * FRACTION_TABLE_SIZE);

      if (iFractionIndex < 0)
         iFractionIndex = - iFractionIndex;

      ic = 1 << iIntegerPart;

      if (ic != 0)
         f2exp = 1.0 / ic;

      ic = (int)(255.0 * f2exp / faExp2FractionTable[iFractionIndex]);

      if (ic < iBias)
         ic = iBias;

      pbGamma[255-i] = (BYTE)ic;
   }
}

/****************************************************************************/
/* PROCEDURE NAME : GplInitializeRandomNumberTable                          */
/* DATE WRITTEN   : 12-28-94                                                */
/* DESCRIPTION    :                                                         */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                                                                          */
/*                                                                          */
/* RETURN VALUES:   Success: NO_ERROR                  0                    */
/*   GplGplInitializeRandomNumberTable need only be invoked once in order   */
/*   to create a seeded random number table.  The table will                */
/*   dynamically change in order to continuously yield                      */
/*   random numbers.                                                        */
/*                                                                          */
/*   Knuth's random number generator was chosen                             */
/*   because it was the fastest useable algorithm                           */
/*   reviewed.  A much faster random bit                                    */
/*   generator was found unacceptable when used to generate                 */
/*   numbers in the range 0..100.                                           */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
LONG GplDitherInstance::
GplInitializeRandomNumberTable ()
{
   LONG  l1, l2;
   int i, j, k;

   // Initialize the last entry in a table with a seed
   l1 = (LONG)(iSeed_d % iBig_d);
   iMix_d[55] = l1;
   l2 = 1;

   // Initialize a table to a group ol numbers that are not random
   for (i = 1; i < 55; i++)
   {
      k = i;
      j = (21 * k) % 55;
      iMix_d[j] = l2;
      l2 = l1 - l2;
      if (l2 < 0)
         l2 = (LONG)(l2 + iBig_d);
      l1 = iMix_d[j];
   }

   // Mix up the table; notice iMix_d[0] is not used
   for (j = 1; j <= 4; j++)
   {
      for (i = 1; i < 56; i++)
      {
         iMix_d[i] = iMix_d[i] - iMix_d[1 + ((i+30) % 55)];
         if (iMix_d[i] < 0)
            iMix_d[i] = (int)(iMix_d[i] + iBig_d);
      }
   }

   // Reset indexes for random table usage
   lidx1_d = 0;
   lidx2_d = 31;  // 31 is the proper number to use here according to Knuth

   return 0;
}

ULONG ulPrimColors[7] = {CLR_RED, CLR_YELLOW, CLR_GREEN, CLR_CYAN, CLR_BLUE, CLR_PINK, CLR_RED};

BYTE aPaintmixer[16][16] = {
{0xFF, 0x20, 0xd0, 0x30, 0xf2, 0x12, 0xc2, 0x22, 0xfd, 0x1d, 0xcd, 0x2d, 0xf3, 0x13, 0xc3, 0x23},
{0x50, 0xb0, 0x80, 0xa0, 0x42, 0xa2, 0x72, 0x92, 0x4d, 0xad, 0x7d, 0x9d, 0x43, 0xa3, 0x73, 0x93},
{0x40, 0xe0, 0x10, 0xf0, 0x32, 0xd2, 0x02, 0xe2, 0x3d, 0xdd, 0x0d, 0xed, 0x33, 0xd3, 0x03, 0xe3},
{0x90, 0x70, 0xc0, 0x60, 0x82, 0x62, 0xb2, 0x52, 0x8d, 0x6d, 0xbd, 0x5d, 0x83, 0x63, 0xb3, 0x53},
{0xf5, 0x15, 0xc5, 0x25, 0xfb, 0x1b, 0xcb, 0x2b, 0xf8, 0x18, 0xc8, 0x28, 0xfa, 0x1a, 0xca, 0x2a},
{0x45, 0xa5, 0x75, 0x95, 0x4b, 0xab, 0x7b, 0x9b, 0x48, 0xa8, 0x78, 0x98, 0x4a, 0xaa, 0x7a, 0x9a},
{0x35, 0xd5, 0x05, 0xe5, 0x3b, 0xdb, 0x0b, 0xeb, 0x38, 0xd8, 0x08, 0xe8, 0x3a, 0xda, 0x0a, 0xea},
{0x85, 0x65, 0xb5, 0x55, 0x8b, 0x6b, 0xbb, 0x5b, 0x88, 0x68, 0xb8, 0x58, 0x8a, 0x6a, 0xba, 0x5a},
{0xf4, 0x14, 0xc4, 0x24, 0xfe, 0x1e, 0xce, 0x2e, 0xf1, 0x11, 0xc1, 0x21, 0xff, 0x1f, 0xcf, 0x2f},
{0x44, 0xa4, 0x74, 0x94, 0x4e, 0xae, 0x7e, 0x9e, 0x41, 0xa1, 0x71, 0x91, 0x4f, 0xaf, 0x7f, 0x9f},
{0x34, 0xd4, 0x04, 0xe4, 0x3e, 0xde, 0x0e, 0xee, 0x31, 0xd1, 0x01, 0xe1, 0x3f, 0xdf, 0x0f, 0xef},
{0x84, 0x64, 0xb4, 0x54, 0x8e, 0x6e, 0xbe, 0x5e, 0x81, 0x61, 0xb1, 0x51, 0x8f, 0x6f, 0xbf, 0x5f},
{0xf9, 0x19, 0xc9, 0x29, 0xf7, 0x17, 0xc7, 0x27, 0xfc, 0x1c, 0xcc, 0x2c, 0xf6, 0x16, 0xc6, 0x26},
{0x49, 0xa9, 0x79, 0x99, 0x47, 0xa7, 0x77, 0x97, 0x4c, 0xac, 0x7c, 0x9c, 0x46, 0xa6, 0x76, 0x96},
{0x39, 0xd9, 0x09, 0xe9, 0x37, 0xd7, 0x07, 0xe7, 0x3c, 0xdc, 0x0c, 0xec, 0x36, 0xd6, 0x06, 0xe6},
{0x89, 0x69, 0xb9, 0x59, 0x87, 0x67, 0xb7, 0x57, 0x8c, 0x6c, 0xbc, 0x5c, 0x86, 0x66, 0xb6, 0x56} };
//
// Recursive Tesselated Tiles of Rectangular Threshold Arrays
//                       or
//                 Ordered Squares
//
BYTE aOrdered[16][16] = {
{  2, 236,  60, 220,  16, 232,  56, 216,   3, 233,  57, 217,  13, 229,  53, 213},
{130,  66, 188, 124, 144,  80, 184, 120, 131,  67, 185, 121, 141,  77, 181, 117},
{ 34, 194,  18, 252,  48, 208,  32, 248,  35, 195,  19, 249,  45, 205,  29, 245},
{162,  98, 146,  82, 176, 112, 160,  96, 163,  99, 147,  83, 173, 109, 157,  93},
{ 10, 226,  50, 210,   6, 240,  64, 224,  11, 227,  51, 211,   7, 237,  61, 221},
{138,  74, 178, 114, 134,  70, 192, 128, 139,  75, 179, 115, 135,  71, 189, 125},
{ 42, 202,  26, 242,  38, 198,  22, 255,  43, 203,  27, 243,  39, 199,  23, 253},
{170, 106, 154,  90, 166, 102, 150,  86, 171, 107, 155,  91, 167, 103, 151,  87},
{  4, 234,  58, 218,  14, 230,  54, 214,   1, 235,  59, 219,  15, 231,  55, 215},
{132,  68, 186, 122, 142,  78, 182, 118, 129,  65, 187, 123, 143,  79, 183, 119},
{ 36, 196,  20, 250,  46, 206,  30, 246,  33, 193,  17, 251,  47, 207,  31, 247},
{164, 100, 148,  84, 174, 110, 168,  94, 161,  97, 145,  81, 175, 111, 159,  95},
{ 12, 228,  52, 212,   8, 238,  62, 222,   9, 225,  49, 209,   5, 239,  63, 223},
{140,  76, 180, 116, 136,  72, 190, 126, 137,  73, 177, 113, 133,  69, 191, 127},
{ 44, 204,  28, 244,  40, 200,  24, 254,  41, 201,  25, 241,  37, 197,  21, 255},
{172, 108, 156,  92, 168, 104, 152,  88, 169, 105, 153,  89, 165, 101, 149,  85} };

RGB2 paHalftone4x4_24[18][4][4] = {
{ {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}} },

{ {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{1,1,1,0},   {0,1,1,0},  {1,0,1,0},  {1,1,0,0}},
  {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}} },

{ {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{0,1,1,0},   {1,0,1,0},  {1,1,0,0},  {1,1,1,0}},
  {{1,1,0,0},   {1,1,1,0},  {0,1,1,0},  {1,0,1,0}},
  {{1,1,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,1,0}} },

{ {{1,1,0,0},   {1,1,1,0},  {1,1,1,0},  {0,1,1,0}},
  {{0,1,1,0},   {1,0,1,0},  {1,1,1,0},  {1,1,1,0}},
  {{1,1,1,0},   {1,1,0,0},  {0,1,1,0},  {1,0,1,0}},
  {{1,0,1,0},   {1,1,1,0},  {1,1,1,0},  {1,1,0,0}} },

{ {{1,0,1,0},   {1,1,0,0},  {0,1,1,0},  {1,1,1,0}},
  {{0,1,1,0},   {1,1,1,0},  {1,0,1,0},  {1,1,0,0}},
  {{1,1,1,0},   {0,1,1,0},  {1,1,0,0},  {0,1,1,0}},
  {{1,1,0,0},   {1,0,1,0},  {1,1,1,0},  {1,0,1,0}} },

{ {{0,1,1,0},   {1,1,0,0},  {1,0,1,0},  {0,1,1,0}},
  {{1,0,0,0},   {0,1,1,0},  {1,1,1,0},  {1,0,0,0}},
  {{0,1,1,0},   {1,0,1,0},  {1,1,0,0},  {1,1,1,0}},
  {{1,1,1,0},   {1,1,0,0},  {0,1,1,0},  {1,0,1,0}} },

{ {{0,1,0,0},   {1,0,1,0},  {0,1,0,0},  {1,1,1,0}},
  {{0,1,1,0},   {1,1,1,0},  {1,0,1,0},  {1,0,1,0}},
  {{1,0,0,0},   {0,1,0,0},  {0,1,1,0},  {1,1,0,0}},
  {{1,1,1,0},   {1,0,1,0},  {1,1,0,0},  {0,0,1,0}} },

{ {{1,0,1,0},   {0,1,0,0},  {1,0,1,0},  {0,1,0,0}},
  {{0,1,0,0},   {1,0,1,0},  {0,1,0,0},  {1,0,1,0}},
  {{1,0,1,0},   {0,1,0,0},  {1,0,1,0},  {0,1,0,0}},
  {{0,1,1,0},   {1,0,1,0},  {1,1,0,0},  {1,1,1,0}} },

{ {{0,1,1,0},   {1,0,1,0},  {0,1,0,0},  {1,0,0,0}},
  {{1,0,1,0},   {0,1,0,0},  {1,0,1,0},  {0,1,1,0}},
  {{0,1,0,0},   {1,0,0,0},  {0,1,0,0},  {1,0,1,0}},
  {{1,0,0,0},   {0,1,1,0},  {1,0,1,0},  {0,1,0,0}} },

{ {{0,1,0,0},   {1,0,1,0},  {0,1,0,0},  {1,1,0,0}},
  {{1,0,1,0},   {0,1,0,0},  {0,0,0,0},  {1,0,1,0}},
  {{0,1,1,0},   {1,0,0,0},  {0,1,1,0},  {0,1,0,0}},
  {{1,0,0,0},   {0,0,1,0},  {1,0,0,0},  {0,0,1,0}} },

{ {{1,0,0,0},   {0,1,0,0},  {0,0,0,0},  {0,0,1,0}},
  {{0,1,0,0},   {1,0,0,0},  {0,1,1,0},  {1,0,0,0}},
  {{1,0,1,0},   {0,1,1,0},  {1,0,0,0},  {0,1,0,0}},
  {{0,0,1,0},   {0,0,0,0},  {0,1,0,0},  {1,0,1,0}} },

{ {{0,0,0,0},   {0,0,1,0},  {0,1,0,0},  {1,0,0,0}},
  {{1,0,1,0},   {0,1,0,0},  {1,0,0,0},  {0,0,1,0}},
  {{0,1,0,0},   {1,0,0,0},  {0,0,1,0},  {0,0,0,0}},
  {{0,0,1,0},   {0,0,0,0},  {0,1,0,0},  {1,1,0,0}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,1,1,0},  {0,0,0,0}},
  {{0,1,0,0},   {0,0,1,0},  {1,0,0,0},  {1,0,0,0}},
  {{0,0,1,0},   {1,1,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{1,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,1,1,0}} },

{ {{0,0,0,0},   {1,0,0,0},  {0,0,0,0},  {0,0,1,0}},
  {{0,0,1,0},   {0,0,0,0},  {0,1,0,0},  {0,0,0,0}},
  {{1,0,0,0},   {0,1,0,0},  {0,0,1,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {1,0,0,0},  {0,1,0,0}} },

{ {{1,0,0,0},   {0,0,0,0},  {0,1,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,1,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {1,0,0,0}},
  {{0,1,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,1,0}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,1,0},  {0,0,0,0},  {0,1,0,0}},
  {{0,0,0,0},   {1,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}} } };

RGB2 paHalftone4x4[18][4][4] = {
{ {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}} },

{ {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,0,1,1},  {1,1,0,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}} },

{ {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{0,1,1,1},   {1,0,1,1},  {1,1,0,1},  {1,1,1,1}},
  {{1,1,0,1},   {1,1,1,1},  {0,1,1,1},  {1,0,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,1,1,1},  {1,1,1,1}} },

{ {{1,1,0,1},   {1,1,1,1},  {1,1,1,1},  {0,1,1,1}},
  {{1,1,1,1},   {1,0,1,1},  {1,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,0,1},  {1,1,1,1},  {1,0,1,1}},
  {{0,0,1,1},   {1,1,1,1},  {0,1,1,1},  {1,1,0,1}} },

{ {{0,0,1,1},   {1,1,0,1},  {0,1,1,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,1,1},  {1,0,1,1},  {1,1,0,1}},
  {{1,1,1,1},   {0,1,1,1},  {1,1,0,1},  {0,1,1,1}},
  {{1,1,0,1},   {1,0,1,1},  {1,1,1,1},  {1,0,1,1}} },

{ {{0,1,1,1},   {1,1,0,1},  {1,0,1,1},  {0,1,1,1}},
  {{1,0,0,1},   {0,1,1,1},  {1,1,1,1},  {1,0,0,1}},
  {{0,1,1,1},   {1,0,1,1},  {1,1,0,1},  {1,1,1,1}},
  {{1,1,1,1},   {1,1,0,1},  {0,1,1,1},  {1,0,1,1}} },

{ {{0,1,0,1},   {1,0,1,1},  {0,1,0,1},  {1,1,1,1}},
  {{0,1,1,1},   {1,1,1,1},  {1,0,1,1},  {1,0,1,1}},
  {{1,0,0,1},   {0,1,0,1},  {0,1,1,1},  {1,1,0,1}},
  {{1,1,1,1},   {1,0,1,1},  {1,1,0,1},  {0,0,1,1}} },

{ {{1,0,1,1},   {0,1,0,1},  {1,0,1,1},  {0,1,0,1}},
  {{0,1,0,1},   {1,0,1,1},  {0,1,0,1},  {1,0,1,1}},
  {{1,0,1,1},   {1,1,0,1},  {1,0,1,1},  {0,1,0,1}},
  {{0,1,1,1},   {1,0,1,1},  {1,1,0,1},  {1,1,1,1}} },

{ {{0,1,1,1},   {1,0,1,1},  {0,1,0,1},  {1,0,0,1}},
  {{1,0,1,1},   {0,1,0,1},  {1,0,1,1},  {0,1,1,1}},
  {{0,1,0,1},   {1,0,0,1},  {0,1,0,1},  {1,0,1,1}},
  {{1,0,0,1},   {0,1,1,1},  {1,0,1,1},  {0,1,0,1}} },

{ {{0,1,0,1},   {1,0,1,1},  {0,1,0,1},  {1,1,0,1}},
  {{1,0,1,1},   {0,1,0,1},  {0,0,0,1},  {1,0,1,1}},
  {{0,1,1,1},   {1,0,0,1},  {0,1,1,1},  {0,1,0,1}},
  {{1,0,0,1},   {0,0,1,1},  {1,0,0,1},  {0,0,1,1}} },

{ {{1,0,0,1},   {0,1,0,1},  {0,0,0,0},  {0,0,1,1}},
  {{0,1,0,1},   {1,0,0,1},  {0,1,1,1},  {1,0,0,1}},
  {{1,0,1,1},   {0,1,1,1},  {1,0,0,1},  {0,1,0,1}},
  {{0,0,1,1},   {0,0,0,0},  {0,1,0,1},  {1,0,1,1}} },

{ {{0,0,0,0},   {0,0,1,1},  {0,1,0,1},  {1,0,0,1}},
  {{1,0,1,1},   {0,1,0,1},  {1,0,0,1},  {0,0,1,1}},
  {{0,1,0,1},   {1,0,0,1},  {0,0,1,1},  {0,0,0,0}},
  {{0,0,1,1},   {0,0,0,0},  {0,1,0,1},  {1,1,0,1}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,1,1,1},  {0,0,0,0}},
  {{0,1,0,1},   {0,0,1,1},  {1,0,0,1},  {1,0,0,1}},
  {{0,0,1,1},   {1,1,0,1},  {0,0,0,0},  {0,0,0,0}},
  {{1,0,0,1},   {0,0,0,0},  {0,0,0,0},  {0,1,1,1}} },

{ {{0,0,0,0},   {1,0,0,1},  {0,0,0,0},  {0,0,1,1}},
  {{0,0,1,1},   {0,0,0,1},  {0,1,0,1},  {0,0,0,0}},
  {{1,0,0,1},   {0,1,0,1},  {0,0,1,1},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {1,0,0,1},  {0,1,0,1}} },

{ {{1,0,0,1},   {0,0,0,0},  {0,1,0,1},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,1,1},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {1,0,0,1}},
  {{0,1,0,1},   {0,0,0,0},  {0,0,0,0},  {0,0,1,1}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,1,1},  {0,0,0,0},  {0,1,0,1}},
  {{0,0,0,0},   {1,0,0,1},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}} },

{ {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}},
  {{0,0,0,0},   {0,0,0,0},  {0,0,0,0},  {0,0,0,0}} } };

RGB2 paHalftone8x8[66][8][8] = {
{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {1,1,1,1}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}} },

{ {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} },

{ {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}} } };


ULONG abDither[]=
{
3674,2973,188,1971,2842,39,2197,824,4070,650,3309,1431,3073,1629,1955,2733,1782,3145,2033,3835,
3422,186,3631,1687,2144,407,1183,3649,1594,2345,3104,3530,6,1017,2257,663,3059,2380,1497,4083,
937,3143,1303,3442,452,113,3923,2711,1243,1900,814,2300,99,3015,2455,263,904,1163,2137,684,
1270,2312,4081,2651,645,3445,1143,4002,3242,1447,2549,3048,2004,236,3651,2295,408,2524,3367,
1123,484,953,10,1652,1179,1885,706,2573,3061,3911,2682,1914,3280,226,718,2147,3999,1540,2916,
251,2010,1105,381,3556,1903,2139,329,2543,1627,3629,1460,692,3003,4075,2558,1731,3885,1099,
1377,4045,2792,1758,2608,3739,3511,454,1036,1556,349,2145,1738,943,485,3627,605,3821,1587,2652,
944,2074,1258,3925,710,2168,3625,3991,2954,2219,2681,3228,3768,1060,50,1410,556,2226,985,3881,
2603,1772,518,3677,2662,1267,3780,3393,2749,688,1399,3697,768,2956,997,3270,2452,1811,2194,
203,1397,658,3199,2122,460,3614,2273,3144,52,3342,1454,2439,2000,2902,855,2496,3120,2699,2308,
1242,1866,318,1133,3532,2928,56,3737,3155,210,2848,1365,1562,2451,822,282,1487,457,2401,2025,
3395,1650,2873,388,1333,2965,1113,1979,2416,777,3179,114,1623,2236,3247,37,2665,1750,4008,1188,
2029,496,3818,939,3484,2920,3647,980,2694,3312,206,821,1323,566,1883,968,178,3203,3907,3569,
63,1526,3866,241,3345,2828,2399,3214,685,1743,2463,1518,870,1845,2618,359,3294,598,3080,3929,
3524,2843,1761,4031,3169,790,3749,3483,2386,94,3217,3408,297,1435,3904,1841,2504,483,3958,1063,
2360,3110,545,2227,2826,60,3382,1169,372,1967,2349,533,1519,1927,1679,2962,3828,2528,4005,3016,
2717,1684,1224,1922,3322,1315,753,2068,1645,4028,850,1394,2181,3965,517,3387,1156,2329,3495,
1720,3781,1137,1930,1330,2120,947,647,1257,166,2509,1139,1831,1524,4069,848,1682,3594,973,2988,
2105,1197,878,2840,1959,1516,3409,183,3735,1376,3146,1724,2516,2776,1614,3124,26,3847,3478,
2420,1059,2038,1532,338,2166,775,3771,2271,2632,440,2894,3686,3034,1016,92,3502,1938,3008,268,
2754,1981,3028,4084,776,2084,95,2758,2359,3385,132,2621,3004,3669,2170,2757,273,3066,643,2129,
2679,2844,2274,420,2586,633,3731,3528,3052,248,3808,1260,853,1863,2643,788,3967,617,3708,1292,
4025,1149,2596,744,399,3701,2817,638,3545,1138,3375,247,631,4060,1052,2392,1830,529,2215,2500,
2707,419,3794,1321,1004,3695,156,553,1282,2934,1018,3575,409,741,1605,3837,1868,365,1457,893,
3894,1960,3642,470,1388,167,1154,3953,1551,3353,314,1357,1708,2180,595,2553,2924,3586,324,2339,
1503,2080,223,2249,837,2881,2113,1416,3093,116,1294,3231,1783,2388,1577,2854,1415,2034,3187,
1613,295,3377,3833,1486,1144,770,3333,1658,2598,2206,1591,2523,3258,3843,1472,2566,1802,4042,
3184,1176,2266,3321,3544,1670,2352,3261,962,2520,3434,3805,2005,3072,1795,2793,2382,764,2642,
3291,987,4056,2011,1638,1103,3337,2970,983,3549,3204,1763,293,3366,1855,3998,2252,873,2627,
3949,11,3108,3666,931,3435,88,3897,2785,1298,655,3094,3591,1805,2372,16,3223,671,3585,1872,
286,2284,495,3114,2161,966,2830,214,549,2555,1278,434,2862,55,1220,2994,1745,736,3230,539,40,
1055,4020,1915,131,3665,1536,427,2436,3196,536,3814,120,1884,2723,456,2530,3630,610,988,2715,
1695,3451,499,2110,812,1911,405,2522,585,2244,1159,841,2584,2124,189,2874,2048,3880,1228,2912,
916,3952,1385,2816,860,1696,3659,24,1373,2409,3726,823,2013,3077,4053,668,2152,1539,3962,284,
2418,1332,2220,3709,1448,2128,3201,2890,1232,2298,3496,5,1387,2803,2189,762,3444,1350,1074,
3855,1512,3018,2412,179,3654,1094,1449,2972,3840,1204,2739,1747,3036,1471,3508,1943,3658,1701,
4050,946,618,317,3469,2085,461,3082,1106,2007,3374,3922,679,2977,1958,3272,1511,2725,3431,999,
1854,3747,2654,3568,1910,1023,2730,3602,862,2545,3443,265,924,3871,651,1820,3069,846,3934,1755,
1198,2501,4091,1628,2327,62,2056,1223,3906,416,3150,1999,2541,266,3325,2198,1351,3978,308,2448,
2886,151,3176,428,2429,1531,3149,2628,1430,1764,2483,123,3514,2365,204,1275,2600,1090,3516,
604,287,3908,1636,129,2443,1432,844,219,2315,3372,429,1624,2876,1853,591,3040,1692,2450,369,
2778,2108,1049,2624,3691,289,3091,2016,636,2961,2771,3260,717,2208,1359,2820,1568,686,3729,
1035,140,3534,780,2098,965,3802,674,1403,1114,2765,3397,2184,1067,3969,2788,3722,1558,750,2693,
3156,1608,2135,346,4010,1768,2317,1238,2169,2991,402,1142,3293,3112,614,2944,1185,4086,115,
3765,1314,2686,1124,3522,1465,3751,193,3381,1921,689,1468,3302,150,3742,908,504,1712,3527,1888,
816,3363,4068,2281,1779,2899,2411,1619,3207,1846,2622,1648,3303,2319,3928,1824,65,537,3609,
779,260,1200,2202,4030,574,1836,3811,2797,803,2490,2911,1022,3198,759,3688,3488,2796,2020,1676,
3887,1421,2171,1968,713,3264,2311,473,3982,1975,751,3183,1272,2361,478,3565,2852,2384,1153,
1829,3394,3956,2563,288,3759,2649,38,934,336,3181,626,2690,459,3639,45,4087,1265,352,2031,869,
2998,3733,1310,1952,2357,3051,3341,1923,2959,1053,410,3606,1171,1461,3417,71,3804,1931,2587,
534,1781,4007,36,2561,964,310,3498,2458,3100,1072,1554,2102,3354,252,2547,1728,4052,2938,1565,
2195,990,362,2678,1362,2118,1527,1012,3119,1174,2952,1693,2481,3559,1311,1966,3944,1434,1107,
3390,2823,583,3064,3525,2683,702,1607,2550,3249,432,1678,864,35,1489,2435,3241,139,2287,3057,
2052,509,1599,2770,238,1492,2356,1259,711,2261,3778,2780,1801,184,874,3876,2805,27,2978,909,
2250,1165,2655,68,805,3769,1709,4012,3046,3640,435,2447,111,2285,576,2077,1446,3839,1118,2151,
3356,881,2983,2355,813,2162,1886,3703,1068,279,2207,3900,169,2868,1006,3520,2613,3653,3869,
2024,918,3475,1774,669,3679,1322,3282,879,3955,3402,941,3133,3607,1588,481,3068,1291,3687,2616,
398,1733,3550,1379,3844,572,3662,386,2049,3332,3126,596,1993,200,886,2870,3212,3714,1293,4022,
3470,421,3075,2802,565,117,1726,3773,376,1578,2546,222,1480,2421,1767,1353,3447,1140,2060,4064,
1411,2256,687,1249,2879,2569,1369,4088,2685,256,2426,2979,2234,1180,2042,2884,371,1905,2695,
1111,3388,635,1507,2009,3299,1214,2389,1913,2729,1573,3039,3448,1028,1428,2432,1279,2591,3437,
2239,714,1653,1951,2751,1804,727,2390,211,1891,3646,2578,2242,1216,3148,3874,3449,959,3992,
3265,2807,494,3157,2472,577,1786,355,2761,194,3131,524,1643,340,843,2165,1083,3870,1674,707,
103,3744,2485,1378,3287,160,2116,2395,4023,2948,2233,544,3948,950,715,3244,174,2146,1776,3964,
2795,141,3834,1075,1843,1458,3985,1134,3346,272,954,2636,3277,1630,1025,4006,1496,756,2740,
1995,603,1285,2922,728,0,1980,3795,1584,938,3031,3693,3300,2104,3946,1842,2325,3327,3829,2919,
3186,1860,3384,437,3538,2724,1756,609,4072,856,3819,1706,782,327,1026,3595,136,3122,2577,313,
3611,1266,2477,628,857,2289,422,2941,3235,521,3580,13,2518,615,2987,3612,1390,3879,804,2314,
2917,315,3221,3597,91,3037,1792,2290,2612,1227,3584,849,2277,125,2676,1302,809,1534,1047,3490,
1177,127,1970,1517,593,18,1254,2575,1988,1445,3197,257,2292,2981,1199,3512,2811,1945,2505,1660,
1425,2853,1797,2041,4077,1054,2915,3800,1622,1920,3535,1537,833,2336,2713,3088,2099,3788,1555,
2187,74,1982,3113,1264,3458,500,1834,1130,2406,1413,415,3738,1649,2126,2999,360,3909,1881,3467,
2373,245,2964,2482,732,3641,2634,996,3715,2376,2800,3993,975,3065,826,3660,1076,1953,1550,2604,
9,3117,1356,3830,3347,666,1115,3472,2335,1482,417,3153,31,2648,3281,1189,2111,3937,1703,322,
1375,929,414,2404,1191,3430,641,2532,175,2091,3838,2667,919,4034,2039,3529,3229,240,551,3350,
1452,2773,1173,619,4015,1669,3790,482,2841,1735,2178,382,3025,1346,3560,1590,2268,331,3877,
2408,2827,3453,442,2176,3696,558,960,220,2688,2182,3752,85,765,2763,3376,1976,1328,3685,300,
649,2552,177,3711,2857,1977,3903,3254,1753,2908,4058,2767,1066,3690,1667,1343,2204,3362,196,
2836,806,1117,2697,4073,1032,2484,1732,3134,2148,992,3252,1946,32,3192,1414,4048,3351,867,2057,
677,215,3278,2094,547,1672,106,1312,3973,739,1787,3245,2348,4001,1857,378,3012,1284,3883,1717,
559,2232,995,2394,4029,2975,1020,3170,1289,763,3462,1147,2633,230,884,491,1502,1822,3310,570,
2875,712,1570,2525,1818,3798,1520,2383,1895,673,3517,64,3740,451,2614,1341,2240,2714,1215,627,
2425,168,1668,2735,3816,2544,1157,2976,1400,2672,3323,902,3017,2475,1120,1498,2032,2903,840,
1545,2457,3191,912,2597,3590,1564,2855,748,1788,1439,3439,2223,1851,2442,546,1639,3644,2075,
3135,3730,2269,285,3033,2437,20,3921,3175,464,1296,2246,78,3163,2932,1349,2072,836,2856,1525,
262,3432,3655,896,3842,2082,3513,1136,3136,453,1893,3452,1752,4059,705,3567,1847,2055,316,3868,
2722,3436,413,1222,3317,3699,522,2109,1897,294,3984,138,3111,3526,502,2054,79,2786,389,4085,
2995,51,2309,703,1319,1917,2691,976,3822,1218,3572,1918,1088,3473,3002,606,3896,935,366,3694,
3233,2322,3977,1912,3045,774,1600,347,2984,1850,1495,2851,3913,2231,1301,781,47,2326,1038,3767,
510,2270,3195,1625,72,704,3635,2557,195,1690,1048,3446,2891,1380,2427,854,2131,1245,2664,3875,
1603,3664,898,3234,1467,1014,3360,3957,2507,118,3487,745,2064,1473,2305,842,2737,281,2465,3596,
1675,2123,2564,1585,180,1093,1281,639,2514,3888,2353,1119,3324,97,2542,552,1013,255,3734,2708,
3220,2889,354,2527,1476,1178,2859,1001,3750,2164,1404,3107,2282,4039,2719,57,3824,1126,3311,
1825,3779,350,3262,1061,2495,660,1221,2028,2658,1793,2819,466,1547,3224,1697,3981,2837,334,
3129,1631,4017,2026,1443,758,3369,2774,563,3931,1821,2701,3587,3308,147,2014,1773,501,2673,
3996,785,3617,2368,3335,1440,2017,920,1642,2138,3054,3493,4019,233,2424,1940,2986,1759,571,
925,1962,654,1521,2237,404,3006,592,2745,1478,2278,235,3011,3378,2369,3799,344,3592,2149,1203,
928,2953,384,1125,2570,652,3420,142,2623,984,3250,198,1908,1273,1010,3424,3079,476,2172,1633,
1011,2806,3518,3090,1391,2191,1616,1961,2957,1715,600,2480,3638,3927,1335,124,1799,634,3313,
1325,819,3479,149,3859,2839,3551,3256,1217,2637,3622,1657,1024,3501,1949,791,3995,1688,1898,
112,1406,845,3103,170,3433,3882,2023,2385,3601,1355,2153,3774,1808,479,3678,1141,2263,4055,
2955,2346,1463,258,2459,894,2980,4080,586,1256,3706,192,991,3194,379,1201,4032,3092,205,1167,
445,3266,851,2742,2036,2605,448,3963,2710,1097,2479,1287,341,1784,3038,723,2021,4074,2364,4,
2936,2548,1290,3540,527,2860,3947,2560,1664,2333,731,2716,580,1785,44,3307,911,2893,1253,2367,
3049,2813,1721,449,54,3810,769,3621,1956,3791,12,1474,2615,2088,847,2444,3825,3476,2762,80,
2158,3555,752,2808,1919,2279,3758,1084,3624,1549,2318,1711,304,3162,1494,2063,2350,3939,128,
2486,932,254,1423,3239,3841,446,2117,958,3174,2218,1081,589,3670,1274,1867,3761,1402,3021,4076,
1601,516,1973,3954,1533,693,3756,2030,2510,3295,1182,2140,1671,2867,1295,3464,2253,3216,326,
1716,2933,1870,490,1515,2585,971,1810,1548,3440,2565,1408,2960,325,738,3106,3809,930,2177,3365,
3725,523,810,3406,1572,3785,3167,2831,1813,742,1172,1595,3667,2732,303,1522,3334,2006,2939,
397,3284,2536,312,1042,2213,2775,2471,3180,89,3357,306,993,1363,3573,865,2709,3177,514,2567,
697,1800,463,1128,3933,3410,681,2286,1297,3914,807,2921,2391,3852,512,8,4090,1661,2441,3383,
90,1386,2864,648,1839,21,2620,2925,1135,2159,511,1327,3570,2212,2669,3411,144,3029,1844,4026,
724,2656,30,3899,1552,948,2103,3154,3497,760,250,3710,1098,832,2196,2568,3926,3067,646,1571,
1852,3983,299,1033,3285,3862,2415,3030,1560,104,2731,1056,3251,2018,3608,246,3329,1329,2078,
3032,680,2183,1233,1828,2728,1996,3523,1209,3009,4067,989,1705,3648,2738,1936,332,1078,3972,
562,2002,2449,871,2303,1192,3441,1749,2438,1162,3537,2768,130,1686,1235,3864,1942,1491,2666,
3455,1848,1641,159,412,3407,2414,187,1424,2992,3645,2012,164,2684,927,1933,3547,2493,385,3005,
1677,644,2230,1151,3159,1019,2689,914,3536,3832,520,1037,3975,443,2511,1583,2262,3246,1384,
191,661,3084,3336,2534,1723,271,3138,1370,3861,584,202,3755,1336,3001,786,2283,623,3950,2431,
2909,444,2340,3101,1307,526,3628,2931,1234,2280,1941,2834,3743,2167,1146,2328,1640,815,1367,
3713,2192,1250,4009,1501,3736,48,2645,4037,450,1737,3724,290,1890,3139,227,2556,3267,2132,172,
3705,729,296,1978,3460,2456,4000,2296,1469,798,2914,3720,998,3489,1646,2590,2838,1957,469,2133,
3205,259,1876,3675,1442,887,3316,1760,153,4035,2097,719,2720,3263,1058,4089,923,560,3226,725,
2744,3416,4046,2877,301,616,3116,754,2106,956,3389,1381,1984,2865,2517,1485,3405,2354,1602,
1344,2905,820,1488,2370,3178,2804,901,3853,433,1241,1775,951,98,3893,2092,1252,2718,42,2163,
3301,1477,3109,3616,910,4041,1655,3348,1069,2058,2702,694,3554,1029,2971,353,2434,1455,1742,
3510,19,2635,1334,3552,1794,122,1481,431,3164,2375,1702,3352,207,1823,2750,2381,863,3563,171,
767,3873,540,2794,3626,640,3917,1744,3454,1110,1892,1288,2576,3074,2114,2880,3728,2692,3403,
1609,396,2374,690,1856,4095,797,345,1030,2506,87,1420,2592,2878,390,3085,3,3826,2224,2594,1632,
3704,915,3890,3118,474,2086,1662,3022,311,3919,2539,1000,2136,1878,3605,1073,2611,2942,3884,
1196,3096,368,1790,3210,2267,1269,2070,107,1131,2235,1972,34,3014,582,4033,3566,1729,1450,49,
818,1148,486,1882,3010,3588,3243,1441,3676,2937,1161,2358,3918,1798,2790,3468,1244,573,2310,
3971,1337,1861,1155,579,3206,1997,105,2209,787,2513,1475,3776,2272,1187,1998,2935,3757,691,
1280,23,3959,458,1426,2247,569,3707,1589,3970,1071,2963,3338,895,4054,3105,2674,963,3723,2497,
394,2210,197,667,3361,2323,3603,3190,2487,2193,221,1089,2810,528,2531,209,2071,3427,1582,734,
2245,298,2076,3634,859,1586,2526,3380,283,2821,1405,3421,1195,2766,3663,242,2861,620,3401,839,
2398,541,3306,2650,3465,2825,1580,2470,801,3519,119,2069,2478,682,2625,1929,423,1699,2464,1508,
333,3283,1246,2772,1621,3253,1008,2687,3943,1948,1544,613,4062,1324,783,3831,1986,942,1685,
3071,1339,624,3215,3727,2996,1108,3858,1833,3240,201,2947,2045,3748,830,4014,2488,530,1864,
1317,3288,1109,3966,1837,3158,81,1566,1096,1734,2229,882,2059,3684,1916,3269,1092,2818,1342,
3414,28,1438,3770,2847,3474,568,1840,3557,716,2019,1418,3783,2951,1212,447,3027,267,2769,1722,
3450,2644,1506,2297,3968,3506,393,3786,2607,121,1939,480,1483,3123,665,2407,982,3561,477,1513,
2344,1769,217,3047,3850,2331,2015,880,2671,358,1372,3539,4063,3053,467,3848,307,3182,1219,243,
3000,1740,3851,321,3044,3671,2338,722,253,2125,1318,3827,2302,162,3932,2403,800,1819,2130,2519,
875,3820,1104,2087,109,3056,471,3339,58,1205,2174,1780,2863,981,2445,3578,2640,46,2756,4078,
1719,1300,2631,1050,3165,699,3583,994,1561,2930,436,3564,1615,2454,2143,2791,1894,237,2515,
1389,2904,670,4024,2698,1504,2306,608,889,2154,1620,978,2700,3988,1102,3007,2606,926,3168,2901,
367,3426,76,3689,1656,3504,1374,3255,2422,970,3753,1849,683,2492,2753,811,3271,1529,4043,1309,
825,1680,2225,1210,361,2142,3319,83,3940,2900,1964,2188,2705,43,4051,1765,3081,148,3857,1003,
662,1247,2324,3619,969,3429,1789,2150,508,1015,3480,2562,4082,1858,3296,1213,3151,1994,3386,
59,1575,1739,587,1070,1932,1326,3142,698,2291,154,2906,1877,373,3598,1248,2888,1597,3161,2008,
3846,542,2337,323,2095,3171,3895,3358,761,3741,3050,1901,607,3668,1612,335,1164,3438,642,2083,
2582,1207,3343,554,2949,3719,3268,771,2040,1538,2629,133,2397,3315,3784,77,1239,2789,438,3574,
146,1757,538,2417,794,3531,2199,4071,2660,3613,1546,2777,4021,1031,2646,531,3916,746,2241,2589,
244,4036,1079,1417,173,3600,1181,3413,2945,152,1062,1954,2892,1401,2468,861,2779,2294,1345,
2538,3889,3202,1395,933,3657,784,2288,1991,1456,1736,2574,33,3128,3986,1160,3024,1611,766,1873,
3141,2050,1433,2363,828,2579,2898,1451,3910,356,2799,1202,3314,487,2466,2160,320,1226,3328,
2047,1559,3102,1436,3400,1707,2115,876,3515,2430,3013,1698,2653,747,1826,2529,515,3546,212,
1637,425,3509,1129,224,3368,817,507,1681,2410,2835,375,1593,3994,2696,291,3486,426,3807,1924,
872,455,3548,3902,1361,2871,275,3632,695,3019,3872,330,3700,2251,3218,1880,3754,1392,234,1806,
838,3815,3026,611,1748,3732,2405,913,2755,1,3817,401,3279,590,1889,370,955,2186,3974,1366,3789,
1541,2342,2704,4004,3232,1777,3813,2046,3086,1838,2968,3760,185,3463,1871,3219,82,1043,3097,
2203,1121,2872,1398,2301,2748,232,2067,2537,917,2248,1710,3364,1057,1563,2107,1268,922,622,
1086,2533,2089,3098,2866,1112,29,1965,2580,3491,199,439,3980,1989,1132,2969,1340,2677,2264,
3938,2812,3682,41,3078,629,2782,900,2035,1184,678,2221,967,2581,1464,4061,61,1005,2228,1306,
599,2940,3797,2428,1347,827,3920,1618,2476,581,3692,1730,3370,659,1122,3712,395,3987,2446,7,
1907,3461,3062,1704,2641,3423,86,888,3579,1479,3945,2377,3259,1409,986,2923,1271,3189,672,2313,
3604,1814,3121,757,1236,1509,3318,2051,1644,3481,261,3274,3698,3058,14,1354,2926,305,733,2351,
2752,3610,2001,2639,1145,1484,696,2101,2787,3618,1875,708,3412,137,3200,1261,2173,2997,1576,
3237,2657,1229,2943,602,2712,4057,163,465,2845,3961,1634,2316,675,3359,418,1666,3673,721,2259,
1874,3562,1651,2551,276,961,1569,489,3717,157,2572,1045,475,2460,2260,1095,1865,406,2498,3905,
3379,1950,3652,3160,561,1240,1542,400,3960,2343,278,3418,1751,492,3290,228,2990,2044,4092,952,
2599,364,3936,84,1947,532,1422,2141,3211,883,1152,2307,3633,1987,1308,302,3806,1899,2675,2185,
1194,2781,270,4079,2661,100,2112,3863,3331,2846,4049,2503,2022,3428,1791,2927,3898,829,1299,
4016,2910,1437,1727,802,513,1596,2680,1065,1762,3492,3276,793,3063,3718,2554,972,4047,1237,
2609,1470,2341,1168,2721,1505,1904,808,3521,2815,2387,3396,3849,218,1626,3745,2499,1444,3305,
795,3137,2462,2966,1007,182,3043,550,3166,899,1579,3392,1085,497,1358,835,1928,70,1158,2217,
890,3193,1407,213,3620,3132,2593,110,2190,3582,2801,2379,3792,2121,96,3979,2474,239,2155,1683,
135,1902,3188,2265,66,3764,921,392,3542,525,3070,3762,2275,1371,1080,1741,743,977,3581,1859,
2885,701,264,1809,2201,1127,564,1523,3636,1360,3997,1816,2073,3836,2467,612,3083,3680,2726,
2393,1493,3494,3042,383,3867,676,2347,2746,1896,557,1581,700,3297,1044,1263,181,907,3236,1368,
2946,1887,2809,1034,3533,1429,834,2974,1574,2832,1985,3130,1691,3878,1815,17,2494,632,3172,
190,4040,3035,2222,2602,498,2061,3419,3865,2760,411,4027,3466,2043,2583,789,3399,2402,73,1262,
2897,1427,2205,1746,309,3227,637,3775,1700,2668,1331,3576,1663,403,2119,3391,957,3856,1983,
3076,4094,1827,3477,424,2299,653,1567,3812,472,2659,3298,3915,548,357,1091,3500,2433,772,2883,
2134,1255,3415,1617,3656,2081,2706,292,1500,1170,3140,53,1352,1606,974,2540,3238,1713,25,2887,
380,1598,1046,3275,3593,363,1992,799,3941,2950,1251,2079,225,1027,2882,2003,143,4065,3020,1166,
3746,2703,2320,249,543,2489,1510,2989,2617,1150,3683,868,3147,1230,2027,2321,1283,3615,2157,
3854,656,1383,3304,269,1064,4013,2783,936,505,1862,1276,2440,3787,3320,858,4011,2419,2993,3661,
165,1277,885,2330,3892,1937,3721,2734,709,1771,2610,3782,3340,108,940,2559,4018,2293,3371,519,
2453,3289,1009,1528,2508,69,1778,1364,3558,2759,778,2090,280,3935,1963,3349,161,2423,4038,726,
2,2764,1665,2502,1832,2727,134,2243,3623,2595,1462,339,2362,2967,3891,3457,664,430,1673,2736,
2254,1211,621,2127,1909,2850,3571,657,3095,1316,2211,216,3023,2304,1190,555,1604,2366,3505,
1835,735,1535,3672,1231,1869,796,2214,3503,730,3185,462,2958,1635,1116,3326,3702,1320,1689,
597,2747,2175,1754,337,2918,1934,3456,1041,229,3373,1225,3990,1557,906,1925,3099,749,3344,1714,
67,1039,2858,1969,3643,145,1812,3471,328,3115,3793,503,1453,2663,1101,468,3482,945,1499,4044,
155,2829,3125,1051,1393,2798,351,3087,2630,15,2982,3951,274,2833,1304,1935,4003,852,2200,3912,
22,2400,979,3173,3589,1082,1396,3060,3637,1530,3803,588,3152,892,3041,2066,575,2913,3222,488,
3716,2037,1193,2216,2601,3208,1553,2179,3055,1087,3886,773,2638,1543,1021,2276,1796,4093,3330,
1718,2535,3845,2100,3213,891,3650,1926,2156,3860,578,3599,2053,877,3763,1412,1725,2588,567,
3801,2378,3404,2619,343,3127,1807,506,2896,2258,126,2512,493,3398,949,2571,391,2238,3930,1514,
2626,374,3766,2371,1654,102,2473,3942,277,3553,1382,866,4066,720,348,2521,1459,3257,1990,3989,
231,3209,2461,93,2062,319,2929,630,1879,377,2491,1338,441,2647,75,3292,1175,1647,2255,625,3225,
1100,3541,2093,1610,158,1040,1466,2065,1206,3796,2670,740,1490,3976,1906,3823,792,2096,1186,
1803,1348,2413,737,3577,1766,1077,3425,1286,2743,1817,1002,2869,594,3777,176,2396,3507,1305,
2814,535,2332,903,2895,1313,3499,755,2822,905,3772,1419,1208,2741,3355,1694,3924,3485,831,1770,
2907,2469,208,3901,2784,1944,2334,897,3089,387,3286,3681,2849,601,3459,1592,1974,3543,3273,
342,2985,1659,2824,101,3248,
};


// Defines for a Color or Color index locations in the
// bytes of the source bitmap depending on color count in the bitmap

// 1 bit per piexel
#define BIT0(b)         (ULONG)((*b & 0x80) >> 7)
#define BIT1(b)         (ULONG)((*b & 0x40) >> 6)
#define BIT2(b)         (ULONG)((*b & 0x20) >> 5)
#define BIT3(b)         (ULONG)((*b & 0x20) >> 4)
#define BIT4(b)         (ULONG)((*b & 0x08) >> 3)
#define BIT5(b)         (ULONG)((*b & 0x04) >> 2)
#define BIT6(b)         (ULONG)((*b & 0x02) >> 1)
#define BIT7(b)         (ULONG)((*b & 0x01)     )

// 2 bits per pixel
#define BIT01(b)        (ULONG)((*b & 0xC0) >> 6)
#define BIT23(b)        (ULONG)((*b & 0x30) >> 4)
#define BIT45(b)        (ULONG)((*b & 0x0C) >> 2)
#define BIT67(b)        (ULONG)((*b & 0x03)     )

// 4 bits per pixel
#define BIT0123(b)      (ULONG)((*b & 0xF0) >> 4)
#define BIT4567(b)      (ULONG)((*b & 0x0F)     )

// 8 bits per pixel
#define BITBYTE(b)      (ULONG)((*b & 0xFF)     )

// 16 bits per pixel
#define BIT01234(b)     (ULONG)((*b & 0xf8))
#define BIT56789(b)     (ULONG)(((*b & 0x07) << 5) & ((*(b+1) & 0xE0) >> 3))
#define BITBCDEF(b)     (ULONG)((*(b+1) & 0x1F) << 5)

// 24 bits per pixel
#define BITBLUE(b)      (*(b+0))
#define BITGREEN(b)     (*(b+1))
#define BITRED(b)       (*(b+2))

// Defines for color conversion RGB to CMYK
#define BLUEOFFSET      (33L*33L*4L)
#define GREENOFFSET     (33L*4L)
#define REDOFFSET       4L

#define BLUEPOS(iB)     (BLUEOFFSET  *(LONG)iB)
#define GREENPOS(iG)    (GREENOFFSET *(LONG)iG)
#define REDPOS(iR)      (REDOFFSET   *(LONG)iR)

#define LCYAN(pb)       ((LONG)(*(pb+0)))
#define LMAGENTA(pb)    ((LONG)(*(pb+1)))
#define LYELLOW(pb)     ((LONG)(*(pb+2)))
#define LBLACK(pb)      ((LONG)(*(pb+3)))

/****************************************************************************/
/* PROCEDURE NAME : ToCMYK                                                  */
/* AUTHOR         :                                                         */
/* DATE WRITTEN   : 8/4/00                                                  */
/* DESCRIPTION    : RGB to CMYK conversion                                  */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
void
ToCMYK (PTOCMYK pToCMYK)
{
#define BRIGHTNESS_THRESHOLD 2


    static TOCMYK saveToCMYK = {255, 255, 255, 0, 0, 0, 0, 0, 0, false};
    byte iCyanIn, iMagentaIn, iYellowIn, iMin, iMax, iColorRem;
    float fMult;
    float fBrightness = .30;
    byte  iMin_Add;

   // increase performance by caching last value

   if (saveToCMYK.bR == pToCMYK->bR &&       // previous color is the same ?
       saveToCMYK.bG == pToCMYK->bG &&
       saveToCMYK.bB == pToCMYK->bB &&
       saveToCMYK.bMult == pToCMYK->bMult)
   {
       pToCMYK->lC = saveToCMYK.lC;   // return result from previous conversion
       pToCMYK->lM = saveToCMYK.lM;
       pToCMYK->lY = saveToCMYK.lY;
       pToCMYK->lK = saveToCMYK.lK;

       return;
   }

   iCyanIn    = 0xFF & ~pToCMYK->bR;
   iMagentaIn = 0xFF & ~pToCMYK->bG;
   iYellowIn  = 0xFF & ~pToCMYK->bB;

   iMin = MIN3(iCyanIn, iMagentaIn, iYellowIn);
   iMax = MAX3(iCyanIn, iMagentaIn, iYellowIn);

   // Inkjets produce too much composite black therefore some special
   // handling is necessary.  If we decrease the total amount of color
   // for each pen, we lighten the output but don't come up with a
   // visually brighter output.  If we delete a small amount from the
   // minimum color(s) we can increase the perception that the R, G, or B
   // value has acutally increased thus increasing the perceived brightness

   fMult = (float)(iMax-iMin)/255.0;
   fMult *= fBrightness;
   iColorRem = (byte)((float)iMin * fMult);

   if(iColorRem > BRIGHTNESS_THRESHOLD)
   {
       iMin_Add = iMin+7;

       if(iCyanIn == iMin)
       {
           iCyanIn -= iColorRem;

           if(iMagentaIn < iMin_Add)
           {
               iMagentaIn -= (byte) iColorRem;
           }
           else
           {
               if(iYellowIn < iMin_Add)
                  iYellowIn -= (byte) iColorRem;
           }
       }
       else if(iMagentaIn == iMin)
       {
           iMagentaIn -= iColorRem;

           if(iCyanIn < iMin_Add)
           {
               iCyanIn -= (byte) iColorRem;
           }
           else
           {
               if(iYellowIn < iMin_Add)
                  iYellowIn -= (byte) iColorRem;
           }
       }
       else if(iYellowIn == iMin)
       {
           iYellowIn -= iColorRem;

           if(iCyanIn < iMin_Add)
           {
               iCyanIn -= (byte) iColorRem;
           }
           else
           {
               if(iMagentaIn < iMin_Add)
                  iMagentaIn -= (byte) iColorRem;
           }
       }
   }

   // if near pure black, we want to increase black output
   // but if not, rely mostly on composites for smooth color to dark
   // transitions

   if(iMin > 223)
      pToCMYK->lK =(ULONG)((float) iMin  * .3);  // add but deplete black
   else
      pToCMYK->lK = (ULONG)0;

   pToCMYK->lC = (ULONG)iCyanIn    ;
   pToCMYK->lM = (ULONG)iMagentaIn ;
   pToCMYK->lY = (ULONG)iYellowIn  ;

   memcpy(&saveToCMYK, pToCMYK, sizeof(TOCMYK));

}

/****************************************************************************/
/* PROCEDURE NAME : GplCreateHSVcolorTable                                  */
/* DATE WRITTEN   : 10/26/94                                                */
/* DESCRIPTION    :  See below                                              */
/*                                                                          */
/*                  Control Flow:                                           */
/*                                                                          */
/* PARAMETERS:     PHSVARRAY phsv, PDITHEREQUEST pReq, PDITHERESULT  pReply */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
/*
      GplCreateHSVcolorTable

   Inputs:  pSrcInfo pointer to a BITMAPINFO object that includes
                     a complete 256 entry color table. Since it is
                     assumed that this will be a static  function for now,
                     BITMAPINFO size is not used to derrive num entries.

            phsv     pointer to an HSV array [256]

   Outputs: (hsvTable) an HSV color table is stuffed by this function
                           struct {
                                  ULONG ulHue;            0..7e8   index.color of color fraction
                                  ULONG ulSaturation;     0..256   0..1e8 %
                                  ULONG ulValue;          0..256   0..1e8 %
                                  BOOL  fIsNotWhite;
                                  } hsv[256];

TO DO NOTE: Accomodate 1, 2, 4 bit bitmaps. (1, 4, 16)
*/
LONG GplDitherInstance::
GplCreateHSVcolorTable (PBITMAPINFO2 pbmi2)
{
   LONG ldelta, li, lh, lv, lmin, lr, lg, lb, lr1, lg1, lb1;
   ULONG ulDelta;

   if (phsvTable_d == NULL)
      return INVALID_PARAMETER;

   // Only do this function for an 8-bit color table
   if (256 != iNumColors_d)
      return 0;

   // Note: then end of this loop may changed to use the bitmapinfo size if valid
   for (li = 0; li < 256; li++)
   {
      lr = pbmi2->argbColor[li].bRed;
      lg = pbmi2->argbColor[li].bGreen;
      lb = pbmi2->argbColor[li].bBlue;

      if (  lr == MAX_RGB_VALUE
         && lg == MAX_RGB_VALUE
         && lb == MAX_RGB_VALUE
         )
         phsvTable_d->is[li].fIsNotWhite = false;
      else
         phsvTable_d->is[li].fIsNotWhite = true;

      lv = MAX3 (lr, lg, lb);

#ifdef DELTA_LEVEL
      //Level
      phsvTable_d->is[li].ulValue = lv - (LONG)delta_d.lValue;
#else
      // Percentage
      phsvTable_d->is[li].ulValue = (lv * (256 - (LONG)delta_d.lValue)) / 256;
#endif
      if ((LONG)phsvTable_d->is[li].ulValue < 0)
         phsvTable_d->is[li].ulValue = 0;
      else if ((LONG)phsvTable_d->is[li].ulValue > MAX_RGB_VALUE)
         phsvTable_d->is[li].ulValue = MAX_RGB_VALUE;
      lmin = MIN3 (lr, lg, lb);
      lh = 0;
      if (lv != 0)
      {
         ldelta = lv - lmin;
         phsvTable_d->is[li].ulSaturation = (ldelta << PRECISION) / lv;
         if (phsvTable_d->is[li].fIsNotWhite)
         {
#ifdef DELTA_LEVEL
            //Level
            phsvTable_d->is[li].ulSaturation = (LONG)phsvTable_d->is[li].ulSaturation + delta_d.lSaturation;
#else
            // Percentage
            phsvTable_d->is[li].ulSaturation = ((LONG)phsvTable_d->is[li].ulSaturation * (delta_d.lSaturation + 256))/256;
#endif
            if ((LONG)phsvTable_d->is[li].ulSaturation < 0)
               phsvTable_d->is[li].ulSaturation = 0;
            else if ((LONG)phsvTable_d->is[li].ulSaturation > 256)
               phsvTable_d->is[li].ulSaturation = 256;
         }

         if (  phsvTable_d->is[li].ulSaturation != 0
            && ldelta
            && phsvTable_d->is[li].fIsNotWhite
            )
         {
            lr1 = ((lv-lr) << PRECISION)/ldelta;
            lg1 = ((lv-lg) << PRECISION)/ldelta;
            lb1 = ((lv-lb) << PRECISION)/ldelta;
            if (lv == lr)
               if (lmin == lg)
                  lh = (5 << PRECISION) + lb1;
               else
                  lh = (1 << PRECISION) - lg1;
            else if (lv == lg)
               if (lmin == lb)
                  lh = (1 << PRECISION) + lr1;
               else
                  lh = (3<<PRECISION) - lb1;
            else if (lmin == lr)
               lh = (3 << PRECISION) + lg1;
            else
               lh = (5 << PRECISION) - lr1;

            lh = ((lh * 60) + (delta_d.lHue << PRECISION)) / 60;

            if (lh < 0)
               lh  += (6 << PRECISION);
            else if (lh >= (6 << PRECISION))
               lh -= (6 << PRECISION);
            phsvTable_d->is[li].ulHue = lh ;
         }
      }

      if (phsvTable_d->is[li].fIsNotWhite)
      {
         phsvTable_d->is[li].ulBlack = MAX_RGB_VALUE - (LONG)phsvTable_d->is[li].ulValue;

         phsvTable_d->is[li].fValueSat = false;
         if (  phsvTable_d->is[li].ulSaturation != 0
            && phsvTable_d->is[li].ulValue != 0
            )
            phsvTable_d->is[li].fValueSat = true;

         phsvTable_d->is[li].ulBlackWhite = (LONG)phsvTable_d->is[li].ulBlack
                                          + ((((256 - (LONG)phsvTable_d->is[li].ulSaturation) * (LONG)phsvTable_d->is[li].ulValue) >> PRECISION));

         lh = phsvTable_d->is[li].ulHue;
         ulDelta = lh & 0xff;

         if (ulDelta > 0x80)
            phsvTable_d->is[li].fIsGreaterThan0x80 = true;
         else
            phsvTable_d->is[li].fIsGreaterThan0x80 = false;

         phsvTable_d->is[li].ulGT80 = (LONG)phsvTable_d->is[li].ulBlackWhite +
                                      (((MAX_RGB_VALUE - (LONG)phsvTable_d->is[li].ulBlackWhite) * (LONG)ulDelta ) >> PRECISION);

         phsvTable_d->is[li].ulLE80 = (LONG)phsvTable_d->is[li].ulBlackWhite +
                               (((MAX_RGB_VALUE - (LONG)phsvTable_d->is[li].ulBlackWhite) * (0x100 - (LONG)ulDelta) ) >> PRECISION);

         phsvTable_d->is[li].ulcolor1 = ulPrimColors[((lh >> PRECISION) & 7) + 1];
         phsvTable_d->is[li].ulcolor  = ulPrimColors[(lh >> PRECISION) & 7];
      }
   }

   return 0;
}

/****************************************************************************/
/* PROCEDURE NAME : ToCMYK6                                                 */
/* AUTHOR         :                                                         */
/* DATE WRITTEN   : 11/7/00                                                 */
/* DESCRIPTION    : RGB to CMYK conversion for six color devices using      */
/*                  light cyan and light magenta                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
void
ToCMYK6 (PTOCMYK pToCMYK, byte *pbLightTable, byte *pbDarkTable)
{
#define BRIGHTNESS_THRESHOLD 2


   static TOCMYK saveToCMYK = {255, 255, 255, 0, 0, 0, 0, 0, 0, false};
   byte iCyanIn, iMagentaIn, iYellowIn, iMin, iMax, iColorRem;
   byte iLCyanIn    = 0;  //@@SIX-COLOR
   byte iLMagentaIn = 0;  //@@SIX-COLOR
   float fBrightness = .30;
//   float iTotalChange;
   float fMult;
   byte iMin_Add;

   // increase performance by caching last value

   if (saveToCMYK.bR == pToCMYK->bR &&       // previous color is the same ?
       saveToCMYK.bG == pToCMYK->bG &&
       saveToCMYK.bB == pToCMYK->bB &&
       saveToCMYK.bMult == pToCMYK->bMult)
   {
       pToCMYK->lC = saveToCMYK.lC;   // return result from previous conversion
       pToCMYK->lM = saveToCMYK.lM;
       pToCMYK->lY = saveToCMYK.lY;
       pToCMYK->lK = saveToCMYK.lK;
       pToCMYK->lLC = saveToCMYK.lLC;     //@@SIX-COLOR
       pToCMYK->lLM = saveToCMYK.lLM;     //@@SIX-COLOR

       return;
   }

   iCyanIn    = 0xFF & ~pToCMYK->bR;
   iMagentaIn = 0xFF & ~pToCMYK->bG;
   iYellowIn  = 0xFF & ~pToCMYK->bB;

   iMin = MIN3(iCyanIn, iMagentaIn, iYellowIn);
   iMax = MAX3(iCyanIn, iMagentaIn, iYellowIn);

      // check to see if there is a large difference in colors so that we can decide
      // if it is worth altering the colors to produce brightening effects.
     // if we don't need the math, don't do it
   if((iMax - iMin) > 10 )
   {                // Brighten the higher colors based on the amount of difference
       fMult = (float)(iMax-iMin)/255.0;
       fMult *= fBrightness;
       iColorRem = (byte)((float)iMin * fMult);

       if(iColorRem > BRIGHTNESS_THRESHOLD)
       {
           iMin_Add = iMin+7;
           if(iCyanIn == iMin)
           {
               iCyanIn -= iColorRem;

               if(iMagentaIn < iMin_Add)
               {
                   iMagentaIn -= (byte) iColorRem;
               }
               else
               {
                   if(iYellowIn < iMin_Add)
                      iYellowIn -= (byte) iColorRem;
               }
           }
           else if(iMagentaIn == iMin)
           {
               iMagentaIn -= iColorRem;

               if(iCyanIn < iMin_Add)
               {
                   iCyanIn -= (byte) iColorRem;
               }
               else
               {
                   if(iYellowIn < iMin_Add)
                      iYellowIn -= (byte) iColorRem;
               }
           }
           else if(iYellowIn == iMin)
           {
               iYellowIn -= iColorRem;

               if(iCyanIn < iMin_Add)
               {
                   iCyanIn -= (byte) iColorRem;
               }
               else
               {
                   if(iMagentaIn < iMin_Add)
                      iMagentaIn -= (byte) iColorRem;
               }
           }
       }
   }

   // if near pure black, we want to increase black output
   // but if not, rely mostly on composites for smooth color to dark
   // transitions

   if(iMin > 223)
      pToCMYK->lK =(ULONG)((float) iMin  * .3);  // add but deplete black
   else
      pToCMYK->lK = (ULONG)0;

   // To force color brightness, we need to add to the lower value to get
   // a perceived brightness but it is contradictory to how to get higher
   // intensity of a color.  For higher intensity (saturation) we have to
   // raise the values of the higher colors

   //@@SIX-COLOR - derive light cyan from cyan plane
   if(iCyanIn)
   {
       iLCyanIn = pbLightTable[iCyanIn];
       iCyanIn  = pbDarkTable[iCyanIn];
   }

   //@@SIX-COLOR - derive light magenta from magenta plane
   if(iMagentaIn)
   {
       iLMagentaIn = pbLightTable[iMagentaIn];
       iMagentaIn = pbDarkTable[iMagentaIn];
   }

   pToCMYK->lC = (ULONG)iCyanIn      ;
   pToCMYK->lM = (ULONG)iMagentaIn   ;
   pToCMYK->lY = (ULONG)iYellowIn    ;
   pToCMYK->lLC = (ULONG)iLCyanIn    ;
   pToCMYK->lLM = (ULONG)iLMagentaIn ;

   memcpy(&saveToCMYK, pToCMYK, sizeof(TOCMYK));

}

