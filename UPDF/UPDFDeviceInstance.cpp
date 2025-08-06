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
#include "UPDFDeviceInstance.hpp"
#include "defines.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

UPDFObjectStore::
UPDFObjectStore (UPDFDevice *pUPDFDevice)
{
   pUPDFDevice_d = pUPDFDevice;
}

UPDFObjectStore::
UPDFObjectStore (UPDFObjectStore *pUPDFObjectStore)
{
   pUPDFDevice_d = 0;

   if (pUPDFObjectStore)
   {
      pUPDFDevice_d = pUPDFObjectStore->pUPDFDevice_d;
   }
}

UPDFObjectStore::
~UPDFObjectStore ()
{
}

void UPDFObjectStore::
addXMLNode (PSZCRO     pszKey,
            XmlNodePtr xmlValue)
{
   std::string stringKey = pszKey;

   xmlObjectMap_d[stringKey] = xmlValue;
}

void UPDFObjectStore::
addStringKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string stringKey   = pszKey;
   std::string stringValue = pszValue;

   xmlObjectMap_d[stringKey] = 0;
   xmlStringMap_d[stringKey] = stringValue;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": Warning: Placing " << SAFE_PRINT_PSZ (pszKey) << "=" << SAFE_PRINT_PSZ (pszValue) << " in string map!" << std::endl;
#endif
}

void UPDFObjectStore::
applyJobProperties (PSZCRO pszJobProperties)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO      pszKey                 = pEnum->getCurrentKey ();
      PSZCRO      pszValue               = pEnum->getCurrentValue ();
      std::string stringKey   (pszKey);
      std::string stringValue (pszValue);
      XmlNodePtr  elm                    = 0;

      elm = xmlObjectMap_d[stringKey];

      if (elm)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": xmlObjectMap_d[" << stringKey << "] = " << elm << std::endl;
#endif

         elm = FINDUDRENTRYKEYVALUE (pUPDFDevice_d,
                                     "ID",
                                     pszValue,
                                     UPDFDeviceInstance);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": new elm = " << elm << std::endl;
#endif

         if (elm)
         {
            xmlObjectMap_d[stringKey] = elm;
         }
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": Adding " << SAFE_PRINT_PSZ (pszKey) << "=" << SAFE_PRINT_PSZ (pszValue) << " to the string map." << std::endl;
#endif

         xmlStringMap_d[pszKey] = stringValue;
      }

      pEnum->nextElement ();
   }

   delete pEnum;
}

bool UPDFObjectStore::
applyRequiredJobProperties (PSZCRO pszJobProperties,
                                   ...)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   va_list                list;
   PSZRO                  pszKey                     = 0;
   bool                   fFound                     = false;

   va_start (list, pszJobProperties);

   do
   {
      pszKey = va_arg (list, PSZCRO);

      if (pszKey)
      {
         fFound = jobProp.hasJobProperty (pszKey);
      }
      else
      {
         fFound = true;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": pszKey = " << SAFE_PRINT_PSZ (pszKey) << ", fFound = " << fFound << std::endl;
#endif
   } while (  pszKey
           && fFound
           );

   va_end (list);

   if (fFound)
   {
      pEnum = jobProp.getEnumeration ();

      while (pEnum->hasMoreElements ())
      {
         PSZCRO      pszKey                 = pEnum->getCurrentKey ();
         PSZCRO      pszValue               = pEnum->getCurrentValue ();
         std::string stringKey   (pszKey);
         std::string stringValue (pszValue);
         XmlNodePtr  elm                    = 0;

         elm = xmlObjectMap_d[stringKey];

         if (elm)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": xmlObjectMap_d[" << stringKey << "] = " << elm << std::endl;
#endif

            elm = FINDUDRENTRYKEYVALUE (pUPDFDevice_d,
                                        "ID",
                                        pszValue,
                                        UPDFDeviceInstance);

#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": new elm = " << elm << std::endl;
#endif

            if (elm)
            {
               xmlObjectMap_d[stringKey] = elm;
            }
         }
         else
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": Adding " << SAFE_PRINT_PSZ (pszKey) << "=" << SAFE_PRINT_PSZ (pszValue) << " to the string map." << std::endl;
#endif

            xmlStringMap_d[pszKey] = stringValue;
         }

         pEnum->nextElement ();
      }

      delete pEnum;
   }

   return fFound;
}

