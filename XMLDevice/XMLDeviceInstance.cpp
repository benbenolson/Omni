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
#include "XMLDeviceInstance.hpp"
#include "XMLDevice.hpp"
#include "JobProperties.hpp"

#include <typeinfo>
#include <cstdint>

static PSZCRO vapszExtraKeys[] = {
   "XMLMasterFile"
};

#define KEY_DEVICENAME vapszExtraKeys[0]

XMLDeviceInstance::
XMLDeviceInstance (DeviceInstance    *pInstance,
                   PFNDELETEINSTANCE  pfnDeleteInstance,
                   PrintDevice       *pDevice)
   : DeviceInstance (pDevice)
{
   hmodLibrary_d       = 0;
   pfnDeleteInstance_d = 0;
   pInstance_d         = 0;

   if (pfnDeleteInstance)
   {
      pfnDeleteInstance_d = pfnDeleteInstance;
      pInstance_d         = pInstance;
   }
}

XMLDeviceInstance::
XMLDeviceInstance (GModule     *hmodLibrary,
                   PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
   PFNCREATEINSTANCE  pfnCreateInstance = 0;

   hmodLibrary_d       = hmodLibrary;
   pfnDeleteInstance_d = 0;
   pInstance_d         = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": hmodLibrary_d = " << hmodLibrary_d << std::endl;
#endif

   if (hmodLibrary_d)
   {
      int rc = 0;

      rc = ::g_module_symbol (hmodLibrary_d,
                              "createInstance",
                              (void **)&pfnCreateInstance);

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pfnCreateInstance = " << (void *)pfnCreateInstance << std::endl;
#endif

      if (!rc)
      {
         std::cerr << "g_module_error returns " << g_module_error () << std::endl;
      }

      rc = ::g_module_symbol (hmodLibrary_d,
                              "deleteInstance",
                              (void **)&pfnDeleteInstance_d);

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pfnDeleteInstance_d = " << (void *)pfnDeleteInstance_d << std::endl;
#endif

      if (!rc)
      {
         std::cerr << "g_module_error returns " << g_module_error () << std::endl;
      }

      if (  pfnCreateInstance
         || pfnDeleteInstance_d
         )
      {
         pInstance_d = pfnCreateInstance (pDevice);

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pInstance_d = " << pInstance_d << std::endl;
#endif
      }
   }
}

XMLDeviceInstance::
~XMLDeviceInstance ()
{
   if (pInstance_d)
   {
      pfnDeleteInstance_d (pInstance_d);
      pInstance_d = 0;
   }

   if (hmodLibrary_d)
   {
      g_module_close (hmodLibrary_d);
      hmodLibrary_d = 0;
   }
}

void XMLDeviceInstance::
initializeInstance (PSZCRO pszJobProperties)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
   {
      pInstance_d->initializeInstance (pszJobProperties);
   }
}

bool XMLDeviceInstance::
hasError ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->hasError ();
   else
      return false;
}

std::string * XMLDeviceInstance::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream  oss;
   std::string        *pstringRet = new std::string (KEY_DEVICENAME);

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": fInDeviceSpecific = " << fInDeviceSpecific << std::endl;
#endif

   if (pstringRet)
   {
      *pstringRet += "=";
      addDeviceNameValue (pstringRet, true);

      oss << *pstringRet;

      delete pstringRet;
   }

   if (pInstance_d)
   {
      pstringRet = pInstance_d->getJobProperties (fInDeviceSpecific);

      if (pstringRet)
      {
         if (oss.str ()[0])
         {
            oss << " ";
         }

         oss << *pstringRet;

         delete pstringRet;
      }
   }

   return new std::string (oss.str ());
}

bool XMLDeviceInstance::
setJobProperties (PSZCRO pszJobProperties)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

   if (pInstance_d)
   {
      return pInstance_d->setJobProperties (pszJobProperties);
   }

   return false;
}

class MasterFileEnumerator : public Enumeration
{
public:

   MasterFileEnumerator (std::string *pstringMaster)
   {
      if (pstringMaster)
      {
         fReturned_d    = false;
         stringMaster_d = *pstringMaster;
      }
      else
      {
         fReturned_d = true;
      }
   }

   virtual ~
   MasterFileEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return !fReturned_d;
   }

   virtual void *
   nextElement ()
   {
      void *pvRet = 0;

      if (!fReturned_d)
      {
         pvRet = new JobProperties (stringMaster_d);

         fReturned_d = true;
      }

      return pvRet;
   }

private:
   std::string stringMaster_d;
   bool        fReturned_d;
};

Enumeration * XMLDeviceInstance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   EnumEnumerator *pRet          = new EnumEnumerator ();
   std::string    *pstringMaster = new std::string (KEY_DEVICENAME);

   if (pstringMaster)
   {
      *pstringMaster += "=";
      addDeviceNameValue (pstringMaster, true);

      pRet->addElement (new MasterFileEnumerator (pstringMaster));

      delete pstringMaster;
   }

   if (pInstance_d)
   {
      Enumeration *pEnumInstance = 0;

      pEnumInstance = pInstance_d->getGroupEnumeration (fInDeviceSpecific);

      while (  pEnumInstance
            && pEnumInstance->hasMoreElements ()
            )
      {
         Enumeration *pElm = (Enumeration *)pEnumInstance->nextElement ();

         if (pElm)
         {
            pRet->addElement (pElm);
         }
      }

      delete pEnumInstance;
   }

   return pRet;
}

