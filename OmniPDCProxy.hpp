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
 *
 * Idea borrowed from http://hpinkjet.sourceforge.net
 */
#ifndef _OmniPDCProxy
#define _OmniPDCProxy

#include "Device.hpp"
#include "PrinterCommand.hpp"

#include <unistd.h>

class OmniPDCProxy : public Device
{
public:
                         OmniPDCProxy              (PSZRO               pszClientExe,
                                                    PSZCRO              pszDeviceName,
                                                    PSZCRO              pszJobProperties,
                                                    bool                fAdvanced = false,
                                                    int                 fdStdOut  = STDOUT_FILENO,
                                                    int                 fdStdErr  = STDERR_FILENO);
   virtual              ~OmniPDCProxy              ();

   bool                  hasError                  ();

   void                  initialize                ();

   PSZCRO                getVersion                ();

   PSZCRO                getDriverName             ();
   PSZCRO                getDeviceName             ();
   PSZCRO                getShortName              ();
   PSZCRO                getLibraryName            ();
   EOMNICLASS            getOmniClass              ();

#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet        *getCurrentBooklet         ();
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies         *getCurrentCopies          ();
#endif
   PSZCRO                getCurrentDitherID        ();
   DeviceForm           *getCurrentForm            ();
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging        *getCurrentJogging         ();
#endif
   DeviceMedia          *getCurrentMedia           ();
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp            *getCurrentNUp             ();
#endif
   DeviceOrientation    *getCurrentOrientation     ();
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin      *getCurrentOutputBin       ();
#endif
   DevicePrintMode      *getCurrentPrintMode       ();
   DeviceResolution     *getCurrentResolution      ();
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling        *getCurrentScaling         ();
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate   *getCurrentSheetCollate    ();
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide           *getCurrentSide            ();
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching      *getCurrentStitching       ();
#endif
   DeviceTray           *getCurrentTray            ();
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming       *getCurrentTrimming        ();
#endif

   DeviceGamma          *getCurrentGamma           ();

   DeviceInstance       *getInstance               ();

   int                   getPDLLevel               ();
   int                   getPDLSubLevel            ();
   int                   getPDLMajorRevisionLevel  ();
   int                   getPDLMinorRevisionLevel  ();

   bool                  hasCapability             (long                lMask);
   bool                  hasRasterCapability       (long                lMask);
   bool                  hasDeviceOption           (PSZCRO              pszDeviceOption);

   int                   getScanlineMultiple       ();

   bool                  beginJob                  ();
   bool                  beginJob                  (PSZCRO              pszJobProperties);
   bool                  newFrame                  ();
   bool                  newFrame                  (PSZCRO              pszJobProperties);
   bool                  endJob                    ();
   bool                  abortJob                  ();

   bool                  rasterize                 (PBYTE               pbBits,
                                                    PBITMAPINFO2        pbmi,
                                                    PRECTL              prectlPageLocation,
                                                    BITBLT_TYPE         eType);

   void                  setOutputStream           (FILE               *pFile);
   void                  setOutputFunction         (PFNOUTPUTFUNCTION   pfnOutputFunction,
                                                    void               *pMagicCookie);
   void                  setErrorStream            (FILE               *pFile);

   bool                  setLanguage               (int                 iLanguageID);
   int                   getLanguage               ();
   Enumeration          *getLanguages              ();
   StringResource       *getLanguageResource       ();

   std::string          *getJobProperties          (bool                fInDeviceSpecific = false);
   bool                  setJobProperties          (PSZCRO              pszJobProperties);
   Enumeration          *listJobProperties         (bool                fInDeviceSpecific = false);

   std::string          *getJobPropertyType        (PSZCRO              pszKey);
   std::string          *getJobProperty            (PSZCRO              pszKey);
   std::string          *translateKeyValue         (PSZCRO              pszKey,
                                                    PSZCRO              pszValue);

   Enumeration          *getDitherEnumeration      (bool                fInDeviceSpecific = false);

#ifndef RETAIL
   virtual void          outputSelf                ();
#endif
   std::string           toString                  (std::ostringstream& oss);
   friend std::ostream&  operator<<                (std::ostream&       os,
                                                    const OmniPDCProxy& self);

private:
   void                  cleanupInstance           ();
   bool                  queryPDLInfo              ();

   PSZ                  pszClientExe_d;

   bool                 fHasError_d;

   bool                 fAdvanced_d;

   int                  fdS2C_d;
   int                  fdC2S_d;
   char                *pszS2C_d;
   char                *pszC2S_d;

   int                  idBuffer1_d;
   int                  cbBuffer1_d;
   byte                *pbBuffer1_d;
   int                  idBuffer2_d;
   int                  cbBuffer2_d;
   byte                *pbBuffer2_d;

   PrinterCommand      *pCmd_d;

   char                *pszVersion_d;
   char                *pszDriverName_d;
   char                *pszDeviceName_d;
   char                *pszShortName_d;
   char                *pszLibraryName_d;
   EOMNICLASS           eOmniClass_d;

   int                  iLanguageID_d;
   StringResource      *pLanguage_d;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet       *pBooklet_d;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies        *pCopies_d;
#endif
   char                *pszDitherID_d;
   DeviceForm          *pForm_d;
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging       *pJogging_d;
#endif
   DeviceMedia         *pMedia_d;
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp           *pNUp_d;
#endif
   DeviceOrientation   *pOrientation_d;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin     *pOutputBin_d;
#endif
   DevicePrintMode     *pPrintMode_d;
   DeviceResolution    *pResolution_d;
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling       *pScaling_d;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate  *pSheetCollate_d;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide          *pSide_d;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching     *pStitching_d;
#endif
   DeviceTray          *pTray_d;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming      *pTrimming_d;
#endif
   DeviceGamma         *pGamma_d;

   bool                 fQueriedPDLInfo_d;
   int                  iPDLLevel_d;
   int                  iPDLSubLevel_d;
   int                  iPDLMajorRevisionLevel_d;
   int                  iPDLMinorRevisionLevel_d;
};

#endif
