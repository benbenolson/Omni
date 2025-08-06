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
#ifndef _DebugOutput
#define _DebugOutput

#include <cstdio>
#include <ostream>

#include <syslog.h>

#include "defines.hpp"

class DebugOutput
{
public:
   static void           logMessage                             (int         iLevel,
                                                                 const char *format,
                                                                             ...);
   static FILE          *getErrorStreamFILE                     ();
   static std::ostream&  getErrorStream                         ();
   static void           setErrorStream                         (FILE       *pFile);

   static void           applyDebugEnvironment                  ();
   static void           applyAllDebugOutput                    (PSZRO       pszJobProperties);

   static void           setDebugOutput                         (PSZRO       pszValue);

   static bool           shouldOutputDeviceBlitter              ();
   static bool           shouldOutputDeviceDither               ();
   static bool           shouldOutputDeviceInstance             ();
   static bool           shouldOutputDevicePrintMode            ();
   static bool           shouldOutputDeviceTester               ();
   static bool           shouldOutputGplCompression             ();
   static bool           shouldOutputGplDitherInstance          ();
   static bool           shouldOutputOmni                       ();
   static bool           shouldOutputOmniInterface              ();
   static bool           shouldOutputOmniProxy                  ();
   static bool           shouldOutputPrintDevice                ();
   static bool           shouldOutputBlitter                    ();
   static bool           shouldOutputInstance                   ();
   static bool           shouldOutputOmniPDCProxy               ();
   static bool           shouldOutputOmniServer                 ();
   static bool           shouldOutputPrinterCommand             ();
   static bool           shouldOutputPluggableInstance          ();
   static bool           shouldOutputPluggableBlitter           ();
   static bool           shouldOutputDriverInfo                 ();
   static bool           shouldOutputPDCBlitter                 ();
   static bool           shouldOutputPDCBlitterClient           ();
   static bool           shouldOutputDeviceString               ();
   static bool           shouldOutputUPDFDevice                 ();
   static bool           shouldOutputUPDFDeviceBooklet          ();
   static bool           shouldOutputUPDFDeviceCopies           ();
   static bool           shouldOutputUPDFDeviceForm             ();
   static bool           shouldOutputUPDFDeviceJogging          ();
   static bool           shouldOutputUPDFDeviceMedia            ();
   static bool           shouldOutputUPDFDeviceNUp              ();
   static bool           shouldOutputUPDFDeviceOrientation      ();
   static bool           shouldOutputUPDFDeviceOutputBin        ();
   static bool           shouldOutputUPDFDevicePrintMode        ();
   static bool           shouldOutputUPDFDeviceResolution       ();
   static bool           shouldOutputUPDFDeviceScaling          ();
   static bool           shouldOutputUPDFDeviceSheetCollate     ();
   static bool           shouldOutputUPDFDeviceSide             ();
   static bool           shouldOutputUPDFDeviceStitching        ();
   static bool           shouldOutputUPDFDeviceTray             ();
   static bool           shouldOutputUPDFDeviceTrimming         ();
   static bool           shouldOutputUPDFDeviceInstance         ();
   static bool           shouldOutputUPDFDeviceBlitter          ();
   static bool           shouldOutputXMLDevice                  ();
   static bool           shouldOutputXMLDeviceBooklet           ();
   static bool           shouldOutputXMLDeviceCopies            ();
   static bool           shouldOutputXMLDeviceForm              ();
   static bool           shouldOutputXMLDeviceJogging           ();
   static bool           shouldOutputXMLDeviceMedia             ();
   static bool           shouldOutputXMLDeviceNUp               ();
   static bool           shouldOutputXMLDeviceOrientation       ();
   static bool           shouldOutputXMLDeviceOutputBin         ();
   static bool           shouldOutputXMLDevicePrintMode         ();
   static bool           shouldOutputXMLDeviceResolution        ();
   static bool           shouldOutputXMLDeviceScaling           ();
   static bool           shouldOutputXMLDeviceSheetCollate      ();
   static bool           shouldOutputXMLDeviceSide              ();
   static bool           shouldOutputXMLDeviceStitching         ();
   static bool           shouldOutputXMLDeviceTray              ();
   static bool           shouldOutputXMLDeviceTrimming          ();
   static bool           shouldOutputXMLDeviceInstance          ();
   static bool           shouldOutputXMLDeviceBlitter           ();
   static bool           shouldOutputPDCInterface               ();

