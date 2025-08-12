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
#include "DeviceOrientation.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "Rotation"
};

#define JOBPROP_ROTATION     vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "Landscape",
   "Portrait",
   "ReverseLandscape",
   "ReversePortrait"
};

/* Function prototypes...
*/

DeviceOrientation::
DeviceOrientation (Device *pDevice,
                   PSZRO   pszJobProperties,
                   bool    fSimulationRequired)
{
   pDevice_d             = pDevice;
   pszRotation_d         = 0;
   indexRotation_d       = -1;
   fSimulationRequired_d = fSimulationRequired;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &pszRotation_d, &indexRotation_d);
   }
}

DeviceOrientation::
~DeviceOrientation ()
{
   if (pszRotation_d)
   {
      free ((void *)pszRotation_d);
   }

   pDevice_d             = 0;
   pszRotation_d         = 0;
   indexRotation_d       = -1;
   fSimulationRequired_d = false;
}

bool DeviceOrientation::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceOrientation::
isEqual (PSZCRO pszJobProperties)
{
   int indexRotation = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexRotation))
   {
      return indexRotation == indexRotation_d;
   }

   return false;
}

std::string * DeviceOrientation::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DOR1_"
       << indexRotation_d;

   return new std::string (oss.str ());
}

DeviceOrientation * DeviceOrientation::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceOrientation *pOrientationRet = 0;
   int                indexRotation   = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DOR1_", 5))
   {
      if (  1             == sscanf (pszCreateHash, "DOR1_%d", &indexRotation)
         && 0             <= indexRotation
         && indexRotation <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_ROTATION
             << "="
             << vapszNames[indexRotation];

         pOrientationRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pOrientationRet;
}

bool DeviceOrientation::
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

std::string * DeviceOrientation::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_ROTATION))
   {
      if (pszRotation_d)
      {
         std::ostringstream oss;

         oss << "string " << pszRotation_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceOrientation::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_ROTATION))
   {
      if (pszRotation_d)
      {
         return new std::string (pszRotation_d);
      }
   }

   return 0;
}

std::string * DeviceOrientation::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_ROTATION, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_ORIENTATION);
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
                                                    StringResource::STRINGGROUP_ORIENTATIONS,
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

std::string * DeviceOrientation::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_ORIENTATIONS,
                                              pszRotation_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceOrientation::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszRotationID = 0;

   if (fInDeviceSpecific)
   {
      pszRotationID = getDeviceID ();
   }

   if (!pszRotationID)
   {
      pszRotationID = pszRotation_d;
   }

   if (pszRotationID)
   {
      std::ostringstream oss;

      oss << JOBPROP_ROTATION << "=" << pszRotationID;

      return new std::string (oss.str ());
   }

   return 0;
}

bool DeviceOrientation::
needsSimulation ()
{
   return fSimulationRequired_d;
}

PSZCRO DeviceOrientation::
getDeviceID ()
{
   return 0;
}

std::string * DeviceOrientation::
getRotation ()
{
   if (pszRotation_d)
   {
      return new std::string (pszRotation_d);
   }

   return 0;
}

class OrientationEnumerator : public Enumeration
{
public:

   OrientationEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   OrientationEnumerator ()
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

      oss << JOBPROP_ROTATION << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceOrientation::
getAllEnumeration ()
{
   return new OrientationEnumerator ();
}

#ifndef RETAIL

void DeviceOrientation::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceOrientation::
toString (std::ostringstream& oss)
{
   oss << "{DeviceOrientation: "
       << "pszRotation_d = " << SAFE_PRINT_PSZ (pszRotation_d)
       << ", fSimulationRequired_d = " << fSimulationRequired_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceOrientation& const_self)
{
   DeviceOrientation& self = const_cast<DeviceOrientation&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceOrientation::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszRotation,
               int    *pindexRotation)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_ROTATION))
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

               if (pindexRotation)
               {
                  *pindexRotation = iMid;
               }
               if (ppszRotation)
               {
                  *ppszRotation = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszRotation)
                  {
                     strcpy ((char *)*ppszRotation, pszValue);
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

DefaultOrientation::
DefaultOrientation (Device *pDevice,
                    PSZRO   pszJobProperties)
   : DeviceOrientation (pDevice,
                        pszJobProperties,
                        DEFAULT_SIMULATION_REQUIRED)
{
}

DeviceOrientation * DefaultOrientation::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int iOrientation = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iOrientation))
   {
      if (DEFAULT_ORIENTATION_INDEX == iOrientation)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultOrientation (pDevice,
                                  oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceOrientation * DefaultOrientation::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultOrientation::
isSupported (PSZCRO pszJobProperties)
{
   int iOrientation = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iOrientation))
   {
      return DEFAULT_ORIENTATION_INDEX == iOrientation;
   }

   return false;
}

Enumeration * DefaultOrientation::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultOrientationValueEnumerator : public Enumeration
   {
   public:
      DefaultOrientationValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultOrientationValueEnumerator ()
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

   return new DefaultOrientationValueEnumerator ();
}

void DefaultOrientation::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_ROTATION << "=" << vapszNames[DEFAULT_ORIENTATION_INDEX];
}
