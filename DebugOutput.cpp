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
#include <cstring>
#include <iostream>
#include <fstream>

#include "DebugOutput.hpp"
#include "StdioFilebuf.hpp"

#include <JobProperties.hpp>

void DebugOutput::
logMessage (int iLevel, const char *format, ...)
{
   va_list  list;

   va_start (list, format);
   vsyslog (iLevel, format, list);
   va_end (list);
}

static std::ostream *vostreamErr_d = &std::cerr;
static FILE         *vpfpErr_d     = stderr;

FILE * DebugOutput::
getErrorStreamFILE ()
{
   return vpfpErr_d;
}

std::ostream& DebugOutput::
getErrorStream ()
{
   return *vostreamErr_d;
}

void DebugOutput::
setErrorStream (FILE *pFile)
{
   // @TBD store and clean up new filebuf
   vostreamErr_d = new std::ostream (new stdio_filebuf (pFile));
}

void DebugOutput::
applyDebugEnvironment ()
{
#ifndef RETAIL
   char *pszDebugOptions = 0;

   pszDebugOptions = getenv ("OMNI_DEBUG_OPTIONS");
   if (   pszDebugOptions
      && *pszDebugOptions
      )
   {
      char *pszString = 0;
      char *pszWord   = 0;
      char *pszSpace  = 0;

      pszString = (char *)malloc (strlen (pszDebugOptions) + 1);
      if (pszString)
      {
         strcpy (pszString, pszDebugOptions);

         pszWord = pszString;
         while (  pszWord
               && *pszWord
               )
         {
            pszSpace = strchr (pszWord, ' ');
            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            DebugOutput::setDebugOutput (pszWord);

            if (pszSpace)
            {
               pszWord = pszSpace + 1;
            }
            else
            {
               pszWord = 0;
            }
         }

         free (pszString);
      }
   }
#endif
}

void DebugOutput::
applyAllDebugOutput (PSZRO pszJobProperties)
{
#ifndef RETAIL
   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      JobProperties          jobProp (pszJobProperties);
      JobPropertyEnumerator *pEnum                      = 0;

      pEnum = jobProp.getEnumeration ("debugoutput");

      while (pEnum->hasMoreElements ())
      {
         PSZCRO pszValue = pEnum->getCurrentValue ();

         setDebugOutput ((PSZRO)pszValue);

         pEnum->nextElement ();
      }

      delete pEnum;
   }
#endif
}

static bool vfOutputDeviceBlitter_d              = false;
static bool vfOutputDeviceDither_d               = false;
static bool vfOutputDeviceInstance_d             = false;
static bool vfOutputDevicePrintMode_d            = false;
static bool vfOutputDeviceTester_d               = false;
static bool vfOutputGplCompression_d             = false;
static bool vfOutputGplDitherInstance_d          = false;
static bool vfOutputOmni_d                       = false;
static bool vfOutputOmniInterface_d              = false;
static bool vfOutputOmniProxy_d                  = false;
static bool vfOutputPrintDevice_d                = false;
static bool vfOutputBlitter_d                    = false;
static bool vfOutputInstance_d                   = false;
static bool vfOutputOmniPDCProxy_d               = false;
static bool vfOutputOmniServer_d                 = false;
static bool vfOutputPrinterCommand_d             = false;
static bool vfOutputPluggableInstance_d          = false;
static bool vfOutputPluggableBlitter_d           = false;
static bool vfOutputDriverInfo_d                 = false;
static bool vfOutputPDCBlitter_d                 = false;
static bool vfOutputPDCBlitterClient_d           = false;
static bool vfOutputDeviceString_d               = false;
static bool vfOutputUPDFDevice_d                 = false;
static bool vfOutputUPDFDeviceBooklet_d          = false;
static bool vfOutputUPDFDeviceCopies_d           = false;
static bool vfOutputUPDFDeviceForm_d             = false;
static bool vfOutputUPDFDeviceJogging_d          = false;
static bool vfOutputUPDFDeviceMedia_d            = false;
static bool vfOutputUPDFDeviceNUp_d              = false;
static bool vfOutputUPDFDeviceOrientation_d      = false;
static bool vfOutputUPDFDeviceOutputBin_d        = false;
static bool vfOutputUPDFDevicePrintMode_d        = false;
static bool vfOutputUPDFDeviceResolution_d       = false;
static bool vfOutputUPDFDeviceScaling_d          = false;
static bool vfOutputUPDFDeviceSheetCollate_d     = false;
static bool vfOutputUPDFDeviceSide_d             = false;
static bool vfOutputUPDFDeviceStitching_d        = false;
static bool vfOutputUPDFDeviceTray_d             = false;
static bool vfOutputUPDFDeviceTrimming_d         = false;
static bool vfOutputUPDFDeviceInstance_d         = false;
static bool vfOutputUPDFDeviceBlitter_d          = false;
static bool vfOutputXMLDevice_d                  = false;
static bool vfOutputXMLDeviceBooklet_d           = false;
static bool vfOutputXMLDeviceCopies_d            = false;
static bool vfOutputXMLDeviceForm_d              = false;
static bool vfOutputXMLDeviceJogging_d           = false;
static bool vfOutputXMLDeviceMedia_d             = false;
static bool vfOutputXMLDeviceNUp_d               = false;
static bool vfOutputXMLDeviceOrientation_d       = false;
static bool vfOutputXMLDeviceOutputBin_d         = false;
static bool vfOutputXMLDevicePrintMode_d         = false;
static bool vfOutputXMLDeviceResolution_d        = false;
static bool vfOutputXMLDeviceScaling_d           = false;
static bool vfOutputXMLDeviceSheetCollate_d      = false;
static bool vfOutputXMLDeviceSide_d              = false;
static bool vfOutputXMLDeviceStitching_d         = false;
static bool vfOutputXMLDeviceTray_d              = false;
static bool vfOutputXMLDeviceTrimming_d          = false;
static bool vfOutputXMLDeviceInstance_d          = false;
static bool vfOutputXMLDeviceBlitter_d           = false;
static bool vfOutputPDCInterface_d               = false;

