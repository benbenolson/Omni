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
#include "DeviceNUp.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "NumberUp",
   "NumberUpDirection"
};

#define JOBPROP_NUP           vapszJobPropertyKeys[0]
#define JOBPROP_NUP_DIRECTION vapszJobPropertyKeys[1]

static PSZCRO vaDirections[] = {
   "TobottomToleft",
   "TobottomToright",
   "ToleftTobottom",
   "ToleftTotop",
   "TorightTobottom",
   "TorightTotop",
   "TotopToleft",
   "TotopToright"
};

static int vaNUpValues[][2] = {
   {1, 1},
   {2, 1},
   {1, 2},
   {2, 2},
   {3, 3},
   {4, 4}
};

/* Function prototypes...
*/

DeviceNUp::
DeviceNUp (Device     *pDevice,
           PSZRO       pszJobProperties,
           BinaryData *pbdData,
           bool        fSimulationRequired)
{
   pDevice_d             = pDevice;
   iX_d                  = 0;
   iY_d                  = 0;
   pszDirection_d        = 0;
   indexDirection_d      = -1;
   pbdData_d             = pbdData;
   fSimulationRequired_d = fSimulationRequired;
   pszDeviceID_d         = 0;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      if (getComponents (pszJobProperties,
                         &iX_d,
                         &iY_d,
                         &pszDirection_d,
                         &indexDirection_d))
      {
      }
   }
}

DeviceNUp::
~DeviceNUp ()
{
   if (pszDirection_d)
   {
      free ((void *)pszDirection_d);
   }

   delete pbdData_d;

   if (pszDeviceID_d)
   {
      free ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }

   pDevice_d             = 0;
   iX_d                  = 0;
   iY_d                  = 0;
   pszDirection_d        = 0;
   indexDirection_d      = -1;
   pbdData_d             = 0;
   fSimulationRequired_d = false;
   pszDeviceID_d         = 0;
}

bool DeviceNUp::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0, 0, 0);
}

bool DeviceNUp::
isEqual (PSZCRO pszJobProperties)
{
   int iX             = -1;
   int iY             = -1;
   int indexDirection = -1;

   if (getComponents (pszJobProperties,
                      &iX,
                      &iY,
                      0,
                      &indexDirection))
   {
      return    iX             == iX_d
             && iY             == iY_d
             && indexDirection == indexDirection_d;
   }

   return false;
}

std::string * DeviceNUp::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DNU1_"
       << iX_d
       << "_"
       << iY_d
       << "_"
       << indexDirection_d;

   return new std::string (oss.str ());
}

DeviceNUp * DeviceNUp::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceNUp *pNUpRet        = 0;
   int        iX             = -1;
   int        iY             = -1;
   int        indexDirection = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DNU1_", 5))
   {
      PSZRO pszScan = pszCreateHash + 5;

      if (0 == sscanf (pszScan, "%d", &iX))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &iY))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &indexDirection))
      {
         return 0;
      }

      if (  indexDirection <  0
         || indexDirection >= (int)dimof (vaDirections)
         )
      {
         return 0;
      }

      std::ostringstream oss;

      oss << JOBPROP_NUP
          << "="
          << iX
          << "x"
          << iY;

      oss << " "
          << JOBPROP_NUP_DIRECTION
          << "="
          << vaDirections[indexDirection];

      pNUpRet = create (pDevice, oss.str ().c_str ());
   }

   return pNUpRet;
}

bool DeviceNUp::
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

std::string * DeviceNUp::
getJobPropertyType (PSZCRO pszKey)
{
   std::stringstream oss;
   std::string       stringValue;

   if (0 == strcasecmp (pszKey, JOBPROP_NUP))
   {
      oss << "string "
          << iX_d
          << "X"
          << iY_d;

      stringValue = oss.str ();
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_NUP_DIRECTION))
   {
      oss << "string "
          << pszDirection_d;

      stringValue = oss.str ();
   }

   if (stringValue.length ())
   {
      return new std::string (stringValue);
   }

   return 0;
}

std::string * DeviceNUp::
getJobProperty (PSZCRO pszKey)
{
   std::stringstream oss;
   std::string       stringValue;

   if (0 == strcasecmp (pszKey, JOBPROP_NUP))
   {
      oss << iX_d
          << "X"
          << iY_d;

      stringValue = oss.str ();
   }
   else if (0 == strcasecmp (pszKey, JOBPROP_NUP_DIRECTION))
   {
      oss << pszDirection_d;

      stringValue = oss.str ();
   }

   if (stringValue.length ())
   {
      return new std::string (stringValue);
   }

   return 0;
}

