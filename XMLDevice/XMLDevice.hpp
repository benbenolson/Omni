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
#ifndef _XMLDevice
#define _XMLDevice

#include <Device.hpp>
#include <OmniProxy.hpp>

#include <map>
#include <cstdint>

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

struct XMLDeviceInstance;
struct XMLDeviceBlitter;

class XMLDevice : public PrintDevice
{
public:
                               XMLDevice                (PSZCRO               pszMasterXML,
                                                         XmlDocPtr            docDevice,
                                                         PSZCRO               pszDriverName,
                                                         PSZCRO               pszDeviceName,
                                                         PSZCRO               pszShortName,
                                                         PSZCRO               pszJobProperties,
                                                         PSZCRO               pszXMLDeviceName);
                              ~XMLDevice                ();

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
   virtual DeviceTrimming     *getDefaultTrimming      ();
#endif

   virtual DeviceCommand      *getDefaultCommands       ();
   virtual DeviceData         *getDefaultData           ();
   virtual DeviceString       *getDefaultString         ();

   virtual DeviceGamma        *getCurrentGamma          ();

   bool                        hasDeviceOption          (PSZCRO               pszDeviceOption);

   static XMLDevice           *isAXMLDevice             (Device              *pDevice);

   XmlNodePtr                  findEntryKeyValue        (XmlNodePtr           root,
                                                         PSZCRO               pszKey,
                                                         PSZCRO               pszValue,
                                                         bool                 fDebugOutput);

   XmlDocPtr                   getDeviceXML             (PSZCRO               pszDeviceType);

#ifdef INCLUDE_JP_UPDF_BOOKLET
   XmlDocPtr                   getDocBooklets           ();
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   XmlDocPtr                   getDocCopies             ();
#endif
   XmlDocPtr                   getDocForms              ();
#ifdef INCLUDE_JP_UPDF_JOGGING
   XmlDocPtr                   getDocJoggings           ();
#endif
   XmlDocPtr                   getDocMedias             ();
#ifdef INCLUDE_JP_COMMON_NUP
   XmlDocPtr                   getDocNUps               ();
#endif
   XmlDocPtr                   getDocOrientations       ();
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   XmlDocPtr                   getDocOutputBins         ();
#endif
   XmlDocPtr                   getDocPrintModes         ();
   XmlDocPtr                   getDocResolutions        ();
#ifdef INCLUDE_JP_COMMON_SCALING
   XmlDocPtr                   getDocScalings           ();
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   XmlDocPtr                   getDocSheetCollates      ();
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   XmlDocPtr                   getDocSides              ();
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   XmlDocPtr                   getDocStitchings         ();
#endif
   XmlDocPtr                   getDocTrays              ();
#ifdef INCLUDE_JP_COMMON_TRIMMING
   XmlDocPtr                   getDocTrimmings          ();
#endif
   XmlDocPtr                   getDocGammas             ();

   PSZCRO                      getXMLDeviceName         ();

   DeviceInstance             *getDeviceInstance        ();
   DeviceBlitter              *getDeviceBlitter         ();

   static bool                 parseBinaryData          (PSZRO                pszData,
                                                         byte               **ppbData,
                                                         int                 *pcbData);

   static std::string         *getXMLJobProperties      (XmlNodePtr           root,
                                                         XmlDocPtr            doc,
                                                         char                *pszXMLNodeName);

#ifndef RETAIL
   virtual void                outputSelf               ();
#endif
   virtual std::string         toString                 (std::ostringstream&  oss);
   friend std::ostream&        operator<<               (std::ostream&        os,
                                                         const XMLDevice&     self);

private:
   void                        initializeDevice         ();

   std::string       *pstringMasterXMLPath_d;
   char              *pszXMLDeviceName_d;

   PSZRO              pszDriverName_d;
   PSZRO              pszDeviceName_d;
   PSZRO              pszShortName_d;

