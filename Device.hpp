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
#ifndef _Device
#define _Device

#include "config.h"

#define PDC_INTERFACE_ENABLED           1

//#define INCLUDE_JP_UPDF_BOOKLET         1
//#define INCLUDE_JP_UPDF_JOGGING         1
#define INCLUDE_JP_COMMON_COPIES        1
#define INCLUDE_JP_COMMON_NUP           1
#define INCLUDE_JP_COMMON_OUTPUT_BIN    1
#define INCLUDE_JP_COMMON_SCALING       1
#define INCLUDE_JP_COMMON_SIDE          1
#define INCLUDE_JP_COMMON_SHEET_COLLATE 1
#define INCLUDE_JP_COMMON_STITCHING     1
#define INCLUDE_JP_COMMON_TRIMMING      1

/* Foreward references to bypass cyclic references...
*/
struct Device;
struct DeviceBlitter;
#ifdef INCLUDE_JP_UPDF_BOOKLET
struct DeviceBooklet;
#endif
struct DeviceCommand;
#ifdef INCLUDE_JP_COMMON_COPIES
struct DeviceCopies;
#endif
struct DeviceData;
struct DeviceDither;
struct DeviceForm;
struct DeviceGamma;
struct DeviceInstance;
#ifdef INCLUDE_JP_UPDF_JOGGING
struct DeviceJogging;
#endif
struct DeviceMedia;
#ifdef INCLUDE_JP_COMMON_NUP
struct DeviceNUp;
#endif
struct DeviceOrientation;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
struct DeviceOutputBin;
#endif
struct DevicePrintMode;
struct DeviceResolution;
#ifdef INCLUDE_JP_COMMON_SCALING
struct DeviceScaling;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
struct DeviceSide;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
struct DeviceSheetCollate;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
struct DeviceStitching;
#endif
struct DeviceString;
struct DeviceTray;
#ifdef INCLUDE_JP_COMMON_TRIMMING
struct DeviceTrimming;
#endif
struct HardCopyCap;
struct JobProperties;
struct PluggableBlitter;
struct PluggableInstance;
struct PrintDevice;

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

#include "defines.hpp"

#include "BinaryData.hpp"
#include "Enumeration.hpp"
#include "HardCopyCap.hpp"
#include "DeviceDither.hpp"
#include "GplCompression.hpp"
#include "GplDitherInstance.hpp"

#include "Capability.hpp"
#include "DebugOutput.hpp"
#include "DeviceBlitter.hpp"
#ifdef INCLUDE_JP_UPDF_BOOKLET
#include "DeviceBooklet.hpp"
#endif
#include "DeviceCommand.hpp"
#ifdef INCLUDE_JP_COMMON_COPIES
#include "DeviceCopies.hpp"
#endif
#include "DeviceData.hpp"
#include "DeviceForm.hpp"
#include "DeviceGamma.hpp"
#include "DeviceInstance.hpp"
#ifdef INCLUDE_JP_UPDF_JOGGING
#include "DeviceJogging.hpp"
#endif
#include "DeviceMedia.hpp"
#ifdef INCLUDE_JP_COMMON_NUP
#include "DeviceNUp.hpp"
#endif
#include "DeviceOrientation.hpp"
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
#include "DeviceOutputBin.hpp"
#endif
#include "DevicePrintMode.hpp"
#include "DeviceResolution.hpp"
#ifdef INCLUDE_JP_COMMON_SCALING
#include "DeviceScaling.hpp"
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
#include "DeviceSide.hpp"
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
#include "DeviceSheetCollate.hpp"
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
#include "DeviceStitching.hpp"
#endif
#include "DeviceString.hpp"
#ifdef INCLUDE_JP_COMMON_TRIMMING
#include "DeviceTrimming.hpp"
#endif
#include "DeviceTray.hpp"
#include "PDL.hpp"
#include "PluggableBlitter.hpp"
#include "PluggableInstance.hpp"
#include "RasterCapabilities.hpp"
#include "StringResource.hpp"

// In device's Device.cpp file
typedef PSZCRO             (*PFNGETVERSION)       ();                                // getVersion
typedef Enumeration *      (*PFNENUMERATEDEVICES) (PSZCRO          pszLibraryName,   // getDeviceEnumeration
                                                   bool            fBuildOnly);
typedef Device *           (*PFNNEWDEVICE)        (bool            fAdvanced);       // newDeviceW_Advanced
typedef Device *           (*PFNNEWDEVICEWARGS)   (PSZCRO          pszJobProperties, // newDeviceW_JopProp_Advanced
                                                   bool            fAdvanced);
typedef void               (*PFNDELETEDEVICE)     (Device         *pDevice);
// In device's DeviceInstance.cpp file
typedef DeviceInstance *   (*PFNCREATEINSTANCE)   (PrintDevice    *pDevice);         // createInstance
typedef void               (*PFNDELETEINSTANCE)   (DeviceInstance *pInstance);       // deleteInstance
// In device's DeviceBlitter.cpp file
typedef DeviceBlitter *    (*PFNCREATEBLITTER)    (PrintDevice    *pDevice);         // createBlitter
typedef void               (*PFNDELETEBLITTER)    (DeviceBlitter  *pBlitter);        // deleteBlitter

