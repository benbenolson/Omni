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
#include "DeviceScaling.hpp"
#include "JobProperties.hpp"

#include <errno.h>

static PSZCRO vapszJobPropertyKeys[] = {
   "ScalingType",
   "ScalingPercentage"
};

#define JOBPROP_SCALING_TYPE       vapszJobPropertyKeys[0]
#define JOBPROP_SCALING_PERCENTAGE vapszJobPropertyKeys[1]

static PSZCRO vapszNames[] = {
   "Clip",
   "FitToPage",
   "RotateAndOrFit",
   "None"
};

DeviceScaling::
DeviceScaling (Device     *pDevice,
               PSZRO       pszJobProperties,
               BinaryData *pbdData,
               double      dMinimumScale,
               double      dMaximumScale)
{
   pDevice_d            = pDevice;
   pszScalingType_d     = 0;
   indexScaling_d       = -1;
   dScalingPercentage_d = 0.0;
   dMinimumScale_d      = dMinimumScale;
   dMaximumScale_d      = dMaximumScale;
   pbdData_d            = pbdData;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties,
                     &pszScalingType_d,
                     &indexScaling_d,
                     &dScalingPercentage_d);
   }
}

DeviceScaling::
~DeviceScaling ()
{
   if (pszScalingType_d)
   {
      free ((void *)pszScalingType_d);
   }

   pDevice_d            = 0;
   pszScalingType_d     = 0;
   indexScaling_d       = -1;
   dScalingPercentage_d = 0.0;
   dMinimumScale_d      = 0.0;
   dMaximumScale_d      = 0.0;
   pbdData_d            = 0;
}

bool DeviceScaling::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0, 0);
}

bool DeviceScaling::
isEqual (PSZCRO pszJobProperties)
{
   int    indexScaling       = -1;
   double dScalingPercentage = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexScaling,
                      &dScalingPercentage))
   {
      return    indexScaling       == indexScaling_d
             && dScalingPercentage == dScalingPercentage_d;
   }

   return false;
}

std::string * DeviceScaling::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DSC1_"
       << indexScaling_d
       << "_"
       << dScalingPercentage_d;

   return new std::string (oss.str ());
}

DeviceScaling * DeviceScaling::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceScaling *pScalingRet        = 0;
   int            indexScaling       = -1;
   double         dScalingPercentage = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DSC1_", 5))
   {
      PSZRO pszScan = pszCreateHash + 5;

      if (0 == sscanf (pszScan, "%d", &indexScaling))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%lf", &dScalingPercentage))
      {
         return 0;
      }

      if (  indexScaling <  0
         || indexScaling >= (int)dimof (vapszNames)
         )
      {
         return 0;
      }

      std::ostringstream oss;

      oss << JOBPROP_SCALING_TYPE
          << "="
          << vapszNames[indexScaling];

      oss << " "
          << JOBPROP_SCALING_PERCENTAGE
          << "="
          << dScalingPercentage;

      pScalingRet = create (pDevice, oss.str ().c_str ());
   }

   return pScalingRet;
}

bool DeviceScaling::
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

std::string * DeviceScaling::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_SCALING_TYPE))
   {
      if (pszScalingType_d)
      {
         std::ostringstream oss;

         oss << "string " << pszScalingType_d;

         return new std::string (oss.str ());
      }
   }
   else if (0 == strcmp (pszKey, JOBPROP_SCALING_PERCENTAGE))
   {
      std::ostringstream oss;

      oss << "float " << dScalingPercentage_d;

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceScaling::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_SCALING_TYPE))
   {
      if (pszScalingType_d)
      {
         std::ostringstream oss;

         oss << pszScalingType_d;

         return new std::string (oss.str ());
      }
   }
   else if (0 == strcmp (pszKey, JOBPROP_SCALING_PERCENTAGE))
   {
      std::ostringstream oss;

      oss << dScalingPercentage_d;

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceScaling::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_SCALING_TYPE, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_SCALING_TYPE);
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
                                                    StringResource::STRINGGROUP_SCALINGS,
                                                    pszValue);

         if (pszXLateValue)
         {
            *pRet += "=";
            *pRet += pszXLateValue;
         }
      }
   }
   else if (0 == strcasecmp (JOBPROP_SCALING_PERCENTAGE, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_SCALING_PERCENTAGE);
      if (pszXLateKey)
      {
         oss << pszXLateKey;

         if (  pszValue
            && *pszValue
            )
         {
            oss << "="
                << pszValue;
         }

         return new std::string (oss.str ());
      }
   }

   return pRet;
}

std::string * DeviceScaling::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   oss << dScalingPercentage_d
       << " ";

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_SCALINGS,
                                              pszScalingType_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceScaling::
