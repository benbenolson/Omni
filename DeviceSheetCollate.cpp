/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2002
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
#include "DeviceSheetCollate.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "SheetCollate"
};

#define JOBPROP_SHEETCOLLATE  vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "SheetAndJobCollated",
   "SheetCollated",
   "SheetUncollated"
};

/* Function prototypes...
*/

DeviceSheetCollate::
DeviceSheetCollate (Device     *pDevice,
                    PSZRO       pszJobProperties,
                    BinaryData *pbdData)
{
   pDevice_d           = pDevice;
   pszSheetCollate_d   = 0;
   indexSheetCollate_d = -1;
   pbdData_d           = pbdData;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties,
                     &pszSheetCollate_d,
                     &indexSheetCollate_d);
   }
}

DeviceSheetCollate::
~DeviceSheetCollate ()
{
   if (pszSheetCollate_d)
   {
      free ((void *)pszSheetCollate_d);
   }

   delete pbdData_d;

   pDevice_d           = 0;
   pszSheetCollate_d   = 0;
   indexSheetCollate_d = -1;
   pbdData_d           = 0;
}

bool DeviceSheetCollate::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceSheetCollate::
isEqual (PSZCRO pszJobProperties)
{
   int indexSheetCollate = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexSheetCollate))
   {
      return indexSheetCollate == indexSheetCollate_d;
   }

   return false;
}

std::string * DeviceSheetCollate::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DSH1_"
       << indexSheetCollate_d;

   return new std::string (oss.str ());
}

DeviceSheetCollate * DeviceSheetCollate::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceSheetCollate *pSheetCollateRet  = 0;
   int                 indexSheetCollate = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DSH1_", 5))
   {
      if (  1                 == sscanf (pszCreateHash, "DSH1_%d", &indexSheetCollate)
         && 0                 <= indexSheetCollate
         && indexSheetCollate <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_SHEETCOLLATE
             << "="
             << vapszNames[indexSheetCollate];

         pSheetCollateRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pSheetCollateRet;
}

bool DeviceSheetCollate::
handlesKey (PSZCRO pszKey)
{
   if (  !pszKey
      || !*pszKey
      )
   {
      return false;
   }

   for (int i = 0; i < (int)dimof (vapszJobPropertyKeys); i++)
   {
      if (0 == strcmp (pszKey, vapszJobPropertyKeys[i]))
      {
         return true;
      }
   }

   return false;
}

std::string * DeviceSheetCollate::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_SHEETCOLLATE))
   {
      if (pszSheetCollate_d)
      {
         std::ostringstream oss;

         oss << "string " << pszSheetCollate_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceSheetCollate::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_SHEETCOLLATE))
   {
      if (pszSheetCollate_d)
      {
         return new std::string (pszSheetCollate_d);
      }
   }

   return 0;
}

std::string * DeviceSheetCollate::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_SHEETCOLLATE, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_SHEETCOLLATE);
      if (pszXLateKey)
      {
         pRet = new std::string (pszXLateKey);
      }

      if (  pszValue
         && *pszValue
         && pRet
         )
      {
         PSZRO pszXLateValue = 0;

         pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                                    StringResource::STRINGGROUP_SHEET_COLLATES,
                                                    pszValue);

         if (pszXLateValue)
         {
            *pRet += "=";
            *pRet += pszXLateValue;
         }
      }
   }

   return pRet;
}

std::string * DeviceSheetCollate::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_SHEET_COLLATES,
                                              pszSheetCollate_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceSheetCollate::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszSheetCollateID = 0;

   if (fInDeviceSpecific)
   {
      pszSheetCollateID = getDeviceID ();
   }

   if (!pszSheetCollateID)
   {
      pszSheetCollateID = pszSheetCollate_d;
   }

   if (pszSheetCollateID)
   {
      std::ostringstream oss;

      oss << JOBPROP_SHEETCOLLATE << "=" << pszSheetCollateID;

      return new std::string (oss.str ());
   }

   return 0;
}

BinaryData * DeviceSheetCollate::
getData ()
{
   return pbdData_d;
}

PSZCRO DeviceSheetCollate::
getDeviceID ()
{
   return 0;
}

std::string * DeviceSheetCollate::
getSheetCollate ()
{
   if (pszSheetCollate_d)
   {
      return new std::string (pszSheetCollate_d);
   }

   return 0;
}

