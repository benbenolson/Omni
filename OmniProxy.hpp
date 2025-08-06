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
#ifndef _OmniProxy
#define _OmniProxy

#include "Device.hpp"

#include <cstdio>

class OmniProxy : public Device
{
public:
   typedef struct _BitbltHeader {
      off_t       pos;                  // @TBD-VERIFY was: fpos_t      pos;
      RECTL       rectlPageLocation;
      int         cPlanes;
      int         cBitCount;
      int         cbBitmapHeader;
      int         cbBitmapData;
      BITBLT_TYPE eType;
   } BITBLT_HEADER, *PBITBLT_HEADER;

   typedef struct _BitbltHeaders {
      struct _BitbltHeaders *pNext;
      int                    cHeaders;
      BITBLT_HEADER          aHeaders[1];
   } BITBLT_HEADERS, *PBITBLT_HEADERS;

   #define BITBLT_HEADERS_BLOCK 4096
   #define BITBLT_HEADERS_COUNT ( ( BITBLT_HEADERS_BLOCK                    \
                                  - sizeof (((PBITBLT_HEADERS)0)->pNext)    \
                                  - sizeof (((PBITBLT_HEADERS)0)->cHeaders) \
                                  )                                         \
                                / sizeof (BITBLT_HEADERS_BLOCK)             \
                                )

                              OmniProxy                 (Device             *pDevice);
   virtual                   ~OmniProxy                 ();

   bool                       hasError                  ();

   void                       initialize                ();

   PSZCRO                     getVersion                ();

   PSZCRO                     getDriverName             ();
   PSZCRO                     getDeviceName             ();
   PSZCRO                     getShortName              ();
   PSZCRO                     getLibraryName            ();
   EOMNICLASS                 getOmniClass              ();

#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet             *getCurrentBooklet         ();
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies              *getCurrentCopies          ();
#endif
   PSZCRO                     getCurrentDitherID        ();
   DeviceForm                *getCurrentForm            ();
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging             *getCurrentJogging         ();
#endif
   DeviceMedia               *getCurrentMedia           ();
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp                 *getCurrentNUp             ();
#endif
   DeviceOrientation         *getCurrentOrientation     ();
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin           *getCurrentOutputBin       ();
#endif
   DevicePrintMode           *getCurrentPrintMode       ();
   DeviceResolution          *getCurrentResolution      ();
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling             *getCurrentScaling         ();
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate        *getCurrentSheetCollate    ();
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide                *getCurrentSide            ();
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching           *getCurrentStitching       ();
#endif
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming            *getCurrentTrimming        ();
#endif
   DeviceTray                *getCurrentTray            ();

   DeviceGamma               *getCurrentGamma           ();

   DeviceInstance            *getInstance               ();

   int                        getPDLLevel               ();
   int                        getPDLSubLevel            ();
   int                        getPDLMajorRevisionLevel  ();
   int                        getPDLMinorRevisionLevel  ();

   bool                       hasCapability             (long                lMask);
   bool                       hasRasterCapability       (long                lMask);
   bool                       hasDeviceOption           (PSZCRO              pszDeviceOption);

   int                        getScanlineMultiple       ();

   bool                       beginJob                  ();
   bool                       beginJob                  (PSZCRO              pszJobProperties);
   bool                       newFrame                  ();
   bool                       newFrame                  (PSZCRO              pszJobProperties);
   bool                       endJob                    ();
   bool                       abortJob                  ();

   bool                       rasterize                 (PBYTE               pbBits,
                                                         PBITMAPINFO2        pbmi,
                                                         PRECTL              prectlPageLocation,
                                                         BITBLT_TYPE         eType);

   void                       setOutputStream           (FILE               *pFile);
   void                       setOutputFunction         (PFNOUTPUTFUNCTION   pfnOutputFunction,
                                                         void               *pMagicCookie);
   void                       setErrorStream            (FILE               *pFile);

   bool                       setLanguage               (int                 iLanguageID);
   int                        getLanguage               ();
   Enumeration               *getLanguages              ();
   StringResource            *getLanguageResource       ();

   std::string               *getJobProperties          (bool                fInDeviceSpecific);
   bool                       setJobProperties          (PSZCRO              pszJobProperties);
   Enumeration               *listJobProperties         (bool                fInDeviceSpecific);

   std::string               *getJobPropertyType        (PSZCRO              pszKey);
   std::string               *getJobProperty            (PSZCRO              pszKey);
   std::string               *translateKeyValue         (PSZCRO              pszKey,
                                                         PSZCRO              pszValue);

#ifndef RETAIL
   virtual void               outputSelf                ();
#endif
   std::string                toString                  (std::ostringstream& oss);

private:
   void                      allocateBitmapInfo        ();
   void                      freeBitmapInfo            ();
   void                      replayBitmaps             ();

   Device          *pDevice_d;
   PBITBLT_HEADERS  pHeadersRoot_d;
   PBITBLT_HEADERS  pHeadersCurrent_d;
   PBITBLT_HEADER   pHeaderCurrent_d;
   FILE            *fp_d;
   int              fd_d;
   int              iMaxScanLines_d;
};

#endif