getJobProperties (bool fInDeviceSpecific)
{
   if (  fInDeviceSpecific
      && getDeviceID ()
      )
   {
      std::ostringstream oss;

      oss << "Scaling"
          << "="
          << getDeviceID ();

      return new std::string (oss.str ());
   }
   else if (pszScalingType_d)
   {
      std::ostringstream oss;
      std::ostringstream oss2;

      oss2 << JOBPROP_SCALING_TYPE << "=" << pszScalingType_d
           << " "
           << JOBPROP_SCALING_PERCENTAGE << "=" << dScalingPercentage_d;

      JobProperties::standarizeJPOrder (oss, oss2.str ());

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceScaling::
getType ()
{
   if (pszScalingType_d)
   {
      return new std::string (pszScalingType_d);
   }

   return 0;
}

BinaryData * DeviceScaling::
getData ()
{
   return pbdData_d;
}

double DeviceScaling::
getMinimumPercentage ()
{
   return dMinimumScale_d;
}

double DeviceScaling::
getMaximumPercentage ()
{
   return dMaximumScale_d;
}

PSZCRO DeviceScaling::
getDeviceID ()
{
   return 0;
}

std::string * DeviceScaling::
getScalingType ()
{
   if (pszScalingType_d)
   {
      return new std::string (pszScalingType_d);
   }

   return 0;
}

double DeviceScaling::
getScalingPercentage ()
{
   return dScalingPercentage_d;
}

int DeviceScaling::
allowedTypeIndex (PSZCRO pszAllowedType)
{
   if (  !pszAllowedType
      || !*pszAllowedType
      )
   {
      return -1;
   }

   int iLow  = 0;
   int iMid  = (int)dimof (vapszNames) / 2;
   int iHigh = (int)dimof (vapszNames) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszAllowedType, vapszNames[iMid]);

      if (0 == iResult)
      {
         return iMid;
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

   return -1;
}

#ifndef RETAIL

void DeviceScaling::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceScaling::
toString (std::ostringstream& oss)
{
   oss << "{DeviceScaling: "
       << "pszScalingType_d = " << SAFE_PRINT_PSZ (pszScalingType_d)
       << " dScalingPercentage_d = " << dScalingPercentage_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceScaling& const_self)
{
   DeviceScaling&     self = const_cast<DeviceScaling&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceScaling::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszScalingType,
               int    *pindexScaling,
               double *pdPercentage)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_SCALING_TYPE))
      {
         int indexScalingType;

         indexScalingType = allowedTypeIndex (pszValue);

         if (-1 != indexScalingType)
         {
            fRet = true;

            if (pindexScaling)
            {
               *pindexScaling = indexScalingType;
            }
            if (ppszScalingType)
            {
               *ppszScalingType = (PSZRO)malloc (strlen (pszValue) + 1);
               if (*ppszScalingType)
               {
                  strcpy ((char *)*ppszScalingType, pszValue);
               }
            }
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_SCALING_PERCENTAGE))
      {
         char   *pszEnd = 0;
         double  dValue = 0;

         errno  = 0;
         dValue = strtod (pszValue, &pszEnd);

         if (  pszEnd != pszValue
            && 0 == errno
            )
         {
            fRet = true;

            if (pdPercentage)
            {
               *pdPercentage = dValue;
            }
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

DefaultScaling::
DefaultScaling (Device     *pDevice,
                PSZRO       pszJobProperties)
   : DeviceScaling (pDevice,
                    pszJobProperties,
                    0,
                    DEFAULT_SCALING_MINIMUM,
                    DEFAULT_SCALING_MAXIMUM)
{
}

DeviceScaling * DefaultScaling::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int    indexScaling       = -1;
   double dScalingPercentage = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexScaling,
                      &dScalingPercentage))
   {
      if (  DEFAULT_SCALING_TYPE       == indexScaling
         && DEFAULT_SCALING_PERCENTAGE == dScalingPercentage
         )
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultScaling (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceScaling * DefaultScaling::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultScaling::
isSupported (PSZCRO pszJobProperties)
{
   int    indexScaling       = -1;
   double dScalingPercentage = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexScaling,
                      &dScalingPercentage))
   {
      return    DEFAULT_SCALING_TYPE       == indexScaling
             && DEFAULT_SCALING_PERCENTAGE == dScalingPercentage;
   }

   return false;
}

Enumeration * DefaultScaling::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultScalingValueEnumerator : public Enumeration
   {
   public:
      DefaultScalingValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultScalingValueEnumerator ()
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

   return new DefaultScalingValueEnumerator ();
}

void DefaultScaling::
writeDefaultJP (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss2 << JOBPROP_SCALING_TYPE
        << "="
        << vapszNames[DEFAULT_SCALING_TYPE]
        << " "
        << JOBPROP_SCALING_PERCENTAGE
        << "="
        << DEFAULT_SCALING_PERCENTAGE;

   JobProperties::standarizeJPOrder (oss, oss2.str ());
}

#ifndef RETAIL

void DefaultScaling::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultScaling::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultScaling: "
       << DeviceScaling::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultScaling& const_self)
{
   DefaultScaling&    self = const_cast<DefaultScaling&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