typedef void               (*PFNOUTPUTFUNCTION)   (void           *pMagicCookie,
                                                   PUCHRO          puchData,
                                                   int             iSize);

typedef enum _eOmniClass {
   OMNI_CLASS_UNKNOWN = 0,
   OMNI_CLASS_COMPILED,
   OMNI_CLASS_XML,
   OMNI_CLASS_UPDF
} EOMNICLASS, *PEOMNICLASS;
#define OMNI_CLASS_COUNT (OMNI_CLASS_UPDF - OMNI_CLASS_UNKNOWN)

class Device
{
public:
                                 Device                      ();
   virtual                      ~Device                      ();

   virtual bool                  hasError                    () = 0;

   virtual void                  initialize                  () = 0;

   virtual PSZCRO                getVersion                  () = 0;

   virtual PSZCRO                getDriverName               () = 0;
   virtual PSZCRO                getDeviceName               () = 0;
   virtual PSZCRO                getShortName                () = 0;
   virtual PSZCRO                getLibraryName              () = 0;
   virtual EOMNICLASS            getOmniClass                () = 0;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   virtual DeviceBooklet        *getCurrentBooklet           () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   virtual DeviceCopies         *getCurrentCopies            () = 0;
#endif
   virtual PSZCRO                getCurrentDitherID          () = 0;
   virtual DeviceForm           *getCurrentForm              () = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   virtual DeviceJogging        *getCurrentJogging           () = 0;
#endif
   virtual DeviceMedia          *getCurrentMedia             () = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   virtual DeviceNUp            *getCurrentNUp               () = 0;
#endif
   virtual DeviceOrientation    *getCurrentOrientation       () = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   virtual DeviceOutputBin      *getCurrentOutputBin         () = 0;
#endif
   virtual DevicePrintMode      *getCurrentPrintMode         () = 0;
   virtual DeviceResolution     *getCurrentResolution        () = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   virtual DeviceScaling        *getCurrentScaling           () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   virtual DeviceSheetCollate   *getCurrentSheetCollate      () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   virtual DeviceSide           *getCurrentSide              () = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   virtual DeviceStitching      *getCurrentStitching         () = 0;
#endif
   virtual DeviceTray           *getCurrentTray              () = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   virtual DeviceTrimming       *getCurrentTrimming          () = 0;
#endif

   virtual DeviceGamma          *getCurrentGamma             () = 0;

   virtual DeviceInstance       *getInstance                 () = 0;

   virtual int                   getPDLLevel                 () = 0;
   virtual int                   getPDLSubLevel              () = 0;
   virtual int                   getPDLMajorRevisionLevel    () = 0;
   virtual int                   getPDLMinorRevisionLevel    () = 0;

   virtual bool                  hasCapability               (long                lMask) = 0;
   virtual bool                  hasRasterCapability         (long                lMask) = 0;
   virtual bool                  hasDeviceOption             (PSZCRO              pszDeviceOption) = 0;

   virtual int                   getScanlineMultiple         () = 0;

   virtual bool                  beginJob                    () = 0;
   virtual bool                  beginJob                    (PSZCRO              pszJobProperties) = 0;
   virtual bool                  newFrame                    () = 0;
   virtual bool                  newFrame                    (PSZCRO              pszJobProperties) = 0;
   virtual bool                  endJob                      () = 0;
   virtual bool                  abortJob                    () = 0;

   virtual bool                  rasterize                   (PBYTE               pbBits,
                                                              PBITMAPINFO2        pbmi,
                                                              PRECTL              prectlPageLocation,
                                                              BITBLT_TYPE         eType) = 0;

   virtual void                  setOutputStream             (FILE               *pFile) = 0;
   virtual void                  setOutputFunction           (PFNOUTPUTFUNCTION   pfnOutputFunction,
                                                              void               *pMagicCookie) = 0;
   virtual void                  setErrorStream              (FILE               *pFile) = 0;

   virtual bool                  setLanguage                 (int                 iLanguageID) = 0;
   virtual int                   getLanguage                 () = 0;
   virtual Enumeration          *getLanguages                () = 0;
   virtual StringResource       *getLanguageResource         () = 0;

   virtual std::string          *getJobProperties            (bool                fInDeviceSpecific = false) = 0;
   virtual bool                  setJobProperties            (PSZCRO              pszJobProperties) = 0;
   virtual Enumeration          *listJobProperties           (bool                fInDeviceSpecific = false) = 0;

   virtual std::string          *getJobPropertyType          (PSZCRO              pszKey) = 0;
   virtual std::string          *getJobProperty              (PSZCRO              pszKey) = 0;
   virtual std::string          *translateKeyValue           (PSZCRO              pszKey,
                                                              PSZCRO              pszValue) = 0;

#ifndef RETAIL
   virtual void                  outputSelf                  () = 0;
#endif
   virtual std::string           toString                    (std::ostringstream& oss) = 0;
   friend std::ostream&          operator<<                  (std::ostream&       os,
                                                              const Device&       self);
};

#include "PrintDevice.hpp"

#endif
