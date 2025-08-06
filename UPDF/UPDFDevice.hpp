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
#ifndef _UPDFDevice
#define _UPDFDevice

#include <Device.hpp>
#include <OmniProxy.hpp>

#include "XMLInterface.hpp"

extern "C" {
   PSZCRO       getVersion                  ();
   Enumeration *getDeviceEnumeration        (PSZCRO  pszLibraryName,
                                             bool    fBuildOnly);
   Device      *newDeviceW_Advanced         (bool    fAdvanced);
   Device      *newDeviceW_JopProp_Advanced (PSZCRO  pszJobProperties,
                                             bool    fAdvanced);
   void         deleteDevice                (Device *pDevice);
};

struct UPDFDeviceInstance;

class UPDFDevice : public PrintDevice
{
public:
                               UPDFDevice               (XmlDocPtr    docDC,
                                                         XmlDocPtr    docUDR,
                                                         PSZCRO       pszDriverName,
                                                         PSZCRO       pszDeviceName,
                                                         PSZCRO       pszShortName,
                                                         PSZCRO       pszJobProperties,
                                                         PSZCRO       pszXMLDeviceName);
                              ~UPDFDevice               ();

#ifdef INCLUDE_JP_UPDF_BOOKLET
   virtual DeviceBooklet      *getDefaultBooklet        ();
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   virtual DeviceCopies       *getDefaultCopies         ();
#endif
   virtual PSZCRO              getDefaultDitherID       ();
   virtual DeviceForm         *getDefaultForm           ();
#ifdef INCLUDE_JP_UPDF_JOGGING
   virtual DeviceJogging      *getDefaultJogging        ();
#endif
   virtual DeviceMedia        *getDefaultMedia          ();
#ifdef INCLUDE_JP_COMMON_NUP
   virtual DeviceNUp          *getDefaultNUp            ();
#endif
   virtual DeviceOrientation  *getDefaultOrientation    ();
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   virtual DeviceOutputBin    *getDefaultOutputBin      ();
#endif
   virtual DevicePrintMode    *getDefaultPrintMode      ();
   virtual DeviceResolution   *getDefaultResolution     ();
#ifdef INCLUDE_JP_COMMON_SCALING
   virtual DeviceScaling      *getDefaultScaling        ();
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   virtual DeviceSheetCollate *getDefaultSheetCollate   ();
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   virtual DeviceSide         *getDefaultSide           ();
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   virtual DeviceStitching    *getDefaultStitching      ();
#endif
   virtual DeviceTray         *getDefaultTray           ();
#ifdef INCLUDE_JP_COMMON_TRIMMING
   virtual DeviceTrimming     *getDefaultTrimming       ();
#endif

   virtual DeviceCommand      *getDefaultCommands       ();
   virtual DeviceData         *getDefaultData           ();
   virtual DeviceString       *getDefaultString         ();

   virtual DeviceGamma        *getCurrentGamma          ();

   static UPDFDevice          *isAUPDFDevice            (Device      *pDevice);

   XmlNodePtr                  findDCEntry              (XmlNodePtr   root,
                                                         PSZCRO       pszName,
                                                         bool         fDebugOutput);
   XmlNodePtr                  findUDREntry             (XmlNodePtr   root,
                                                         PSZCRO       pszName,
                                                         bool         fDebugOutput);
   XmlNodePtr                  findLocaleEntry          (XmlNodePtr   root,
                                                         PSZCRO       pszName,
                                                         bool         fDebugOutput);
   XmlNodePtr                  findCSEntry              (XmlNodePtr   root,
                                                         PSZCRO       pszName,
                                                         bool         fDebugOutput);

   XmlNodePtr                  findDCEntryKeyValue      (PSZCRO       pszKey,
                                                         PSZCRO       pszValue,
                                                         bool         fDebugOutput);
   XmlNodePtr                  findUDREntryKeyValue     (PSZCRO       pszKey,
                                                         PSZCRO       pszValue,
                                                         bool         fDebugOutput);
   XmlNodePtr                  findLocaleEntryKeyValue  (PSZCRO       pszKey,
                                                         PSZCRO       pszValue,
                                                         bool         fDebugOutput);
   XmlNodePtr                  findCSEntryKeyValue      (PSZCRO       pszKey,
                                                         PSZCRO       pszValue,
                                                         bool         fDebugOutput);

