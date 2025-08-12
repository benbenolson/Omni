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
#include "DeviceSide.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "Sides"
};

#define JOBPROP_SIDES          vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "OneSidedBackflipX",
   "OneSidedBackflipY",
   "OneSidedFront",
   "TwoSidedFlipX",
   "TwoSidedFlipY"
};

/* Function prototypes...
*/

DeviceSide::
DeviceSide (Device     *pDevice,
            PSZRO       pszJobProperties,
            BinaryData *pbdData,
            bool        fSimulationRequired)
{
   pDevice_d             = pDevice;
   pszSide_d             = 0;
   indexSide_d           = -1;
   pbdData_d             = pbdData;
   fSimulationRequired_d = fSimulationRequired;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &pszSide_d, &indexSide_d);
   }
}

DeviceSide::
~DeviceSide ()
{
   if (pszSide_d)
   {
      free ((void *)pszSide_d);
   }

   delete pbdData_d;

   pDevice_d             = 0;
   pszSide_d             = 0;
   indexSide_d           = -1;
   pbdData_d             = 0;
   fSimulationRequired_d = false;
}

bool DeviceSide::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceSide::
isEqual (PSZCRO pszJobProperties)
{
   int indexSide = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexSide))
   {
      return indexSide == indexSide_d;
   }

   return false;
}

std::string * DeviceSide::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DSI1_"
       << indexSide_d;

   return new std::string (oss.str ());
}

DeviceSide * DeviceSide::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceSide *pSideRet  = 0;
   int         indexSide = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DSI1_", 5))
   {
      if (  1         == sscanf (pszCreateHash, "DSI1_%d", &indexSide)
         && 0         <= indexSide
         && indexSide <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_SIDES
             << "="
             << vapszNames[indexSide];

         pSideRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pSideRet;
}

bool DeviceSide::
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

std::string * DeviceSide::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_SIDES))
   {
      if (pszSide_d)
      {
         std::ostringstream oss;

         oss << "string " << pszSide_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceSide::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_SIDES))
   {
      if (pszSide_d)
      {
         return new std::string (pszSide_d);
      }
   }

   return 0;
}

std::string * DeviceSide::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_SIDES, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_SIDE);
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
                                                    StringResource::STRINGGROUP_SIDES,
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

std::string * DeviceSide::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_SIDES,
                                              pszSide_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceSide::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszSideID = 0;

   if (fInDeviceSpecific)
   {
      pszSideID = getDeviceID ();
   }

   if (!pszSideID)
   {
      pszSideID = pszSide_d;
   }

   if (pszSideID)
   {
      std::ostringstream oss;

      oss << JOBPROP_SIDES << "=" << pszSideID;

      return new std::string (oss.str ());
   }

   return 0;
}

BinaryData * DeviceSide::
getData ()
{
   return pbdData_d;
}

bool DeviceSide::
needsSimulation ()
{
   return fSimulationRequired_d;
}

PSZCRO DeviceSide::
getDeviceID ()
{
   return 0;
}

std::string * DeviceSide::
getSide ()
{
   if (pszSide_d)
   {
      return new std::string (pszSide_d);
   }

   return 0;
}

class SideEnumerator : public Enumeration
{
public:

   SideEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   SideEnumerator ()
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

      oss << JOBPROP_SIDES << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceSide::
getAllEnumeration ()
{
   return new SideEnumerator ();
}

#ifndef RETAIL

void DeviceSide::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceSide::
toString (std::ostringstream& oss)
{
   oss << "{DeviceSide: "
       << "pszSide_d = " << SAFE_PRINT_PSZ (pszSide_d)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceSide& const_self)
{
   DeviceSide&        self = const_cast<DeviceSide&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceSide::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszSide,
               int    *pindexSide)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_SIDES))
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

               if (pindexSide)
               {
                  *pindexSide = iMid;
               }
               if (ppszSide)
               {
                  *ppszSide = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszSide)
                  {
                     strcpy ((char *)*ppszSide, pszValue);
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

DefaultSide::
DefaultSide (Device     *pDevice,
             PSZRO       pszJobProperties)
   : DeviceSide (pDevice,
                 pszJobProperties,
                 0)
{
}

DeviceSide * DefaultSide::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int indexSide = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexSide))
   {
      if (DEFAULT_INDEX_SIDE == indexSide)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultSide (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceSide * DefaultSide::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultSide::
isSupported (PSZCRO pszJobProperties)
{
   int indexSide = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexSide))
   {
      return DEFAULT_INDEX_SIDE == indexSide;
   }

   return false;
}

Enumeration * DefaultSide::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultSideValueEnumerator : public Enumeration
   {
   public:
      DefaultSideValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultSideValueEnumerator ()
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

   return new DefaultSideValueEnumerator ();
}

void DefaultSide::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_SIDES
       << "="
       << vapszNames[DEFAULT_INDEX_SIDE];
}

#ifndef RETAIL

void DefaultSide::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultSide::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultSide: "
       << DeviceSide::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultSide& const_self)
{
   DefaultSide&       self = const_cast<DefaultSide&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