std::string * DeviceNUp::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_NUP, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_NUP);
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
   else if (0 == strcasecmp (JOBPROP_NUP_DIRECTION, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_NUP_DIRECTION);
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
                                                    StringResource::STRINGGROUP_NUPS,
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

std::string * DeviceNUp::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   oss << iX_d
       << "X"
       << iY_d
       << " ";

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_NUPS,
                                              pszDirection_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceNUp::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   if (  fInDeviceSpecific
      && getDeviceID ()
      )
   {
      oss << JOBPROP_NUP
          << "="
          << getDeviceID ();
   }
   else
   {
      std::ostringstream oss2;

      oss2 << JOBPROP_NUP
           << "="
           << iX_d
           << "X"
           << iY_d
           << " "
           << JOBPROP_NUP_DIRECTION
           << "="
           << pszDirection_d;

      JobProperties::standarizeJPOrder (oss, oss2.str ());
   }

   return new std::string (oss.str ());
}

BinaryData * DeviceNUp::
getData ()
{
   return pbdData_d;
}

bool DeviceNUp::
needsSimulation ()
{
   return fSimulationRequired_d;
}

PSZCRO DeviceNUp::
getDeviceID ()
{
   return 0;
}

int DeviceNUp::
getXPages ()
{
   return iX_d;
}

int DeviceNUp::
getYPages ()
{
   return iY_d;
}

std::string * DeviceNUp::
getDirection ()
{
   if (pszDirection_d)
   {
      return new std::string (pszDirection_d);
   }

   return 0;
}

int DeviceNUp::
directionIndex (PSZCRO pszDirection)
{
   if (  !pszDirection
      || !*pszDirection
      )
   {
      return -1;
   }

   int iLow  = 0;
   int iMid  = (int)dimof (vaDirections) / 2;
   int iHigh = (int)dimof (vaDirections) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszDirection, vaDirections[iMid]);

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

class NUpEnumerator : public Enumeration
{
public:

   NUpEnumerator ()
   {
      iNUpIndex_d          = (int)dimof (vaNUpValues);
      iNUpDirectionIndex_d = 0;
   }

   virtual ~
   NUpEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return iNUpIndex_d < (int)dimof (vaNUpValues);
   }

   virtual void *
   nextElement ()
   {
      if (!hasMoreElements ())
      {
         return false;
      }

      std::ostringstream  oss;
      std::string         stringName;
      char               *pszNUpJP   = 0;

      oss << JOBPROP_NUP << "=" << vaNUpValues[iNUpIndex_d][0] << "x" << vaNUpValues[iNUpIndex_d][1];
      oss << " " << JOBPROP_NUP_DIRECTION << vaDirections[iNUpDirectionIndex_d];

      stringName = oss.str ();

      pszNUpJP = (char *)malloc (stringName.length () + 1);
      if (pszNUpJP)
      {
         strcpy (pszNUpJP, stringName.c_str ());
      }

      iNUpDirectionIndex_d++;

      if (iNUpDirectionIndex_d >= (int)dimof (vaDirections))
      {
         iNUpIndex_d++;
         iNUpDirectionIndex_d = 0;
      }

      return pszNUpJP;
   }

private:
   int     iNUpIndex_d;
   int     iNUpDirectionIndex_d;
};

#ifndef RETAIL

void DeviceNUp::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceNUp::
toString (std::ostringstream& oss)
{
   oss << "{DeviceNUp: "
       << "iX_d = " << iX_d
       << ", iY_d = " << iY_d
       << ", pszDirection_d = " << SAFE_PRINT_PSZ (pszDirection_d)
       << ", fSimulationRequired_d = " << fSimulationRequired_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceNUp& const_self)
{
   DeviceNUp&          self = const_cast<DeviceNUp&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}