XmlNodePtr UPDFObjectStore::
getXMLNode (PSZCRO pszKey)
{
   std::string stringKey = pszKey;

   return xmlObjectMap_d[stringKey];
}

PSZCRO UPDFObjectStore::
getStringValue (PSZCRO pszKey)
{
   std::string  stringKey  = pszKey;
   std::string  stringElm;
   PSZRO        pszElm     = 0;

   stringElm = xmlStringMap_d[stringKey];

   if (stringElm.length ())
   {
      pszElm = (PSZ)malloc (stringElm.length () + 1);
      if (pszElm)
      {
         strcpy ((char *)pszElm, (char *)stringElm.c_str ());
      }
   }

   return pszElm;
}

UPDFDeviceInstance::
UPDFDeviceInstance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
   pUPDFObjectStore_d  = new UPDFObjectStore (UPDFDevice::isAUPDFDevice (pDevice));

   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);

   if (pUPDFDevice)
   {
      loadNonDominantDefaults (pUPDFDevice, pUPDFDevice->getRootUDR ());
   }
   loadLocaleDefaults ();
}

UPDFDeviceInstance::
~UPDFDeviceInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::~" << __FUNCTION__ << " ()" << std::endl;
#endif

   delete pUPDFObjectStore_d;
   pUPDFObjectStore_d = 0;
}

void UPDFDeviceInstance::
initializeInstance (PSZCRO pszJobProperties)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

   processDependencies ();
}

bool UPDFDeviceInstance::
hasError ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   return false;
}

std::string * UPDFDeviceInstance::
getJobProperties (bool fInDeviceSpecific)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " (" << fInDeviceSpecific << ")" << std::endl;
#endif

   return new std::string ("");
}

bool UPDFDeviceInstance::
setJobProperties (PSZCRO pszJobProperties)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

   return true;
}

Enumeration * UPDFDeviceInstance::
getGroupEnumeration (bool fInDeviceSpecific)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " (" << fInDeviceSpecific << ")" << std::endl;
#endif

   return new NullEnumerator ();
}

class DeviceJobPropertyKeyEnumerator : public Enumeration
{
public:
   DeviceJobPropertyKeyEnumerator (XMLObjectMap::iterator first,
                                   XMLObjectMap::iterator last)
   {
      next_d = first;
      last_d = last;
   }

   virtual ~
   DeviceJobPropertyKeyEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return next_d != last_d;
   }

   virtual void *
   nextElement ()
   {
      void *pvRet = 0;

      if (next_d != last_d)
      {
         pvRet = (void *)next_d->first.c_str ();

         next_d++;
      }

      return pvRet;
   }

private:
   XMLObjectMap::iterator next_d;
   XMLObjectMap::iterator last_d;
};

class DeviceJobPropertyKeyValueEnumerator : public Enumeration
{
public:
   DeviceJobPropertyKeyValueEnumerator (XmlNodePtr elm)
   {
      elm_d     = 0;
      pszNext_d = 0;
      pszLast_d = 0;

      if (  elm
         && XMLGetChildrenNode (elm)
         )
      {
         elm_d = XMLFirstNode (XMLGetChildrenNode (elm));

         pszNext_d = getNextID ();
      }
   }

   virtual ~
   DeviceJobPropertyKeyValueEnumerator ()
   {
      if (pszNext_d)
      {
         XMLFree ((void *)pszNext_d);
         pszNext_d = 0;
      }

      if (pszLast_d)
      {
         XMLFree ((void *)pszLast_d);
         pszLast_d = 0;
      }
   }