class SheetCollateEnumerator : public Enumeration
{
public:

   SheetCollateEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   SheetCollateEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return iIndex_d < (int)dimof (vapszNames);
   }

   virtual void *
   nextElement ()
   {
      if (!hasMoreElements ())
      {
         return nullptr;
      }

      std::ostringstream  oss;
      void               *pvRet = 0;

      oss << JOBPROP_SHEETCOLLATE << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceSheetCollate::
getAllEnumeration ()
{
   return new SheetCollateEnumerator ();
}

#ifndef RETAIL

void DeviceSheetCollate::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceSheetCollate::
toString (std::ostringstream& oss)
{
   oss << "{DeviceSheetCollate: "
       << "pszSheetCollate_d = " << SAFE_PRINT_PSZ (pszSheetCollate_d)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceSheetCollate& const_self)
{
   DeviceSheetCollate&   self = const_cast<DeviceSheetCollate&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceSheetCollate::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszSheetCollate,
               int    *pindexSheetCollate)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_SHEETCOLLATE))
      {
         int iLow  = 0;
         int iMid  = (int)dimof (vapszNames) / 2;
         int iHigh = (int)dimof (vapszNames) - 1;
         int iResult;

         while (iLow <= iHigh)
         {
            iResult = strcmp (pszValue, vapszNames[iMid]);

            if (0 == iResult)
            {
               fRet = true;

               if (pindexSheetCollate)
               {
                  *pindexSheetCollate = iMid;
               }
               if (ppszSheetCollate)
               {
                  *ppszSheetCollate = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszSheetCollate)
                  {
                     strcpy ((char *)*ppszSheetCollate, pszValue);
                  }
               }
               break;
            }
            else if (0 > iResult)
            {
               // str1 < str2
               iHigh = iMid - 1;
               iMid  = iLow + (iHigh - iLow) / 2;
            }
            else // (0 < iResult)
            {
               // str1 > str2
               iLow  = iMid + 1;
               iMid  = iLow + (iHigh - iLow) / 2;
            }
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

DefaultSheetCollate::
DefaultSheetCollate (Device     *pDevice,
                     PSZRO       pszJobProperties)
   : DeviceSheetCollate (pDevice,
                         pszJobProperties,
                         0)
{
}

DeviceSheetCollate * DefaultSheetCollate::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int indexSheetCollate = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexSheetCollate))
   {
      if (DEFAULT_INDEX_SHEET_COLLATE == indexSheetCollate)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultSheetCollate (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceSheetCollate * DefaultSheetCollate::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultSheetCollate::
isSupported (PSZCRO pszJobProperties)
{
   int indexSheetCollate = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexSheetCollate))
   {
      return DEFAULT_INDEX_SHEET_COLLATE == indexSheetCollate;
   }

   return false;
}

Enumeration * DefaultSheetCollate::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultSheetCollateValueEnumerator : public Enumeration
   {
   public:
      DefaultSheetCollateValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultSheetCollateValueEnumerator ()
      {
         fReturnedValue_d = true;
      }

      virtual bool hasMoreElements ()
      {
         return !fReturnedValue_d;
      }

      virtual void *nextElement ()
      {
         void *pvRet = 0;

         if (!fReturnedValue_d)
         {
            std::ostringstream oss;

            fReturnedValue_d = true;

            writeDefaultJP (oss);

            stringReturn_d = oss.str ();

            pvRet = (void *)new JobProperties (stringReturn_d);
         }

         return pvRet;
      }

   private:
      bool        fReturnedValue_d;
      std::string stringReturn_d;
   };

   return new DefaultSheetCollateValueEnumerator ();
}

void DefaultSheetCollate::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_SHEETCOLLATE
       << "="
       << vapszNames[DEFAULT_INDEX_SHEET_COLLATE];
}

#ifndef RETAIL

void DefaultSheetCollate::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultSheetCollate::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultSheetCollate: "
       << DeviceSheetCollate::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultSheetCollate& const_self)
{
   DefaultSheetCollate& self = const_cast<DefaultSheetCollate&>(const_self);
   std::ostringstream   oss;

   os << self.toString (oss);

   return os;
}
