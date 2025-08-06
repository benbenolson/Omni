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
#ifndef _OmniDomParser
#define _OmniDomParser

#include <string>
#include <fstream>

#include "XMLInterface.hpp"

#include "Device.hpp"
#include "DeviceInfo.hpp"

class OmniDomParser : public XmlParseError
{
public:
                 OmniDomParser              (const char  *pszXMLFile,
                                             DeviceInfo  *pdi,
                                             bool         fAutoconf,
                                             bool         fAutoconfNoInst);
   virtual      ~OmniDomParser              ();

   bool          parse                      ();
   void          setErrorCondition          (); // for ReportError interface

   bool          fileModified               ();

private:
   std::string  *getNameOut                 ();
   bool          shouldCreateFile           (std::string       strFileName);
   void          copyFile                   (const char       *pszFileNameIn);
   bool          openOutputFiles            (std::string      *pstringName,
                                             std::ofstream   **ppofstreamHPP,
                                             std::ofstream   **ppofstreamCPP);
   void          addToLibraries3            (const char       *pszFileName);
   void          addToLibraries3            (std::string&      stringFileName);
   bool          parseBinaryData            (const char       *pszData,
                                             byte            **ppbData,
                                             int              *pcbData);
   void          processDOMNode             (XmlNodePtr        node);
   bool          processDeviceCopies        (XmlNodePtr        deviceCopiesNode);
   bool          processDeviceForms         (XmlNodePtr        deviceFormsNode);
   bool          processDeviceMedias        (XmlNodePtr        deviceMediasNode);
   bool          processDeviceNUps          (XmlNodePtr        deviceNUpsNode);
   bool          processDeviceOrientations  (XmlNodePtr        deviceOrientationsNode);
   bool          processDeviceOutputBins    (XmlNodePtr        deviceOutputBinsNode);
   bool          processDevicePrintModes    (XmlNodePtr        devicePrintModesNode);
   bool          processDeviceResolutions   (XmlNodePtr        deviceResolutionsNode);
   bool          processDeviceScalings      (XmlNodePtr        deviceScalingsNode);
   bool          processDeviceSheetCollates (XmlNodePtr        deviceSheetCollatesNode);
   bool          processDeviceSides         (XmlNodePtr        deviceSidesNode);
   bool          processDeviceStitchings    (XmlNodePtr        deviceStitchingsNode);
   bool          processDeviceTrays         (XmlNodePtr        deviceTraysNode);
   bool          processDeviceTrimmings     (XmlNodePtr        deviceTrimmingsNode);
   bool          processDeviceConnections   (XmlNodePtr        deviceConnectionsNode);
   bool          processDeviceGammas        (XmlNodePtr        deviceGammasNode);
   bool          processDeviceCommands      (XmlNodePtr        deviceCommandsNode);
   bool          processDeviceDatas         (XmlNodePtr        deviceDatas);
   bool          processDeviceStrings       (XmlNodePtr        deviceDatas);
   bool          preprocessDriver           (XmlNodePtr        driverNode);
   bool          processDriver              (XmlNodePtr        driverNode);

   XmlDocPtr    doc_d;
   std::string *pstrXMLFile_d;
   DeviceInfo  *pdi_d;
   bool         fErrorCondition_d;
   bool         fAutoconf_d;
   bool         fAutoconfNoInst_d;
   bool         fFileModified_d;
};

#endif