   virtual bool
   hasMoreElements ()
   {
      return pszNext_d != 0;
   }

   virtual void *
   nextElement ()
   {
      if (pszLast_d)
      {
         XMLFree ((void *)pszLast_d);
         pszLast_d = 0;
      }

      pszLast_d = pszNext_d;
      pszNext_d = getNextID ();

      return (void *)pszLast_d;
   }

private:
   PSZCRO
   getNextID ()
   {
      while (elm_d != 0)
      {
         PSZCRO pszID = XMLGetProp (elm_d, "ID");

         elm_d = XMLNextNode (elm_d);

         if (pszID)
         {
            return pszID;
         }
      }

      return 0;
   }

   XmlNodePtr  elm_d;
   PSZRO       pszNext_d;
   PSZRO       pszLast_d;
};

std::string * UPDFDeviceInstance::
getJobPropertyType (PSZCRO pszKey)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ("
                          << pszKey << ")"
                          << std::endl;
#endif

   XmlNodePtr   node       = 0;
   std::string *pstringRet = 0;

   if (  pszKey
      && *pszKey
      )
   {
      node = pUPDFObjectStore_d->getXMLNode (pszKey);

      if (node)
      {
         PSZCRO pszID = XMLGetProp (node, "ID");

         if (pszID)
         {
            pstringRet = new std::string ("string");

            *pstringRet += pszID;

            XMLFree ((void *)pszID);
         }
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " returns "
                          << (pstringRet ? *pstringRet : "NULL")
                          << std::endl;
#endif

   return pstringRet;
}

std::string * UPDFDeviceInstance::
getJobProperty (PSZCRO pszKey)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ("
                          << pszKey << ")"
                          << std::endl;
#endif

   XmlNodePtr   node       = 0;
   std::string *pstringRet = 0;

   if (  pszKey
      && *pszKey
      )
   {
      node = pUPDFObjectStore_d->getXMLNode (pszKey);

      if (node)
      {
         PSZCRO pszID = XMLGetProp (node, "ID");

         if (pszID)
         {
            pstringRet = new std::string (pszID);

            XMLFree ((void *)pszID);
         }
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " returns "
                          << (pstringRet ? *pstringRet : "NULL")
                          << std::endl;
#endif

   return pstringRet;
}

std::string * UPDFDeviceInstance::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ("
                          << pszKey << ", "
                          << pszValue << ")"
                          << std::endl;
#endif

   UPDFDevice  *pUPDFDevice  = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr   node         = 0;
   std::string *pstringRet   = 0;

   if (  pszKey
      && *pszKey
      && pUPDFDevice
      )
   {
      node = pUPDFObjectStore_d->getXMLNode (pszKey);

      if (node)
      {
         node = FINDUDRENTRYKEYVALUE (pUPDFDevice,
                                      "ID",
                                      pszValue,
                                      UPDFDeviceInstance);

         PSZCRO pszNameID = XMLGetProp (node, "Name_ID");

         if (pszNameID)
         {
            XmlNodePtr nodeLocale = FINDLOCALEENTRYKEYVALUE (pUPDFDevice,
                                                             "Name_ID",
                                                             pszNameID,
                                                             UPDFDeviceInstance);

            if (nodeLocale)
            {
               PSZCRO pszLocalizedString = XMLGetProp (nodeLocale,
                                                       "Localized_String");

               if (pszLocalizedString)
               {
                  pstringRet = new std::string (pszKey);

                  *pstringRet += "=";
                  *pstringRet += pszLocalizedString;

                  XMLFree ((void *)pszLocalizedString);
               }
            }

            XMLFree ((void *)pszNameID);
         }
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " returns "
                          << (pstringRet ? *pstringRet : "NULL")
                          << std::endl;
#endif

   return pstringRet;
}

bool UPDFDeviceInstance::
beginJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   return executeEvent ("Event_PDL_Start", true);
}

bool UPDFDeviceInstance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " (with props)" << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;
#endif

   // @TBD - reinitialize with new job properties

   // Call common code
   return beginJob ();
}

bool UPDFDeviceInstance::
newFrame ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   ditherNewFrame ();

   ptlPrintHead_d.x = 0;
   ptlPrintHead_d.y = 0;

   return true;
}

bool UPDFDeviceInstance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " (with props)" << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;
#endif

   // @TBD - reinitialize with new job properties

   // Call common code
   return newFrame ();
}

bool UPDFDeviceInstance::
endJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   return true;
}

bool UPDFDeviceInstance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " ()" << std::endl;
#endif

   return true;
}

