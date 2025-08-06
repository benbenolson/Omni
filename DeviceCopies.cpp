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
#include "DeviceCopies.hpp"
#include "JobProperties.hpp"

static PSZCRO vapszJobPropertyKeys[] = {
   "Copies"
};

#define JOBPROP_COPIES     vapszJobPropertyKeys[0]

DeviceCopies::
DeviceCopies (Device     *pDevice,
              PSZRO       pszJobProperties,
              BinaryData *pbdData,
              int         iMinimum,
              int         iMaximum,
              bool        fSimulationRequired)
{
   pDevice_d             = pDevice;
   iNumCopies_d          = 0;
   pbdData_d             = pbdData;
   iMinimum_d            = iMinimum;
   iMaximum_d            = iMaximum;
   fSimulationRequired_d = fSimulationRequired;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &iNumCopies_d);
   }
}

DeviceCopies::
~DeviceCopies ()
{
   delete pbdData_d;

   pDevice_d             = 0;
   iNumCopies_d          = 0;
   pbdData_d             = 0;
   iMinimum_d            = 0;
   iMaximum_d            = 0;
   fSimulationRequired_d = false;
}

bool DeviceCopies::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0);
}

bool DeviceCopies::
isEqual (PSZCRO pszJobProperties)
{
   int iCopies = -1;

   if (getComponents (pszJobProperties,
                      &iCopies))
   {
      return iCopies == iNumCopies_d;
   }

   return false;
}

std::string * DeviceCopies::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DCO1_"
       << iNumCopies_d;

   return new std::string (oss.str ());
}

DeviceCopies * DeviceCopies::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceCopies *pCopiesRet = 0;
   int           iCopies    = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DCO1_", 5))
   {
      if (1 == sscanf (pszCreateHash, "DCO1_%d", &iCopies))
      {
         std::ostringstream oss;

         oss << JOBPROP_COPIES
             << "="
             << iCopies;

         pCopiesRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pCopiesRet;
}

bool DeviceCopies::
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

std::string * DeviceCopies::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_COPIES))
   {
      std::ostringstream oss;

      oss << "integer " << iNumCopies_d;

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceCopies::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_COPIES))
   {
      std::ostringstream oss;

      oss << iNumCopies_d;

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceCopies::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_COPIES, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_COPIES);
      if (pszXLateKey)
      {
         oss << pszXLateKey;

         if (  pszValue
            && *pszValue
            )
         {
            std::string            stringValue      = pszValue;
            std::string::size_type posComma1        = std::string::npos;
            std::string::size_type posComma2        = std::string::npos;
            PSZRO                  pszXLateFrom     = 0;
            PSZRO                  pszXLateTo       = 0;
            PSZRO                  pszXLateInfinite = 0;

            pszXLateFrom = StringResource::getString (pDevice_d->getLanguageResource (),
                                                      StringResource::STRINGGROUP_DEVICE_COMMON,
                                                      StringResource::DEVICE_COMMON_FROM);
            if (!pszXLateFrom)
            {
               pszXLateFrom = "From";
            }
            pszXLateTo = StringResource::getString (pDevice_d->getLanguageResource (),
                                                    StringResource::STRINGGROUP_DEVICE_COMMON,
                                                    StringResource::DEVICE_COMMON_TO);
            if (!pszXLateTo)
            {
               pszXLateTo = "to";
            }
            pszXLateInfinite = StringResource::getString (pDevice_d->getLanguageResource (),
                                                          StringResource::STRINGGROUP_DEVICE_COMMON,
                                                          StringResource::DEVICE_COMMON_INFINITE);
            if (!pszXLateInfinite)
            {
               pszXLateInfinite = "infinite";
            }

            oss << "=";

            posComma1 = stringValue.find (",");
            if (posComma1 != std::string::npos)
            {
               posComma2 = stringValue.find (",", posComma1 + 1);

               if (posComma2 != std::string::npos)
               {
                  oss << pszXLateFrom
                      << " "
                      << stringValue.substr (posComma1 + 1, posComma2 - posComma1 - 1)
                      << " "
                      << pszXLateTo
                      << " "
                      << stringValue.substr (posComma2 + 1);
               }
               else
               {
                  oss << pszXLateFrom
                      << " "
                      << stringValue.substr (posComma1 + 1)
                      << " "
                      << pszXLateTo
                      << " "
                      << pszXLateInfinite;
               }
            }
            else
            {
               oss << pszValue;
            }
         }

         return new std::string (oss.str ());
      }
   }

   return pRet;
}

