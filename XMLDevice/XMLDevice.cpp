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
#include <Device.hpp> // Yank in INCLUDE_XXX defines

#ifdef INCLUDE_JP_UPDF_BOOKLET
#include <XMLDeviceBooklet.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
#include <XMLDeviceCopies.hpp>
#endif
#include <XMLDeviceForm.hpp>
#ifdef INCLUDE_JP_UPDF_JOGGING
#include <XMLDeviceJogging.hpp>
#endif
#include <XMLDeviceMedia.hpp>
#ifdef INCLUDE_JP_COMMON_NUP
#include <XMLDeviceNUp.hpp>
#endif
#include <XMLDeviceOrientation.hpp>
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
#include <XMLDeviceOutputBin.hpp>
#endif
#include <XMLDevicePrintMode.hpp>
#include <XMLDeviceResolution.hpp>
#ifdef INCLUDE_JP_COMMON_SCALING
#include <XMLDeviceScaling.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
#include <XMLDeviceSheetCollate.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
#include <XMLDeviceSide.hpp>
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
#include <XMLDeviceStitching.hpp>
#endif
#include <XMLDeviceTray.hpp>
#ifdef INCLUDE_JP_COMMON_TRIMMING
#include <XMLDeviceTrimming.hpp>
#endif
#include <XMLDeviceInstance.hpp>
#include <XMLDeviceBlitter.hpp>
#include <XMLDevice.hpp>

#include <Omni.hpp>
#include <JobProperties.hpp>

#include <typeinfo>

static bool              parseHexGroup                  (PSZCRO        pszData,
                                                         unsigned int *puiData);
static XmlDocPtr         getDeviceConfigurationFromFile (PSZCRO        pszMasterXMLFile);
static bool              isDeviceConfigurationXMLFile   (XmlDocPtr     doc);
static void              convertFilename                (char         *pszFileName);
static PSZCRO            convertToLibrary               (char         *pszFileName);

PSZCRO
getVersion ()
{
   return VERSION;
}

class XMLDeviceEnumeration : public Enumeration
{
public:
   XMLDeviceEnumeration (PSZCRO pszLibraryName,
                         bool   fBuildOnly)
   {
      pszLibraryName_d = 0;
      pEnum_d          = Omni::listXMLDevices (fBuildOnly);

      if (  pszLibraryName
         && *pszLibraryName
         )
      {
         pszLibraryName_d = (char *)malloc (strlen (pszLibraryName) + 1);
         if (pszLibraryName_d)
         {
            strcpy (pszLibraryName_d, pszLibraryName);
         }
      }
   }

   virtual
   ~XMLDeviceEnumeration ()
   {
      if (pszLibraryName_d)
      {
         free (pszLibraryName_d);
         pszLibraryName_d = 0;
      }

      delete pEnum_d;
      pEnum_d = 0;
   }

   virtual bool
   hasMoreElements ()
   {
      if (!pEnum_d)
      {
         return false;
      }

      return pEnum_d->hasMoreElements ();
   }

   virtual void *
   nextElement ()
   {
      char *pszMasterXML = 0;
      void *pvRet        = 0;

      if (pEnum_d)
      {
         pszMasterXML = (char *)pEnum_d->nextElement ();
      }

      if (pszMasterXML)
      {
         std::ostringstream oss;

         oss << "XMLMasterFile=\""
             << pszMasterXML
             << "\""
             << std::ends;

         PSZCRO pszJobProperties = oss.str ().c_str ();

         pvRet = new OmniDevice (pszLibraryName_d,
                                 pszJobProperties);
      }

      return pvRet;
   }

private:
   char        *pszLibraryName_d;
   Enumeration *pEnum_d;
};

Enumeration *
getDeviceEnumeration (PSZCRO pszLibraryName,
                      bool   fBuildOnly)
{
   return new XMLDeviceEnumeration (pszLibraryName, fBuildOnly);
}

Device *
newDeviceW_Advanced (bool fAdvanced)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": fAdvanced = " << fAdvanced << std::endl;
#endif

   DebugOutput::logMessage (LOG_INFO, "XMLDevice:newDevice: Advanced = %d", fAdvanced);

   std::cerr << "Error: newDeviceW_Advanced should not be called!" << std::endl;

   return 0;
}

Device *
newDeviceW_JopProp_Advanced (PSZCRO pszJobProperties,
                             bool   fAdvanced)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszJobProperties = \"" << pszJobProperties << "\", fAdvanced = " << fAdvanced << std::endl;
#endif

   if (pszJobProperties)
   {
      DebugOutput::logMessage (LOG_INFO, "XMLDevice:newDevice: JobProperties = \"%s\", Advanced = %d", pszJobProperties, fAdvanced);
   }
   else
   {
      DebugOutput::logMessage (LOG_INFO, "XMLDevice:newDevice: JobProperties = null, Advanced = %d", fAdvanced);
   }

   XMLInitialize ();

   if (  !pszJobProperties
      || !*pszJobProperties
      )
   {
      return 0;
   }

   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   char                  *pszXMLDeviceName           = 0;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, "XMLMasterFile"))
      {
         pszXMLDeviceName = (char *)malloc (strlen (pszValue) + 1);
         if (pszXMLDeviceName)
         {
            strcpy (pszXMLDeviceName, pszValue);
         }
         break;
      }

#ifndef RETAIL
      if (0 == strcmp (pszKey, "debugoutput"))
      {
         DebugOutput::setDebugOutput (pszValue);
      }
#endif

      pEnum->nextElement ();
   }

   delete pEnum;

   if (!pszXMLDeviceName)
   {
      std::cerr << "Error: XMLMasterFile=\"...\" must be specified in the job properties ("
                << (pszJobProperties ? pszJobProperties : "")
                << ")!" << std::endl;

      XMLCleanup ();

      return 0;
   }

   PSZCRO    pszMasterXML = Omni::openXMLFile (pszXMLDeviceName);
   XmlDocPtr docDevice    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszMasterXML = " << (pszMasterXML ? pszMasterXML : "NULL") << std::endl;
#endif

   if (pszMasterXML)
   {
      docDevice = getDeviceConfigurationFromFile (pszMasterXML);
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDevice = " << docDevice << std::endl;
#endif

   if (docDevice)
   {
      XmlNodePtr  rootDevice    = XMLDocGetRootElement (docDevice);
      XmlNodePtr  elm           = 0;
      PSZRO       pszDriverName = 0;
      PSZRO       pszDeviceName = 0;
      PSZ         pszShortName  = 0;

      elm = XMLFirstNode (rootDevice);

      if (elm)
      {
         pszDeviceName = XMLGetProp (elm, "name");
         pszShortName  = (char *)XMLGetProp (elm, "name");
      }

      pszDriverName = getXMLContentString (rootDevice, docDevice, "DriverName");

      if (pszShortName)
      {
         convertFilename (pszShortName);
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ())
      {
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszDriverName = " << pszDriverName << std::endl;
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszDeviceName = " << pszDeviceName << std::endl;
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszShortName  = " << pszShortName << std::endl;
      }
#endif

      if (   pszDriverName
         && *pszDriverName
         &&  pszDeviceName
         && *pszDeviceName
         &&  pszShortName
         && *pszShortName
         )
      {
         PrintDevice *pPrintDevice = 0;

         pPrintDevice = new XMLDevice (pszMasterXML,
                                       docDevice,
                                       pszDriverName,
                                       pszDeviceName,
                                       pszShortName,
                                       pszJobProperties,
                                       pszXMLDeviceName);

         if (pszXMLDeviceName)
         {
            XMLFree ((void *)pszXMLDeviceName);
            pszXMLDeviceName = 0;
         }

         if (pPrintDevice)
         {
            pPrintDevice->initialize ();

            if (fAdvanced)
               return pPrintDevice;
            else
               return new OmniProxy (pPrintDevice);
         }
      }

      if (pszDriverName)
      {
         XMLFree ((void *)pszDriverName);
      }
      if (pszDeviceName)
      {
         XMLFree ((void *)pszDeviceName);
      }
      if (pszShortName)
      {
         XMLFree ((void *)pszShortName);
      }
      if (docDevice)
      {
         XMLFreeDoc (docDevice);
      }
   }

   if (pszMasterXML)
   {
      free ((void *)pszMasterXML);
   }
   if (pszXMLDeviceName)
   {
      free (pszXMLDeviceName);
      pszXMLDeviceName = 0;
   }

   XMLCleanup ();

   return 0;
}

void
deleteDevice (Device *pDevice)
{
   delete pDevice;
}