std::string UPDFDeviceInstance::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceInstance: "
       << DeviceInstance::toString (oss2) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceInstance& const_self)
{
   UPDFDeviceInstance& self = const_cast<UPDFDeviceInstance&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}

void UPDFDeviceInstance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;
}

PSZCRO UPDFDeviceInstance::
getXMLObjectValue (PSZCRO pszObjectName,
                   PSZCRO pszVariableName)
{
   if (  !pszObjectName
      || !pszVariableName
      )
   {
      return 0;
   }

   XmlNodePtr elm = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszObjectName) << "\", \"" << SAFE_PRINT_PSZ(pszVariableName) << "\")" << std::endl;
#endif

   elm = pUPDFObjectStore_d->getXMLNode (pszObjectName);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << " elm = " << elm << std::endl;
#endif

   if (elm)
   {
      return XMLGetProp (elm, pszVariableName);
   }
   else
   {
      return pUPDFObjectStore_d->getStringValue (pszObjectName);
   }

   return 0;
}

XmlNodePtr UPDFDeviceInstance::
getXMLObjectNode (PSZCRO pszObjectName)
{
   if (  !pszObjectName
      || !*pszObjectName
      )
   {
      return 0;
   }

   return pUPDFObjectStore_d->getXMLNode (pszObjectName);
}

bool UPDFDeviceInstance::
executeEvent (PSZCRO pszPrefix, bool fPreEvent)
{
   char        achParameterName[128]; // @TBD
   DeviceData *pData                 = getDeviceData ();
   bool        fExecute              = fPreEvent;

   if (!pData)
      return true;

   int iCount = 0;

   sprintf (achParameterName, "%s_count", pszPrefix);

   if (  !pData->getIntData (achParameterName, &iCount)
      || 0 == iCount
      )
      return true;

   for (int i = 0; i < iCount; i++)
   {
      BinaryData *pbdExpression = 0;
      PSZ         pszEventName  = 0;

      sprintf (achParameterName, "%s_binary_%d", pszPrefix, i);

      if (pData->getBinaryData (achParameterName, &pbdExpression))
      {
      }
      else if (pData->getStringData (achParameterName, &pszEventName))
      {
         fExecute = !fExecute;
      }
      else
      {
      }
   }

   return true;
}

UPDFDeviceInstance * UPDFDeviceInstance::
isAUPDFDeviceInstance (DeviceInstance *pDeviceInstance)
{
   return dynamic_cast<UPDFDeviceInstance *>(pDeviceInstance);
}

UPDFObjectStore * UPDFDeviceInstance::
getObjectStore ()
{
   UPDFObjectStore *pRet = 0;

   pRet = new UPDFObjectStore (pUPDFObjectStore_d);

   return pRet;
}

