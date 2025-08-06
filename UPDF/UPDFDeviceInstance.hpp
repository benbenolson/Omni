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
#ifndef _UPDFInstance
#define _UPDFInstance

#include <UPDFDevice.hpp>
#include "ParameterConverter.hpp"

typedef std::map <std::string, XmlNodePtr> XMLObjectMap;
typedef std::map <std::string, std::string> XMLStringMap;

class UPDFObjectStore
{
public:
               UPDFObjectStore            (UPDFDevice            *pUPDFDevice);
               UPDFObjectStore            (UPDFObjectStore       *pUPDFObjectStore);
              ~UPDFObjectStore            ();

   void        addXMLNode                 (PSZCRO                 pszKey,
                                           XmlNodePtr             xmlValue);
   void        addStringKeyValue          (PSZCRO                 pszKey,
                                           PSZCRO                 pszValue);
   void        applyJobProperties         (PSZCRO                 pszJobProperties);
   bool        applyRequiredJobProperties (PSZCRO                 pszJobProperties,
                                                                  ...);
   XmlNodePtr  getXMLNode                 (PSZCRO                 pszKey);
   PSZCRO      getStringValue             (PSZCRO                 pszKey);

private:

   UPDFDevice          *pUPDFDevice_d;
   XMLObjectMap         xmlObjectMap_d;
   XMLStringMap         xmlStringMap_d;
};

class UPDFDeviceInstance : public DeviceInstance
{
public:
                              UPDFDeviceInstance        (PrintDevice              *pDevice);
   virtual                   ~UPDFDeviceInstance        ();

   void                       initializeInstance        (PSZCRO                    pszJobProperties);

   virtual bool               hasError                  ();

   virtual std::string       *getJobProperties          (bool                      fInDeviceSpecific = false);
   virtual bool               setJobProperties          (PSZCRO                    pszJobProperties);

   virtual Enumeration       *getGroupEnumeration       (bool                      fInDeviceSpecific = false);

   virtual std::string       *getJobPropertyType        (PSZCRO                    pszKey);
   virtual std::string       *getJobProperty            (PSZCRO                    pszKey);
   virtual std::string       *translateKeyValue         (PSZCRO                    pszKey,
                                                         PSZCRO                    pszValue);

   virtual bool               beginJob                  ();
   virtual bool               beginJob                  (bool                      fJobPropertiesChanged);
   virtual bool               newFrame                  ();
   virtual bool               newFrame                  (bool                      fJobPropertiesChanged);
   virtual bool               endJob                    ();
   virtual bool               abortJob                  ();

   virtual std::string        toString                  (std::ostringstream&       oss);
   friend std::ostream&       operator<<                (std::ostream&             os,
                                                         const UPDFDeviceInstance& device);

   void                       setupPrinter              ();

   PSZCRO                     getXMLObjectValue         (PSZCRO                    pszObjectName,
                                                         PSZCRO                    pszVariableName);

   XmlNodePtr                 getXMLObjectNode          (PSZCRO                    pszObjectName);

   bool                       executeEvent              (PSZCRO                    pszPrefix,
                                                         bool                      fPreEvent);

   static UPDFDeviceInstance *isAUPDFDeviceInstance     (DeviceInstance           *pDeviceInstance);

   UPDFObjectStore           *getObjectStore            ();

   POINTL               ptlPrintHead_d;

private:
   void                       loadNonDominantDefaults   (UPDFDevice               *pUPDFDevice,
                                                         XmlNodePtr                node);
   void                       loadLocaleDefaults        ();
   void                       processDependencies       ();

   bool                 fHaveInitialized_d;
   bool                 fHaveSetupPrinter_d;

   UPDFObjectStore     *pUPDFObjectStore_d;
};

#endif