XMLDevice::
XMLDevice (PSZCRO    pszMasterXML,
           XmlDocPtr docDevice,
           PSZCRO    pszDriverName,
           PSZCRO    pszDeviceName,
           PSZCRO    pszShortName,
           PSZCRO    pszJobProperties,
           PSZCRO    pszXMLDeviceName)
   : PrintDevice (pszDriverName,
                  pszDeviceName,
                  pszShortName,
                  "libXMLOmniDevice.so",
                  OMNI_CLASS_XML,
                  pszJobProperties)
{
   // Store the root path where the master XML file is located.
   // All children XML files are loaded from that path.
   pstringMasterXMLPath_d = 0;

   if (  pszMasterXML
      && *pszMasterXML
      )
   {
      std::string            stringMasterXMLPath = pszMasterXML;
      std::string::size_type indexSlash          = 0;

      indexSlash = stringMasterXMLPath.find_last_of ("/");

      if (std::string::npos != indexSlash)
      {
         pstringMasterXMLPath_d = new std::string (stringMasterXMLPath.substr (0, indexSlash + 1));
      }
   }

   if (!pstringMasterXMLPath_d)
   {
      pstringMasterXMLPath_d = new std::string ("");
   }

   pszXMLDeviceName_d           = 0;

   pszDriverName_d              = pszDriverName;
   pszDeviceName_d              = pszDeviceName;
   pszShortName_d               = pszShortName;

   docDevice_d                  = docDevice;
   rootDevice_d                 = 0;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   docDeviceBooklets_d          = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   docDeviceCopies_d            = 0;
#endif
#ifdef INCLUDE_JP_UPDF_JOGGING
   docDeviceJoggings_d          = 0;
#endif
   docDeviceForms_d             = 0;
   docDeviceMedias_d            = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   docDeviceNUps_d              = 0;
#endif
   docDeviceOrientations_d      = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   docDeviceOutputBins_d        = 0;
#endif
   docDevicePrintModes_d        = 0;
   docDeviceResolutions_d       = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   docDeviceScalings_d          = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   docDeviceSheetCollates_d     = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   docDeviceSides_d             = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   docDeviceStitchings_d        = 0;
#endif
   docDeviceTrays_d             = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   docDeviceTrimmings_d         = 0;
#endif
   docDeviceGammas_d            = 0;
   docDeviceCommands_d          = 0;
   docDeviceDatas_d             = 0;
   docDeviceStrings_d           = 0;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   pstringDefaultBooklet_d      = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   pstringDefaultCopies_d       = 0;
#endif
   pszDefaultDitherID_d         = 0;
   pstringDefaultForm_d         = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   pstringDefaultJogging_d      = 0;
#endif
   pstringDefaultMedia_d        = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   pstringDefaultNUp_d          = 0;
#endif
   pstringDefaultOrientation_d  = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   pstringDefaultOutputBin_d    = 0;
#endif
   pstringDefaultPrintMode_d    = 0;
   pstringDefaultResolution_d   = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   pstringDefaultScaling_d      = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   pstringDefaultSheetCollate_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   pstringDefaultSide_d         = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   pstringDefaultStitching_d    = 0;
#endif
   pstringDefaultTray_d         = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   pstringDefaultTrimming_d     = 0;
#endif

   pszInstance_d                = 0;
   pszBlitter_d                 = 0;

   pInstance_d                  = 0;
   pBlitter_d                   = 0;
   pXMLInstance_d               = 0;
   pXMLBlitter_d                = 0;

   if (  pszXMLDeviceName
      && *pszXMLDeviceName
      )
   {
      pszXMLDeviceName_d = (char *)malloc (strlen (pszXMLDeviceName) + 1);
      if (pszXMLDeviceName_d)
      {
         strcpy (pszXMLDeviceName_d, pszXMLDeviceName);
      }
   }

   if (docDevice_d)
   {
      rootDevice_d = XMLDocGetRootElement (docDevice);
   }

   initializeDevice ();

   if (pszMasterXML)
   {
      free ((void *)pszMasterXML);
   }
}

XMLDevice::
~XMLDevice ()
{
   delete pstringMasterXMLPath_d; pstringMasterXMLPath_d = 0;
   if (pszXMLDeviceName_d)
   {
      free (pszXMLDeviceName_d);
      pszXMLDeviceName_d = 0;
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
      XMLFree ((void *)pszShortName_d);
      pszShortName_d  = 0;
   }

   if (docDevice_d)
   {
      XMLFreeDoc (docDevice_d);
      docDevice_d = 0;
   }

#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (pstringDefaultBooklet_d)
   {
      delete pstringDefaultBooklet_d;
      pstringDefaultBooklet_d = 0;
   }
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   if (pstringDefaultCopies_d)
   {
      delete pstringDefaultCopies_d;
      pstringDefaultCopies_d = 0;
   }
#endif
   if (pszDefaultDitherID_d)
   {
      XMLFree ((void *)pszDefaultDitherID_d);
      pszDefaultDitherID_d = 0;
   }
   if (pstringDefaultForm_d)
   {
      delete pstringDefaultForm_d;
      pstringDefaultForm_d = 0;
   }
#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pstringDefaultJogging_d)
   {
      delete pstringDefaultJogging_d;
      pstringDefaultJogging_d = 0;
   }
#endif
   if (pstringDefaultMedia_d)
   {
      delete pstringDefaultMedia_d;
      pstringDefaultMedia_d = 0;
   }
#ifdef INCLUDE_JP_COMMON_NUP
   if (pstringDefaultNUp_d)
   {
      delete pstringDefaultNUp_d;
      pstringDefaultNUp_d = 0;
   }
#endif
   if (pstringDefaultOrientation_d)
   {
      delete pstringDefaultOrientation_d;
      pstringDefaultOrientation_d = 0;
   }
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pstringDefaultOutputBin_d)
   {
      delete pstringDefaultOutputBin_d;
      pstringDefaultOutputBin_d = 0;
   }
#endif
   if (pstringDefaultPrintMode_d)
   {
      delete pstringDefaultPrintMode_d;
      pstringDefaultPrintMode_d = 0;
   }
   if (pstringDefaultResolution_d)
   {
      delete pstringDefaultResolution_d;
      pstringDefaultResolution_d = 0;
   }
#ifdef INCLUDE_JP_COMMON_SCALING
   if (pstringDefaultScaling_d)
   {
      delete pstringDefaultScaling_d;
      pstringDefaultScaling_d = 0;
   }
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pstringDefaultSheetCollate_d)
   {
      delete pstringDefaultSheetCollate_d;
      pstringDefaultSheetCollate_d = 0;
   }
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   if (pstringDefaultSide_d)
   {
      delete pstringDefaultSide_d;
      pstringDefaultSide_d = 0;
   }
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pstringDefaultStitching_d)
   {
      delete pstringDefaultStitching_d;
      pstringDefaultStitching_d = 0;
   }
#endif
   if (pstringDefaultTray_d)
   {
      delete pstringDefaultTray_d;
      pstringDefaultTray_d = 0;
   }
#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pstringDefaultTrimming_d)
   {
      delete pstringDefaultTrimming_d;
      pstringDefaultTrimming_d = 0;
   }
#endif

   if (pszInstance_d)
   {
      XMLFree ((void *)pszInstance_d);
      pszInstance_d = 0;
   }
   if (pszBlitter_d)
   {
      XMLFree ((void *)pszBlitter_d);
      pszBlitter_d = 0;
   }

   for ( HasMapping::iterator nextMap = hasMap_d.begin () ;
         nextMap != hasMap_d.end () ;
         nextMap++ )
   {
      if (nextMap->second)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Freeing \"" << nextMap->first << "\", 0x" << std::hex << (int)nextMap->second << std::dec << std::endl;
#endif

         XMLFreeDoc (nextMap->second);
      }
   }
#ifdef INCLUDE_JP_UPDF_BOOKLET
   docDeviceBooklets_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   docDeviceCopies_d = 0;
#endif
   docDeviceForms_d = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   docDeviceJoggings_d = 0;
#endif
   docDeviceMedias_d = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   docDeviceNUps_d = 0;
#endif
   docDeviceOrientations_d = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   docDeviceOutputBins_d = 0;
#endif
   docDevicePrintModes_d = 0;
   docDeviceResolutions_d = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   docDeviceScalings_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   docDeviceSheetCollates_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   docDeviceSides_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   docDeviceStitchings_d = 0;
#endif
   docDeviceTrays_d = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   docDeviceTrimmings_d = 0;
#endif
   docDeviceGammas_d = 0;
   docDeviceCommands_d = 0;
   docDeviceDatas_d = 0;
   docDeviceStrings_d = 0;

   XMLCleanup ();

   DebugOutput::logMessage (LOG_INFO, "deleted XMLDevice");
}

void XMLDevice::
initializeDevice ()
{
   if (  !docDevice_d
      || !rootDevice_d
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Error: docDevice_d or rootDevice_d is null!" << std::endl;
#endif

      return;
   }

   // Read and set the capabilities
   XmlNodePtr elm           = 0;
   int        iCapabilities = 0;

   elm = XMLFirstNode (rootDevice_d);
   if (elm)
      elm = XMLFirstNode (XMLGetChildrenNode (elm));

   while (elm)
   {
      if (0 == strcmp (XMLGetName (elm), "Capability"))
      {
         PSZRO pszCapability = 0;

         pszCapability = XMLGetProp (elm, "type");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszCapability = " << pszCapability << std::endl;
#endif

         if (pszCapability)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Adding value " << std::hex << Capability::getReservedValue (pszCapability) << std::dec << " to iCapabilities." << std::endl;
#endif

            iCapabilities |= Capability::getReservedValue (pszCapability);

            XMLFree ((void *)pszCapability);
         }
      }

      elm = XMLNextNode (elm);
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iCapabilities = " << std::hex << iCapabilities << std::dec << std::endl;
#endif

   setCapabilities (iCapabilities);

   // Read and set the raster capabilities
   int   iRasterCapabilities = 0;

   elm = XMLFirstNode (rootDevice_d);
   if (elm)
      elm = XMLFirstNode (XMLGetChildrenNode (elm));

   while (elm)
   {
      if (0 == strcmp (XMLGetName (elm), "RasterCapabilities"))
      {
         PSZRO pszRasterCapability = 0;

         pszRasterCapability = XMLGetProp (elm, "type");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszRasterCapability = " << pszRasterCapability << std::endl;
#endif

         if (pszRasterCapability)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Adding value " << std::hex << RasterCapabilities::getReservedValue (pszRasterCapability) << std::dec << " to iRasterCapabilities." << std::endl;
#endif

            iRasterCapabilities |= RasterCapabilities::getReservedValue (pszRasterCapability);

            XMLFree ((void *)pszRasterCapability);
         }
      }

      elm = XMLNextNode (elm);
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iRasterCapabilities = " << std::hex << iRasterCapabilities << std::dec << std::endl;
#endif

   setRasterCapabilities (iRasterCapabilities);

   // Read and set the PDL options
   elm = XMLFindEntry (rootDevice_d, "PDL", false);
   if (elm)
   {
      PSZRO pszLevel              = 0;
      PSZRO pszSubLevel           = 0;
      PSZRO pszMajorRevisionLevel = 0;
      PSZRO pszMinorRevisionLevel = 0;

      pszLevel = XMLGetProp (elm, "level");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszLevel = " << pszLevel << std::endl;
#endif

      pszSubLevel = XMLGetProp (elm, "sublevel");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszSubLevel = " << pszSubLevel << std::endl;
#endif

      pszMajorRevisionLevel = XMLGetProp (elm, "major");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszMajorRevisionLevel = " << pszMajorRevisionLevel << std::endl;
#endif

      pszMinorRevisionLevel = XMLGetProp (elm, "minor");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszMinorRevisionLevel = " << pszMinorRevisionLevel << std::endl;
#endif

      if (  pszLevel
         && pszSubLevel
         && pszMajorRevisionLevel
         && pszMinorRevisionLevel
         )
      {
         int  iLevel              = 0;
         int  iSubLevel           = 0;
         int  iMajorRevisionLevel = 0;
         int  iMinorRevisionLevel = 0;
         PDL *pPDL                = 0;

         if (1 != sscanf (pszLevel, "%d", &iLevel))
         {
            iLevel = PDL::getReservedValue (pszLevel);
         }
         if (1 != sscanf (pszSubLevel, "%d", &iSubLevel))
         {
            iSubLevel = PDL::getReservedValue (pszSubLevel);
         }
         if (1 != sscanf (pszMajorRevisionLevel, "%d", &iMajorRevisionLevel))
         {
            iMajorRevisionLevel = PDL::getReservedValue (pszMajorRevisionLevel);
         }
         if (1 != sscanf (pszMinorRevisionLevel, "%d", &iMinorRevisionLevel))
         {
            iMinorRevisionLevel = PDL::getReservedValue (pszMinorRevisionLevel);
         }

         pPDL = new PDL (iLevel,
                         iSubLevel,
                         iMajorRevisionLevel,
                         iMinorRevisionLevel);

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ())
         {
            if (pPDL)
            {
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": setting PDL " << *pPDL << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": setting NULL PDL! " << std::endl;
            }
         }
#endif

         setPDL (pPDL);
      }

      if (pszLevel)
      {
         XMLFree ((void *)pszLevel);
      }
      if (pszSubLevel)
      {
         XMLFree ((void *)pszSubLevel);
      }
      if (pszMajorRevisionLevel)
      {
         XMLFree ((void *)pszMajorRevisionLevel);
      }
      if (pszMinorRevisionLevel)
      {
         XMLFree ((void *)pszMinorRevisionLevel);
      }
   }

   // Load the instance and blitter
   pszInstance_d = getXMLContentString (rootDevice_d, docDevice_d, "Instance");
   pszBlitter_d  = getXMLContentString (rootDevice_d, docDevice_d, "Blitter");

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ())
   {
      DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszInstance_d = " << (pszInstance_d ? pszInstance_d : "NULL") << std::endl;
      DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszBlitter_d = " << (pszBlitter_d ? pszBlitter_d : "NULL") << std::endl;
   }
