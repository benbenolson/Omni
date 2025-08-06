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
#include "DeviceBooklet.hpp"
#include "StringResource.hpp"

#include <cstdio>
#include <cstring>

/* Function prototypes...
*/
static int    omniNameToID          (PSZCRO pszId);
static int    standardNameToID      (PSZCRO pszId);

#define STRING_OF(x) (apszBookletNames[x - DeviceBooklet::BOOKLET_UNLISTED])

static PSZCRO apszBookletNames[] = {
   "BOOKLET_UNLISTED",
   "BOOKLET_UNSORTED",
   "BOOKLET_SORTED"
};

#ifdef USE_STANDARD_NAMES

static PSZCRO apszStandardBookletNames[] = {
   "unlisted",
   "booklet-unsorted",
   "booklet-sorted"
};

#endif

DeviceBooklet::
DeviceBooklet (Device     *pDevice,
               int         id,
               BinaryData *pbdData,
               bool        fSimulationRequired)
{
   pDevice_d             = pDevice;
   id_d                  = id;
   pbdData_d             = pbdData;
   fSimulationRequired_d = fSimulationRequired;
}

DeviceBooklet::
~DeviceBooklet ()
{
   delete pbdData_d;

   pDevice_d             = 0;
   id_d                  = 0;
   pbdData_d             = 0;
   fSimulationRequired_d = false;
}

PSZCRO DeviceBooklet::
getName (Device *pDevice,
         int     id)
{
   return StringResource::getString (pDevice->getLanguageResource (),
                                     StringResource::STRINGGROUP_BOOKLETS,
                                     id);
}

PSZCRO DeviceBooklet::
getName ()
{
   return StringResource::getString (pDevice_d->getLanguageResource (),
                                     StringResource::STRINGGROUP_BOOKLETS,
                                     id_d);
}

bool DeviceBooklet::
isID (int id)
{
   return id == id_d;
}

int DeviceBooklet::
getID ()
{
   return id_d;
}

PSZCRO DeviceBooklet::
getDeviceID ()
{
   return 0;
}

BinaryData * DeviceBooklet::
getData ()
{
   return pbdData_d;
}

bool DeviceBooklet::
needsSimulation ()
{
   return fSimulationRequired_d;
}

DeviceBooklet * DeviceBooklet::
create (PSZCRO pszId)
{
   return this->createV (nameToID (pszId));
}

int DeviceBooklet::
nameToID (PSZCRO pszId)
{
   if (  !pszId
      || !*pszId
      )
   {
      return BOOKLET_UNLISTED;
   }

   int iBookletID = omniNameToID (pszId);

   if (BOOKLET_UNLISTED != iBookletID)
      return iBookletID;

   iBookletID = standardNameToID (pszId);

   return iBookletID;
}

int
omniNameToID (PSZCRO pszId)
{
   static short int asiEntries[] = {
      DeviceBooklet::BOOKLET_SORTED,
      DeviceBooklet::BOOKLET_UNSORTED
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

   return DeviceBooklet::BOOKLET_UNLISTED;
}

typedef struct _BookletMap {
   int    iOmniId;
   PSZCRO pszStandardName;
} COLLATIONMAP, *PCOLLATIONMAP;

static COLLATIONMAP vaStandardBookletMapping[] = {
   { DeviceBooklet::BOOKLET_SORTED,   "booklet-sorted"   },
   { DeviceBooklet::BOOKLET_UNSORTED, "booklet-unsorted" },
   { DeviceBooklet::BOOKLET_UNLISTED, "unlisted"         }
};

int
standardNameToID (PSZCRO pszId)
{
   int   iLow    = 0;
   int   iMid    = (int)dimof (vaStandardBookletMapping) / 2;
   int   iHigh   = (int)dimof (vaStandardBookletMapping) - 1;
   int   iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, vaStandardBookletMapping[iMid].pszStandardName);

      if (0 == iResult)
      {
         return vaStandardBookletMapping[iMid].iOmniId;
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

   return DeviceBooklet::BOOKLET_UNLISTED;
}

PSZCRO DeviceBooklet::
IDToName (int id)
{
   id -= BOOKLET_UNLISTED;

   if (  0 <= id
      && id < (int)dimof (apszBookletNames)
      )
   {
#ifdef USE_STANDARD_NAMES
      return apszStandardBookletNames[id];
#else
      return apszBookletNames[id];
#endif
   }
   else
   {
      static char achUnknown[21];

      sprintf (achUnknown, "Unknown (%d)", id + BOOKLET_UNSORTED);

      return achUnknown;
   }
}

#ifndef RETAIL

void DeviceBooklet::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceBooklet::
toString (std::ostringstream& oss)
{
   oss << "{DeviceBooklet: "
       << STRING_OF (id_d)
       << ", fSimulationRequired_d = " << fSimulationRequired_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceBooklet& const_self)
{
   DeviceBooklet&      self = const_cast<DeviceBooklet&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