   static void           setOutputDeviceBlitter                 (bool        fOutputDeviceBlitter);
   static void           setOutputDeviceDither                  (bool        fOutputDeviceDither);
   static void           setOutputDeviceInstance                (bool        fOutputDeviceInstance);
   static void           setOutputDevicePrintMode               (bool        fOutputDevicePrintMode);
   static void           setOutputDeviceTester                  (bool        fOutputDeviceTester);
   static void           setOutputGplCompression                (bool        fOutputGplCompression);
   static void           setOutputGplDitherInstance             (bool        fOutputGplDitherInstance);
   static void           setOutputOmni                          (bool        fOutputOmni);
   static void           setOutputOmniInterface                 (bool        fOutputOmniInterface);
   static void           setOutputOmniProxy                     (bool        fOutputOmniProxy);
   static void           setOutputPrintDevice                   (bool        fOutputPrintDevice);
   static void           setOutputBlitter                       (bool        fOutputBlitter);
   static void           setOutputInstance                      (bool        fOutputInstance);
   static void           setOutputOmniPDCProxy                  (bool        fOutputOmniPDCProxy);
   static void           setOutputOmniServer                    (bool        fOutputOmniServer);
   static void           setOutputPrinterCommand                (bool        fOutputPrinterCommand);
   static void           setOutputPluggableInstance             (bool        fOutputPluggableInstance);
   static void           setOutputPluggableBlitter              (bool        fOutputPluggableBlitter);
   static void           setOutputDriverInfo                    (bool        fOutputDriverInfo);
   static void           setOutputPDCBlitter                    (bool        fOutputPDCBlitter);
   static void           setOutputPDCBlitterClient              (bool        fOutputPDCBlitterClient);
   static void           setOutputDeviceString                  (bool        fOutputDeviceString);
   static void           setOutputUPDFDevice                    (bool        fOutputUPDFDevice);
   static void           setOutputUPDFDeviceBooklet             (bool        fOutputUPDFDeviceBooklet);
   static void           setOutputUPDFDeviceCopies              (bool        fOutputUPDFDeviceCopies);
   static void           setOutputUPDFDeviceForm                (bool        fOutputUPDFDeviceForm);
   static void           setOutputUPDFDeviceJogging             (bool        fOutputUPDFDeviceJogging);
   static void           setOutputUPDFDeviceMedia               (bool        fOutputUPDFDeviceMedia);
   static void           setOutputUPDFDeviceNUp                 (bool        fOutputUPDFDeviceNUp);
   static void           setOutputUPDFDeviceOrientation         (bool        fOutputUPDFDeviceOrientation);
   static void           setOutputUPDFDeviceOutputBin           (bool        fOutputUPDFDeviceOutputBin);
   static void           setOutputUPDFDevicePrintMode           (bool        fOutputUPDFDevicePrintMode);
   static void           setOutputUPDFDeviceResolution          (bool        fOutputUPDFDeviceResolution);
   static void           setOutputUPDFDeviceScaling             (bool        fOutputUPDFDeviceScaling);
   static void           setOutputUPDFDeviceSheetCollate        (bool        fOutputUPDFDeviceSheetCollate);
   static void           setOutputUPDFDeviceSide                (bool        fOutputUPDFDeviceSide);
   static void           setOutputUPDFDeviceStitching           (bool        fOutputUPDFDeviceStitching);
   static void           setOutputUPDFDeviceTray                (bool        fOutputUPDFDeviceTray);
   static void           setOutputUPDFDeviceTrimming            (bool        fOutputUPDFDeviceTrimming);
   static void           setOutputUPDFDeviceInstance            (bool        fOutputUPDFDeviceInstance);
   static void           setOutputUPDFDeviceBlitter             (bool        fOutputUPDFDeviceBlitter);
   static void           setOutputXMLDevice                     (bool        fOutputXMLDevice);
   static void           setOutputXMLDeviceBooklet              (bool        fOutputXMLDeviceBooklet);
   static void           setOutputXMLDeviceCopies               (bool        fOutputXMLDeviceCopies);
   static void           setOutputXMLDeviceForm                 (bool        fOutputXMLDeviceForm);
   static void           setOutputXMLDeviceJogging              (bool        fOutputXMLDeviceJogging);
   static void           setOutputXMLDeviceMedia                (bool        fOutputXMLDeviceMedia);
   static void           setOutputXMLDeviceNUp                  (bool        fOutputXMLDeviceNUp);
   static void           setOutputXMLDeviceOrientation          (bool        fOutputXMLDeviceOrientation);
   static void           setOutputXMLDeviceOutputBin            (bool        fOutputXMLDeviceOutputBin);
   static void           setOutputXMLDevicePrintMode            (bool        fOutputXMLDevicePrintMode);
   static void           setOutputXMLDeviceResolution           (bool        fOutputXMLDeviceResolution);
   static void           setOutputXMLDeviceScaling              (bool        fOutputXMLDeviceScaling);
   static void           setOutputXMLDeviceSheetCollate         (bool        fOutputXMLDeviceSheetCollate);
   static void           setOutputXMLDeviceSide                 (bool        fOutputXMLDeviceSide);
   static void           setOutputXMLDeviceStitching            (bool        fOutputXMLDeviceStitching);
   static void           setOutputXMLDeviceTray                 (bool        fOutputXMLDeviceTray);
   static void           setOutputXMLDeviceTrimming             (bool        fOutputXMLDeviceTrimming);
   static void           setOutputXMLDeviceInstance             (bool        fOutputXMLDeviceInstance);
   static void           setOutputXMLDeviceBlitter              (bool        fOutputXMLDeviceBlitter);
   static void           setOutputPDCInterface                  (bool        fOutputPDCInterface);
};

#endif