std::string * XMLDeviceInstance::
getJobPropertyType (PSZCRO pszKey)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pszKey = " << pszKey << std::endl;
#endif

   if (0 == strcmp (pszKey, KEY_DEVICENAME))
   {
      std::string *pRet = new std::string ("string ");

      addDeviceNameValue (pRet, false);

      XMLDevice *pDevice = dynamic_cast<XMLDevice *>(pDevice_d);

      if (!pDevice)
      {
         delete pRet;
         pRet = 0;
      }

      return pRet;
   }

   if (pInstance_d)
      return pInstance_d->getJobPropertyType (pszKey);
   else
      return 0;
}

std::string * XMLDeviceInstance::
getJobProperty (PSZCRO pszKey)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pszKey = " << pszKey << std::endl;
#endif

   if (0 == strcmp (pszKey, KEY_DEVICENAME))
   {
      std::string *pRet = new std::string ("");

      addDeviceNameValue (pRet, false);

      return pRet;
   }

   if (pInstance_d)
      return pInstance_d->getJobProperty (pszKey);
   else
      return 0;
}

std::string * XMLDeviceInstance::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": pszKey = " << pszKey << ", pszValue = " << pszValue << std::endl;
#endif

   if (0 == strcmp (pszKey, KEY_DEVICENAME))
   {
      std::string *pRet = new std::string (pszKey);

      if (pszValue)
      {
         *pRet += "=";
         *pRet += pszValue;
      }

      return pRet;
   }

   if (pInstance_d)
      return pInstance_d->translateKeyValue (pszKey,
                                             pszValue);
   else
      return 0;
}

bool XMLDeviceInstance::
setOutputStream (FILE *pFile)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->setOutputStream (pFile);
   else
      return false;
}

bool XMLDeviceInstance::
setErrorStream (FILE *pFile)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->setErrorStream (pFile);
   else
      return false;
}

bool XMLDeviceInstance::
setLanguage (int iLanguageID)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->setLanguage (iLanguageID);
   else
      return false;
}

bool XMLDeviceInstance::
hasDeviceOption (PSZCRO pszDeviceOption)
{
   if (pDevice_d)
   {
      return pDevice_d->hasDeviceOption (pszDeviceOption);
   }

   return false;
}

bool XMLDeviceInstance::
beginJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->beginJob ();
   else
      return false;
}

bool XMLDeviceInstance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->beginJob (fJobPropertiesChanged);
   else
      return false;
}

bool XMLDeviceInstance::
newFrame ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->newFrame ();
   else
      return false;
}

bool XMLDeviceInstance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->newFrame (fJobPropertiesChanged);
   else
      return false;
}

bool XMLDeviceInstance::
endJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->endJob ();
   else
      return false;
}

bool XMLDeviceInstance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << std::endl;
#endif

   if (pInstance_d)
      return pInstance_d->abortJob ();
   else
      return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceInstance * XMLDeviceInstance::
getDeviceInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": " << typeid (*pInstance_d).name () << std::endl;
#endif

   return pInstance_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void XMLDeviceInstance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceInstance::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceInstance: "
       << "hmodLibrary_d = " << std::hex << hmodLibrary_d
       << ", pfnDeleteInstance_d = " << (uintptr_t)pfnDeleteInstance_d << std::dec
       << ", "
       << DeviceInstance::toString (oss2)
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream&            os,
            const XMLDeviceInstance& const_self)
{
   XMLDeviceInstance& self = const_cast<XMLDeviceInstance&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

void XMLDeviceInstance::
addDeviceNameValue (std::string *pstringXMLDeviceName,
                    bool         fAddQuotes)
{
   if (pDevice_d)
   {
      XMLDevice *pDevice = dynamic_cast<XMLDevice *>(pDevice_d);

      if (pDevice)
      {
// @HACK
         PSZCRO pszRootPathOverride = getenv ("OMNI_XML_ROOT_PATH");
         PSZRO  pszXMLDeviceName    = pDevice->getXMLDeviceName ();

         if (fAddQuotes)
         {
            *pstringXMLDeviceName += "\"";
         }

         if (pszRootPathOverride)
         {
            PSZRO pszSlash = pszXMLDeviceName;

            do
            {
               pszSlash = strchr (pszSlash, '/');

               if (pszSlash)
               {
                  pszSlash         = pszSlash + 1;
                  pszXMLDeviceName = pszSlash;
               }

            } while (pszSlash);

            *pstringXMLDeviceName += pszRootPathOverride;
            *pstringXMLDeviceName += pszXMLDeviceName;
         }
         else
         {
            *pstringXMLDeviceName += pszXMLDeviceName;
         }

         if (fAddQuotes)
         {
            *pstringXMLDeviceName += "\"";
         }
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceInstance ()) DebugOutput::getErrorStream () << "XMLDeviceInstance::" << __FUNCTION__ << ": returns " << *pstringXMLDeviceName << std::endl;
#endif
}
