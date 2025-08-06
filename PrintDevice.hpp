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
#ifndef _PrintDevice
#define _PrintDevice

#include "Device.hpp"

#include <map>

#include <glib.h>
#include <gmodule.h>

class PrintDevice : public Device
{
protected:
                               PrintDevice               (PSZRO                pszDriverName,
                                                          PSZRO                pszDeviceName,
                                                          PSZRO                pszShortName,
                                                          PSZRO                pszLibraryName,
                                                          EOMNICLASS           eOmniClass,
                                                          PSZRO                pszJobProperties);

public:
   virtual                    ~PrintDevice               ();

   bool                        hasError                  ();

   void                        initialize                ();

   PSZCRO                      getVersion                ();

   PSZCRO                      getDriverName             ();
   PSZCRO                      getDeviceName             ();
   PSZCRO                      getShortName              ();
   PSZCRO                      getLibraryName            ();
   EOMNICLASS                  getOmniClass              ();

   virtual DeviceInstance     *getDeviceInstance         ();
   virtual DeviceBlitter      *getDeviceBlitter          ();

#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet              *getCurrentBooklet         ();
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies               *getCurrentCopies          ();
#endif
   PSZCRO                      getCurrentDitherID        ();
   DeviceDither               *getCurrentDither          ();
   DeviceForm                 *getCurrentForm            ();
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging              *getCurrentJogging         ();
#endif
   DeviceMedia                *getCurrentMedia           ();
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp                  *getCurrentNUp             ();
#endif
   DeviceOrientation          *getCurrentOrientation     ();
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin            *getCurrentOutputBin       ();
#endif
   DevicePrintMode            *getCurrentPrintMode       ();
   DeviceResolution           *getCurrentResolution      ();
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling              *getCurrentScaling         ();
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate         *getCurrentSheetCollate    ();
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide                 *getCurrentSide            ();
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching            *getCurrentStitching       ();
#endif
   DeviceTray                 *getCurrentTray            ();
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming             *getCurrentTrimming        ();
#endif

   virtual DeviceGamma        *getCurrentGamma           () = 0;

   DeviceInstance             *getInstance               ();

   DeviceCommand              *getCommands               ();
   DeviceData                 *getDeviceData             ();
   DeviceString               *getDeviceString           ();

   int                         getPDLLevel               ();
   int                         getPDLSubLevel            ();
   int                         getPDLMajorRevisionLevel  ();
   int                         getPDLMinorRevisionLevel  ();

   bool                        hasCapability             (long                 lMask);
   bool                        hasRasterCapability       (long                 lMask);
   bool                        hasDeviceOption           (PSZCRO               pszDeviceOption);

   int                         getScanlineMultiple       ();

   bool                        beginJob                  ();
   bool                        beginJob                  (PSZCRO               pszJobProperties);
   bool                        newFrame                  ();
   bool                        newFrame                  (PSZCRO               pszJobProperties);
   bool                        endJob                    ();
   bool                        abortJob                  ();

   bool                        rasterize                 (PBYTE                pbBits,
                                                          PBITMAPINFO2         pbmi,
                                                          PRECTL               prectlPageLocation,
                                                          BITBLT_TYPE          eType);

   void                        setOutputStream           (FILE                *pFile);
   void                        setOutputFunction         (PFNOUTPUTFUNCTION    pfnOutputFunction,
                                                          void                *pMagicCookie);
   void                        setErrorStream            (FILE                *pFile);

   bool                        setLanguage               (int                  iLanguageID);
   int                         getLanguage               ();
   Enumeration                *getLanguages              ();
   StringResource             *getLanguageResource       ();

   bool                        sendBinaryDataToDevice    (BinaryData          *pData);
   bool                        sendBinaryDataToDevice    (DeviceForm          *pForm);
   bool                        sendBinaryDataToDevice    (DeviceTray          *pTray);
   bool                        sendBinaryDataToDevice    (DeviceMedia         *pMedia);
   bool                        sendBinaryDataToDevice    (DeviceResolution    *pResolution);
   bool                        sendBinaryDataToDevice    (PBYTE                pbData,
                                                          int                  iLength);