void UPDFDeviceInstance::
loadNonDominantDefaults (UPDFDevice *pUPDFDevice, XmlNodePtr node)
{
   if (!node)
      return;

   node = XMLFirstNode (node);

   while (node)
   {
      if (XMLGetChildrenNode (node))
      {
         loadNonDominantDefaults (pUPDFDevice, XMLGetChildrenNode (node));
      }

      PSZCRO pszNDR = XMLGetProp (node, "NonDominantRepresentative");

      if (pszNDR)
      {
         PSZCRO pszID = XMLGetProp (node, "ID");

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": pszNDR = " << pszNDR << ", pszID = " << pszID << std::endl;
#endif

         if (pszID)
         {
            XmlNodePtr elm = pUPDFDevice->findEntryKeyValue (node,
                                                             "ID",
                                                             pszNDR,
                                                             DebugOutput::shouldOutputUPDFDeviceInstance ());

            if (elm)
            {
               pUPDFObjectStore_d->addXMLNode (pszID, elm);
            }

            XMLFree ((void *)pszID);
         }

         XMLFree ((void *)pszNDR);
      }

      node = XMLNextNode (node);
   }
}

void UPDFDeviceInstance::
loadLocaleDefaults ()
{
   UPDFDevice *pUPDFDevice       = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeLocalDefaults = 0;

   if (!pUPDFDevice)
      return;

   if ((nodeLocalDefaults = FINDLOCALEENTRY (pUPDFDevice, nodeLocalDefaults, "LocaleDefaults", UPDFDeviceInstance)) != 0)
   {
      XmlNodePtr nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeLocalDefaults));

      while (nodeItem != 0)
      {
         PSZCRO pszFeature = XMLGetProp (nodeItem, "DefaultFeature");
         PSZCRO pszSetting = XMLGetProp (nodeItem, "DefaultSetting");

         if (  pszFeature
            && pszSetting
            )
         {
            XmlNodePtr elm = 0;

#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": pszFeature = " << SAFE_PRINT_PSZ (pszFeature) << ", pszSetting = " << SAFE_PRINT_PSZ (pszSetting) << std::endl;
#endif

            elm = FINDUDRENTRYKEYVALUE (pUPDFDevice, "ID", pszSetting, UPDFDeviceInstance);

            if (elm)
            {
               pUPDFObjectStore_d->addXMLNode (pszFeature, elm);
            }
            else
            {
               pUPDFObjectStore_d->addStringKeyValue (pszFeature, pszSetting);
            }
         }

         if (pszFeature)
         {
            XMLFree ((void *)pszFeature);
         }
         if (pszSetting)
         {
            XMLFree ((void *)pszSetting);
         }

         nodeItem = XMLNextNode (nodeItem);
      }
   }
}