#endif

#ifdef PDC_INTERFACE_ENABLED
   if (  !pszInstance_d
      && !pszBlitter_d
      )
   {
      PSZCRO pszExeName = getXMLAttribute (rootDevice_d, "PluggableInstance", "exename");
      PSZCRO pszData    = getXMLAttribute (rootDevice_d, "PluggableInstance", "data");

      if (pszExeName)
      {
         pXMLInstance_d = new XMLDeviceInstance (new PluggableInstance (this,
                                                                        pszExeName,
                                                                        pszData),
                                                 deletePluggableInstance,
                                                 this);

         pBlitter_d     = new PluggableBlitter  (this);

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pXMLInstance_d = " << pXMLInstance_d << std::endl;
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pXMLBlitter_d  = " << pXMLBlitter_d << std::endl;
#endif

         setDeviceInstance (pXMLInstance_d);
         setDeviceBlitter  (pBlitter_d);
      }

      if (pszExeName)
      {
         XMLFree ((void *)pszExeName);
      }
      if (pszData)
      {
         XMLFree ((void *)pszData);
      }
   }
   else
#endif
   {
      if (pszInstance_d)
      {
         pszInstance_d = convertToLibrary ((char *)pszInstance_d);
      }
      if (pszBlitter_d)
      {
         pszBlitter_d = convertToLibrary ((char *)pszBlitter_d);
      }

      if (pszInstance_d)
      {
         GModule *hmodInstanceLibrary = 0;
         bool     fSucccess           = false;

         fSucccess = Omni::openLibrary (pszInstance_d, &hmodInstanceLibrary);

         if (fSucccess)
         {
            pXMLInstance_d = new XMLDeviceInstance (hmodInstanceLibrary, this);

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pXMLInstance_d = " << pXMLInstance_d << std::endl;
#endif

            fSucccess = pXMLInstance_d ? true : false;
         }

         if (!fSucccess)
         {
            if (pXMLInstance_d)
            {
               delete pXMLInstance_d;
               pXMLInstance_d = 0;
            }
            if (hmodInstanceLibrary)
            {
               g_module_close (hmodInstanceLibrary);
               hmodInstanceLibrary = 0;
            }
         }
         else
         {
            setDeviceInstance (pXMLInstance_d);
            pInstance_d = pXMLInstance_d;
         }
      }

      if (pszBlitter_d)
      {
         GModule *hmodBlitterLibrary = 0;
         bool     fSucccess          = false;

         fSucccess = Omni::openLibrary (pszBlitter_d, &hmodBlitterLibrary);

         if (fSucccess)
         {
            pXMLBlitter_d = new XMLDeviceBlitter (hmodBlitterLibrary, this);

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pXMLBlitter_d = " << pXMLBlitter_d << std::endl;
#endif

            fSucccess = pXMLBlitter_d ? true : false;
         }

         if (!fSucccess)
         {
            if (pXMLBlitter_d)
            {
               delete pXMLBlitter_d;
               pXMLBlitter_d = 0;
            }
            if (hmodBlitterLibrary)
            {
               g_module_close (hmodBlitterLibrary);
               hmodBlitterLibrary = 0;
            }
         }
         else
         {
            setDeviceBlitter (pXMLBlitter_d);
            pBlitter_d = pXMLBlitter_d;
         }
      }
   }

   // Read all of the Has/Uses and map and load those XML documents
   elm = XMLFirstNode (rootDevice_d);
   if (elm)
      elm = XMLFirstNode (XMLGetChildrenNode (elm));

   while (elm)
   {
      if (  0 == strcmp (XMLGetName (elm), "Has")
         || 0 == strcmp (XMLGetName (elm), "Uses")
         )
      {
         PSZRO pszTargetXMLFile = 0;

         pszTargetXMLFile = XMLNodeListGetString (docDevice_d,
                                                  XMLGetChildrenNode (elm),
                                                  1);

         if (pszTargetXMLFile)
         {
            PSZRO     pszFullTargetXMLFile = 0;
            XmlDocPtr docTargetXML         = 0;

            if (pstringMasterXMLPath_d)
            {
               std::string stringTargetXMLFile;

               stringTargetXMLFile  = *pstringMasterXMLPath_d;
               stringTargetXMLFile += pszTargetXMLFile;

               pszFullTargetXMLFile = Omni::openXMLFile ((char *)stringTargetXMLFile.c_str ());
            }
            else
            {
               pszFullTargetXMLFile = Omni::openXMLFile (pszTargetXMLFile);
            }

            if (pszFullTargetXMLFile)
            {
               docTargetXML = XMLParseFile (pszFullTargetXMLFile);
            }

            if (docTargetXML)
            {
               XmlNodePtr  elmTargetElm = 0;

               elmTargetElm = XMLDocGetRootElement (docTargetXML);
               elmTargetElm = XMLFirstNode (elmTargetElm);

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Mapping \"" << XMLGetName (elmTargetElm) << "\" to \"" << pszFullTargetXMLFile << "\"" << std::endl;
#endif

               hasMap_d[std::string (XMLGetName (elmTargetElm))] = docTargetXML;
            }

            if (pszFullTargetXMLFile)
            {
               free ((void *)pszFullTargetXMLFile);
            }

            XMLFree ((void *)pszTargetXMLFile);
         }
      }

      elm = XMLNextNode (elm);
   }

///DeviceCommand *pDC     = getDefaultCommands ();
///BinaryData    *pbdData = pDC->getCommandData ("cmdBeginRasterGraphics");
///std::cout << *pbdData << std::endl;

///DeviceData *pDD      = getDefaultData ();
///int         iValue   = 0;
///bool        fSuccess = false;
///fSuccess = pDD->getIntData ("Nozzle_Spacing", &iValue);
///std::cout << "Nozzle_Spacing = " << iValue << std::endl;
///std::cout << "fSuccess = " << fSuccess << std::endl;
///fSuccess = pDD->getIntData ("Nozzle_Number", &iValue);
///std::cout << "Nozzle_Number = " << iValue << std::endl;
///std::cout << "fSuccess = " << fSuccess << std::endl;
///fSuccess = pDD->getIntData ("Positioning_x", &iValue);
///std::cout << "Positioning_x = " << iValue << std::endl;
///std::cout << "fSuccess = " << fSuccess << std::endl;
///fSuccess = pDD->getIntData ("DotSize360", &iValue);
///std::cout << "DotSize360 = " << iValue << std::endl;
///std::cout << "fSuccess = " << fSuccess << std::endl;
///fSuccess = pDD->getIntData ("DotSize720", &iValue);
///std::cout << "DotSize720 = " << iValue << std::endl;
///std::cout << "fSuccess = " << fSuccess << std::endl;
///fSuccess = pDD->getIntData ("DotSize1440", &iValue);
///std::cout << "DotSize1440 = " << iValue << std::endl;
///std::cout << "fSuccess = " << fSuccess << std::endl;

///DeviceString *pDS           = getDefaultString ();
///char         *apszStrings[] = {
///   "color-mode",
///   "draft",
///   "fine",
///   "graph",
///   "high-speed",
///   "micro-weave",
///   "none",
///   "off",
///   "on",
///   "photo",
///   "quality",
///   "standard",
///   "super-fine",
///   "super-photo",
///};
///char         *pszString     = 0;
///std::cout << *pDS << std::endl;
///pDS->setLanguage (StringResource::LANGUAGE_EN);
///for (int i = 0; i < (int)dimof (apszStrings); i++)
///{
///   pszString = pDS->getStringV (StringResource::STRINGGROUP_UNKNOWN,
///                                apszStrings[i]);
///   std::cout << "for \""
///             << apszStrings[i]
///             << "\" returns \""
///             << pszString
///             << "\"."
///             << std::endl;
///}
}

