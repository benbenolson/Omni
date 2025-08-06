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
#include "DeviceJogging.hpp"
#include "StringResource.hpp"

#include <cstdio>
#include <cstring>

/* Function prototypes...
*/
static int    omniNameToID          (PSZCRO pszId);
static int    standardNameToID      (PSZCRO pszId);

#define STRING_OF(x) (apszJoggingNames[x - DeviceJogging::JOGGING_UNLISTED])

static PSZCRO apszJoggingNames[] = {
   "JOGGING_UNLISTED",
   "JOGGING_OFF",
   "JOGGING_ON"
};

#ifdef USE_STANDARD_NAMES

static PSZCRO apszStandardJoggingNames[] = {
   "unlisted",
   "jogging-off",
   "jogging-on"
};

#endif

DeviceJogging::
DeviceJogging (Device     *pDevice,
               int         id,
               BinaryData *pbdData,
               bool        fSimulationRequired)
{
   pDevice_d             = pDevice;
   id_d                  = id;
   pbdData_d             = pbdData;
   fSimulationRequired_d = fSimulationRequired;
}

DeviceJogging::
~DeviceJogging ()
{
   pDevice_d             = 0;
   id_d                  = 0;
   pbdData_d             = 0;
   fSimulationRequired_d = false;
}

static PSZCRO
getConvertedName (Device *pDevice,
                  int     id)
{
   switch (id)
   {
   case DeviceJogging::JOGGING_OFF:
   {
      id = StringResource::DEVICE_COMMON_OFF;
      break;
   }

   case DeviceJogging::JOGGING_ON:
   {
      id = StringResource::DEVICE_COMMON_ON;
      break;
   }

   case DeviceJogging::JOGGING_UNLISTED:
   default:
   {
      id = StringResource::DEVICE_COMMON_UNLISTED;
      break;
   }
   }

   return StringResource::getString (pDevice->getLanguageResource (),
                                     StringResource::STRINGGROUP_DEVICE_COMMON,
                                     id);
}

PSZCRO DeviceJogging::
getName (Device *pDevice,
         int     id)
{
   return getConvertedName (pDevice, id);
}

PSZCRO DeviceJogging::
getName ()
{
   return getConvertedName (pDevice_d, id_d);
}

bool DeviceJogging::
isID (int id)
{
   return id == id_d;
}

int DeviceJogging::
getID ()
{
   return id_d;
}

PSZCRO DeviceJogging::
getDeviceID ()
{
   return 0;
}

BinaryData * DeviceJogging::
getData ()
{
   return pbdData_d;
}

bool DeviceJogging::
needsSimulation ()
{
   return fSimulationRequired_d;
}

DeviceJogging * DeviceJogging::
create (PSZCRO pszId)
{
   return this->createV (nameToID (pszId));
}

int DeviceJogging::
nameToID (PSZCRO pszId)
{
   if (  !pszId
      || !*pszId
      )
   {
      return JOGGING_UNLISTED;
   }

   int iJoggingID = omniNameToID (pszId);

   if (JOGGING_UNLISTED != iJoggingID)
      return iJoggingID;

   iJoggingID = standardNameToID (pszId);

   return iJoggingID;
}

int
omniNameToID (PSZCRO pszId)
{
   static short int asiEntries[] = {
      DeviceJogging::JOGGING_OFF,
      DeviceJogging::JOGGING_ON
   };

   int iLow  = 0;
   int iMid  = (int)dimof (asiEntries) / 2;
   int iHigh = (int)dimof (asiEntries) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, STRING_OF (asiEntries[iMid]));

      if (0 == iResult)
      {
         return asiEntries[iMid];
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

   return DeviceJogging::JOGGING_UNLISTED;
}

typedef struct _JoggingMap {
   int    iOmniId;
   PSZCRO pszStandardName;
} COLLATIONMAP, *PCOLLATIONMAP;

static COLLATIONMAP vaStandardJoggingMapping[] = {
   { DeviceJogging::JOGGING_OFF,      "jogging-off" },
   { DeviceJogging::JOGGING_ON,       "jogging-on"  },
   { DeviceJogging::JOGGING_UNLISTED, "unlisted"    }
};

int
standardNameToID (PSZCRO pszId)
{
   int   iLow    = 0;
   int   iMid    = (int)dimof (vaStandardJoggingMapping) / 2;
   int   iHigh   = (int)dimof (vaStandardJoggingMapping) - 1;
   int   iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, vaStandardJoggingMapping[iMid].pszStandardName);

      if (0 == iResult)
      {
         return vaStandardJoggingMapping[iMid].iOmniId;
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

   return DeviceJogging::JOGGING_UNLISTED;
}

PSZCRO DeviceJogging::
IDToName (int id)
{
   id -= JOGGING_UNLISTED;

   if (  0 <= id
      && id < (int)dimof (apszJoggingNames)
      )
   {
#ifdef USE_STANDARD_NAMES
      return apszStandardJoggingNames[id];
#else
      return apszJoggingNames[id];
#endif
   }
   else
   {
      static char achUnknown[21];

      sprintf (achUnknown, "Unknown (%d)", id + JOGGING_OFF);

      return achUnknown;
   }
}

#ifndef RETAIL

void DeviceJogging::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceJogging::
toString (std::ostringstream& oss)
{
   oss << "{DeviceJogging: "
       << STRING_OF (id_d)
       << ", fSimulationRequired_d = " << fSimulationRequired_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceJogging& const_self)
{
   DeviceJogging&      self = const_cast<DeviceJogging&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