   XmlDocPtr          docDevice_d;
   XmlNodePtr         rootDevice_d;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   XmlDocPtr          docDeviceBooklets_d;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   XmlDocPtr          docDeviceCopies_d;
#endif
   XmlDocPtr          docDeviceForms_d;
#ifdef INCLUDE_JP_UPDF_JOGGING
   XmlDocPtr          docDeviceJoggings_d;
#endif
   XmlDocPtr          docDeviceMedias_d;
#ifdef INCLUDE_JP_COMMON_NUP
   XmlDocPtr          docDeviceNUps_d;
#endif
   XmlDocPtr          docDeviceOrientations_d;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   XmlDocPtr          docDeviceOutputBins_d;
#endif
   XmlDocPtr          docDevicePrintModes_d;
   XmlDocPtr          docDeviceResolutions_d;
#ifdef INCLUDE_JP_COMMON_SCALING
   XmlDocPtr          docDeviceScalings_d;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   XmlDocPtr          docDeviceSheetCollates_d;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   XmlDocPtr          docDeviceSides_d;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   XmlDocPtr          docDeviceStitchings_d;
#endif
   XmlDocPtr          docDeviceTrays_d;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   XmlDocPtr          docDeviceTrimmings_d;
#endif
   XmlDocPtr          docDeviceGammas_d;
   XmlDocPtr          docDeviceCommands_d;
   XmlDocPtr          docDeviceDatas_d;
   XmlDocPtr          docDeviceStrings_d;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   std::string       *pstringDefaultBooklet_d;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   std::string       *pstringDefaultCopies_d;
#endif
   PSZRO              pszDefaultDitherID_d;
   std::string       *pstringDefaultForm_d;
#ifdef INCLUDE_JP_UPDF_JOGGING
   std::string       *pstringDefaultJogging_d;
#endif
   std::string       *pstringDefaultMedia_d;
#ifdef INCLUDE_JP_COMMON_NUP
   std::string       *pstringDefaultNUp_d;
#endif
   std::string       *pstringDefaultOrientation_d;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   std::string       *pstringDefaultOutputBin_d;
#endif
   std::string       *pstringDefaultPrintMode_d;
   std::string       *pstringDefaultResolution_d;
#ifdef INCLUDE_JP_COMMON_SCALING
   std::string       *pstringDefaultScaling_d;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   std::string       *pstringDefaultSheetCollate_d;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   std::string       *pstringDefaultSide_d;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   std::string       *pstringDefaultStitching_d;
#endif
   std::string       *pstringDefaultTray_d;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   std::string       *pstringDefaultTrimming_d;
#endif

   PSZRO              pszInstance_d;
   PSZRO              pszBlitter_d;

   DeviceInstance    *pInstance_d;
   DeviceBlitter     *pBlitter_d;

   XMLDeviceInstance *pXMLInstance_d;
   XMLDeviceBlitter  *pXMLBlitter_d;

   typedef std::map <std::string, XmlDocPtr> HasMapping;

   HasMapping         hasMap_d;
};

static inline PSZCRO
getXMLContentString (XmlNodePtr  root,
                     XmlDocPtr   doc,
                     char       *pszXMLNodeName)
{
   if (pszXMLNodeName)
   {
      root = XMLFindEntry (root, pszXMLNodeName, false);
   }

   if (root)
   {
      return XMLNodeListGetString (doc, XMLGetChildrenNode (root), 1);
   }

   return 0;
}

static inline int
getXMLContentInt (XmlNodePtr root,
                 XmlDocPtr  doc,
                 PSZCRO     pszXMLNodeName,
                 bool       fRequired = true,
                 int        iDefault  = 0)
{
   XmlNodePtr elm = XMLFindEntry (root, pszXMLNodeName, false);

   if (elm)
   {
      PSZCRO pszString = XMLNodeListGetString (doc, XMLGetChildrenNode (elm), 1);

      if (pszString)
      {
         bool fSuccess = false;

         if (1 == sscanf (pszString, "%d", &iDefault))
         {
            fSuccess = true;
         }

         free ((void *)pszString);

         if (fSuccess)
         {
            return iDefault;
         }

         std::string stringError = "Could not parse \"";

         stringError += pszString;
         stringError += "\"";

         throw new std::string (stringError);
      }
   }

   if (fRequired)
   {
      std::ostringstream oss;

      oss << "Could not find entry \""
          << pszXMLNodeName
          << "\" for root = 0x"
          << std::hex << (uintptr_t)root << std::dec;

      throw new std::string (oss.str ());
   }
   else
   {
      return iDefault;
   }
}

static inline bool
getXMLContentBool (XmlNodePtr root,
                   XmlDocPtr  doc,
                   PSZCRO     pszXMLNodeName,
                   bool       fRequired = true,
                   bool       fDefault  = false)
{
   XmlNodePtr elm = XMLFindEntry (root, pszXMLNodeName, false);

   if (elm)
   {
      PSZCRO pszString = XMLNodeListGetString (doc, XMLGetChildrenNode (elm), 1);

      if (pszString)
      {
         bool fSuccess = false;

         if (0 == strcasecmp (pszString, "true"))
         {
            fDefault = true;
            fSuccess = true;
         }
         else if (0 == strcasecmp (pszString, "false"))
         {
            fDefault = false;
            fSuccess = true;
         }

         free ((void *)pszString);

         if (fSuccess)
         {
            return fDefault;
         }

         std::string stringError = "Could not parse \"";

         stringError += pszString;
         stringError += "\"";

         throw new std::string (stringError);
      }
   }

   if (fRequired)
   {
      std::ostringstream oss;

      oss << "Could not find entry \""
          << pszXMLNodeName
          << "\" for root = 0x"
          << std::hex << (uintptr_t)root << std::dec;

      throw new std::string (oss.str ());
   }
   else
   {
      return fDefault;
   }
}

static inline PSZCRO
getXMLAttribute (XmlNodePtr root,
                 PSZCRO     pszXMLNodeName,
                 PSZCRO     pszXMLAttributeName)
{
   if (pszXMLNodeName)
   {
      root = XMLFindEntry (root, pszXMLNodeName, false);
   }

   if (root)
   {
      return XMLGetProp (root, pszXMLAttributeName);
   }

   return 0;
}

#endif