void DebugOutput::
setDebugOutput (PSZRO pszValue)
{
#ifndef RETAIL
///getErrorStream () << "setDebugOutput (" << pszValue << ")" << std::endl;

   char *pszSpace;
   int   cbLength;

   while (  pszValue
         && *pszValue
         )
   {
//////getErrorStream () << "pszValue = \"" << pszValue << "\"" << std::endl;

      if ('"' == *pszValue)
         pszValue++;

      pszSpace = strchr (pszValue, ' ');
      if (pszSpace)
      {
         cbLength = pszSpace - pszValue;
      }
      else
      {
         cbLength = strlen (pszValue);
      }

      if ('"' == pszValue[cbLength - 1])
      {
         cbLength--;
      }

//////getErrorStream () << "pszValue = \"" << pszValue << "\", cbLength = " << cbLength << std::endl;

      if (0 == strncasecmp ("DeviceBlitter", pszValue, cbLength))
      {
         setOutputDeviceBlitter (true);
      }
      else if (0 == strncasecmp ("DeviceDither", pszValue, cbLength))
      {
         setOutputDeviceDither (true);
      }
      else if (0 == strncasecmp ("DeviceInstance", pszValue, cbLength))
      {
         setOutputDeviceInstance (true);
      }
      else if (0 == strncasecmp ("DevicePrintMode", pszValue, cbLength))
      {
         setOutputDevicePrintMode (true);
      }
      else if (0 == strncasecmp ("DeviceTester", pszValue, cbLength))
      {
         setOutputDeviceTester (true);
      }
      else if (0 == strncasecmp ("GplCompression", pszValue, cbLength))
      {
         setOutputGplCompression (true);
      }
      else if (0 == strncasecmp ("GplDitherInstance", pszValue, cbLength))
      {
         setOutputGplDitherInstance (true);
      }
      else if (0 == strncasecmp ("Omni", pszValue, cbLength))
      {
         setOutputOmni (true);
      }
      else if (0 == strncasecmp ("OmniInterface", pszValue, cbLength))
      {
         setOutputOmniInterface (true);
      }
      else if (0 == strncasecmp ("OmniProxy", pszValue, cbLength))
      {
         setOutputOmniProxy (true);
      }
      else if (0 == strncasecmp ("PrintDevice", pszValue, cbLength))
      {
         setOutputPrintDevice (true);
      }
      else if (0 == strncasecmp ("Blitter", pszValue, cbLength))
      {
         setOutputBlitter (true);
      }
      else if (0 == strncasecmp ("Instance", pszValue, cbLength))
      {
         setOutputInstance (true);
      }
      else if (0 == strncasecmp ("OmniPDCProxy", pszValue, cbLength))
      {
         setOutputOmniPDCProxy (true);
      }
      else if (0 == strncasecmp ("OmniServer", pszValue, cbLength))
      {
         setOutputOmniServer (true);
      }
      else if (0 == strncasecmp ("PrinterCommand", pszValue, cbLength))
      {
         setOutputPrinterCommand (true);
      }
      else if (0 == strncasecmp ("PluggableInstance", pszValue, cbLength))
      {
         setOutputPluggableInstance (true);
      }
      else if (0 == strncasecmp ("PluggableBlitter", pszValue, cbLength))
      {
         setOutputPluggableBlitter (true);
      }
      else if (0 == strncasecmp ("DriverInfo", pszValue, cbLength))
      {
         setOutputDriverInfo (true);
      }
      else if (0 == strncasecmp ("PDCBlitter", pszValue, cbLength))
      {
         setOutputPDCBlitter (true);
      }
      else if (0 == strncasecmp ("PDCBlitterClient", pszValue, cbLength))
      {
         setOutputPDCBlitterClient (true);
      }
      else if (0 == strncasecmp ("DeviceString", pszValue, cbLength))
      {
         setOutputDeviceString (true);
      }
      else if (0 == strncasecmp ("UPDFDevice", pszValue, cbLength))
      {
         setOutputUPDFDevice (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceOrientation", pszValue, cbLength))
      {
         setOutputUPDFDeviceOrientation (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceForm", pszValue, cbLength))
      {
         setOutputUPDFDeviceForm (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceTray", pszValue, cbLength))
      {
         setOutputUPDFDeviceTray (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceTrimming", pszValue, cbLength))
      {
         setOutputUPDFDeviceTrimming (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceMedia", pszValue, cbLength))
      {
         setOutputUPDFDeviceMedia (true);
      }
      else if (0 == strncasecmp ("UPDFDevicePrintMode", pszValue, cbLength))
      {
         setOutputUPDFDevicePrintMode (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceResolution", pszValue, cbLength))
      {
         setOutputUPDFDeviceResolution (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceBooklet", pszValue, cbLength))
      {
         setOutputUPDFDeviceBooklet (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceSheetCollate", pszValue, cbLength))
      {
         setOutputUPDFDeviceSheetCollate (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceCopies", pszValue, cbLength))
      {
         setOutputUPDFDeviceCopies (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceJogging", pszValue, cbLength))
      {
         setOutputUPDFDeviceJogging (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceNUp", pszValue, cbLength))
      {
         setOutputUPDFDeviceNUp (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceScaling", pszValue, cbLength))
      {
         setOutputUPDFDeviceScaling (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceSide", pszValue, cbLength))
      {
         setOutputUPDFDeviceSide (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceStitching", pszValue, cbLength))
      {
         setOutputUPDFDeviceStitching (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceInstance", pszValue, cbLength))
      {
         setOutputUPDFDeviceInstance (true);
      }
      else if (0 == strncasecmp ("UPDFDeviceBlitter", pszValue, cbLength))
      {
         setOutputUPDFDeviceBlitter (true);
      }
      else if (0 == strncasecmp ("XMLDevice", pszValue, cbLength))
      {
         setOutputXMLDevice (true);
      }
      else if (0 == strncasecmp ("XMLDeviceOrientation", pszValue, cbLength))
      {
         setOutputXMLDeviceOrientation (true);
      }
      else if (0 == strncasecmp ("XMLDeviceForm", pszValue, cbLength))
      {
         setOutputXMLDeviceForm (true);
      }
      else if (0 == strncasecmp ("XMLDeviceTray", pszValue, cbLength))
      {
         setOutputXMLDeviceTray (true);
      }
      else if (0 == strncasecmp ("XMLDeviceTrimming", pszValue, cbLength))
      {
         setOutputXMLDeviceTrimming (true);
      }
      else if (0 == strncasecmp ("XMLDeviceMedia", pszValue, cbLength))
      {
         setOutputXMLDeviceMedia (true);
      }
      else if (0 == strncasecmp ("XMLDevicePrintMode", pszValue, cbLength))
      {
         setOutputXMLDevicePrintMode (true);
      }
      else if (0 == strncasecmp ("XMLDeviceResolution", pszValue, cbLength))
      {
         setOutputXMLDeviceResolution (true);
      }
      else if (0 == strncasecmp ("XMLDeviceBooklet", pszValue, cbLength))
      {
         setOutputXMLDeviceBooklet (true);
      }
      else if (0 == strncasecmp ("XMLDeviceSheetCollate", pszValue, cbLength))
      {
         setOutputXMLDeviceSheetCollate (true);
      }
      else if (0 == strncasecmp ("XMLDeviceCopies", pszValue, cbLength))
      {
         setOutputXMLDeviceCopies (true);
      }
      else if (0 == strncasecmp ("XMLDeviceJogging", pszValue, cbLength))
      {
         setOutputXMLDeviceJogging (true);
      }
      else if (0 == strncasecmp ("XMLDeviceNUp", pszValue, cbLength))
      {
         setOutputXMLDeviceNUp (true);
      }
      else if (0 == strncasecmp ("XMLDeviceScaling", pszValue, cbLength))
      {
         setOutputXMLDeviceScaling (true);
      }
      else if (0 == strncasecmp ("XMLDeviceSide", pszValue, cbLength))
      {
         setOutputXMLDeviceSide (true);
      }
      else if (0 == strncasecmp ("XMLDeviceStitching", pszValue, cbLength))
      {
         setOutputXMLDeviceStitching (true);
      }
      else if (0 == strncasecmp ("XMLDeviceInstance", pszValue, cbLength))
      {
         setOutputXMLDeviceInstance (true);
      }
      else if (0 == strncasecmp ("XMLDeviceBlitter", pszValue, cbLength))
      {
         setOutputXMLDeviceBlitter (true);
      }
      else if (0 == strncasecmp ("PDCInterface", pszValue, cbLength))
      {
         setOutputPDCInterface (true);
      }
      else if (0 == strncasecmp ("all", pszValue, cbLength))
      {
         setOutputDeviceBlitter              (true);
         setOutputDeviceDither               (true);
         setOutputDeviceInstance             (true);
         setOutputDevicePrintMode            (true);
         setOutputDeviceTester               (true);
         setOutputGplCompression             (true);
         setOutputGplDitherInstance          (true);
         setOutputOmni                       (true);
         setOutputOmniInterface              (true);
         setOutputOmniProxy                  (true);
         setOutputPrintDevice                (true);
         setOutputBlitter                    (true);
         setOutputInstance                   (true);
         setOutputOmniPDCProxy               (true);
         setOutputOmniServer                 (true);
         setOutputPrinterCommand             (true);
         setOutputPluggableInstance          (true);
         setOutputPluggableBlitter           (true);
         setOutputDriverInfo                 (true);
         setOutputPDCBlitter                 (true);
         setOutputPDCBlitterClient           (true);
         setOutputDeviceString               (true);
         setOutputUPDFDevice                 (true);
         setOutputUPDFDeviceBooklet          (true);
         setOutputUPDFDeviceCopies           (true);
         setOutputUPDFDeviceForm             (true);
         setOutputUPDFDeviceJogging          (true);
         setOutputUPDFDeviceMedia            (true);
         setOutputUPDFDeviceOrientation      (true);
         setOutputUPDFDeviceOutputBin        (true);
         setOutputUPDFDevicePrintMode        (true);
         setOutputUPDFDeviceResolution       (true);
         setOutputUPDFDeviceScaling          (true);
         setOutputUPDFDeviceSheetCollate     (true);
         setOutputUPDFDeviceSide             (true);
         setOutputUPDFDeviceStitching        (true);
         setOutputUPDFDeviceTray             (true);
         setOutputUPDFDeviceTrimming         (true);
         setOutputUPDFDeviceInstance         (true);
         setOutputUPDFDeviceBlitter          (true);
         setOutputXMLDevice                  (true);
         setOutputXMLDeviceBooklet           (true);
         setOutputXMLDeviceCopies            (true);
         setOutputXMLDeviceForm              (true);
         setOutputXMLDeviceJogging           (true);
         setOutputXMLDeviceMedia             (true);
         setOutputXMLDeviceNUp               (true);
         setOutputXMLDeviceOrientation       (true);
         setOutputXMLDeviceOutputBin         (true);
         setOutputXMLDevicePrintMode         (true);
         setOutputXMLDeviceResolution        (true);
         setOutputXMLDeviceScaling           (true);
         setOutputXMLDeviceSheetCollate      (true);
         setOutputXMLDeviceSide              (true);
         setOutputXMLDeviceStitching         (true);
         setOutputXMLDeviceTray              (true);
         setOutputXMLDeviceTrimming          (true);
         setOutputXMLDeviceInstance          (true);
         setOutputXMLDeviceBlitter           (true);
         setOutputPDCInterface               (true);
      }
      else if (0 == strncasecmp ("PDC", pszValue, cbLength))
      {
         setOutputOmniPDCProxy     (true);
         setOutputOmniServer       (true);
         setOutputPrinterCommand   (true);
         setOutputPDCBlitter       (true);
         setOutputPDCBlitterClient (true);
      }
      else if (0 == strncasecmp ("Pluggable", pszValue, cbLength))
      {
         setOutputPluggableInstance (true);
         setOutputPluggableBlitter  (true);
      }
      else if (0 == strncasecmp ("UPDF", pszValue, cbLength))
      {
         setOutputUPDFDevice                 (true);
         setOutputUPDFDeviceBooklet          (true);
         setOutputUPDFDeviceCopies           (true);
         setOutputUPDFDeviceForm             (true);
         setOutputUPDFDeviceJogging          (true);
         setOutputUPDFDeviceMedia            (true);
         setOutputUPDFDeviceNUp              (true);
         setOutputUPDFDeviceOrientation      (true);
         setOutputUPDFDeviceOutputBin        (true);
         setOutputUPDFDevicePrintMode        (true);
         setOutputUPDFDeviceResolution       (true);
         setOutputUPDFDeviceScaling          (true);
         setOutputUPDFDeviceSheetCollate     (true);
         setOutputUPDFDeviceSide             (true);
         setOutputUPDFDeviceStitching        (true);
         setOutputUPDFDeviceTray             (true);
         setOutputUPDFDeviceTrimming         (true);
         setOutputUPDFDeviceInstance         (true);
         setOutputUPDFDeviceBlitter          (true);
      }
      else if (0 == strncasecmp ("XML", pszValue, cbLength))
      {
         setOutputXMLDevice                  (true);
         setOutputXMLDeviceBooklet           (true);
         setOutputXMLDeviceCopies            (true);
         setOutputXMLDeviceForm              (true);
         setOutputXMLDeviceJogging           (true);
         setOutputXMLDeviceMedia             (true);
         setOutputXMLDeviceNUp               (true);
         setOutputXMLDeviceOrientation       (true);
         setOutputXMLDeviceOutputBin         (true);
         setOutputXMLDevicePrintMode         (true);
         setOutputXMLDeviceResolution        (true);
         setOutputXMLDeviceScaling           (true);
         setOutputXMLDeviceSheetCollate      (true);
         setOutputXMLDeviceSide              (true);
         setOutputXMLDeviceStitching         (true);
         setOutputXMLDeviceTray              (true);
         setOutputXMLDeviceTrimming          (true);
         setOutputXMLDeviceInstance          (true);
         setOutputXMLDeviceBlitter           (true);
      }
      else if (0 == strncasecmp ("Instances", pszValue, cbLength))
      {
         setOutputInstance (true);
         setOutputDeviceInstance (true);
         setOutputPluggableInstance (true);
         setOutputUPDFDeviceInstance (true);
         setOutputXMLDeviceInstance (true);
      }
      else if (0 == strncasecmp ("Blitters", pszValue, cbLength))
      {
         setOutputBlitter (true);
         setOutputDeviceBlitter (true);
         setOutputPluggableBlitter (true);
         setOutputUPDFDeviceBlitter (true);
         setOutputXMLDeviceBlitter (true);
      }

      // Update the environment variable
      char *pszDebugOptions    = getenv ("OMNI_DEBUG_OPTIONS");
      int   cbNewDebugOptions  = 0;
      char *pszNewDebugOptions = 0;

      if (  pszDebugOptions
         && *pszDebugOptions
         )
      {  //                                            ' '
         cbNewDebugOptions = strlen (pszDebugOptions) + 1;
      }  //                                   '\0'
      cbNewDebugOptions += strlen (pszValue) + 1;

      pszNewDebugOptions = (char *)malloc (cbNewDebugOptions);
      if (pszNewDebugOptions)
      {
         if (  pszDebugOptions
            && *pszDebugOptions
            )
            sprintf (pszNewDebugOptions, "%s %s", pszDebugOptions, pszValue);
         else
            strcpy (pszNewDebugOptions, pszValue);

         setenv ("OMNI_DEBUG_OPTIONS", pszNewDebugOptions, 1);

         free (pszNewDebugOptions);
      }

      pszValue += cbLength;

      if (pszSpace)
         pszValue++;
   }
#endif
}

bool DebugOutput::
shouldOutputDeviceBlitter ()
{
   return vfOutputDeviceBlitter_d;
}

bool DebugOutput::
shouldOutputDeviceDither ()
{
   return vfOutputDeviceDither_d;
}

bool DebugOutput::
shouldOutputDeviceInstance ()
{
   return vfOutputDeviceInstance_d;
}

bool DebugOutput::
shouldOutputDevicePrintMode ()
{
   return vfOutputDevicePrintMode_d;
}

bool DebugOutput::
shouldOutputDeviceTester ()
{
   return vfOutputDeviceTester_d;
}

bool DebugOutput::
shouldOutputGplCompression ()
{
   return vfOutputGplCompression_d;
}

bool DebugOutput::
shouldOutputGplDitherInstance ()
{
   return vfOutputGplDitherInstance_d;
}

bool DebugOutput::
shouldOutputOmni ()
{
   return vfOutputOmni_d;
}

bool DebugOutput::
shouldOutputOmniInterface ()
{
   return vfOutputOmniInterface_d;
}

bool DebugOutput::
shouldOutputOmniProxy ()
{
   return vfOutputOmniProxy_d;
}

bool DebugOutput::
shouldOutputPrintDevice ()
{
   return vfOutputPrintDevice_d;
}

bool DebugOutput::
shouldOutputBlitter ()
{
   return vfOutputBlitter_d;
}

bool DebugOutput::
shouldOutputInstance ()
{
   return vfOutputInstance_d;
}

bool DebugOutput::
shouldOutputOmniPDCProxy ()
{
   return vfOutputOmniPDCProxy_d;
}

bool DebugOutput::
shouldOutputOmniServer ()
{
   return vfOutputOmniServer_d;
}

bool DebugOutput::
shouldOutputPrinterCommand ()
{
   return vfOutputPrinterCommand_d;
}

bool DebugOutput::
shouldOutputPluggableInstance ()
{
   return vfOutputPluggableInstance_d;
}

bool DebugOutput::
shouldOutputPluggableBlitter ()
{
   return vfOutputPluggableBlitter_d;
}

bool DebugOutput::
shouldOutputDriverInfo ()
{
   return vfOutputDriverInfo_d;
}

bool DebugOutput::
shouldOutputPDCBlitter ()
{
   return vfOutputPDCBlitter_d;
}

bool DebugOutput::
shouldOutputPDCBlitterClient ()
{
   return vfOutputPDCBlitterClient_d;
}

bool DebugOutput::
shouldOutputDeviceString ()
{
   return vfOutputDeviceString_d;
}

bool DebugOutput::
shouldOutputUPDFDevice ()
{
   return vfOutputUPDFDevice_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceOrientation ()
{
   return vfOutputUPDFDeviceOrientation_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceForm ()
{
   return vfOutputUPDFDeviceForm_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceTray ()
{
   return vfOutputUPDFDeviceTray_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceTrimming ()
{
   return vfOutputUPDFDeviceTrimming_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceMedia ()
{
   return vfOutputUPDFDeviceMedia_d;
}

bool DebugOutput::
shouldOutputUPDFDevicePrintMode ()
{
   return vfOutputUPDFDevicePrintMode_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceResolution ()
{
   return vfOutputUPDFDeviceResolution_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceBooklet ()
{
   return vfOutputUPDFDeviceBooklet_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceSheetCollate ()
{
   return vfOutputUPDFDeviceSheetCollate_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceCopies ()
{
   return vfOutputUPDFDeviceCopies_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceJogging ()
{
   return vfOutputUPDFDeviceJogging_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceNUp ()
{
   return vfOutputUPDFDeviceNUp_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceScaling ()
{
   return vfOutputUPDFDeviceScaling_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceSide ()
{
   return vfOutputUPDFDeviceSide_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceStitching ()
{
   return vfOutputUPDFDeviceStitching_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceOutputBin ()
{
   return vfOutputUPDFDeviceOutputBin_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceInstance ()
{
   return vfOutputUPDFDeviceInstance_d;
}

bool DebugOutput::
shouldOutputUPDFDeviceBlitter ()
{
   return vfOutputUPDFDeviceBlitter_d;
}

bool DebugOutput::
shouldOutputXMLDevice ()
{
   return vfOutputXMLDevice_d;
}

bool DebugOutput::
shouldOutputXMLDeviceOrientation ()
{
   return vfOutputXMLDeviceOrientation_d;
}

bool DebugOutput::
shouldOutputXMLDeviceForm ()
{
   return vfOutputXMLDeviceForm_d;
}

bool DebugOutput::
shouldOutputXMLDeviceTray ()
{
   return vfOutputXMLDeviceTray_d;
}

bool DebugOutput::
shouldOutputXMLDeviceTrimming ()
{
   return vfOutputXMLDeviceTrimming_d;
}

bool DebugOutput::
shouldOutputXMLDeviceMedia ()
{
   return vfOutputXMLDeviceMedia_d;
}

bool DebugOutput::
shouldOutputXMLDevicePrintMode ()
{
   return vfOutputXMLDevicePrintMode_d;
}

bool DebugOutput::
shouldOutputXMLDeviceResolution ()
{
   return vfOutputXMLDeviceResolution_d;
}

bool DebugOutput::
shouldOutputXMLDeviceBooklet ()
{
   return vfOutputXMLDeviceBooklet_d;
}

bool DebugOutput::
shouldOutputXMLDeviceSheetCollate ()
{
   return vfOutputXMLDeviceSheetCollate_d;
}

bool DebugOutput::
shouldOutputXMLDeviceCopies ()
{
   return vfOutputXMLDeviceCopies_d;
}

bool DebugOutput::
shouldOutputXMLDeviceJogging ()
{
   return vfOutputXMLDeviceJogging_d;
}

bool DebugOutput::
shouldOutputXMLDeviceNUp ()
{
   return vfOutputXMLDeviceNUp_d;
}

bool DebugOutput::
shouldOutputXMLDeviceScaling ()
{
   return vfOutputXMLDeviceScaling_d;
}

bool DebugOutput::
shouldOutputXMLDeviceSide ()
{
   return vfOutputXMLDeviceSide_d;
}

bool DebugOutput::
shouldOutputXMLDeviceStitching ()
{
   return vfOutputXMLDeviceStitching_d;
}

bool DebugOutput::
shouldOutputXMLDeviceOutputBin ()
{
   return vfOutputXMLDeviceOutputBin_d;
}

bool DebugOutput::
shouldOutputXMLDeviceInstance ()
{
   return vfOutputXMLDeviceInstance_d;
}

bool DebugOutput::
shouldOutputXMLDeviceBlitter ()
{
   return vfOutputXMLDeviceBlitter_d;
}

bool DebugOutput::
shouldOutputPDCInterface ()
{
   return vfOutputPDCInterface_d;
}

void DebugOutput::
setOutputDeviceBlitter (bool fOutputDeviceBlitter)
{
   vfOutputDeviceBlitter_d = fOutputDeviceBlitter;
}

void DebugOutput::
setOutputDeviceDither (bool fOutputDeviceDither)
{
   vfOutputDeviceDither_d = fOutputDeviceDither;
}

void DebugOutput::
setOutputDeviceInstance (bool fOutputDeviceInstance)
{
   vfOutputDeviceInstance_d = fOutputDeviceInstance;
}

void DebugOutput::
setOutputDevicePrintMode (bool fOutputDevicePrintMode)
{
   vfOutputDevicePrintMode_d = fOutputDevicePrintMode;
}

void DebugOutput::
setOutputDeviceTester (bool fOutputDeviceTester)
{
   vfOutputDeviceTester_d = fOutputDeviceTester;
}

void DebugOutput::
setOutputGplCompression (bool fOutputGplCompression)
{
   vfOutputGplCompression_d = fOutputGplCompression;
}

void DebugOutput::
setOutputGplDitherInstance (bool fOutputGplDitherInstance)
{
   vfOutputGplDitherInstance_d = fOutputGplDitherInstance;
}

void DebugOutput::
setOutputOmni (bool fOutputOmni)
{
   vfOutputOmni_d = fOutputOmni;
}

void DebugOutput::
setOutputOmniInterface (bool fOutputOmniInterface)
{
   vfOutputOmniInterface_d = fOutputOmniInterface;
}

void DebugOutput::
setOutputOmniProxy (bool fOutputOmniProxy)
{
   vfOutputOmniProxy_d = fOutputOmniProxy;
}

void DebugOutput::
setOutputPrintDevice (bool fOutputPrintDevice)
{
   vfOutputPrintDevice_d = fOutputPrintDevice;
}

void DebugOutput::
setOutputBlitter (bool fOutputBlitter)
{
   vfOutputBlitter_d = fOutputBlitter;
}

void DebugOutput::
setOutputInstance (bool fOutputInstance)
{
   vfOutputInstance_d = fOutputInstance;
}

void DebugOutput::
setOutputOmniPDCProxy (bool fOutputOmniPDCProxy)
{
   vfOutputOmniPDCProxy_d = fOutputOmniPDCProxy;
}

void DebugOutput::
setOutputOmniServer (bool fOutputOmniServer)
{
   vfOutputOmniServer_d = fOutputOmniServer;
}

void DebugOutput::
setOutputPrinterCommand (bool fOutputPrinterCommand)
{
   vfOutputPrinterCommand_d = fOutputPrinterCommand;
}

void DebugOutput::
setOutputPluggableInstance (bool fOutputPluggableInstance)
{
   vfOutputPluggableInstance_d = fOutputPluggableInstance;
}

void DebugOutput::
setOutputPluggableBlitter (bool fOutputPluggableBlitter)
{
   vfOutputPluggableBlitter_d = fOutputPluggableBlitter;
}

void DebugOutput::
setOutputDriverInfo (bool fOutputDriverInfo)
{
   vfOutputDriverInfo_d = fOutputDriverInfo;
}

void DebugOutput::
setOutputPDCBlitter (bool fOutputPDCBlitter)
{
   vfOutputPDCBlitter_d = fOutputPDCBlitter;
}

void DebugOutput::
setOutputPDCBlitterClient (bool fOutputPDCBlitterClient)
{
   vfOutputPDCBlitterClient_d = fOutputPDCBlitterClient;
}

void DebugOutput::
setOutputDeviceString (bool fOutputDeviceString)
{
   vfOutputDeviceString_d = fOutputDeviceString;
}

void DebugOutput::
setOutputUPDFDevice (bool fOutputUPDFDevice)
{
   vfOutputUPDFDevice_d = fOutputUPDFDevice;
}

void DebugOutput::
setOutputUPDFDeviceOrientation (bool fOutputUPDFDeviceOrientation)
{
   vfOutputUPDFDeviceOrientation_d = fOutputUPDFDeviceOrientation;
}

void DebugOutput::
setOutputUPDFDeviceForm (bool fOutputUPDFDeviceForm)
{
   vfOutputUPDFDeviceForm_d = fOutputUPDFDeviceForm;
}

void DebugOutput::
setOutputUPDFDeviceTray (bool fOutputUPDFDeviceTray)
{
   vfOutputUPDFDeviceTray_d = fOutputUPDFDeviceTray;
}

void DebugOutput::
setOutputUPDFDeviceTrimming (bool fOutputUPDFDeviceTrimming)
{
   vfOutputUPDFDeviceTrimming_d = fOutputUPDFDeviceTrimming;
}

void DebugOutput::
setOutputUPDFDeviceMedia (bool fOutputUPDFDeviceMedia)
{
   vfOutputUPDFDeviceMedia_d = fOutputUPDFDeviceMedia;
}

void DebugOutput::
setOutputUPDFDevicePrintMode (bool fOutputUPDFDevicePrintMode)
{
   vfOutputUPDFDevicePrintMode_d = fOutputUPDFDevicePrintMode;
}

void DebugOutput::
setOutputUPDFDeviceResolution (bool fOutputUPDFDeviceResolution)
{
   vfOutputUPDFDeviceResolution_d = fOutputUPDFDeviceResolution;
}

void DebugOutput::
setOutputUPDFDeviceBooklet (bool fOutputUPDFDeviceBooklet)
{
   vfOutputUPDFDeviceBooklet_d = fOutputUPDFDeviceBooklet;
}

void DebugOutput::
setOutputUPDFDeviceSheetCollate (bool fOutputUPDFDeviceSheetCollate)
{
   vfOutputUPDFDeviceSheetCollate_d = fOutputUPDFDeviceSheetCollate;
}

void DebugOutput::
setOutputUPDFDeviceCopies (bool fOutputUPDFDeviceCopies)
{
   vfOutputUPDFDeviceCopies_d = fOutputUPDFDeviceCopies;
}

void DebugOutput::
setOutputUPDFDeviceJogging (bool fOutputUPDFDeviceJogging)
{
   vfOutputUPDFDeviceJogging_d = fOutputUPDFDeviceJogging;
}

void DebugOutput::
setOutputUPDFDeviceNUp (bool fOutputUPDFDeviceNUp)
{
   vfOutputUPDFDeviceNUp_d = fOutputUPDFDeviceNUp;
}

void DebugOutput::
setOutputUPDFDeviceScaling (bool fOutputUPDFDeviceScaling)
{
   vfOutputUPDFDeviceScaling_d = fOutputUPDFDeviceScaling;
}

void DebugOutput::
setOutputUPDFDeviceSide (bool fOutputUPDFDeviceSide)
{
   vfOutputUPDFDeviceSide_d = fOutputUPDFDeviceSide;
}

void DebugOutput::
setOutputUPDFDeviceStitching (bool fOutputUPDFDeviceStitching)
{
   vfOutputUPDFDeviceStitching_d = fOutputUPDFDeviceStitching;
}

void DebugOutput::
setOutputUPDFDeviceOutputBin (bool fOutputUPDFDeviceOutputBin)
{
   vfOutputUPDFDeviceOutputBin_d = fOutputUPDFDeviceOutputBin;
}

void DebugOutput::
setOutputUPDFDeviceInstance (bool fOutputUPDFDeviceInstance)
{
   vfOutputUPDFDeviceInstance_d = fOutputUPDFDeviceInstance;
}

void DebugOutput::
setOutputUPDFDeviceBlitter (bool fOutputUPDFDeviceBlitter)
{
   vfOutputUPDFDeviceBlitter_d = fOutputUPDFDeviceBlitter;
}

void DebugOutput::
setOutputXMLDevice (bool fOutputXMLDevice)
{
   vfOutputXMLDevice_d = fOutputXMLDevice;
}

void DebugOutput::
setOutputXMLDeviceOrientation (bool fOutputXMLDeviceOrientation)
{
   vfOutputXMLDeviceOrientation_d = fOutputXMLDeviceOrientation;
}

void DebugOutput::
setOutputXMLDeviceForm (bool fOutputXMLDeviceForm)
{
   vfOutputXMLDeviceForm_d = fOutputXMLDeviceForm;
}

void DebugOutput::
setOutputXMLDeviceTray (bool fOutputXMLDeviceTray)
{
   vfOutputXMLDeviceTray_d = fOutputXMLDeviceTray;
}

void DebugOutput::
setOutputXMLDeviceTrimming (bool fOutputXMLDeviceTrimming)
{
   vfOutputXMLDeviceTrimming_d = fOutputXMLDeviceTrimming;
}

void DebugOutput::
setOutputXMLDeviceMedia (bool fOutputXMLDeviceMedia)
{
   vfOutputXMLDeviceMedia_d = fOutputXMLDeviceMedia;
}

void DebugOutput::
setOutputXMLDevicePrintMode (bool fOutputXMLDevicePrintMode)
{
   vfOutputXMLDevicePrintMode_d = fOutputXMLDevicePrintMode;
}

void DebugOutput::
setOutputXMLDeviceResolution (bool fOutputXMLDeviceResolution)
{
   vfOutputXMLDeviceResolution_d = fOutputXMLDeviceResolution;
}

void DebugOutput::
setOutputXMLDeviceBooklet (bool fOutputXMLDeviceBooklet)
{
   vfOutputXMLDeviceBooklet_d = fOutputXMLDeviceBooklet;
}

void DebugOutput::
setOutputXMLDeviceSheetCollate (bool fOutputXMLDeviceSheetCollate)
{
   vfOutputXMLDeviceSheetCollate_d = fOutputXMLDeviceSheetCollate;
}

void DebugOutput::
setOutputXMLDeviceCopies (bool fOutputXMLDeviceCopies)
{
   vfOutputXMLDeviceCopies_d = fOutputXMLDeviceCopies;
}

void DebugOutput::
setOutputXMLDeviceJogging (bool fOutputXMLDeviceJogging)
{
   vfOutputXMLDeviceJogging_d = fOutputXMLDeviceJogging;
}

void DebugOutput::
setOutputXMLDeviceNUp (bool fOutputXMLDeviceNUp)
{
   vfOutputXMLDeviceNUp_d = fOutputXMLDeviceNUp;
}

void DebugOutput::
setOutputXMLDeviceScaling (bool fOutputXMLDeviceScaling)
{
   vfOutputXMLDeviceScaling_d = fOutputXMLDeviceScaling;
}

void DebugOutput::
setOutputXMLDeviceSide (bool fOutputXMLDeviceSide)
{
   vfOutputXMLDeviceSide_d = fOutputXMLDeviceSide;
}

void DebugOutput::
setOutputXMLDeviceStitching (bool fOutputXMLDeviceStitching)
{
   vfOutputXMLDeviceStitching_d = fOutputXMLDeviceStitching;
}

void DebugOutput::
setOutputXMLDeviceOutputBin (bool fOutputXMLDeviceOutputBin)
{
   vfOutputXMLDeviceOutputBin_d = fOutputXMLDeviceOutputBin;
}

void DebugOutput::
setOutputXMLDeviceInstance (bool fOutputXMLDeviceInstance)
{
   vfOutputXMLDeviceInstance_d = fOutputXMLDeviceInstance;
}

void DebugOutput::
setOutputXMLDeviceBlitter (bool fOutputXMLDeviceBlitter)
{
   vfOutputXMLDeviceBlitter_d = fOutputXMLDeviceBlitter;
}

void DebugOutput::
setOutputPDCInterface (bool fOutputPDCInterface)
{
   vfOutputPDCInterface_d = fOutputPDCInterface;
}