#ifdef INCLUDE_JP_UPDF_BOOKLET
DeviceBooklet * XMLDevice::
getDefaultBooklet ()
{
   // @TBD
   return 0;
}
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
DeviceCopies * XMLDevice::
getDefaultCopies ()
{
   DeviceCopies *pCopiesRet = 0;

   if (!docDeviceCopies_d)
   {
      docDeviceCopies_d = getDeviceXML ("deviceCopies");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceCopies_d = " << docDeviceCopies_d << std::endl;
#endif
   }

   if (!pstringDefaultCopies_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultCopies_d = getXMLJobProperties (elm, docDevice_d, "Copies");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultCopies_d = " << (pstringDefaultCopies_d ? *pstringDefaultCopies_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultCopies_d)
   {
      if (docDeviceCopies_d)
      {
         pCopiesRet = XMLDeviceCopies::createS (this, pstringDefaultCopies_d->c_str ());
      }
   }

   if (!pCopiesRet)
   {
      std::ostringstream oss;

      DefaultCopies::writeDefaultJP (oss);

      pCopiesRet = new DefaultCopies (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pCopiesRet << std::endl;
#endif

   return pCopiesRet;
}
#endif

PSZCRO XMLDevice::
getDefaultDitherID ()
{
   if (!pszDefaultDitherID_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pszDefaultDitherID_d = getXMLContentString (elm, docDevice_d, "dither");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszDefaultDitherID_d = " << pszDefaultDitherID_d << std::endl;
#endif
      }
   }

   return pszDefaultDitherID_d;
}

DeviceForm * XMLDevice::
getDefaultForm ()
{
   DeviceForm *pDFRet = 0;

   if (!docDeviceForms_d)
   {
      docDeviceForms_d = getDeviceXML ("deviceForms");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceForms_d = " << docDeviceForms_d << std::endl;
#endif
   }

   if (  !pstringDefaultForm_d
      && docDeviceForms_d
      )
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultForm_d = getXMLJobProperties (elm, docDevice_d, "Form");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultForm_d = " << (pstringDefaultForm_d ? *pstringDefaultForm_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultForm_d)
   {
      pDFRet = XMLDeviceForm::createS (this, pstringDefaultForm_d->c_str ());

//////// Using member function pointers in C++
//////// http://www.vmlinux.org/jakov/community.borland.com/15837.html
//////Enumeration * (DeviceForm::*PM) (void);
//////PM = &DeviceForm::getEnumeration;
//////std::cout << "pDFRet_d->getEnumeration = " << std::hex << &(pDFRet_d->*PM) << std::dec << std::endl;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDFRet << std::endl;
#endif

   return pDFRet;
}

#ifdef INCLUDE_JP_UPDF_JOGGING
DeviceJogging * XMLDevice::
getDefaultJogging ()
{
   // @TBD
   return 0;
}
#endif

DeviceMedia * XMLDevice::
getDefaultMedia ()
{
   DeviceMedia *pDMRet = 0;

   if (!docDeviceMedias_d)
   {
      docDeviceMedias_d = getDeviceXML ("deviceMedias");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceMedias_d = " << docDeviceMedias_d << std::endl;
#endif
   }

   if (  !pstringDefaultMedia_d
      && docDeviceMedias_d
      )
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultMedia_d = getXMLJobProperties (elm, docDevice_d, "media");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultMedia_d = " << (pstringDefaultMedia_d ? *pstringDefaultMedia_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultMedia_d)
   {
      pDMRet = XMLDeviceMedia::createS (this, pstringDefaultMedia_d->c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDMRet << std::endl;
#endif

   return pDMRet;
}

#ifdef INCLUDE_JP_COMMON_NUP
DeviceNUp * XMLDevice::
getDefaultNUp ()
{
   DeviceNUp *pNUPRet = 0;

   if (!docDeviceNUps_d)
   {
      docDeviceNUps_d = getDeviceXML ("deviceNumberUps");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceNUps_d = " << docDeviceNUps_d << std::endl;
#endif
   }

   if (!pstringDefaultNUp_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultNUp_d = getXMLJobProperties (elm, docDevice_d, "NumberUp");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultNUp_d = " << (pstringDefaultNUp_d ? *pstringDefaultNUp_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultNUp_d)
   {
      if (docDeviceNUps_d)
      {
         pNUPRet = XMLDeviceNUp::createS (this, pstringDefaultNUp_d->c_str ());
      }
   }

   if (!pNUPRet)
   {
      std::ostringstream oss;

      DefaultNUp::writeDefaultJP (oss);

      pNUPRet = new DefaultNUp (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pNUPRet << std::endl;
#endif

   return pNUPRet;
}
#endif

DeviceOrientation * XMLDevice::
getDefaultOrientation ()
{
   DeviceOrientation *pDORet = 0;

   if (!docDeviceOrientations_d)
   {
      docDeviceOrientations_d = getDeviceXML ("deviceOrientations");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceOrientations_d = " << docDeviceOrientations_d << std::endl;
#endif
   }

   if (  !pstringDefaultOrientation_d
      && docDeviceOrientations_d
      )
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultOrientation_d = getXMLJobProperties (elm, docDevice_d, "Rotation");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultOrientation_d = " << (pstringDefaultOrientation_d ? *pstringDefaultOrientation_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultOrientation_d)
   {
      pDORet = XMLDeviceOrientation::createS (this, pstringDefaultOrientation_d->c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDORet << std::endl;
#endif

   return pDORet;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
DeviceOutputBin * XMLDevice::
getDefaultOutputBin ()
{
   DeviceOutputBin *pOutputBinRet = 0;

   if (!docDeviceOutputBins_d)
   {
      docDeviceOutputBins_d = getDeviceXML ("deviceOutputBins");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceOutputBins_d = " << docDeviceOutputBins_d << std::endl;
#endif
   }

   if (!pstringDefaultOutputBin_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultOutputBin_d = getXMLJobProperties (elm, docDevice_d, "OutputBin");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultOutputBin_d = " << (pstringDefaultOutputBin_d ? *pstringDefaultOutputBin_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultOutputBin_d)
   {
      if (docDeviceOutputBins_d)
      {
         pOutputBinRet = XMLDeviceOutputBin::createS (this, pstringDefaultOutputBin_d->c_str ());
      }
   }

   if (!pOutputBinRet)
   {
      std::ostringstream oss;

      DefaultOutputBin::writeDefaultJP (oss);

      pOutputBinRet = new DefaultOutputBin (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pOutputBinRet << std::endl;
#endif

   return pOutputBinRet;
}
#endif

DevicePrintMode * XMLDevice::
getDefaultPrintMode ()
{
   DevicePrintMode *pDPMRet = 0;

   if (!docDevicePrintModes_d)
   {
      docDevicePrintModes_d = getDeviceXML ("devicePrintModes");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDevicePrintModes_d = " << docDevicePrintModes_d << std::endl;
#endif
   }

   if (  !pstringDefaultPrintMode_d
      && docDevicePrintModes_d
      )
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultPrintMode_d = getXMLJobProperties (elm, docDevice_d, "printmode");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultPrintMode_d = " << (pstringDefaultPrintMode_d ? *pstringDefaultPrintMode_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultPrintMode_d)
   {
      pDPMRet = XMLDevicePrintMode::createS (this, pstringDefaultPrintMode_d->c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDPMRet << std::endl;
#endif

   return pDPMRet;
}

DeviceResolution * XMLDevice::
getDefaultResolution ()
{
   DeviceResolution *pDRRet = 0;

   if (!docDeviceResolutions_d)
   {
      docDeviceResolutions_d = getDeviceXML ("deviceResolutions");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceResolutions_d = " << docDeviceResolutions_d << std::endl;
#endif
   }

   if (  !pstringDefaultResolution_d
      && docDeviceResolutions_d
      )
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultResolution_d = getXMLJobProperties (elm, docDevice_d, "Resolution");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultResolution_d = " << (pstringDefaultResolution_d ? *pstringDefaultResolution_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultResolution_d)
   {
      pDRRet = XMLDeviceResolution::createS (this, pstringDefaultResolution_d->c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDRRet << std::endl;
#endif

   return pDRRet;
}

#ifdef INCLUDE_JP_COMMON_SCALING
DeviceScaling * XMLDevice::
getDefaultScaling ()
{
   DeviceScaling *pScalingRet = 0;

   if (!docDeviceScalings_d)
   {
      docDeviceScalings_d = getDeviceXML ("deviceScalings");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceScalings_d = " << docDeviceScalings_d << std::endl;
#endif
   }

   if (!pstringDefaultScaling_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultScaling_d = getXMLJobProperties (elm, docDevice_d, "Scaling");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultScaling_d = " << (pstringDefaultScaling_d ? *pstringDefaultScaling_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultScaling_d)
   {
      if (docDeviceScalings_d)
      {
         pScalingRet = XMLDeviceScaling::createS (this, pstringDefaultScaling_d->c_str ());
      }
   }

   if (!pScalingRet)
   {
      std::ostringstream oss;

      DefaultScaling::writeDefaultJP (oss);

      pScalingRet = new DefaultScaling (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pScalingRet << std::endl;
#endif

   return pScalingRet;
}
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
DeviceSheetCollate * XMLDevice::
getDefaultSheetCollate ()
{
   DeviceSheetCollate *pSheetCollateRet = 0;

   if (!docDeviceSheetCollates_d)
   {
      docDeviceSheetCollates_d = getDeviceXML ("deviceSheetCollates");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceSheetCollates_d = " << docDeviceSheetCollates_d << std::endl;
#endif
   }

   if (!pstringDefaultSheetCollate_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultSheetCollate_d = getXMLJobProperties (elm, docDevice_d, "SheetCollate");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultSheetCollate_d = " << (pstringDefaultSheetCollate_d ? *pstringDefaultSheetCollate_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultSheetCollate_d)
   {
      if (docDeviceSheetCollates_d)
      {
         pSheetCollateRet = XMLDeviceSheetCollate::createS (this, pstringDefaultSheetCollate_d->c_str ());
      }
   }

   if (!pSheetCollateRet)
   {
      std::ostringstream oss;

      DefaultSheetCollate::writeDefaultJP (oss);

      pSheetCollateRet = new DefaultSheetCollate (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pSheetCollateRet << std::endl;
#endif

   return pSheetCollateRet;
}
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
DeviceSide * XMLDevice::
getDefaultSide ()
{
   DeviceSide *pSideRet = 0;

   if (!docDeviceSides_d)
   {
      docDeviceSides_d = getDeviceXML ("deviceSides");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceSides_d = " << docDeviceSides_d << std::endl;
#endif
   }

   if (!pstringDefaultSide_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultSide_d = getXMLJobProperties (elm, docDevice_d, "Sides");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultSide_d = " << (pstringDefaultSide_d ? *pstringDefaultSide_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultSide_d)
   {
      if (docDeviceSides_d)
      {
         pSideRet = XMLDeviceSide::createS (this, pstringDefaultSide_d->c_str ());
      }
   }

   if (!pSideRet)
   {
      std::ostringstream oss;

      DefaultSide::writeDefaultJP (oss);

      pSideRet = new DefaultSide (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pSideRet << std::endl;
#endif

   return pSideRet;
}
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
DeviceStitching * XMLDevice::
getDefaultStitching ()
{
   DeviceStitching *pStitchingRet = 0;

   if (!docDeviceStitchings_d)
   {
      docDeviceStitchings_d = getDeviceXML ("deviceStitchings");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceStitchings_d = " << docDeviceStitchings_d << std::endl;
#endif
   }

   if (!pstringDefaultStitching_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultStitching_d = getXMLJobProperties (elm, docDevice_d, "Stitching");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultStitching_d = " << (pstringDefaultStitching_d ? *pstringDefaultStitching_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultStitching_d)
   {
      if (docDeviceStitchings_d)
      {
         pStitchingRet = XMLDeviceStitching::createS (this, pstringDefaultStitching_d->c_str ());
      }
   }

   if (!pStitchingRet)
   {
      std::ostringstream oss;

      DefaultStitching::writeDefaultJP (oss);

      pStitchingRet = new DefaultStitching (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pStitchingRet << std::endl;
#endif

   return pStitchingRet;
}
#endif

DeviceTray * XMLDevice::
getDefaultTray ()
{
   DeviceTray *pDTRet = 0;

   if (!docDeviceTrays_d)
   {
      docDeviceTrays_d = getDeviceXML ("deviceTrays");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceTrays_d = " << docDeviceTrays_d << std::endl;
#endif
   }

   if (  !pstringDefaultTray_d
      && docDeviceTrays_d
      )
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultTray_d = getXMLJobProperties (elm, docDevice_d, "InputTray");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultTray_d = " << (pstringDefaultTray_d ? *pstringDefaultTray_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultTray_d)
   {
      pDTRet = XMLDeviceTray::createS (this, pstringDefaultTray_d->c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDTRet << std::endl;
#endif

   return pDTRet;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING
DeviceTrimming * XMLDevice::
getDefaultTrimming ()
{
   DeviceTrimming *pTrimmingRet = 0;

   if (!docDeviceTrimmings_d)
   {
      docDeviceTrimmings_d = getDeviceXML ("deviceTrimmings");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceTrimmings_d = " << docDeviceTrimmings_d << std::endl;
#endif
   }

   if (!pstringDefaultTrimming_d)
   {
      XmlNodePtr elm = 0;

      elm = XMLFindEntry (rootDevice_d, "DefaultJobProperties", false);

      if (elm)
      {
         pstringDefaultTrimming_d = getXMLJobProperties (elm, docDevice_d, "Trimming");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringDefaultTrimming_d = " << (pstringDefaultTrimming_d ? *pstringDefaultTrimming_d : "") << std::endl;
#endif
      }
   }

   if (pstringDefaultTrimming_d)
   {
      if (docDeviceTrimmings_d)
      {
         pTrimmingRet = XMLDeviceTrimming::createS (this, pstringDefaultTrimming_d->c_str ());
      }
   }

   if (!pTrimmingRet)
   {
      std::ostringstream oss;

      DefaultTrimming::writeDefaultJP (oss);

      pTrimmingRet = new DefaultTrimming (this, oss.str ().c_str ());
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pTrimmingRet << std::endl;
#endif

   return pTrimmingRet;
}
#endif

DeviceCommand * XMLDevice::
getDefaultCommands ()
{
   DeviceCommand *pDCRet = 0;

   if (!docDeviceCommands_d)
   {
      docDeviceCommands_d = getDeviceXML ("deviceCommands");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceCommands_d = " << docDeviceCommands_d << std::endl;
#endif
   }

   if (docDeviceCommands_d)
   {
      XmlNodePtr  rootDeviceCommands = XMLDocGetRootElement (docDeviceCommands_d);
      XmlNodePtr  elmDeviceCommand   = 0;

      elmDeviceCommand = XMLFirstNode (rootDeviceCommands);
      if (elmDeviceCommand)
         elmDeviceCommand = XMLFirstNode (XMLGetChildrenNode (elmDeviceCommand));

      pDCRet = new DeviceCommand ();

      while (elmDeviceCommand)
      {
         PSZRO       pszName    = 0;
         PSZRO       pszCommand = 0;
         BinaryData *pbdData    = 0;

         try
         {
            // Read in the name
            pszName = XMLGetProp (elmDeviceCommand, "name");

            // Read in the command
            pszCommand = getXMLContentString (elmDeviceCommand, docDeviceCommands_d, 0);

            if (pszCommand)
            {
               byte *pbData = 0;
               int   cbData = 0;

               if (XMLDevice::parseBinaryData (pszCommand,
                                               &pbData,
                                               &cbData))
               {
                  pbdData = new BinaryDataDelete (pbData, cbData);
               }
            }

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ())
            {
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszName = " << pszName << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszCommand = " << pszCommand << std::endl;
            }
#endif

            if (  pszName
               && pbdData
               )
            {
               pDCRet->add (pszName, pbdData);
            }
         }
         catch (std::string *pstringError)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

            delete pstringError;
         }

         if (pszName)
         {
            XMLFree ((void *)pszName);
         }
         if (pszCommand)
         {
            XMLFree ((void *)pszCommand);
         }

         elmDeviceCommand = XMLNextNode (elmDeviceCommand);
      }
   }

   return pDCRet;
}

DeviceData * XMLDevice::
getDefaultData ()
{
   DeviceData *pDDRet = 0;

   if (!docDeviceDatas_d)
   {
      docDeviceDatas_d = getDeviceXML ("deviceDatas");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceDatas_d = " << docDeviceDatas_d << std::endl;
#endif
   }

   if (docDeviceDatas_d)
   {
      XmlNodePtr rootDeviceDatas = XMLDocGetRootElement (docDeviceDatas_d);
      XmlNodePtr elmDeviceData   = 0;

      elmDeviceData = XMLFirstNode (rootDeviceDatas);
      if (elmDeviceData)
         elmDeviceData = XMLFirstNode (XMLGetChildrenNode (elmDeviceData));

      pDDRet = new DeviceData ();

      while (elmDeviceData)
      {
         PSZRO       pszName = 0;
         PSZRO       pszType = 0;
         PSZRO       pszData = 0;
         BinaryData *pbdData = 0;
         byte       *pbData  = 0;
         int         cbData  = 0;

         try
         {
            // Read in the name
            pszName = XMLGetProp (elmDeviceData, "name");

            // Read in the type
            pszType = XMLGetProp (elmDeviceData, "type");

            // Read in the command
            pszData = getXMLContentString (elmDeviceData, docDeviceDatas_d, 0);

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ())
            {
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszName = " << pszName << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszType = " << pszType << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszData = " << pszData << std::endl;
            }
#endif

            if (pszData)
            {
               if (0 == strcmp (pszType, "string"))
               {
                  cbData = strlen (pszData) + 1;
                  pbData = new byte [cbData];

                  if (pbData)
                  {
                     strcpy ((char *)pbData, pszData);
                     pbdData = new BinaryDataDelete (pbData, cbData);
                     if (!pbdData)
                     {
                        delete[] pbData;
                        pbData = 0;
                        // @TBD
                     }
                  }
                  else
                  {
                     // @TBD
                  }
               }
               else if (0 == strcmp (pszType, "boolean"))
               {
                  cbData = 4;
                  pbData = new byte [cbData];

                  if (pbData)
                  {
                     memset (pbData, 0, cbData);

                     if (0 == strcasecmp (pszData, "true"))
                     {
                        pbData[0] = 1;
                     }
                     else if (0 == strcasecmp (pszData, "false"))
                     {
                        pbData[0] = 0;
                     }
                     else
                     {
                        delete[] pbData;
                        pbData = 0;
                        // @TBD
                     }

                     if (pbData)
                     {
                        pbdData = new BinaryDataDelete (pbData, cbData);
                        if (!pbdData)
                        {
                           delete[] pbData;
                           pbData = 0;
                           // @TBD
                        }
                     }
                  }
                  else
                  {
                     // @TBD
                  }
               }
               else if (0 == strcmp (pszType, "integer"))
               {
                  cbData = 4;
                  pbData = new byte [cbData];

                  if (pbData)
                  {
                     if (0 == sscanf (pszData, "%d", (int *)pbData))
                     {
                        delete[] pbData;
                        pbData = 0;
                        // @TBD
                     }

                     if (pbData)
                     {
                        pbdData = new BinaryDataDelete (pbData, cbData);
                        if (!pbdData)
                        {
                           delete[] pbData;
                           pbData = 0;
                           // @TBD
                        }
                     }
                  }
                  else
                  {
                     // @TBD
                  }
               }
               else if (0 == strcmp (pszType, "byte"))
               {
                  cbData = 1;
                  pbData = new byte [cbData];

                  if (pbData)
                  {
                     if (0 == sscanf (pszData, "%c", (char *)pbData))
                     {
                        delete[] pbData;
                        pbData = 0;
                        // @TBD
                     }

                     if (pbData)
                     {
                        pbdData = new BinaryDataDelete (pbData, cbData);
                        if (!pbdData)
                        {
                           delete[] pbData;
                           pbData = 0;
                           // @TBD
                        }
                     }
                  }
                  else
                  {
                     // @TBD
                  }
               }
               else if (0 == strcmp (pszType, "binary"))
               {
                  if (XMLDevice::parseBinaryData (pszData,
                                                  &pbData,
                                                  &cbData))
                  {
                     pbdData = new BinaryDataDelete (pbData, cbData);
                  }
               }
               else if (0 == strcmp (pszType, "bytearray"))
               {
                  int iCnt,
                      iTempCnt = 0;

                  for (iCnt = 0; *(pszData + iCnt); iCnt++)
                  {
                     if (!isxdigit (*(pszData + iCnt)))
                     {
                        if (isspace (*(pszData + iCnt)) == 0)
                        {
                           std::cerr << "Error: Data in bytearray is not understood \""
                                     << *(pszData + iCnt)
                                     << "\" for "
                                     << pszData
                                     << "."
                                     << std::endl;
                           // @TBD
                        }
                     }
                     else
                     {
                        iTempCnt++;
                     }
                  }

                  cbData = iTempCnt / 2;
                  pbData = new byte [cbData];
                  if (!pbData)
                  {
                     // @TBD
                  }

                  int          iEnd = strlen (pszData);
                  int          iLoc = 0;
                  bool         bOK;
                  char         cTemp[2];
                  unsigned int iValue;

                  iTempCnt = 0;

                  for (iCnt = 0; iCnt < iEnd; iCnt++)
                  {
                     if (isxdigit (*(pszData + iCnt)))
                     {
                        cTemp[iLoc++] = *(pszData + iCnt);
                        if (iLoc == 2)
                        {
                           bOK = parseHexGroup ((PSZCRO)cTemp, &iValue);
                           if (!bOK)
                           {
                              std::cerr << "Error: Data in bytearray is not understood \""
                                        << *(pszData + iCnt)
                                        << "\" for "
                                        << pszData
                                        << "."
                                        << std::endl;

                              delete[] pbData;
                              pbData = 0;
                              // @TBD
                           }

                           *(pbData + iTempCnt++) = (unsigned char)iValue;
                           iLoc = 0;
                        }
                     }
                  }

                  if (iLoc)
                  {
                     std::cerr << "Error: Data in bytearray is missing bytes\""
                               << *(pszData+iCnt)
                               << "\" for "
                               << pszData
                               << "."
                               << std::endl;

                     delete[] pbData;
                     pbData = 0;
                     // @TBD
                  }

                  if (pbData)
                  {
                     pbdData = new BinaryDataDelete (pbData, cbData);
                     if (!pbdData)
                     {
                        delete[] pbData;
                        pbData = 0;
                        // @TBD
                     }
                  }
               }
               else
               {
                  // @TBD
               }

               if (pbdData)
               {
#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Adding data (\"" << pszName << "\", " << *pbdData << ")" << std::endl;
#endif

                  pDDRet->add (pszName, pbdData);
               }
            }

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ())
            {
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszName = " << pszName << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszData = " << pszData << std::endl;
            }
#endif
         }
         catch (std::string *pstringError)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

            delete pstringError;
         }

         if (pszName)
         {
            XMLFree ((void *)pszName);
         }
         if (pszType)
         {
            XMLFree ((void *)pszType);
         }
         if (pszData)
         {
            XMLFree ((void *)pszData);
         }

         elmDeviceData = XMLNextNode (elmDeviceData);
      }
   }

   return pDDRet;
}

DeviceString * XMLDevice::
getDefaultString ()
{
   DeviceString *pDSRet = 0;

   if (!docDeviceStrings_d)
   {
      docDeviceStrings_d = getDeviceXML ("deviceStrings");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceStrings_d = " << docDeviceStrings_d << std::endl;
#endif
   }

   if (docDeviceStrings_d)
   {
      XmlNodePtr  rootDeviceStrings  = XMLDocGetRootElement (docDeviceStrings_d);
      XmlNodePtr  elmDeviceString    = 0;
      XmlNodePtr  elmDeviceLanguages = 0;
      XmlNodePtr  elmDeviceLanguage  = 0;

      elmDeviceString = XMLFirstNode (rootDeviceStrings);
      if (elmDeviceString)
         elmDeviceString = XMLFirstNode (XMLGetChildrenNode (elmDeviceString));

      pDSRet = new DeviceString ();

      while (elmDeviceString)
      {
         PSZRO pszLanguage = 0;
         PSZRO pszInternal = 0;
         PSZRO pszExternal = 0;

         // Read in the internal name
         elmDeviceLanguages = XMLFindEntry (elmDeviceString, "name", false);
         if (elmDeviceLanguages)
         {
            pszInternal = getXMLContentString (elmDeviceLanguages,
                                               docDeviceStrings_d,
                                               0);
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszInternal = " << pszInternal << std::endl;
#endif

         if (pszInternal)
         {
            elmDeviceLanguage  = 0;
            elmDeviceLanguages = XMLFindEntry (elmDeviceString,
                                            "languages",
                                            false);
            if (elmDeviceLanguages)
               elmDeviceLanguage = XMLGetChildrenNode (elmDeviceLanguages);
            if (elmDeviceLanguage)
               elmDeviceLanguage = XMLFirstNode (elmDeviceLanguage);

            while (elmDeviceLanguage)
            {
               if (elmDeviceLanguage)
               {
                  // Read in the external name
                  pszExternal = getXMLContentString (elmDeviceLanguage,
                                                     docDeviceStrings_d,
                                                     0);

#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszExternal = " << pszExternal << std::endl;
#endif

                  pszLanguage = XMLGetName (elmDeviceLanguage);

#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszLanguage = " << pszLanguage << std::endl;
#endif

                  if (  pszLanguage
                     && pszInternal
                     && pszExternal
                     )
                  {
                     pDSRet->add (pszLanguage,
                                  pszInternal,
                                  pszExternal);
                  }

                  if (pszExternal)
                  {
                     XMLFree ((void *)pszExternal);
                  }
               }

               elmDeviceLanguage = XMLNextNode (elmDeviceLanguage);
            }
         }

         if (pszInternal)
         {
            XMLFree ((void *)pszInternal);
         }

         elmDeviceString = XMLNextNode (elmDeviceString);
      }
   }

   return pDSRet;
}

DeviceGamma * XMLDevice::
getCurrentGamma ()
{
   if (!docDeviceGammas_d)
   {
      docDeviceGammas_d = getDeviceXML ("deviceGammaTables");

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": docDeviceGammas_d = " << docDeviceGammas_d << std::endl;
#endif
   }

   DeviceResolution *pDR         = 0;
   DeviceMedia      *pDM         = 0;
   DevicePrintMode  *pDPM        = 0;
   PSZRO             pszDitherID = 0;
   DeviceGamma      *pDGRet      = 0;

                                          // NOTE: for testing at init time
   pDR         = getCurrentResolution (); // getDefaultResolution ();
   pDM         = getCurrentMedia ();      // getDefaultMedia ();
   pDPM        = getCurrentPrintMode ();  // getDefaultPrintMode ();
   pszDitherID = getCurrentDitherID ();   // getDefaultDitherID ();

   if (  docDeviceGammas_d
      && pDR
      && pDM
      && pDPM
      && pszDitherID
      )
   {
      PSZRO      pszDitherCatagoryID = 0;
      XmlNodePtr rootDeviceGammas    = XMLDocGetRootElement (docDeviceGammas_d);
      XmlNodePtr elmDeviceGamma      = 0;

      pszDitherCatagoryID = DeviceDither::getDitherCatagory (pszDitherID);

      elmDeviceGamma = XMLFirstNode (rootDeviceGammas);
      if (elmDeviceGamma)
         elmDeviceGamma = XMLFirstNode (XMLGetChildrenNode (elmDeviceGamma));

      while (  elmDeviceGamma
            && !pDGRet
            )
      {
         std::string *pstringResolution     = 0;
         std::string *pstringMedia          = 0;
         std::string *pstringPrintMode      = 0;
         PSZRO        pszDitherCatagory     = 0;

         try
         {
            // Read in the resolution
            pstringResolution = getXMLJobProperties (elmDeviceGamma, docDeviceGammas_d, "Resolution");
            // Read in the media
            pstringMedia = getXMLJobProperties (elmDeviceGamma, docDeviceGammas_d, "media");
            // Read in the print mode
            pstringPrintMode = getXMLJobProperties (elmDeviceGamma, docDeviceGammas_d, "printmode");
            // Read in the dither catagory
            pszDitherCatagory = getXMLContentString (elmDeviceGamma, docDeviceGammas_d, "gammaTableDitherCatagory");

#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ())
            {
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringResolution = " << (pstringResolution ? *pstringResolution : "") << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringMedia      = " << (pstringMedia ? *pstringMedia : "") << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pstringPrintMode  = " << (pstringPrintMode ? *pstringPrintMode : "") << std::endl;
               DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszDitherCatagory = " << pszDitherCatagory << std::endl;
            }
#endif

            if (  pstringResolution
               && pDR->isEqual (pstringResolution->c_str ())
               && pstringMedia
               && pDM->isEqual (pstringMedia->c_str ())
               && pstringPrintMode
               && pDPM->isEqual (pstringPrintMode->c_str ())
               && 0 == strcmp (pszDitherCatagory, pszDitherCatagoryID)
               )
            {
               try
               {
                  int iCGamma = 0;
                  int iMGamma = 0;
                  int iYGamma = 0;
                  int iKGamma = 0;
                  int iCBias  = 0;
                  int iMBias  = 0;
                  int iYBias  = 0;
                  int iKBias  = 0;

                  iCGamma = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableCGamma");
                  iMGamma = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableMGamma");
                  iYGamma = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableYGamma");
                  iKGamma = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableKGamma");
                  iCBias  = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableCBias");
                  iMBias  = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableMBias");
                  iYBias  = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableYBias");
                  iKBias  = getXMLContentInt (elmDeviceGamma, docDeviceGammas_d, "gammaTableKBias");

#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDevice ())
                  {
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iCGamma = " << iCGamma << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iMGamma = " << iMGamma << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iYGamma = " << iYGamma << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iKGamma = " << iKGamma << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iCBias  = " << iCBias  << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iMBias  = " << iMBias  << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iYBias  = " << iYBias  << std::endl;
                     DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": iKBias  = " << iKBias  << std::endl;
                  }
#endif

                  pDGRet = new DeviceGamma (iCGamma,
                                            iMGamma,
                                            iYGamma,
                                            iKGamma,
                                            iCBias,
                                            iMBias,
                                            iYBias,
                                            iKBias);
               }
               catch (std::string *pstringError)
               {
#ifndef RETAIL
                  if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

                  delete pstringError;
               }
            }
         }
         catch (std::string *pstringError)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

            delete pstringError;
         }

         delete pstringResolution;
         delete pstringMedia;
         delete pstringPrintMode;
         if (pszDitherCatagory)
         {
            XMLFree ((void *)pszDitherCatagory);
         }

         elmDeviceGamma = XMLNextNode (elmDeviceGamma);
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ())
   {
      if (!pDGRet)
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returning NULL gamma!" << std::endl;
      else
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returns " << pDGRet << std::endl;
   }
#endif

   return pDGRet;
}

bool XMLDevice::
hasDeviceOption (PSZCRO pszHasDeviceOption)
{
   XmlNodePtr elm = 0;

   elm = XMLFirstNode (rootDevice_d);
   if (elm)
      elm = XMLFirstNode (XMLGetChildrenNode (elm));

   while (elm)
   {
      if (0 == strcmp (XMLGetName (elm), "DeviceOptions"))
      {
         PSZRO pszDeviceOption = 0;

         pszDeviceOption = XMLGetProp (elm, "type");

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": pszDeviceOption = " << pszDeviceOption << std::endl;
#endif

         if (0 == strcmp (pszHasDeviceOption, pszDeviceOption))
         {
            XMLFree ((void *)pszDeviceOption);

            return true;
         }

         if (pszDeviceOption)
         {
            XMLFree ((void *)pszDeviceOption);
         }
      }

      elm = XMLNextNode (elm);
   }

   return false;
}

XMLDevice * XMLDevice::
isAXMLDevice (Device *pDevice)
{
   return dynamic_cast<XMLDevice *>(pDevice);
}

XmlNodePtr XMLDevice::
findEntryKeyValue (XmlNodePtr root,
                   PSZCRO     pszKey,
                   PSZCRO     pszValue,
                   bool       fShouldDebugOutput)
{
   if (root == 0)
      return 0;

   XmlNodePtr elm      = root;
   XmlNodePtr elmFound = 0;

   while (elm != 0)
   {
      PSZRO pszElmValue = XMLGetProp (elm, pszKey);

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
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Found ";
      }
      else
      {
         DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Did not find ";
      }
      DebugOutput::getErrorStream () << pszValue << std::endl;
   }
#endif

   return elm;
}

XmlDocPtr XMLDevice::
getDeviceXML (PSZCRO pszDeviceType)
{
   XmlDocPtr docReturn = 0;

   docReturn = hasMap_d[std::string (pszDeviceType)];

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": Returning 0x" << std::hex << (int)docReturn << std::dec << " from \"" << pszDeviceType << "\"" << std::endl;
#endif

   return docReturn;
}

#ifdef INCLUDE_JP_UPDF_BOOKLET
XmlDocPtr XMLDevice::
getDocBooklets ()
{
   return docDeviceBooklets_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
XmlDocPtr XMLDevice::
getDocCopies ()
{
   return docDeviceCopies_d;
}
#endif

XmlDocPtr XMLDevice::
getDocForms ()
{
   return docDeviceForms_d;
}

#ifdef INCLUDE_JP_UPDF_JOGGING
XmlDocPtr XMLDevice::
getDocJoggings ()
{
   return docDeviceJoggings_d;
}
#endif

XmlDocPtr XMLDevice::
getDocMedias ()
{
   return docDeviceMedias_d;
}

#ifdef INCLUDE_JP_COMMON_NUP
XmlDocPtr XMLDevice::
getDocNUps ()
{
   return docDeviceNUps_d;
}
#endif

XmlDocPtr XMLDevice::
getDocOrientations ()
{
   return docDeviceOrientations_d;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
XmlDocPtr XMLDevice::
getDocOutputBins ()
{
   return docDeviceOutputBins_d;
}
#endif

XmlDocPtr XMLDevice::
getDocPrintModes ()
{
   return docDevicePrintModes_d;
}

XmlDocPtr XMLDevice::
getDocResolutions ()
{
   return docDeviceResolutions_d;
}

#ifdef INCLUDE_JP_COMMON_SCALING
XmlDocPtr XMLDevice::
getDocScalings ()
{
   return docDeviceScalings_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
XmlDocPtr XMLDevice::
getDocSheetCollates ()
{
   return docDeviceSheetCollates_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
XmlDocPtr XMLDevice::
getDocSides ()
{
   return docDeviceSides_d;
}
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
XmlDocPtr XMLDevice::
getDocStitchings ()
{
   return docDeviceStitchings_d;
}
#endif

XmlDocPtr XMLDevice::
getDocTrays ()
{
   return docDeviceTrays_d;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING
XmlDocPtr XMLDevice::
getDocTrimmings ()
{
   return docDeviceTrimmings_d;
}
#endif

XmlDocPtr XMLDevice::
getDocGammas ()
{
   return docDeviceGammas_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

PSZCRO XMLDevice::
getXMLDeviceName ()
{
   return pszXMLDeviceName_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceInstance * XMLDevice::
getDeviceInstance ()
{
   DeviceInstance *pRet = 0;

   // Ask the instance for the child instance
   if (pXMLInstance_d)
      pRet = pXMLInstance_d->getDeviceInstance ();
   else
      pRet = pInstance_d;

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returning " << pRet << std::endl;
#endif

   return pRet;
}

DeviceBlitter * XMLDevice::
getDeviceBlitter ()
{
   DeviceBlitter *pRet = 0;

   // Ask the blitter for the child blitter
   if (pXMLBlitter_d)
      pRet = pXMLBlitter_d->getDeviceBlitter ();
   else
      pRet = pBlitter_d;

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDevice ()) DebugOutput::getErrorStream () << "XMLDevice::" << __FUNCTION__ << ": returning " << pRet << std::endl;
#endif

   return pRet;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

static bool
parseHexGroup (PSZCRO        pszData,
               unsigned int *puiData)
{
   if (  isxdigit (pszData[0])
      && isxdigit (pszData[1])
      )
   {
      return 1 == sscanf (pszData, "%x", puiData);
   }
   else
   {
      return false;
   }
}

bool XMLDevice::
parseBinaryData (PSZRO    pszData,
                 byte   **ppbData,
                 int     *pcbData)
{

   int          cbDataAlloc    = 64;
   int          cbDataLeft     = 64;
   byte        *pbdData        = new byte [cbDataAlloc];
   byte        *pbdDataCurrent = pbdData;
   bool         fContinue      = true;
   bool         fInString      = false;
   byte         bChar          = 0;
   byte         bNextChar      = 0;
   bool         fCharFound     = false;
   bool         fNextCharFound = false;
   std::string  stringError;

   *ppbData = pbdData;
   *pcbData = cbDataAlloc - cbDataLeft;

   while (  *pszData
         && fContinue
         )
   {
      if (fNextCharFound)
      {
         bChar          = bNextChar;
         fNextCharFound = false;
         fCharFound     = true;
      }
      else
      {
         if (fInString)
         {
            switch (*pszData)
            {
            case '\\':
            {
               break;
            }

            case '"':
            {
               pszData++;

               fInString = false;
               continue;
            }

            default:
            {
               bChar      = *pszData++;
               fCharFound = true;
               break;
            }
            }
         }
         else
         {
            while (  '\x0a' == *pszData
                  || '\x0d' == *pszData
                  || ' '    == *pszData
                  )
               pszData++;

            switch (*pszData)
            {
            case '_':
            {
               pszData++;

               if (0 == strncmp (pszData, "ESC_", 4))
               {
                  bChar      = 0x1b;
                  fCharFound = true;
                  pszData += 4;
               }
               else if (0 == strncmp (pszData, "NUL_", 4))
               {
                  bChar      = 0;
                  fCharFound = true;
                  pszData += 4;
               }
               else if (0 == strncmp (pszData, "LF_", 3))
               {
                  bChar      = 0x0a;
                  fCharFound = true;
                  pszData += 3;
               }
               else if (0 == strncmp (pszData, "CR_", 3))
               {
                  bChar      = 0x0d;
                  fCharFound = true;
                  pszData += 3;
               }
               else if (0 == strncmp (pszData, "FF_", 3))
               {
                  bChar      = 0x0c;
                  fCharFound = true;
                  pszData += 3;
               }
               else
               {
                  stringError  = "Error: Unknown define ";
                  stringError += pszData;
                  fContinue    = false;
               }
               break;
            }

            case 'H':
            {
               pszData++;

               if (0 == strncmp (pszData, "EX2S", 4))
               {
                  pszData += 4;

                  while (' ' == *pszData)
                     pszData++;

                  if ('(' == *pszData)
                  {
                     pszData++;
                     while (' ' == *pszData)
                        pszData++;

                     unsigned int uiData  = 0;
                     unsigned int uiData2 = 0;

                     if (parseHexGroup (pszData, &uiData))
                     {
                        pszData += 2;

                        while (' ' == *pszData)
                           pszData++;

                        if (',' == *pszData)
                        {
                           pszData++;

                           while (' ' == *pszData)
                              pszData++;

                           if (parseHexGroup (pszData, &uiData2))
                           {
                              pszData += 2;

                              while (' ' == *pszData)
                                 pszData++;

                              if (')' == *pszData)
                              {
                                 pszData++;

                                 bChar          = (byte)uiData2;
                                 bNextChar      = (byte)uiData;
                                 fCharFound     = true;
                                 fNextCharFound = true;
                              }
                              else
                              {
                                 stringError  = "Error: Expecting ')', found '";
                                 stringError += *pszData;
                                 stringError += "' @ ";
                                 stringError += __LINE__;
                                 fContinue    = false;
                              }
                           }
                           else
                           {
                              stringError  = "Error: Expecting ',', found '";
                              stringError += *pszData;
                              stringError += "' @ ";
                              stringError += __LINE__;
                              fContinue    = false;
                           }
                        }
                        else
                        {
                           stringError  = "Error: Expecting ',', found '";
                           stringError += *pszData;
                           stringError += "' @ ";
                           stringError += __LINE__;
                           fContinue    = false;
                        }
                     }
                     else
                     {
                        stringError  = "Error: Expecting 2 std::hex digits, found '";
                        stringError += pszData[0];
                        stringError += pszData[1];
                        stringError += "' @ ";
                        stringError += __LINE__;
                        fContinue    = false;
                     }
                  }
                  else
                  {
                     stringError  = "Error: Expecting '(', found '";
                     stringError += *pszData;
                     stringError += "' @ ";
                     stringError += __LINE__;
                     fContinue    = false;
                  }
               }
               else if (0 == strncmp (pszData, "EX", 2))
               {
                  pszData += 2;

                  while (' ' == *pszData)
                     pszData++;

                  if ('(' == *pszData)
                  {
                     pszData++;
                     while (' ' == *pszData)
                        pszData++;

                     unsigned int uiData = 0;

                     if (parseHexGroup (pszData, &uiData))
                     {
                        pszData += 2;

                        while (' ' == *pszData)
                           pszData++;

                        if (')' == *pszData)
                        {
                           pszData++;

                           bChar      = (byte)uiData;
                           fCharFound = true;
                        }
                        else
                        {
                           stringError  = "Error: Expecting ')', found '";
                           stringError += *pszData;
                           stringError += "' @ ";
                           stringError += __LINE__;
                           fContinue    = false;
                        }
                     }
                     else
                     {
                        stringError  = "Error: Expecting 2 std::hex digits, found '";
                        stringError += pszData[0];
                        stringError += pszData[1];
                        stringError += "' @ ";
                        stringError += __LINE__;
                        fContinue    = false;
                     }
                  }
                  else
                  {
                     stringError  = "Error: Expecting '(', found '";
                     stringError += *pszData;
                     stringError += "' @ ";
                     stringError += __LINE__;
                     fContinue    = false;
                  }
               }
               break;
            }

            case 'A':
            {
               pszData++;

               if (0 == strncmp (pszData, "SCII", 4))
               {
                  pszData += 4;

                  while (' ' == *pszData)
                     pszData++;

                  if ('(' == *pszData)
                  {
                     pszData++;
                     while (' ' == *pszData)
                        pszData++;

                     bChar      = (byte)*pszData++;
                     fCharFound = true;

                     if (')' == *pszData)
                     {
                        pszData++;
                     }
                     else
                     {
                        stringError  = "Error: Expecting ')', found '";
                        stringError += *pszData;
                        stringError += "' @ ";
                        stringError += __LINE__;
                        fContinue    = false;
                     }
                  }
                  else
                  {
                     stringError  = "Error: Expecting '(', found '";
                     stringError += *pszData;
                     stringError += "' @ ";
                     stringError += __LINE__;
                     fContinue    = false;
                  }
               }
               break;
            }

            case '"':
            {
               pszData++;
               fInString = true;
               break;
            }
            }
         }
      }

      if (fCharFound)
      {
/////////if (isprint (bChar))
/////////   std::cerr << "bChar = '" << bChar << "'" << std::endl;
/////////else
/////////   std::cerr << "bChar = 0x" << std::hex << (int)bChar << std::endl;

         *pbdDataCurrent++ = bChar;
         cbDataLeft--;

         bChar      = 0;
         fCharFound = false;

         if (0 == cbDataLeft)
         {
            byte *pbTemp = new byte [cbDataAlloc + 64];

            if (pbTemp)
            {
               memcpy (pbTemp, pbdData, cbDataAlloc);

               delete[] pbdData;

               pbdData        = pbTemp;
               pbdDataCurrent = pbdData + cbDataAlloc;
               cbDataAlloc   += 64;
               cbDataLeft     = 64;
            }
            else
            {
               stringError  = "Error: Allocation of ";
               stringError += (cbDataAlloc + 64);
               stringError += " bytes failed @ ";
               stringError += __LINE__;
               fContinue    = false;
            }
         }
      }
      else if (  !fInString
              && *pszData
              )
      {
         delete[] pbdData;

         return false;
      }
   }

   if (fContinue)
   {
      *ppbData = pbdData;
      *pcbData = cbDataAlloc - cbDataLeft;
   }
   else
   {
      delete[] pbdData;

      throw new std::string (stringError);
   }

   return fContinue;
}

std::string * XMLDevice::
getXMLJobProperties (XmlNodePtr  root,
                     XmlDocPtr   doc,
                     char       *pszXMLNodeName)
{
   if (  !root
      && !doc
      )
   {
      return 0;
   }

   if (pszXMLNodeName)
   {
      root = XMLFindEntry (root, pszXMLNodeName, false);
      if (!root)
      {
         return 0;
      }
   }

   if (XMLFirstNode (XMLGetChildrenNode (root)))
   {
      std::ostringstream oss;
      bool               fFirst = true;

      root = XMLFirstNode (XMLGetChildrenNode (root));

      while (root != 0)
      {
         if (fFirst)
         {
            fFirst = false;
         }
         else
         {
            oss << ' ';
         }

         PSZCRO pszFormat = XMLGetProp (root, "FORMAT");

         if (pszFormat)
         {
            if (0 == strcmp (pszFormat, "XbyY"))
            {
               XmlNodePtr params      = XMLFirstNode (XMLGetChildrenNode (root));
               bool       fFirstParam = true;

               if (params != 0)
               {
                  oss << XMLGetName (root) << "=";
               }

               while (params != 0)
               {
                  PSZCRO pszContent = XMLNodeListGetString (doc,
                                                         XMLGetChildrenNode (params),
                                                         1);

                  if (pszContent)
                  {
                     if (fFirstParam)
                     {
                        fFirstParam = false;
                     }
                     else
                     {
                        oss << 'X';
                     }

                     oss << pszContent;

                     XMLFree ((void *)pszContent);
                  }

                  params = XMLNextNode (params);
               }
            }

            XMLFree ((void *)pszFormat);
         }
         else
         {
            PSZCRO pszContent = XMLNodeListGetString (doc,
                                                   XMLGetChildrenNode (root),
                                                   1);

            if (pszContent)
            {
               oss << XMLGetName (root) << "=" << pszContent;

               XMLFree ((void *)pszContent);
            }
         }

         root = XMLNextNode (root);
      }

      return new std::string (oss.str ());
   }
   else
   {
      PSZCRO pszContent = XMLNodeListGetString (doc,
                                             XMLGetChildrenNode (root),
                                             1);

      if (pszContent)
      {
         std::ostringstream oss;

         oss << XMLGetName (root) << "=" << pszContent;

         XMLFree ((void *)pszContent);

         return new std::string (oss.str ());
      }
   }

   return 0;
}

static XmlDocPtr
getDeviceConfigurationFromFile (PSZCRO pszMasterXMLFile)
{
   XmlDocPtr  docRet   = 0;

   docRet = XMLParseFile (pszMasterXMLFile);
///XMLValidateDocument (&validator, docRet); // @TBD

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
         if (0 == strcmp (XMLGetName (nodeElm), "Device"))
         {
            fRc = true;
         }
      }
   }

   return fRc;
}

static void
convertFilename (char *pszFileName)
{
   while (*pszFileName)
   {
      switch (*pszFileName)
      {
      case '-':
      case '+':
      case '(':
      case ')':
      case ' ':
      case '/':
      case '\\':
         *pszFileName = '_';
      }

      pszFileName++;
   }
}

PSZCRO
convertToLibrary (char *pszFileName)
{
   if (  !pszFileName
      || !*pszFileName
      )
   {
      return pszFileName;
   }

   convertFilename (pszFileName);

   char *pszDot = strchr (pszFileName, '.');

   if (pszDot)
   {
      *pszDot = '\0';
   }

   char *pszRet     = 0;
   int   cbFileName = 0;

   cbFileName = 3                    // 'lib'
              + strlen (pszFileName)
              + 3                    // '.so'
              + 1;                   // '\0'

   pszRet = (char *)malloc (cbFileName);
   if (pszRet)
   {
      strcpy (pszRet, "lib");
      strcat (pszRet, pszFileName);
      strcat (pszRet, ".so");

      free (pszFileName);

      return pszRet;
   }
   else
   {
      return pszFileName;
   }
}

#ifndef RETAIL

void XMLDevice::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDevice::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDevice: "
       << " pstringMasterXMLPath_d = \"" << SAFE_PRINT_STRING (pstringMasterXMLPath_d) << "\""
       << ", pszXMLDeviceName_d = \"" << SAFE_PRINT_PSZ(pszXMLDeviceName_d) << "\""
       << ", pszDriverName_d = \"" << SAFE_PRINT_PSZ(pszDriverName_d) << "\""
       << ", pszDeviceName_d = \"" << SAFE_PRINT_PSZ(pszDeviceName_d) << "\""
       << ", pszShortName_d = \"" << SAFE_PRINT_PSZ(pszShortName_d) << "\""
#ifdef INCLUDE_JP_UPDF_BOOKLET
       << ", pstringDefaultBooklet_d = \"" << SAFE_PRINT_STRING(pstringDefaultBooklet_d) << "\""
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
       << ", pstringDefaultCopies_d = \"" << SAFE_PRINT_STRING(pstringDefaultCopies_d) << "\""
#endif
       << ", pszDefaultDitherID_d = \"" << SAFE_PRINT_PSZ(pszDefaultDitherID_d) << "\""
       << ", pstringDefaultForm_d = \"" << SAFE_PRINT_STRING(pstringDefaultForm_d) << "\""
#ifdef INCLUDE_JP_UPDF_JOGGING
       << ", pstringDefaultJogging_d = \"" << SAFE_PRINT_STRING(pstringDefaultJogging_d) << "\""
#endif
       << ", pstringDefaultMedia_d = \"" << SAFE_PRINT_STRING(pstringDefaultMedia_d) << "\""
#ifdef INCLUDE_JP_COMMON_NUP
       << ", pstringDefaultNUp_d = \"" << SAFE_PRINT_STRING(pstringDefaultNUp_d) << "\""
#endif
       << ", pstringDefaultOrientation_d = \"" << SAFE_PRINT_STRING(pstringDefaultOrientation_d) << "\""
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
       << ", pstringDefaultOutputBin_d = \"" << SAFE_PRINT_STRING(pstringDefaultOutputBin_d) << "\""
#endif
       << ", pstringDefaultPrintMode_d = \"" << SAFE_PRINT_STRING(pstringDefaultPrintMode_d) << "\""
       << ", pstringDefaultResolution_d = \"" << SAFE_PRINT_STRING(pstringDefaultResolution_d) << "\""
#ifdef INCLUDE_JP_COMMON_SCALING
       << ", pstringDefaultScaling_d = \"" << SAFE_PRINT_STRING(pstringDefaultScaling_d) << "\""
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
       << ", pstringDefaultSheetCollate_d = \"" << SAFE_PRINT_STRING(pstringDefaultSheetCollate_d) << "\""
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
       << ", pstringDefaultSide_d = \"" << SAFE_PRINT_STRING(pstringDefaultSide_d) << "\""
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
       << ", pstringDefaultStitching_d = \"" << SAFE_PRINT_STRING(pstringDefaultStitching_d) << "\""
#endif
       << ", pstringDefaultTray_d = \"" << SAFE_PRINT_STRING(pstringDefaultTray_d) << "\""
#ifdef INCLUDE_JP_COMMON_TRIMMING
       << ", pstringDefaultTrimming_d = \"" << SAFE_PRINT_STRING(pstringDefaultTrimming_d) << "\""
#endif
       << ", pszInstance_d = \"" << SAFE_PRINT_PSZ(pszInstance_d) << "\""
       << ", pszBlitter_d = \"" << SAFE_PRINT_PSZ(pszBlitter_d) << "\""
       << ", pInstance_d = " << pInstance_d
       << ", pBlitter_d = " << pBlitter_d
       << ", pXMLInstance_d = " << pXMLInstance_d
       << ", pXMLBlitter_d = " << pXMLBlitter_d
       << " "
       << PrintDevice::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDevice& const_self)
{
   XMLDevice&         self = const_cast<XMLDevice&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