   bool                        sendPrintfToDevice        (BinaryData          *pData,
                                                                               ...);
   bool                        sendVPrintfToDevice       (BinaryData          *pData,
                                                          va_list              list);

   std::string                *getJobProperties          (bool                 fInDeviceSpecific = false);
   bool                        setJobProperties          (PSZCRO               pszJobProperties);
   Enumeration                *listJobProperties         (bool                 fInDeviceSpecific);

   std::string                *getJobPropertyType        (PSZCRO               pszKey);
   std::string                *getJobProperty            (PSZCRO               pszKey);
   std::string                *translateKeyValue         (PSZCRO               pszKey,
                                                          PSZCRO               pszValue);

   void                        loadLibrary               (PSZCRO               pszLibraryName);
   void                       *dlsym                     (PSZCRO               pszLibraryName,
                                                          PSZCRO               pszSymbol);

#ifndef RETAIL
   virtual void                outputSelf                ();
#endif
   virtual std::string         toString                  (std::ostringstream&  oss);
   friend std::ostream&        operator<<                (std::ostream&        os,
                                                          const PrintDevice&   self);

protected:
   void                        setDeviceInstance         (DeviceInstance      *pInstance);
   void                        setDeviceBlitter          (DeviceBlitter       *pBlitter);
   void                        setPDL                    (PDL                 *pPDL);

   void                        setCapabilities           (long                 lCapabilities);
   void                        setRasterCapabilities     (long                 lCapabilities);

#ifdef INCLUDE_JP_UPDF_BOOKLET
   virtual DeviceBooklet      *getDefaultBooklet         () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   virtual DeviceCopies       *getDefaultCopies          () = 0;
#endif
   virtual PSZCRO              getDefaultDitherID        () = 0;
   virtual DeviceForm         *getDefaultForm            () = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   virtual DeviceJogging      *getDefaultJogging         () = 0;
#endif
   virtual DeviceMedia        *getDefaultMedia           () = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   virtual DeviceNUp          *getDefaultNUp             () = 0;
#endif
   virtual DeviceOrientation  *getDefaultOrientation     () = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   virtual DeviceOutputBin    *getDefaultOutputBin       () = 0;
#endif
   virtual DevicePrintMode    *getDefaultPrintMode       () = 0;
   virtual DeviceResolution   *getDefaultResolution      () = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   virtual DeviceScaling      *getDefaultScaling         () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   virtual DeviceSheetCollate *getDefaultSheetCollate    () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   virtual DeviceSide         *getDefaultSide            () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   virtual DeviceStitching    *getDefaultStitching       () = 0;
#endif
   virtual DeviceTray         *getDefaultTray            () = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   virtual DeviceTrimming     *getDefaultTrimming        () = 0;
#endif

   virtual DeviceCommand      *getDefaultCommands        () = 0;
   virtual DeviceData         *getDefaultData            () = 0;
   virtual DeviceString       *getDefaultString          () = 0;

private:
   bool                        initializeJobProperties   ();
   void                        cleanupProperties         ();

   std::ostream        *outputStream_d;
   std::streambuf      *outputStreamBuf_d;
   bool                 fShouldDeleteOutputStream_d;

   PFNOUTPUTFUNCTION    pfnOutputFunction_d;
   void                *pMagicCookie_d;

   int                  iLanguageID_d;
   StringResource      *pLanguage_d;

   PSZRO                pszDriverName_d;
   PSZRO                pszDeviceName_d;
   PSZRO                pszShortName_d;
   PSZRO                pszLibraryName_d;
   EOMNICLASS           eOmniClass_d;
   char                *pszJobProperties_d;

   long                 lCapabilities_d;
   long                 lRasterCapabilities_d;

   DeviceInstance      *pInstance_d;
   DeviceBlitter       *pBlitter_d;
   PDL                 *pPDL_d;

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

   DeviceCommand       *pCommand_d;
   DeviceData          *pData_d;
   DeviceString        *pString_d;

   char                *pszLoadedLibrary_d;  // @TBD expand to many
   GModule             *hModLibrary_d;
};

#endif
