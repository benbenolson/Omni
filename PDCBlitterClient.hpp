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
#ifndef _PDCBlitterClient
#define _PDCBlitterClient

#include <unistd.h>

#include <string>

#include <glib.h>
#include <gmodule.h>

#include "Device.hpp"

class DeviceObjectHolder
{
public:
                       DeviceObjectHolder     ();
                      ~DeviceObjectHolder     ();

#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet      *getCurrentBooklet      ();
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies       *getCurrentCopies       ();
#endif
   DeviceForm         *getCurrentForm         ();
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging      *getCurrentJogging      ();
#endif
   DeviceMedia        *getCurrentMedia        ();
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp          *getCurrentNUp          ();
#endif
   DeviceOrientation  *getCurrentOrientation  ();
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin    *getCurrentOutputBin    ();
#endif
   DevicePrintMode    *getCurrentPrintMode    ();
   DeviceResolution   *getCurrentResolution   ();
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling      *getCurrentScaling      ();
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate *getCurrentSheetCollate ();
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide         *getCurrentSide         ();
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching    *getCurrentStitching    ();
#endif
   DeviceTray         *getCurrentTray         ();
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming     *getCurrentTrimming     ();
#endif
   DeviceGamma        *getCurrentGamma        ();

#ifdef INCLUDE_JP_UPDF_BOOKLET
   void                setCurrentBooklet      (DeviceBooklet      *pBooklet);
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   void                setCurrentCopies       (DeviceCopies       *pCopies);
#endif
   void                setCurrentForm         (DeviceForm         *pForm);
#ifdef INCLUDE_JP_UPDF_JOGGING
   void                setCurrentJogging      (DeviceJogging      *pJogging);
#endif
   void                setCurrentMedia        (DeviceMedia        *pMedia);
#ifdef INCLUDE_JP_COMMON_NUP
   void                setCurrentNUp          (DeviceNUp          *pNUp);
#endif
   void                setCurrentOrientation  (DeviceOrientation  *pOrientation);
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   void                setCurrentOutputBin    (DeviceOutputBin    *pOutputBin);
#endif
   void                setCurrentPrintMode    (DevicePrintMode    *pPrintMode);
   void                setCurrentResolution   (DeviceResolution   *pResolution);
#ifdef INCLUDE_JP_COMMON_SCALING
   void                setCurrentScaling      (DeviceScaling      *pScaling);
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   void                setCurrentSheetCollate (DeviceSheetCollate *pSheetCollate);
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   void                setCurrentSide         (DeviceSide         *pSide);
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   void                setCurrentStitching    (DeviceStitching    *pStitching);
#endif
   void                setCurrentTray         (DeviceTray         *pTray);
#ifdef INCLUDE_JP_COMMON_TRIMMING
   void                setCurrentTrimming     (DeviceTrimming     *pTrimming);
#endif
   void                setCurrentGamma        (DeviceGamma        *pGamma);

private:
#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet      *pBooklet_d;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies       *pCopies_d;
#endif
   DeviceForm         *pForm_d;
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging      *pJogging_d;
#endif
   DeviceMedia        *pMedia_d;
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp          *pNUp_d;
#endif
   DeviceOrientation  *pOrientation_d;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin    *pOutputBin_d;
#endif
   DevicePrintMode    *pPrintMode_d;
   DeviceResolution   *pResolution_d;
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling      *pScaling_d;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate *pSheetCollate_d;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide         *pSide_d;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching    *pStitching_d;
#endif
   DeviceTray         *pTray_d;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming     *pTrimming_d;
#endif
   DeviceGamma        *pGamma_d;
};

typedef void *HANDLE, **PHANDLE;

typedef GModule *          (*PFNINITIALIZEINSTANCE)            (PSZCRO              pszDeviceName,
                                                                PHANDLE             phInstance);
typedef PSZCRO             (*PFNGETDRIVERNAME)                 ();
typedef std::string *      (*PFNGETJOBPROPERTIES)              (HANDLE              hInstance);
typedef bool               (*PFNSETJOBPROPERTIES)              (HANDLE              hInstance,
                                                                PSZCRO              pszJobProperties,
                                                                DeviceObjectHolder *pOrientationH);
typedef Enumeration *      (*PFNENUMERATEINSTANCEPROPS)        (HANDLE              hInstance,
                                                                bool                fInDeviceSpecific);
typedef std::string *      (*PFNGETJOBPROPERTYTYPE)            (HANDLE              hInstance,
                                                                PSZCRO              pszKey);
typedef PSZCRO             (*PFNGETJOBPROPERTY)                (HANDLE              hInstance,
                                                                PSZCRO              pszKey);
typedef std::string *      (*PFNTRANSLATEKEYVALUE)             (HANDLE              hInstance,
                                                                StringResource     *pResource,
                                                                PSZCRO              pszKeyValue);
typedef bool               (*PFNSTARTPAGE)                     (HANDLE              hInstance);
typedef bool               (*PFNENDPAGE)                       (HANDLE              hInstance,
                                                                int                 iCanceled);
typedef bool               (*PFNBEGINJOB)                      (HANDLE              hInstance);
typedef bool               (*PFNENDJOB)                        (HANDLE              hInstance);
typedef bool               (*PFNABORTJOB)                      (HANDLE              hInstance);
typedef int                (*PFNRASTERIZE)                     (HANDLE              hInstance,
                                                                PBYTE               pbBits,
                                                                PBITMAPINFO2        pbmi2,
                                                                PRECTL              prectlPageLocation,
                                                                BITBLT_TYPE         eType);
typedef bool               (*PFNFREEINSTANCE)                  (GModule            *hmodDevice,
                                                                HANDLE              hInstance);
typedef bool               (*PFNSETOUTPUTSTREAM)               (HANDLE              hInstance,
                                                                int                 fdHandle);

typedef struct _PDCBlitterHookPoints {
   PFNINITIALIZEINSTANCE        pfnInitializeInstance;
   PFNGETDRIVERNAME             pfnGetDriverName;
   PFNGETJOBPROPERTIES          pfnGetJobProperties;
   PFNSETJOBPROPERTIES          pfnSetJobProperties;
   PFNENUMERATEINSTANCEPROPS    pfnEnumerateInstanceProps;
   PFNGETJOBPROPERTYTYPE        pfnGetJobPropertyType;
   PFNGETJOBPROPERTY            pfnGetJobProperty;
   PFNTRANSLATEKEYVALUE         pfnTranslateKeyValue;
   PFNSTARTPAGE                 pfnStartPage;
   PFNENDPAGE                   pfnEndPage;
   PFNBEGINJOB                  pfnBeginJob;
   PFNENDJOB                    pfnEndJob;
   PFNABORTJOB                  pfnAbortJob;
   PFNRASTERIZE                 pfnRasterize;
   PFNFREEINSTANCE              pfnFreeInstance;
   PFNSETOUTPUTSTREAM           pfnSetOutputStream;
} PDC_B_HOOKPOINTS, *PPDC_B_HOOKPOINTS;

void          initializePDCBlitter          (PPDC_B_HOOKPOINTS pHookPoints);

#endif
