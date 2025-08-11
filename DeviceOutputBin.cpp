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
#include "DeviceOutputBin.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "OutputBin"
};

#define JOBPROP_OUTPUTBIN    vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "Booklet",
   "Bottom",
   "Center",
   "FaceDown",
   "FaceUp",
   "FitMedia",
   "LargeCapacity",
   "Left",
   "Mailbox-1",
   "Mailbox-2",
   "Mailbox-3",
   "Mailbox-4",
   "Mailbox-5",
   "Mailbox-6",
   "Mailbox-7",
   "Mailbox-8",
   "Mailbox-9",
   "Rear",
   "Right",
   "Side",
   "Stacker-1",
   "Stacker-2",
   "Stacker-3",
   "Stacker-4",
   "Stacker-5",
   "Stacker-6",
   "Stacker-7",
   "Stacker-8",
   "Stacker-9",
   "Top",
   "Tray-1",
   "Tray-2",
   "Tray-3",
   "Tray-4",
   "Tray-5",
   "Tray-6",
   "Tray-7",
   "Tray-8",
   "Tray-9"
};

/* Function prototypes...
*/

DeviceOutputBin::
DeviceOutputBin (Device     *pDevice,
                 PSZRO       pszJobProperties,
                 BinaryData *pbdData)
{
   pDevice_d        = pDevice;
   pszOutputBin_d   = 0;
   indexOutputBin_d = -1;
   pbdData_d        = pbdData;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &pszOutputBin_d, &indexOutputBin_d);
   }
}

DeviceOutputBin::
~DeviceOutputBin ()
{
   if (pszOutputBin_d)
   {
      free ((void *)pszOutputBin_d);
   }

   delete pbdData_d;

   pDevice_d        = 0;
   pszOutputBin_d   = 0;
   indexOutputBin_d = -1;
   pbdData_d        = 0;
}

bool DeviceOutputBin::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceOutputBin::
isEqual (PSZCRO pszJobProperties)
{
   int indexOutputBin = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexOutputBin))
   {
      return indexOutputBin == indexOutputBin_d;
   }

   return false;
}

std::string * DeviceOutputBin::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DOB1_"
       << indexOutputBin_d;

   return new std::string (oss.str ());
}

DeviceOutputBin * DeviceOutputBin::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceOutputBin *pOutputBinRet  = 0;
   int              indexOutputBin = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DOB1_", 5))
   {
      if (  1              == sscanf (pszCreateHash, "DOB1_%d", &indexOutputBin)
         && 0              <= indexOutputBin
         && indexOutputBin <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_OUTPUTBIN
             << "="
             << vapszNames[indexOutputBin];

         pOutputBinRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pOutputBinRet;
}

bool DeviceOutputBin::
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

std::string * DeviceOutputBin::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_OUTPUTBIN))
   {
      if (pszOutputBin_d)
      {
         std::ostringstream oss;

         oss << "string " << pszOutputBin_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceOutputBin::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_OUTPUTBIN))
   {
      if (pszOutputBin_d)
      {
         return new std::string (pszOutputBin_d);
      }
   }

   return 0;
}

std::string * DeviceOutputBin::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_OUTPUTBIN, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_OUTPUTBIN);
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
                                                    StringResource::STRINGGROUP_OUTPUT_BINS,
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

std::string * DeviceOutputBin::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_OUTPUT_BINS,
                                              pszOutputBin_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceOutputBin::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszOutputBinID = 0;

   if (fInDeviceSpecific)
   {
      pszOutputBinID = getDeviceID ();
   }

   if (!pszOutputBinID)
   {
      pszOutputBinID = pszOutputBin_d;
   }

   if (pszOutputBinID)
   {
      std::ostringstream oss;

      oss << JOBPROP_OUTPUTBIN << "=" << pszOutputBinID;

      return new std::string (oss.str ());
   }

   return 0;
}

BinaryData * DeviceOutputBin::
getData ()
{
   return pbdData_d;
}

PSZCRO DeviceOutputBin::
getDeviceID ()
{
   return 0;
}

std::string * DeviceOutputBin::
getOutputBin ()
{
   if (pszOutputBin_d)
   {
      return new std::string (pszOutputBin_d);
   }

   return 0;
}

class OutputBinEnumerator : public Enumeration
{
public:

   OutputBinEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   OutputBinEnumerator ()
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
         return false;
      }

      std::ostringstream  oss;
      void               *pvRet = 0;

      oss << JOBPROP_OUTPUTBIN << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceOutputBin::
getAllEnumeration ()
{
   return new OutputBinEnumerator ();
}

#ifndef RETAIL

void DeviceOutputBin::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceOutputBin::
toString (std::ostringstream& oss)
{
   oss << "{DeviceOutputBin: "
       << "pszOutputBin_d = " << SAFE_PRINT_PSZ (pszOutputBin_d)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceOutputBin& const_self)
{
   DeviceOutputBin&   self = const_cast<DeviceOutputBin&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceOutputBin::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszOutputBin,
               int    *pindexOutputBin)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_OUTPUTBIN))
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

               if (pindexOutputBin)
               {
                  *pindexOutputBin = iMid;
               }
               if (ppszOutputBin)
               {
                  *ppszOutputBin = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszOutputBin)
                  {
                     strcpy ((char *)*ppszOutputBin, pszValue);
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

DefaultOutputBin::
DefaultOutputBin (Device     *pDevice,
                  PSZRO       pszJobProperties)
   : DeviceOutputBin (pDevice,
                      pszJobProperties,
                      0)
{
}

DeviceOutputBin * DefaultOutputBin::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int indexOutputBin = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexOutputBin))
   {
      if (DEFAULT_INDEX_OUTPUT_BIN == indexOutputBin)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultOutputBin (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceOutputBin * DefaultOutputBin::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultOutputBin::
isSupported (PSZCRO pszJobProperties)
{
   int indexOutputBin = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexOutputBin))
   {
      return DEFAULT_INDEX_OUTPUT_BIN == indexOutputBin;
   }

   return false;
}

Enumeration * DefaultOutputBin::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultOutputBinValueEnumerator : public Enumeration
   {
   public:
      DefaultOutputBinValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultOutputBinValueEnumerator ()
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

   return new DefaultOutputBinValueEnumerator ();
}

void DefaultOutputBin::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_OUTPUTBIN
       << "="
       << vapszNames[DEFAULT_INDEX_OUTPUT_BIN];
}

#ifndef RETAIL

void DefaultOutputBin::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultOutputBin::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultOutputBin: "
       << DeviceOutputBin::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultOutputBin& const_self)
{
   DefaultOutputBin&  self = const_cast<DefaultOutputBin&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