std::string * DeviceCopies::
getAllTranslation ()
{
   std::ostringstream oss;

   oss << iNumCopies_d;

   return new std::string (oss.str ());
}

std::string * DeviceCopies::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   oss << JOBPROP_COPIES << "=";

   if (  fInDeviceSpecific
      && getDeviceID ()
      )
   {
      oss << getDeviceID ();
   }
   else
   {
      oss << "{"
          << iNumCopies_d
          << ","
          << iMinimum_d
          << ","
          << iMaximum_d
          << "}";
   }

   return new std::string (oss.str ());
}

BinaryData * DeviceCopies::
getData ()
{
   return pbdData_d;
}

int DeviceCopies::
getMinimum ()
{
   return iMinimum_d;
}

int DeviceCopies::
getMaximum ()
{
   return iMaximum_d;
}

bool DeviceCopies::
needsSimulation ()
{
   return fSimulationRequired_d;
}

PSZCRO DeviceCopies::
getDeviceID ()
{
   return 0;
}

int DeviceCopies::
getNumCopies ()
{
   return iNumCopies_d;
}

#ifndef RETAIL

void DeviceCopies::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceCopies::
toString (std::ostringstream& oss)
{
   oss << "{DeviceCopies: "
       << "iNumCopies_d = " << iNumCopies_d
       << ", fSimulationRequired_d = " << fSimulationRequired_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceCopies& const_self)
{
   DeviceCopies&      self = const_cast<DeviceCopies&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceCopies::
getComponents (PSZCRO  pszJobProperties,
               int    *piValue)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_COPIES))
      {
         int iCopies = 0;

         iCopies = atoi (pszValue);

         if (piValue)
         {
            *piValue = iCopies;
         }

         if (iCopies)
         {
            fRet = true;
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

DefaultCopies::
DefaultCopies (Device *pDevice,
               PSZRO   pszJobProperties)
   : DeviceCopies (pDevice,
                   pszJobProperties,
                   0,
                   DEFAULT_MINIMUM_COPIES,
                   DEFAULT_MAXIMUM_COPIES,
                   DEFAULT_SIMULATION_REQUIRED)
{
}

DeviceCopies * DefaultCopies::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int iCopies = -1;

   if (getComponents (pszJobProperties,
                      &iCopies))
   {
      if (DEFAULT_COPIES == iCopies)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultCopies (pDevice,
                                   oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceCopies * DefaultCopies::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultCopies::
isSupported (PSZCRO pszJobProperties)
{
   int iCopies = -1;

   if (getComponents (pszJobProperties,
                      &iCopies))
   {
      return DEFAULT_COPIES == iCopies;
   }

   return false;
}

Enumeration * DefaultCopies::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultCopiesValueEnumerator : public Enumeration
   {
   public:
      DefaultCopiesValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultCopiesValueEnumerator ()
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

            oss << JOBPROP_COPIES
                << "={"
                << DEFAULT_COPIES
                << ","
                << DEFAULT_MINIMUM_COPIES
                << ","
                << DEFAULT_MAXIMUM_COPIES
                << "}";

            stringReturn_d = oss.str ();

            pvRet = (void *)new JobProperties (stringReturn_d);
         }

         return pvRet;
      }

   private:
      bool        fReturnedValue_d;
      std::string stringReturn_d;
   };

   return new DefaultCopiesValueEnumerator ();
}

void DefaultCopies::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_COPIES
       << "={"
       << DEFAULT_COPIES
       << ","
       << DEFAULT_MINIMUM_COPIES
       << ","
       << DEFAULT_MAXIMUM_COPIES
       << "}";
}

#ifndef RETAIL

void DefaultCopies::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultCopies::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultCopies: "
       << DeviceCopies::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultCopies& const_self)
{
   DefaultCopies&     self = const_cast<DefaultCopies&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