void UPDFDeviceInstance::
processDependencies ()
{
   UPDFDevice *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeDependencies = 0;
   XmlNodePtr  nodeDependency   = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": Error: !pUPDFDevice" << std::endl;
#endif
      return;
   }

   if (  ((nodeDependencies = FINDUDRENTRY (pUPDFDevice, nodeDependencies, "PrintCapabilities",  UPDFDeviceInstance)) == 0)
      || ((nodeDependencies = FINDUDRENTRY (pUPDFDevice, nodeDependencies, "Dependencies",       UPDFDeviceInstance)) == 0)
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": No dependencies found." << std::endl;
#endif
      return;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": nodeDependencies = " << nodeDependencies << std::endl;
#endif

   nodeDependency = XMLFirstNode (XMLGetChildrenNode (nodeDependencies));

   while (nodeDependency)
   {
      XmlNodePtr node    = 0;
      bool       fActive = true;

      node = XMLFirstNode (XMLGetChildrenNode (nodeDependency));
      while (node)
      {
         if (0 == strcmp ("FeatureCondition", XMLGetName (node)))
         {
            PSZCRO     pszFeatureID         = XMLGetProp (node, "FeatureID");
            PSZCRO     pszConditionOperator = XMLGetProp (node, "ConditionOperator");
            PSZCRO     pszFeatureSetting    = XMLGetProp (node, "FeatureSetting");
            XmlNodePtr elm                  = 0;
            bool       fResult              = false;

#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": pszFeatureID = " << SAFE_PRINT_PSZ (pszFeatureID) << ", pszConditionOperator = " << SAFE_PRINT_PSZ (pszConditionOperator) << ", pszFeatureSetting = " << SAFE_PRINT_PSZ (pszFeatureSetting) << std::endl;
#endif

            if (  pszFeatureID
               && pszConditionOperator
               && pszFeatureSetting
               )
            {
               if (0 == strcmp (pszConditionOperator, "Equal"))
               {
                  elm = pUPDFObjectStore_d->getXMLNode (pszFeatureID);

                  if (elm)
                  {
                     PSZCRO pszID = XMLGetProp (elm, "ID");

#ifndef RETAIL
                     if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": pszID = " << SAFE_PRINT_PSZ (pszID) << std::endl;
#endif

                     if (  pszID
                        && 0 == strcmp (pszID, pszFeatureSetting)
                        )
                     {
                        fResult = true;
                     }

                     if (pszID)
                     {
                        XMLFree ((void *)pszID);
                     }
                  }
               }
               else
               {
#ifndef RETAIL
                  if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": Error: Unknown ConditionOperator!" << std::endl;
#endif
               }
            }

            if (pszFeatureID)
            {
               XMLFree ((void *)pszFeatureID);
            }
            if (pszConditionOperator)
            {
               XMLFree ((void *)pszConditionOperator);
            }
            if (pszFeatureSetting)
            {
               XMLFree ((void *)pszFeatureSetting);
            }

            fActive = fActive && fResult;
         }

         node = XMLNextNode (node);
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": fActive = " << fActive << std::endl;
#endif

      if (fActive)
      {
         node = XMLFirstNode (XMLGetChildrenNode (nodeDependency));
         while (node)
         {
            XmlNodePtr nodeAction = 0;

            if (0 == strcmp ("Action", XMLGetName (node)))
            {
               nodeAction = XMLFirstNode (XMLGetChildrenNode (node));
               while (nodeAction)
               {
                  if (0 == strcmp ("ActionSelection", XMLGetName (nodeAction)))
                  {
                     XmlNodePtr nodeActionSelection = 0;

                     nodeActionSelection = XMLFirstNode (XMLGetChildrenNode (nodeAction));
                     while (nodeActionSelection)
                     {
                        if (0 == strcmp ("SetFeature", XMLGetName (nodeActionSelection)))
                        {
                           PSZCRO pszFeatureID      = XMLGetProp (nodeActionSelection, "FeatureID");
                           PSZCRO pszFeatureSetting = XMLGetProp (nodeActionSelection, "FeatureSetting");

#ifndef RETAIL
                           if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": pszFeatureID = " << SAFE_PRINT_PSZ (pszFeatureID) << ", pszFeatureSetting = " << SAFE_PRINT_PSZ (pszFeatureSetting) << std::endl;
#endif

                           if (  pszFeatureID
                              && pszFeatureSetting
                              )
                           {
                              XmlNodePtr elm = 0;

                              elm = FINDUDRENTRYKEYVALUE (pUPDFDevice,
                                                          "ID",
                                                          pszFeatureSetting,
                                                          UPDFDeviceInstance);

#ifndef RETAIL
                              if (DebugOutput::shouldOutputUPDFDeviceInstance ()) DebugOutput::getErrorStream () << "UPDFDeviceInstance::" << __FUNCTION__ << ": elm = " << elm << std::endl;
#endif

                              if (elm)
                              {
                                 pUPDFObjectStore_d->addXMLNode (pszFeatureID, elm);
                              }
                           }

                           if (pszFeatureID)
                           {
                              XMLFree ((void *)pszFeatureID);
                           }
                           if (pszFeatureSetting)
                           {
                              XMLFree ((void *)pszFeatureSetting);
                           }
                        }

                        nodeActionSelection = XMLNextNode (nodeActionSelection);
                     }
                  }

                  nodeAction = XMLNextNode (nodeAction);
               }
            }

            node = XMLNextNode (node);
         }
      }

      nodeDependency = XMLNextNode (nodeDependency);
   }
}
