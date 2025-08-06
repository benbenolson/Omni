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

#ifdef INCLUDE_JP_UPDF_BOOKLET
#include <UPDFDeviceBooklet.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
#include <UPDFDeviceCopies.hpp>
#endif
#include <UPDFDeviceForm.hpp>
#ifdef INCLUDE_JP_UPDF_JOGGING
#include <UPDFDeviceJogging.hpp>
#endif
#include <UPDFDeviceMedia.hpp>
#ifdef INCLUDE_JP_COMMON_NUP
#include <UPDFDeviceNUp.hpp>
#endif
#include <UPDFDeviceOrientation.hpp>
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
#include <UPDFDeviceOutputBin.hpp>
#endif
#include <UPDFDevicePrintMode.hpp>
#include <UPDFDeviceResolution.hpp>
#ifdef INCLUDE_JP_COMMON_SCALING
//#include <UPDFDeviceScaling.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
#include <UPDFDeviceSheetCollate.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
#include <UPDFDeviceSide.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
///#include <UPDFDeviceStitching.hpp>
#endif
#include <UPDFDeviceTray.hpp>
#ifdef INCLUDE_JP_COMMON_TRIMMING
#include <UPDFDeviceTrimming.hpp>
#endif
//#include <UPDFDeviceCommands.hpp>
#include <UPDFDeviceInstance.hpp>
#include <UPDFDeviceBlitter.hpp>
#include <UPDFDevice.hpp>
#include <JobProperties.hpp>

#include <glob.h>

static bool      isDeviceConfigurationXMLFile       (XmlDocPtr   doc);
static XmlDocPtr getDeviceConfigurationFromFile     (PSZCRO      pszMasterXMLFile);
static XmlDocPtr getDefaultDeviceConfiguration      ();
static XmlDocPtr getUDRFromDeviceConfiguration      (XmlDocPtr   docDC);
static bool      getOmniNames                       (XmlDocPtr   docDC,
                                                     PSZRO      *ppszDriverName,
                                                     PSZRO      *ppszDeviceName,
                                                     PSZRO      *ppszShortName);

PSZCRO
getVersion ()
{
   return VERSION;
}

Enumeration *
getDeviceEnumeration (PSZCRO pszLibraryName,
                      bool   fBuildOnly)
{
   // @TBD
   return 0;
}

Device *
newDeviceW_Advanced (bool fAdvanced)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": fAdvanced = " << fAdvanced << std::endl;
#endif

   XmlDocPtr docDC         = getDefaultDeviceConfiguration ();
   XmlDocPtr docUDR        = 0;
   PSZRO     pszDriverName = 0;
   PSZRO     pszDeviceName = 0;
   PSZRO     pszShortName  = 0;

   if (docDC)
   {
      docUDR = getUDRFromDeviceConfiguration (docDC);
   }

   if (  docDC
      && docUDR
      && getOmniNames (docUDR,
                       &pszDriverName,
                       &pszDeviceName,
                       &pszShortName)
      )
   {
      PrintDevice *pPrintDevice = new UPDFDevice (docDC,
                                                  docUDR,
                                                  pszDriverName,
                                                  pszDeviceName,
                                                  pszShortName,
                                                  0,  // @TBD
                                                  0); // @TBD

      if (pPrintDevice)
      {
         pPrintDevice->initialize ();

         if (fAdvanced)
            return pPrintDevice;
         else
            return new OmniProxy (pPrintDevice);
      }
   }

   return 0;
}

Device *
newDeviceW_JopProp_Advanced (PSZCRO pszJobProperties,
                             bool   fAdvanced)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": pszJobProperties = \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\", fAdvanced = " << fAdvanced << std::endl;