char *
validateNUp (PSZCRO pszNUp)
{
   PSZRO pszTest        = pszNUp;
   bool  fFirst         = true;
   int   indexLowerCase = -1;
   char *pszReturn      = 0;

   if (  !pszNUp
      || !*pszNUp
      )
   {
      return 0;
   }

   do
   {
      if ('x' == *pszTest)
      {
         if (fFirst)
         {
            return 0;
         }
         else
         {
            pszTest++;
            break;
         }
      }
      else if ('X' == *pszTest)
      {
         indexLowerCase = pszTest - pszNUp;

         if (fFirst)
         {
            return 0;
         }
         else
         {
            pszTest++;
            break;
         }
      }
      else if (  '0' > *pszTest
              || '9' < *pszTest
              )
      {
         return 0;
      }

      pszTest++;

      fFirst = false;

   } while (*pszTest);

   fFirst = true;
   do
   {
      if (!*pszTest)
      {
         if (fFirst)
         {
            return 0;
         }
         else
         {
            break;
         }
      }
      else if (  '0' > *pszTest
              || '9' < *pszTest
              )
      {
         return 0;
      }

      pszTest++;

      fFirst = false;

   } while (*pszTest);

   pszReturn = (char *)malloc (strlen (pszNUp) + 1);

   if (pszReturn)
   {
      strcpy (pszReturn, pszNUp);

      if (-1 != indexLowerCase)
      {
         pszReturn[indexLowerCase] = 'x';
      }
   }

   return pszReturn;
}

bool DeviceNUp::
getComponents (PSZCRO  pszJobProperties,
               int    *piX,
               int    *piY,
               PSZRO  *ppszNUpDirection,
               int    *pindexDirection)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_NUP))
      {
         char *pszNUpName = 0;

         pszNUpName = validateNUp (pszValue);

         if (pszNUpName)
         {
            char *pszTimes = 0;

            pszTimes = strchr (pszNUpName, 'x');
            if (pszTimes)
            {
               *pszTimes = '\0';
            }

            if (pszTimes)
            {
               int iX = 0;
               int iY = 0;

               iX = atoi (pszNUpName);
               iY = atoi (pszTimes + 1);

               if (piX)
               {
                  *piX = iX;
               }
               if (piY)
               {
                  *piY = iY;
               }

               if (  iX
                  && iY
                  )
               {
                  fRet = true;
               }
            }

            free ((void *)pszNUpName);
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_NUP_DIRECTION))
      {
         int indexDirection = -1;

         indexDirection = directionIndex (pszValue);

         if (-1 != indexDirection)
         {
            fRet = true;

            if (pindexDirection)
            {
               *pindexDirection = indexDirection;
            }
            if (ppszNUpDirection)
            {
               *ppszNUpDirection = (PSZRO)malloc (strlen (pszValue) + 1);
               if (*ppszNUpDirection)
               {
                  strcpy ((char *)*ppszNUpDirection, pszValue);
               }
            }
            break;
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

DefaultNUp::
DefaultNUp (Device     *pDevice,
            PSZRO       pszJobProperties)
   : DeviceNUp (pDevice,
                pszJobProperties,
                0,
                DEFAULT_SIMULATION_REQUIRED)
{
}

DeviceNUp * DefaultNUp::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int iX             = -1;
   int iY             = -1;
   int indexDirection = -1;

   if (getComponents (pszJobProperties,
                      &iX,
                      &iY,
                      0,
                      &indexDirection))
   {
      if (  DEFAULT_X               == iX
         && DEFAULT_Y               == iY
         && DEFAULT_INDEX_DIRECTION == indexDirection
         )
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultNUp (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceNUp * DefaultNUp::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultNUp::
isSupported (PSZCRO pszJobProperties)
{
   int iX             = -1;
   int iY             = -1;
   int indexDirection = -1;

   if (getComponents (pszJobProperties,
                      &iX,
                      &iY,
                      0,
                      &indexDirection))
   {
      return    DEFAULT_X               == iX
             && DEFAULT_Y               == iY
             && DEFAULT_INDEX_DIRECTION == indexDirection;
   }

   return false;
}

Enumeration * DefaultNUp::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultNUpValueEnumerator : public Enumeration
   {
   public:
      DefaultNUpValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultNUpValueEnumerator ()
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

   return new DefaultNUpValueEnumerator ();
}

void DefaultNUp::
writeDefaultJP (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss2 << JOBPROP_NUP
        << "="
        << DEFAULT_X
        << "X"
        << DEFAULT_Y
        << " "
        << JOBPROP_NUP_DIRECTION
        << "="
        << vaDirections[DEFAULT_INDEX_DIRECTION];

   JobProperties::standarizeJPOrder (oss, oss2.str ());
}

#ifndef RETAIL

void DefaultNUp::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultNUp::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultNUp: "
       << DeviceNUp::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultNUp& const_self)
{
   DefaultNUp&        self = const_cast<DefaultNUp&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
