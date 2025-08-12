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
#include "DeviceTrimming.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "Trimming"
};

#define JOBPROP_TRIMMING       vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "Face",
   "Gutter",
   "None",
   "Tab",
   "Trim"
};

/* Function prototypes...
*/

DeviceTrimming::
DeviceTrimming (Device     *pDevice,
                PSZRO       pszJobProperties,
                BinaryData *pbdData)
{
   pDevice_d       = pDevice;
   pszTrimming_d   = 0;
   indexTrimming_d = -1;
   pbdData_d       = pbdData;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &pszTrimming_d, &indexTrimming_d);
   }
}

DeviceTrimming::
~DeviceTrimming ()
{
   if (pszTrimming_d)
   {
      free ((void *)pszTrimming_d);
   }

   delete pbdData_d;

   pDevice_d       = 0;
   pszTrimming_d   = 0;
   indexTrimming_d = -1;
   pbdData_d       = 0;
}

bool DeviceTrimming::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceTrimming::
isEqual (PSZCRO pszJobProperties)
{
   int indexTrimming = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexTrimming))
   {
      return indexTrimming == indexTrimming_d;
   }

   return false;
}

std::string * DeviceTrimming::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DTI1_"
       << indexTrimming_d;

   return new std::string (oss.str ());
}

DeviceTrimming * DeviceTrimming::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceTrimming *pTrimmingRet  = 0;
   int             indexTrimming = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DTI1_", 5))
   {
      if (  1             == sscanf (pszCreateHash, "DTI1_%d", &indexTrimming)
         && 0             <= indexTrimming
         && indexTrimming <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_TRIMMING
             << "="
             << vapszNames[indexTrimming];

         pTrimmingRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pTrimmingRet;
}

bool DeviceTrimming::
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

std::string * DeviceTrimming::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_TRIMMING))
   {
      if (pszTrimming_d)
      {
         std::ostringstream oss;

         oss << "string " << pszTrimming_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceTrimming::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_TRIMMING))
   {
      if (pszTrimming_d)
      {
         return new std::string (pszTrimming_d);
      }
   }

   return 0;
}

std::string * DeviceTrimming::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_TRIMMING, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_TRIMMING);
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
                                                    StringResource::STRINGGROUP_TRIMMINGS,
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

std::string * DeviceTrimming::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_TRIMMINGS,
                                              pszTrimming_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceTrimming::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszTrimmingID = 0;

   if (fInDeviceSpecific)
   {
      pszTrimmingID = getDeviceID ();
   }

   if (!pszTrimmingID)
   {
      pszTrimmingID = pszTrimming_d;
   }

   if (pszTrimmingID)
   {
      std::ostringstream oss;

      oss << JOBPROP_TRIMMING << "=" << pszTrimmingID;

      return new std::string (oss.str ());
   }

   return 0;
}

BinaryData * DeviceTrimming::
getData ()
{
   return pbdData_d;
}

PSZCRO DeviceTrimming::
getDeviceID ()
{
   return 0;
}

std::string * DeviceTrimming::
getTrimming ()
{
   if (pszTrimming_d)
   {
      return new std::string (pszTrimming_d);
   }

   return 0;
}

class TrimmingEnumerator : public Enumeration
{
public:

   TrimmingEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   TrimmingEnumerator ()
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

      oss << JOBPROP_TRIMMING << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceTrimming::
getAllEnumeration ()
{
   return new TrimmingEnumerator ();
}

#ifndef RETAIL

void DeviceTrimming::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceTrimming::
toString (std::ostringstream& oss)
{
   oss << "{DeviceTrimming: "
       << "pszTrimming_d = " << SAFE_PRINT_PSZ (pszTrimming_d)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceTrimming& const_self)
{
   DeviceTrimming&    self = const_cast<DeviceTrimming&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceTrimming::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszTrimming,
               int    *pindexTrimming)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_TRIMMING))
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

               if (pindexTrimming)
               {
                  *pindexTrimming = iMid;
               }
               if (ppszTrimming)
               {
                  *ppszTrimming = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszTrimming)
                  {
                     strcpy ((char *)*ppszTrimming, pszValue);
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

DefaultTrimming::
DefaultTrimming (Device     *pDevice,
                 PSZRO       pszJobProperties)
   : DeviceTrimming (pDevice,
                     pszJobProperties,
                     0)
{
}

DeviceTrimming * DefaultTrimming::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int indexTrimming = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexTrimming))
   {
      if (DEFAULT_INDEX_TRIMMING == indexTrimming)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultTrimming (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceTrimming * DefaultTrimming::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultTrimming::
isSupported (PSZCRO pszJobProperties)
{
   int indexTrimming = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexTrimming))
   {
      return DEFAULT_INDEX_TRIMMING == indexTrimming;
   }

   return false;
}

Enumeration * DefaultTrimming::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultTrimmingValueEnumerator : public Enumeration
   {
   public:
      DefaultTrimmingValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultTrimmingValueEnumerator ()
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

   return new DefaultTrimmingValueEnumerator ();
}

void DefaultTrimming::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_TRIMMING
       << "="
       << vapszNames[DEFAULT_INDEX_TRIMMING];
}

#ifndef RETAIL

void DefaultTrimming::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultTrimming::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultTrimming: "
       << DeviceTrimming::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultTrimming& const_self)
{
   DefaultTrimming&   self = const_cast<DefaultTrimming&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