#endif

   XmlDocPtr  docDC         = 0;
   XmlDocPtr  docUDR        = 0;
   PSZRO      pszDriverName = 0;
   PSZRO      pszDeviceName = 0;
   PSZRO      pszShortName  = 0;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      JobProperties          jobProp (pszJobProperties);
      JobPropertyEnumerator *pEnum                      = 0;

      pEnum = jobProp.getEnumeration ();

      while (pEnum->hasMoreElements ())
      {
         PSZCRO pszKey   = pEnum->getCurrentKey ();
         PSZCRO pszValue = pEnum->getCurrentValue ();

         if (0 == strcmp (pszKey, "UPDFMasterFile"))
         {
            docDC = getDeviceConfigurationFromFile (pszValue);

            if (!docDC)
            {
               DebugOutput::getErrorStream () << "Warning: Could not load Device Configuration from " << pszValue << std::endl;
            }
         }
#ifndef RETAIL
         else if (0 == strcmp (pszKey, "debugoutput"))
         {
            DebugOutput::setDebugOutput (pszValue);
         }
#endif
         pEnum->nextElement ();
      }

      delete pEnum;
   }

   if (!docDC)
   {
      docDC = getDefaultDeviceConfiguration ();
   }

   if (docDC)
   {
      docUDR = getUDRFromDeviceConfiguration (docDC);
   }

   if (  docDC
      && docUDR
      && getOmniNames (docUDR,
                       &pszDriverName,
                       &pszDeviceName,
                       &pszShortName)
      )
   {
      PrintDevice *pPrintDevice = new UPDFDevice (docDC,
                                                  docUDR,
                                                  pszDriverName,
                                                  pszDeviceName,
                                                  pszShortName,
                                                  pszJobProperties,
                                                  0);               // @TBD

      pPrintDevice->initialize ();

      if (fAdvanced)
         return pPrintDevice;
      else
         return new OmniProxy (pPrintDevice);
   }

   if (docDC)
   {
      XMLFreeDoc (docDC);
   }

   return 0;
}

void
deleteDevice (Device *pDevice)
{
   delete pDevice;
}

static bool
isDeviceConfigurationXMLFile (XmlDocPtr doc)
{
   XmlNodePtr nodeRoot = XMLDocGetRootElement (doc);
   bool       fRc      = false;

   if (nodeRoot)
   {
      XmlNodePtr nodeElm = 0;

      nodeElm = XMLFirstNode (nodeRoot);

      if (nodeElm)
      {
         if (0 == strcmp (XMLGetName (nodeElm), "DeviceConfiguration"))
         {
            fRc = true;
         }
      }
   }

   return fRc;
}

static XmlDocPtr
getDeviceConfigurationFromFile (PSZCRO pszMasterXMLFile)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": pszMasterXMLFile = " << SAFE_PRINT_PSZ(pszMasterXMLFile) << std::endl;
#endif

   XmlDocPtr  docRet   = 0;

   docRet = XMLParseFile (pszMasterXMLFile);
///XMLValidateDocument (&validator, docRet); @TBD

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": docRet = " << docRet << std::endl;
#endif

   if (docRet)
   {
      if (isDeviceConfigurationXMLFile (docRet))
      {
         return docRet;
      }

      XMLFreeDoc (docRet);
   }

   return 0;
}

static XmlDocPtr
getDefaultDeviceConfiguration ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": ()" << std::endl;
#endif

   XmlDocPtr docRet  = 0;
   glob_t    globbuf;
   int       rc;

   memset (&globbuf, 0, sizeof (globbuf));
   rc = glob ("*.xml", 0, NULL, &globbuf);

   if (0 == rc)
   {
      for (int i = 0; i < (int)globbuf.gl_pathc; i++)
      {
         docRet = XMLParseFile (globbuf.gl_pathv[i]);
/////////XMLValidateDocument (&validator, docRet); @TBD

         if (docRet)
         {
            if (isDeviceConfigurationXMLFile (docRet))
            {
               break;
            }

            XMLFreeDoc (docRet);
         }
      }
   }

   globfree (&globbuf);

   return docRet;
}

static XmlDocPtr
getUDRFromDeviceConfiguration (XmlDocPtr docDC)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": (" << docDC << ")" << std::endl;
#endif

   XmlDocPtr   docRet     = 0;
   XmlNodePtr  nodeRootDC = 0;
   PSZRO       pszUDR     = 0;

   nodeRootDC = XMLDocGetRootElement (docDC);

   pszUDR = XMLGetProp (nodeRootDC, "UnitDescriptionReference");

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice: " << __FUNCTION__ << ": pszUDR = " << SAFE_PRINT_PSZ(pszUDR) << std::endl;
#endif

   if (pszUDR)
   {
      std::string stringUDR = pszUDR;

      stringUDR += ".xml";

      docRet = XMLParseFile (stringUDR.c_str ());

      if (docRet)
      {
/////////XMLValidateDocument (&validator, docRet); @TBD
      }

      XMLFree ((void *)pszUDR);
   }

   return docRet;
}