   static XmlNodePtr           findEntry                (XmlNodePtr   root,
                                                         PSZCRO       pszName,
                                                         bool         fDebugOutput);

   XmlNodePtr                  findEntryKeyValue        (XmlNodePtr   root,
                                                         PSZCRO       pszKey,
                                                         PSZCRO       pszValue,
                                                         bool         fDebugOutput);

   int                         getXVirtualUnits         ();
   int                         getYVirtualUnits         ();

   XmlNodePtr                  getRootDC                ();
   XmlNodePtr                  getRootUDR               ();
   XmlNodePtr                  getRootLocale            ();
   XmlNodePtr                  getRootCS                ();

private:
   void                        commonInit               (XmlDocPtr    docDC,
                                                         XmlDocPtr    docUDR);

   PSZCRO                      getDefaultFeature        (PSZCRO       pszKey);

   bool                        setVirtualUnits          ();

   UPDFDeviceInstance *pInstance_d;

   XmlDocPtr           docDC_d;          // Device Configuration
   XmlNodePtr          nodeRootDC_d;

   XmlDocPtr           docUDR_d;         // Unit Description Reference
   XmlNodePtr          nodeRootUDR_d;

   XmlDocPtr           docLocale_d;      // Locale
   XmlNodePtr          nodeRootLocale_d;

   XmlDocPtr           docCS_d;          // Command Sequences
   XmlNodePtr          nodeRootCS_d;

   int                 iVirtualUnitsX_d;
   int                 iVirtualUnitsY_d;

   PSZRO               pszDriverName_d;
   PSZRO               pszDeviceName_d;
   PSZRO               pszShortName_d;
};

#define FINDDCENTRY(pUPDFDevice, \
                    elm,         \
                    pszName,     \
                    pszFunction) \
   pUPDFDevice->findDCEntry (elm, pszName, DebugOutput::shouldOutput##pszFunction ())
#define FINDUDRENTRY(pUPDFDevice, \
                     elm,         \
                     pszName,     \
                     pszFunction) \
   pUPDFDevice->findUDREntry (elm, pszName, DebugOutput::shouldOutput##pszFunction ())
#define FINDLOCALEENTRY(pUPDFDevice, \
                        elm,         \
                        pszName,     \
                        pszFunction) \
   pUPDFDevice->findLocaleEntry (elm, pszName, DebugOutput::shouldOutput##pszFunction ())
#define FINDCSENTRY(pUPDFDevice, \
                    elm,         \
                    pszName,     \
                    pszFunction) \
   pUPDFDevice->findCSEntry (elm, pszName, DebugOutput::shouldOutput##pszFunction ())

#define FINDDCENTRYKEYVALUE(pUPDFDevice, \
                            pszKey,      \
                            pszValue,    \
                            pszFunction) \
   pUPDFDevice->findDCEntryKeyValue (pszKey, pszValue, DebugOutput::shouldOutput##pszFunction ())
#define FINDUDRENTRYKEYVALUE(pUPDFDevice, \
                             pszKey,      \
                             pszValue,    \
                             pszFunction) \
   pUPDFDevice->findUDREntryKeyValue (pszKey, pszValue, DebugOutput::shouldOutput##pszFunction ())
#define FINDLOCALEENTRYKEYVALUE(pUPDFDevice, \
                                pszKey,      \
                                pszValue,    \
                                pszFunction) \
   pUPDFDevice->findLocaleEntryKeyValue (pszKey, pszValue, DebugOutput::shouldOutput##pszFunction ())
#define FINDCSENTRYKEYVALUE(pUPDFDevice, \
                            pszKey,      \
                            pszValue,    \
                            pszFunction) \
   pUPDFDevice->findCSEntryKeyValue (pszKey, pszValue, DebugOutput::shouldOutput##pszFunction ())

#endif
