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
#include "DeviceTray.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "InputTray"
};

#define JOBPROP_TRAY           vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "AnyLargeFormat",
   "AnySmallFormat",
   "AutoSelect",
   "Bottom",
   "BypassTray",
   "BypassTray-1",
   "BypassTray-2",
   "BypassTray-3",
   "BypassTray-4",
   "BypassTray-5",
   "BypassTray-6",
   "BypassTray-7",
   "BypassTray-8",
   "BypassTray-9",
   "Continuous",
   "Disc",
   "Disc-1",
   "Disc-2",
   "Disc-3",
   "Disc-4",
   "Disc-5",
   "Disc-6",
   "Disc-7",
   "Disc-8",
   "Disc-9",
   "Envelope",
   "Envelope-1",
   "Envelope-2",
   "Envelope-3",
   "Envelope-4",
   "Envelope-5",
   "Envelope-6",
   "Envelope-7",
   "Envelope-8",
   "Envelope-9",
   "Front",
   "InsertTray",
   "InsertTray-1",
   "InsertTray-2",
   "InsertTray-3",
   "InsertTray-4",
   "InsertTray-5",
   "InsertTray-6",
   "InsertTray-7",
   "InsertTray-8",
   "InsertTray-9",
   "LargeCapacity",
   "LargeCapacity-1",
   "LargeCapacity-2",
   "LargeCapacity-3",
   "LargeCapacity-4",
   "LargeCapacity-5",
   "LargeCapacity-6",
   "LargeCapacity-7",
   "LargeCapacity-8",
   "LargeCapacity-9",
   "Left",
   "Middle",
   "PanelSelect",
   "Rear",
   "Right",
   "Roll",
   "Roll-1",
   "Roll-2",
   "Roll-3",
   "Roll-4",
   "Roll-5",
   "Roll-6",
   "Roll-7",
   "Roll-8",
   "Roll-9",
   "Side",
   "Top",
   "Tray",
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

DeviceTray::
DeviceTray (Device     *pDevice,
            PSZRO       pszJobProperties,
            int         iType,
            BinaryData *pbdData)
{
   pDevice_d    = pDevice;
   pszTray_d    = 0;
   indexTray_d  = -1;
   iType_d      = iType;
   pbdData_d    = pbdData;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &pszTray_d, &indexTray_d);
   }
}

DeviceTray::
~DeviceTray ()
{
   if (pszTray_d)
   {
      free ((void *)pszTray_d);
   }

   delete pbdData_d;

   pDevice_d    = 0;
   pszTray_d    = 0;
   indexTray_d  = -1;
   iType_d      = 0;
   pbdData_d    = 0;
}

bool DeviceTray::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceTray::
isEqual (PSZCRO pszJobProperties)
{
   int indexTray = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexTray))
   {
      return indexTray == indexTray_d;
   }

   return false;
}

std::string * DeviceTray::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DTR1_"
       << indexTray_d;

   return new std::string (oss.str ());
}

DeviceTray * DeviceTray::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceTray *pTrayRet  = 0;
   int         indexTray = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DTR1_", 5))
   {
      if (  1         == sscanf (pszCreateHash, "DTR1_%d", &indexTray)
         && 0         <= indexTray
         && indexTray <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_TRAY
             << "="
             << vapszNames[indexTray];

         pTrayRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pTrayRet;
}

bool DeviceTray::
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

std::string * DeviceTray::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_TRAY))
   {
      if (pszTray_d)
      {
         std::ostringstream oss;

         oss << "string " << pszTray_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceTray::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_TRAY))
   {
      if (pszTray_d)
      {
         return new std::string (pszTray_d);
      }
   }

   return 0;
}

std::string * DeviceTray::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_TRAY, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_TRAY);
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
                                                    StringResource::STRINGGROUP_TRAYS,
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

std::string * DeviceTray::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_TRAYS,
                                              pszTray_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceTray::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszTrayID = 0;

   if (fInDeviceSpecific)
   {
      pszTrayID = getDeviceID ();
   }

   if (!pszTrayID)
   {
      pszTrayID = pszTray_d;
   }

   if (pszTrayID)
   {
      std::ostringstream oss;

      oss << JOBPROP_TRAY << "=" << pszTrayID;

      return new std::string (oss.str ());
   }

   return 0;
}

int DeviceTray::
getType ()
{
   return iType_d;
}

BinaryData * DeviceTray::
getData ()
{
   return pbdData_d;
}

PSZCRO DeviceTray::
getDeviceID ()
{
   return 0;
}

std::string * DeviceTray::
getInputTray ()
{
   if (pszTray_d)
   {
      return new std::string (pszTray_d);
   }

   return 0;
}

typedef struct _ReservedMap {
   PSZCRO pszName;
   int    iValue;
} RESERVEDMAP, *PRESERVEDMAP;

static const RESERVEDMAP vaReservedKeywords[] = {
   { "TRAY_TYPE_AUTO",         DeviceTray::TRAY_TYPE_AUTO         },
   { "TRAY_TYPE_MANUAL",       DeviceTray::TRAY_TYPE_MANUAL       },
   { "TRAY_TYPE_ZERO_MARGINS", DeviceTray::TRAY_TYPE_ZERO_MARGINS }
};

bool DeviceTray::
isReservedKeyword (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return true;
   }

   return false;
}

int DeviceTray::
getReservedValue (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return vaReservedKeywords[i].iValue;
   }

   return 0;
}

class TrayEnumerator : public Enumeration
{
public:

   TrayEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   TrayEnumerator ()
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
         return NULL;
      }

      std::ostringstream  oss;
      void               *pvRet = 0;

      oss << JOBPROP_TRAY << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceTray::
getAllEnumeration ()
{
   return new TrayEnumerator ();
}

#ifndef RETAIL

void DeviceTray::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceTray::
toString (std::ostringstream& oss)
{
   oss << "{DeviceTray: "
       << "pszTray_d = " << SAFE_PRINT_PSZ (pszTray_d)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceTray& const_self)
{
   DeviceTray&        self = const_cast<DeviceTray&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceTray::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszTray,
               int    *pindexTray)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_TRAY))
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

               if (pindexTray)
               {
                  *pindexTray = iMid;
               }
               if (ppszTray)
               {
                  *ppszTray = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszTray)
                  {
                     strcpy ((char *)*ppszTray, pszValue);
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

DefaultTray::
DefaultTray (Device *pDevice,
             PSZRO   pszJobProperties)
   : DeviceTray (pDevice,
                 pszJobProperties,
                 DEFAULT_TYPE,
                 0)
{
}

DeviceTray * DefaultTray::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int iTray = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iTray))
   {
      if (DEFAULT_TRAY_INDEX == iTray)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultTray (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceTray * DefaultTray::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultTray::
isSupported (PSZCRO pszJobProperties)
{
   int iTray = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iTray))
   {
      return DEFAULT_TRAY_INDEX == iTray;
   }

   return false;
}

Enumeration * DefaultTray::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultTrayValueEnumerator : public Enumeration
   {
   public:
      DefaultTrayValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultTrayValueEnumerator ()
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

   return new DefaultTrayValueEnumerator ();
}

void DefaultTray::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_TRAY << "=" << vapszNames[DEFAULT_TRAY_INDEX];
}