static bool
getOmniNames (XmlDocPtr   docDC,
              PSZRO      *ppszDriverName,
              PSZRO      *ppszDeviceName,
              PSZRO      *ppszShortName)
{
   bool fRc = false;

   if (  docDC
      && ppszDriverName
      )
   {
      XmlNodePtr nodeDeviceHeader = XMLDocGetRootElement (docDC);

      nodeDeviceHeader = UPDFDevice::findEntry (nodeDeviceHeader,
                                                "DeviceHeader",
                                                DebugOutput::shouldOutputUPDFDevice ());

      if (nodeDeviceHeader)
      {
         *ppszDriverName = XMLGetProp (nodeDeviceHeader, "Manufacturer");
         *ppszDeviceName = XMLGetProp (nodeDeviceHeader, "ProductName");

         if (  *ppszDeviceName
            && 0 < strlen (*ppszDeviceName)
            )
         {
            *ppszShortName = (PSZRO)malloc (strlen (*ppszDeviceName) + 1);

            if (*ppszShortName)
            {
               PSZRO  pszFrom = *ppszDeviceName;
               PSZ    pszTo   = (PSZ)*ppszShortName;

               while (*pszFrom)
               {
                  switch (*pszFrom)
                  {
                  case '-':
                  case '+':
                  case '(':
                  case ')':
                  case ' ':
                  case '/':
                  case '\\':
                  {
                     *pszTo++ = '_';
                     pszFrom++;
                     break;
                  }
                  default:
                  {
                     *pszTo++ = *pszFrom++;
                  }
                  }
               }
               *pszTo++ = '\0';
            }
         }

         if (  *ppszDriverName
            && *ppszDeviceName
            && *ppszShortName
            )
         {
            fRc = true;
         }
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ())
      DebugOutput::getErrorStream () << "UPDFDevice: "
                                     << __FUNCTION__
                                     << ": *ppszDriverName = "
                                     << (ppszDriverName ? *ppszDriverName : "(null)")
                                     << ", *ppszDeviceName = "
                                     << (ppszDeviceName ? *ppszDeviceName : "(null)")
                                     << ", *ppszShortName = "
                                     << (ppszShortName ? *ppszShortName : "(null)")
                                     << std::endl;
#endif

   return fRc;
}

void UPDFDevice::
commonInit (XmlDocPtr docDC,
            XmlDocPtr docUDR)
{
   setCapabilities ( Capability::MIRROR
                   | Capability::MONOCHROME
                   );

   setRasterCapabilities ( RasterCapabilities::TOP_TO_BOTTOM
                         );

   docDC_d  = docDC;
   docUDR_d = docUDR;

   if (  docDC_d
      && docUDR_d
      )
   {
      nodeRootDC_d  = XMLDocGetRootElement (docDC_d);
      nodeRootUDR_d = XMLDocGetRootElement (docUDR_d);

      XmlNodePtr nodeLocale = 0;

      if ((nodeLocale = findEntry (nodeRootDC_d, "Locale", DebugOutput::shouldOutputUPDFDevice ())) != 0)
      {
         PSZCRO pszLocale = XMLGetProp (nodeLocale, "LocaleReference");

         if (pszLocale)
         {
            std::string stringLocale = pszLocale;

            stringLocale += ".xml";

            docLocale_d = XMLParseFile (stringLocale.c_str ());

            if (docLocale_d)
            {
///////////////XMLValidateDocument (&validator, docLocale_d); @TBD

               nodeRootLocale_d = XMLDocGetRootElement (docLocale_d);
            }

            XMLFree ((void *)pszLocale);
         }
      }

      PSZCRO pszCS = XMLGetProp (nodeRootDC_d, "CommandSequencesReference");

      if (pszCS)
      {
         std::string stringCS = pszCS;

         stringCS += ".xml";

         docCS_d = XMLParseFile (stringCS.c_str ());

         if (docCS_d)
         {
////////////XMLValidateDocument (&validator, docCS_d); @TBD

            nodeRootCS_d = XMLDocGetRootElement (docCS_d);
         }

         XMLFree ((void *)pszCS);
      }
   }

   pInstance_d = new UPDFDeviceInstance (this);

   setDeviceInstance (pInstance_d);
   setDeviceBlitter (new UPDFDeviceBlitter (this));

   setPDL (new PDL (PDL::PDL_Epson,PDL::LEVEL_ESCP_2,1,0)); // @TBD

   setVirtualUnits ();
}

UPDFDevice::
UPDFDevice (XmlDocPtr  docDC,
            XmlDocPtr  docUDR,
            PSZCRO     pszDriverName,
            PSZCRO     pszDeviceName,
            PSZCRO     pszShortName,
            PSZCRO     pszJobProperties,
            PSZCRO     pszXMLDeviceName)
   : PrintDevice (pszDriverName,
                  pszDeviceName,
                  pszShortName,
                  "libUPDFOmniDevice.so",
                  OMNI_CLASS_UPDF,
                  pszJobProperties)
{
   pszDriverName_d = pszDriverName;
   pszDeviceName_d = pszDeviceName;
   pszShortName_d  = pszShortName;

   commonInit (docDC, docUDR);
}

UPDFDevice::
~UPDFDevice ()
{
   if (docDC_d)
   {
      XMLFreeDoc (docDC_d);
      docDC_d = 0;
   }
   if (docUDR_d)
   {
      XMLFreeDoc (docUDR_d);
      docUDR_d = 0;
   }
   if (docLocale_d)
   {
      XMLFreeDoc (docLocale_d);
      docLocale_d = 0;
   }
   if (docCS_d)
   {
      XMLFreeDoc (docCS_d);
      docCS_d = 0;
   }
   if (pszDriverName_d)
   {
      XMLFree ((void *)pszDriverName_d);
      pszDriverName_d = 0;
   }
   if (pszDeviceName_d)
   {
      XMLFree ((void *)pszDeviceName_d);
      pszDeviceName_d = 0;
   }
   if (pszShortName_d)
   {
      free ((void *)pszShortName_d);
      pszShortName_d = 0;
   }
}

#ifdef INCLUDE_JP_UPDF_BOOKLET
DeviceBooklet * UPDFDevice::
getDefaultBooklet ()
{
   // @TBD
   return 0;
}
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
DeviceCopies * UPDFDevice::
getDefaultCopies ()
{
   DeviceCopies *pCopies  = 0;
   PSZRO         pszValue = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("Copies", "ID");

      if (pszValue)
      {
         std::ostringstream oss;

         oss << "Copies=" << pszValue;

         pCopies = UPDFDeviceCopies::createS (this, oss.str ().c_str ());
      }
   }

#ifndef RETAIL
   if (!pCopies)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pCopies is NULL, pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pCopies)
   {
      return pCopies;
   }
   else
   {
      std::ostringstream oss;

      DefaultCopies::writeDefaultJP (oss);

      return DefaultCopies::createS (this, oss.str ().c_str ());
   }
}
#endif

PSZCRO UPDFDevice::
getDefaultDitherID ()
{
   // @TBD
   return "DITHER_STUCKI_DIFFUSION";
}

DeviceForm * UPDFDevice::
getDefaultForm ()
{
   DeviceForm *pForm    = 0;
   PSZRO       pszValue = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("MediaSize", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (pszValue)
      {
         std::ostringstream oss;

         oss << "Form=" << pszValue;

         pForm = UPDFDeviceForm::createS (this, oss.str ().c_str ());
      }
   }

#ifndef RETAIL
   if (!pForm)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pForm is NULL, pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pForm)
      return pForm;
   else
      // @TBD - hard defaults
      return 0;
}

#ifdef INCLUDE_JP_UPDF_JOGGING
DeviceJogging * UPDFDevice::
getDefaultJogging ()
{
   // @TBD
   return 0;
}
#endif

DeviceMedia * UPDFDevice::
getDefaultMedia ()
{
   DeviceMedia *pMedia   = 0;
   PSZRO        pszValue = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("MediaType", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (pszValue)
      {
         std::ostringstream oss;
         PSZRO              pszOmniJobProperties = 0;

         if (UPDFDeviceMedia::mapUPDFToOmni (pszValue, &pszOmniJobProperties))
         {
            oss << "media=" << pszOmniJobProperties;

            pMedia = UPDFDeviceMedia::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pMedia)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pMedia is NULL, pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pMedia)
   {
      return pMedia;
   }
   else
   {
      std::ostringstream oss;

      DefaultMedia::writeDefaultJP (oss);

      return DefaultMedia::createS (this, oss.str ().c_str ());
   }
}

#ifdef INCLUDE_JP_COMMON_NUP
DeviceNUp * UPDFDevice::
getDefaultNUp ()
{
   DeviceNUp *pNUp                             = 0;
   PSZRO      pszNumberUp                      = 0;
   PSZRO      pszPresentationDirectionNumberUp = 0;

   if (pInstance_d)
   {
      PSZRO pszOmniJobProperties = 0;

      pszNumberUp                      = pInstance_d->getXMLObjectValue ("NumberUp",                      "ClassifyingID");
      pszPresentationDirectionNumberUp = pInstance_d->getXMLObjectValue ("PresentationDirectionNumberUp", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ())
      {
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszNumberUp                      = " << SAFE_PRINT_PSZ (pszNumberUp) << std::endl;
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszPresentationDirectionNumberUp = " << SAFE_PRINT_PSZ (pszPresentationDirectionNumberUp) << std::endl;
      }
#endif

      if (UPDFDeviceNUp::mapUPDFToOmni (pszNumberUp,
                                        pszPresentationDirectionNumberUp,
                                        0,
                                        0,
                                        &pszOmniJobProperties))
      {
         pNUp = UPDFDeviceNUp::createS (this, pszOmniJobProperties);

         free ((void *)pszOmniJobProperties);
      }
   }

#ifndef RETAIL
   if (!pNUp)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pNUp is NULL, pszNumberUp = " << SAFE_PRINT_PSZ (pszNumberUp) << ", pszPresentationDirectionNumberUp = " << SAFE_PRINT_PSZ (pszPresentationDirectionNumberUp) << std::endl;
   }
#endif

   if (pszNumberUp)
   {
      XMLFree ((void *)pszNumberUp);
   }
   if (pszPresentationDirectionNumberUp)
   {
      XMLFree ((void *)pszPresentationDirectionNumberUp);
   }

   if (pNUp)
   {
      return pNUp;
   }
   else
   {
      std::ostringstream oss;

      DefaultNUp::writeDefaultJP (oss);

      return DefaultNUp::createS (this, oss.str ().c_str ());
   }
}
#endif

DeviceOrientation * UPDFDevice::
getDefaultOrientation ()
{
   DeviceOrientation *pOrientation   = 0;
   PSZRO              pszOrientation = 0;
   PSZRO              pszRotation    = 0;

   if (pInstance_d)
   {
      PSZRO pszOmniValue = 0;

      pszOrientation = pInstance_d->getXMLObjectValue ("OrientationRequested",
                                                       "ClassifyingID");
      pszRotation    = pInstance_d->getXMLObjectValue ("MediaPageRotation",
                                                       "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ())
      {
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszOrientation = " << SAFE_PRINT_PSZ (pszOrientation) << std::endl;
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszRotation    = " << SAFE_PRINT_PSZ (pszRotation) << std::endl;
      }
#endif

      if (  pszOrientation
         && pszRotation
         )
      {
         if (UPDFDeviceOrientation::mapUPDFToOmni (pszOrientation,
                                                   pszRotation,
                                                   &pszOmniValue))
         {
            std::ostringstream oss;

            oss << "Rotation=" << pszOmniValue;

            pOrientation = UPDFDeviceOrientation::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pOrientation)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pOrientation is NULL. pszOrientation = " << SAFE_PRINT_PSZ (pszOrientation) << ", pszRotation = " << SAFE_PRINT_PSZ (pszRotation) << std::endl;
   }
#endif

   if (pszOrientation)
   {
      XMLFree ((void *)pszOrientation);
   }

   if (pszRotation)
   {
      XMLFree ((void *)pszRotation);
   }

   if (pOrientation)
   {
      return pOrientation;
   }
   else
   {
      std::ostringstream oss;

      DefaultOrientation::writeDefaultJP (oss);

      return DefaultOrientation::createS (this, oss.str ().c_str ());
   }
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
DeviceOutputBin * UPDFDevice::
getDefaultOutputBin ()
{
   DeviceOutputBin *pOutputBin = 0;
   PSZRO            pszValue   = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("OutputBin", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (pszValue)
      {
         std::ostringstream oss;
         PSZRO              pszOmniJobProperties = 0;

         if (UPDFDeviceOutputBin::mapUPDFToOmni (pszValue, &pszOmniJobProperties))
         {
            oss << "OutputBin=" << pszOmniJobProperties;

            pOutputBin = UPDFDeviceOutputBin::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pOutputBin)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pOutputBin)
   {
      return pOutputBin;
   }
   else
   {
      std::ostringstream oss;

      DefaultOutputBin::writeDefaultJP (oss);

      return DefaultOutputBin::createS (this, oss.str ().c_str ());
   }
}
#endif

DevicePrintMode * UPDFDevice::
getDefaultPrintMode ()
{
   DevicePrintMode *pPrintMode = 0;
   PSZRO            pszValue   = 0;

   if (pInstance_d)
   {
      PSZRO pszOmniJobProperties = 0;

      pszValue = pInstance_d->getXMLObjectValue ("Color", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (UPDFDevicePrintMode::mapUPDFToOmni (pszValue,
                                              &pszOmniJobProperties))
      {
         std::ostringstream oss;
         PSZRO              pszOmniJobProperties = 0;

         if (UPDFDevicePrintMode::mapUPDFToOmni (pszValue, &pszOmniJobProperties))
         {
            oss << "printmode=" << pszOmniJobProperties;

            pPrintMode = UPDFDevicePrintMode::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pPrintMode)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pPrintMode)
      return pPrintMode;
   else
      // @TBD - hard defaults
      return 0;
}

DeviceResolution * UPDFDevice::
getDefaultResolution ()
{
   DeviceResolution *pResolution = 0;
   PSZRO             pszValue    = 0;

   if (pInstance_d)
   {
      PSZRO pszOmniJobProperties = 0;

      pszValue = pInstance_d->getXMLObjectValue ("PrinterResolution", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (UPDFDeviceResolution::mapUPDFToOmni (pszValue,
                                               &pszOmniJobProperties))
      {
         pResolution = UPDFDeviceResolution::createS (this, pszOmniJobProperties);

         free ((void *)pszOmniJobProperties);
      }
   }

#ifndef RETAIL
   if (!pResolution)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pResolution)
      return pResolution;
   else
      // @TBD - hard defaults
      return 0;
}

#ifdef INCLUDE_JP_COMMON_SCALING
DeviceScaling * UPDFDevice::
getDefaultScaling ()
{
   std::ostringstream oss;

   DefaultScaling::writeDefaultJP (oss);

   return DefaultScaling::createS (this, oss.str ().c_str ());
}
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
DeviceSheetCollate * UPDFDevice::
getDefaultSheetCollate ()
{
   DeviceSheetCollate *pSheetCollate = 0;
   PSZRO               pszValue      = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("SheetCollate", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (pszValue)
      {
         std::ostringstream oss;
         PSZRO              pszOmniJobProperties = 0;

         if (UPDFDeviceSheetCollate::mapUPDFToOmni (pszValue, &pszOmniJobProperties))
         {
            oss << "SheetCollate=" << pszOmniJobProperties;

            pSheetCollate = UPDFDeviceSheetCollate::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pSheetCollate)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pSheetCollate)
   {
      return pSheetCollate;
   }
   else
   {
      std::ostringstream oss;

      DefaultSheetCollate::writeDefaultJP (oss);

      return DefaultSheetCollate::createS (this, oss.str ().c_str ());
   }
}
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
DeviceSide * UPDFDevice::
getDefaultSide ()
{
   DeviceSide  *pSide    = 0;
   PSZRO        pszValue = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("Sides", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (pszValue)
      {
         std::ostringstream oss;
         PSZRO              pszOmniJobProperties = 0;

         if (UPDFDeviceSide::mapUPDFToOmni (pszValue, &pszOmniJobProperties))
         {
            oss << "Sides=" << pszOmniJobProperties;

            pSide = UPDFDeviceSide::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pSide)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pSide)
   {
      return pSide;
   }
   else
   {
      std::ostringstream oss;

      DefaultSide::writeDefaultJP (oss);

      return DefaultSide::createS (this, oss.str ().c_str ());
   }
}
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
DeviceStitching * UPDFDevice::
getDefaultStitching ()
{
   // @TBD
   std::ostringstream oss;

   DefaultStitching::writeDefaultJP (oss);

   return DefaultStitching::createS (this, oss.str ().c_str ());
}
#endif

DeviceTray * UPDFDevice::
getDefaultTray ()
{
   DeviceTray *pTray    = 0;
   PSZRO       pszValue = 0;

   if (pInstance_d)
   {
      pszValue = pInstance_d->getXMLObjectValue ("MediaInputTrayCheck", "ClassifyingID");

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
#endif

      if (pszValue)
      {
         std::ostringstream oss;
         PSZRO              pszOmniJobProperties = 0;

         if (UPDFDeviceTray::mapUPDFToOmni (pszValue, &pszOmniJobProperties))
         {
            oss << "InputTray=" << pszOmniJobProperties;

            pTray = UPDFDeviceTray::createS (this, oss.str ().c_str ());
         }
      }
   }

#ifndef RETAIL
   if (!pTray)
   {
      if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: pszValue = " << SAFE_PRINT_PSZ (pszValue) << std::endl;
   }
#endif

   if (pszValue)
   {
      XMLFree ((void *)pszValue);
   }

   if (pTray)
   {
      return pTray;
   }
   else
   {
      std::ostringstream oss;

      DefaultTray::writeDefaultJP (oss);

      return DefaultTray::createS (this, oss.str ().c_str ());
   }
}

#ifdef INCLUDE_JP_COMMON_TRIMMING
DeviceTrimming * UPDFDevice::
getDefaultTrimming ()
{
   std::ostringstream oss;

   DefaultTrimming::writeDefaultJP (oss);

   return DefaultTrimming::createS (this, oss.str ().c_str ());
}
#endif

DeviceCommand * UPDFDevice::
getDefaultCommands ()
{
   // @TBD
   return 0;
///return new UPDFDeviceCommands (this);
}

DeviceData * UPDFDevice::
getDefaultData ()
{
   // @TBD
   return 0;
}

DeviceString * UPDFDevice::
getDefaultString ()
{
   // @TBD
   return 0;
}

DeviceGamma * UPDFDevice::
getCurrentGamma ()
{
   // @TBD
   return 0;
}

UPDFDevice * UPDFDevice::
isAUPDFDevice (Device *pDevice)
{
   return dynamic_cast<UPDFDevice *>(pDevice);
}

XmlNodePtr UPDFDevice::
findDCEntry (XmlNodePtr  root,
             PSZCRO      pszName,
             bool        fShouldDebugOutput)
{
   if (root == 0)
   {
      root = nodeRootDC_d;
   }

   return findEntry (root, pszName, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findUDREntry (XmlNodePtr  root,
              PSZCRO      pszName,
              bool        fShouldDebugOutput)
{
   if (root == 0)
   {
      root = nodeRootUDR_d;
   }

   return findEntry (root, pszName, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findLocaleEntry (XmlNodePtr  root,
                 PSZCRO      pszName,
                 bool        fShouldDebugOutput)
{
   if (root == 0)
   {
      root = nodeRootLocale_d;
   }

   return findEntry (root, pszName, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findCSEntry (XmlNodePtr  root,
             PSZCRO      pszName,
             bool        fShouldDebugOutput)
{
   if (root == 0)
   {
      root = nodeRootCS_d;
   }

   return findEntry (root, pszName, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findDCEntryKeyValue (PSZCRO pszKey,
                     PSZCRO pszValue,
                     bool   fShouldDebugOutput)
{
   return findEntryKeyValue (nodeRootDC_d, pszKey, pszValue, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findUDREntryKeyValue (PSZCRO pszKey,
                      PSZCRO pszValue,
                      bool   fShouldDebugOutput)
{
   return findEntryKeyValue (nodeRootUDR_d, pszKey, pszValue, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findLocaleEntryKeyValue (PSZCRO pszKey,
                         PSZCRO pszValue,
                         bool   fShouldDebugOutput)
{
   return findEntryKeyValue (nodeRootLocale_d, pszKey, pszValue, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findCSEntryKeyValue (PSZCRO pszKey,
                     PSZCRO pszValue,
                     bool   fShouldDebugOutput)
{
   return findEntryKeyValue (nodeRootCS_d, pszKey, pszValue, fShouldDebugOutput);
}

XmlNodePtr UPDFDevice::
findEntry (XmlNodePtr  root,
           PSZCRO      pszName,
           bool        fShouldDebugOutput)
{
   if (root == 0)
      return 0;

   XmlNodePtr elm = XMLFirstNode (XMLGetChildrenNode (root));

   while (elm != 0)
   {
#ifndef RETAIL
//////if (DebugOutput::shouldOutputUPDFDevice ()) DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": " << XMLGetName (elm) << " == " << pszName << std::endl;
#endif

      if (0 == strcmp (XMLGetName (elm), pszName))
      {
         break;
      }

      elm = XMLNextNode (elm);
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDevice ())
   {
      if (!elm)
      {
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Did not find " << pszName << std::endl;
      }
   }
#endif

   return elm;
}

XmlNodePtr UPDFDevice::
findEntryKeyValue (XmlNodePtr  root,
                   PSZCRO      pszKey,
                   PSZCRO      pszValue,
                   bool        fShouldDebugOutput)
{
   if (root == 0)
      return 0;

   XmlNodePtr elm      = root;
   XmlNodePtr elmFound = 0;

   while (elm != 0)
   {
      PSZCRO pszElmValue = XMLGetProp (elm, pszKey);

      if (  pszElmValue
         && 0 == strcmp (pszElmValue, pszValue)
         )
      {
         elmFound = elm;
      }

      if (pszElmValue)
      {
         XMLFree ((void *)pszElmValue);
      }

      if (  !elmFound
         && XMLGetChildrenNode (elm)
         )
      {
         elmFound = findEntryKeyValue (XMLGetChildrenNode (elm), pszKey, pszValue, false);
      }

      if (elmFound)
      {
         elm = elmFound;
         break;
      }

      elm = XMLNextNode (elm);
   }

#ifndef RETAIL
   if (fShouldDebugOutput)
   {
      if (elm)
      {
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Found ";
      }
      else
      {
         DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Did not find ";
      }
      DebugOutput::getErrorStream () << pszValue << std::endl;
   }
#endif

   return elm;
}

bool UPDFDevice::
setVirtualUnits ()
{
   XmlNodePtr nodeHeader = 0;
   int        iRet       = 0;

   if (  ((nodeHeader = findUDREntry (nodeHeader, "PrintCapabilities", DebugOutput::shouldOutputUPDFDevice ())) != 0)
      && ((nodeHeader = findUDREntry (nodeHeader, "Header",            DebugOutput::shouldOutputUPDFDevice ())) != 0)
      && ((nodeHeader = findUDREntry (nodeHeader, "VirtualUnits",      DebugOutput::shouldOutputUPDFDevice ())) != 0)
      )
   {
      PSZCRO pszVirtualUnits = XMLNodeListGetString (nodeHeader,
                                                     XMLGetDocNode (nodeHeader),
                                                     1);

      if (pszVirtualUnits)
      {
         int iX = 0,
             iY = 0;

         iRet = sscanf (pszVirtualUnits, "VirtualUnits_%dx%d", &iX, &iY);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDevice ())
            DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": iRet = " << iRet << ", iX = " << iX << ", iY = " << iY << std::endl;
#endif

         if (2 == iRet)
         {
            iVirtualUnitsX_d = iX;
            iVirtualUnitsY_d = iY;
         }

         XMLFree ((void *)pszVirtualUnits);
      }
   }

#ifndef RETAIL
   if (  !iVirtualUnitsX_d
      || !iVirtualUnitsY_d
      )
   {
      DebugOutput::getErrorStream () << "UPDFDevice::" << __FUNCTION__ << ": Error: Could not find virtual units. nodeHeader = " << std::hex << (int)nodeHeader << std::dec << std::endl;
   }
#endif

   return iRet == true;
}

int UPDFDevice::
getXVirtualUnits ()
{
   return iVirtualUnitsX_d;
}

int UPDFDevice::
getYVirtualUnits ()
{
   return iVirtualUnitsY_d;
}

XmlNodePtr UPDFDevice::
getRootDC ()
{
   return nodeRootDC_d;
}

XmlNodePtr UPDFDevice::
getRootUDR ()
{
   return nodeRootUDR_d;
}

XmlNodePtr UPDFDevice::
getRootLocale ()
{
   return nodeRootLocale_d;
}

XmlNodePtr UPDFDevice::
getRootCS ()
{
   return nodeRootCS_d;
}
