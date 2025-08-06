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
#include "DeviceStitching.hpp"
#include "JobProperties.hpp"

static PSZCRO vapszJobPropertyKeys[] = {
   "StitchingPosition",
   "StitchingReferenceEdge",
   "StitchingType",
   "StitchingCount",
   "StitchingAngle"
};

#define JOBPROP_STITCHING_POSITION       vapszJobPropertyKeys[0]
#define JOBPROP_STITCHING_REFERENCE_EDGE vapszJobPropertyKeys[1]
#define JOBPROP_STITCHING_TYPE           vapszJobPropertyKeys[2]
#define JOBPROP_STITCHING_COUNT          vapszJobPropertyKeys[3]
#define JOBPROP_STITCHING_ANGLE          vapszJobPropertyKeys[4]

static PSZCRO apszStitchingReferenceEdge[] = {
   "Bottom",
   "Left",
   "Right",
   "Top"
};
static PSZCRO apszStitchingType[] = {
   "Corner",
   "Saddle",
   "Side"
};

DeviceStitching::
DeviceStitching (Device     *pDevice,
                 PSZRO       pszJobProperties,
                 BinaryData *pbdData)
{
   pDevice_d                     = pDevice;
   iStitchingPosition_d          = 0;
   pszStitchingReferenceEdge_d   = 0;
   indexStitchingReferenceEdge_d = -1;
   pszStitchingType_d            = 0;
   indexStitchingType_d          = -1;
   iStitchingCount_d             = 0;
   iStitchingAngle_d             = 0;
   pbdData_d                     = pbdData;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties,
                     &iStitchingPosition_d,
                     &pszStitchingReferenceEdge_d,
                     &indexStitchingReferenceEdge_d,
                     &pszStitchingType_d,
                     &indexStitchingType_d,
                     &iStitchingCount_d,
                     &iStitchingAngle_d);
   }
}

DeviceStitching::
~DeviceStitching ()
{
   if (pszStitchingReferenceEdge_d)
   {
      free ((void *)pszStitchingReferenceEdge_d);
   }
   if (pszStitchingType_d)
   {
      free ((void *)pszStitchingType_d);
   }

   delete pbdData_d;

   pDevice_d                     = 0;
   iStitchingPosition_d          = 0;
   pszStitchingReferenceEdge_d   = 0;
   indexStitchingReferenceEdge_d = -1;
   pszStitchingType_d            = 0;
   indexStitchingType_d          = -1;
   iStitchingCount_d             = 0;
   iStitchingAngle_d             = 0;
   pbdData_d                     = 0;
}

bool DeviceStitching::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0, 0, 0, 0, 0, 0);
}

bool DeviceStitching::
isEqual (PSZCRO pszJobProperties)
{
   int iStitchingPosition          = -1;
   int indexStitchingReferenceEdge = -1;
   int indexStitchingType          = -1;
   int iStitchingCount             = -1;
   int iStitchingAngle             = -1;

   if (getComponents (pszJobProperties,
                      &iStitchingPosition,
                      0,
                      &indexStitchingReferenceEdge,
                      0,
                      &indexStitchingType,
                      &iStitchingCount,
                      &iStitchingAngle))
   {
      return    iStitchingPosition          == iStitchingPosition_d
             && indexStitchingReferenceEdge == indexStitchingReferenceEdge_d
             && indexStitchingType          == indexStitchingType_d
             && iStitchingCount             == iStitchingCount_d
             && iStitchingAngle             == iStitchingAngle_d;
   }

   return false;
}

std::string * DeviceStitching::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DST1_"
       << iStitchingPosition_d
       << "_"
       << indexStitchingReferenceEdge_d
       << "_"
       << indexStitchingType_d
       << "_"
       << iStitchingCount_d
       << "_"
       << iStitchingAngle_d;

   return new std::string (oss.str ());
}

DeviceStitching * DeviceStitching::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceStitching *pStitchingRet               = 0;
   int              iStitchingPosition          = -1;
   int              indexStitchingReferenceEdge = -1;
   int              indexStitchingType          = -1;
   int              iStitchingCount             = -1;
   int              iStitchingAngle             = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DST1_", 5))
   {
      PSZRO pszScan = pszCreateHash + 5;

      if (0 == sscanf (pszScan, "%d", &iStitchingPosition))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &indexStitchingReferenceEdge))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &indexStitchingType))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &iStitchingCount))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &iStitchingAngle))
      {
         return 0;
      }

      if (  indexStitchingReferenceEdge <  0
         || indexStitchingReferenceEdge >= (int)dimof (apszStitchingReferenceEdge)
         || indexStitchingType          <  0
         || indexStitchingType          >= (int)dimof (apszStitchingType)
         )
      {
         return 0;
      }

      std::ostringstream oss;

      oss << JOBPROP_STITCHING_POSITION
          << "="
          << iStitchingPosition;

      oss << " "
          << JOBPROP_STITCHING_REFERENCE_EDGE
          << "="
          << apszStitchingReferenceEdge[indexStitchingReferenceEdge];

      oss << " "
          << JOBPROP_STITCHING_TYPE
          << "="
          << apszStitchingType[indexStitchingType];

      oss << " "
          << JOBPROP_STITCHING_COUNT
          << "="
          << iStitchingCount;

      oss << " "
          << JOBPROP_STITCHING_ANGLE
          << "="
          << iStitchingAngle;

      pStitchingRet = create (pDevice, oss.str ().c_str ());
   }

   return pStitchingRet;
}

bool DeviceStitching::
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

std::string * DeviceStitching::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_STITCHING_POSITION))
   {
      std::ostringstream oss;

      oss << "integer " << iStitchingPosition_d;

      return new std::string (oss.str ());
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_REFERENCE_EDGE))
   {
      if (pszStitchingReferenceEdge_d)
      {
         std::ostringstream oss;

         oss << "string " << pszStitchingReferenceEdge_d;

         return new std::string (oss.str ());
      }
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_TYPE))
   {
      if (pszStitchingType_d)
      {
         std::ostringstream oss;

         oss << "string " << pszStitchingType_d;

         return new std::string (oss.str ());
      }
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_COUNT))
   {
      std::ostringstream oss;

      oss << "integer " << iStitchingCount_d;

      return new std::string (oss.str ());
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_ANGLE))
   {
      std::ostringstream oss;

      oss << "integer " << iStitchingAngle_d;

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceStitching::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_STITCHING_POSITION))
   {
      std::ostringstream oss;

      oss << iStitchingPosition_d;

      return new std::string (oss.str ());
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_REFERENCE_EDGE))
   {
      if (pszStitchingReferenceEdge_d)
      {
         std::ostringstream oss;

         oss << pszStitchingReferenceEdge_d;

         return new std::string (oss.str ());
      }
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_TYPE))
   {
      if (pszStitchingType_d)
      {
         std::ostringstream oss;

         oss << pszStitchingType_d;

         return new std::string (oss.str ());
      }
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_COUNT))
   {
      std::ostringstream oss;

      oss << iStitchingCount_d;

      return new std::string (oss.str ());
   }
   else if (0 == strcmp (pszKey, JOBPROP_STITCHING_ANGLE))
   {
      std::ostringstream oss;

      oss << iStitchingAngle_d;

      return new std::string (oss.str ());
   }

   return 0;
}

std::string * DeviceStitching::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_STITCHING_POSITION, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_STITCHING_POSITION);
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
   else if (0 == strcasecmp (JOBPROP_STITCHING_REFERENCE_EDGE, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_STITCHING_REFERENCE_EDGE);
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
                                                    StringResource::STRINGGROUP_STITCHING_EDGES,
                                                    pszValue);

         if (pszXLateValue)
         {
            *pRet += "=";
            *pRet += pszXLateValue;
         }
      }
   }
   else if (0 == strcasecmp (JOBPROP_STITCHING_TYPE, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_STITCHING_STITCHING_TYPE);
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
                                                    StringResource::STRINGGROUP_STITCHING_TYPES,
                                                    pszValue);

         if (pszXLateValue)
         {
            *pRet += "=";
            *pRet += pszXLateValue;
         }
      }
   }
   else if (0 == strcasecmp (JOBPROP_STITCHING_COUNT, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_STITCHING_COUNT);
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
   else if (0 == strcasecmp (JOBPROP_STITCHING_ANGLE, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_STITCHING_ANGLE);
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

std::string * DeviceStitching::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   oss << iStitchingPosition_d
       << " ";

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_STITCHING_EDGES,
                                              pszStitchingReferenceEdge_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue
          << " ";
   }

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_STITCHING_TYPES,
                                              pszStitchingType_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue
          << " ";
   }

   oss << iStitchingCount_d
       << " "
       << iStitchingAngle_d;

   return new std::string (oss.str ());
}

std::string * DeviceStitching::
getJobProperties (bool fInDeviceSpecific)
{
   if (  fInDeviceSpecific
      && getDeviceID ()
      )
   {
      std::ostringstream oss;

      oss << "Stitching"
          << "="
          << getDeviceID ();

      return new std::string (oss.str ());
   }
   else if (  pszStitchingReferenceEdge_d
           && pszStitchingType_d
           )
   {
      std::ostringstream oss;
      std::ostringstream oss2;

      oss2 <<        JOBPROP_STITCHING_POSITION       << "=" << iStitchingPosition_d;
      oss2 << " " << JOBPROP_STITCHING_REFERENCE_EDGE << "=" << pszStitchingReferenceEdge_d;
      oss2 << " " << JOBPROP_STITCHING_TYPE           << "=" << pszStitchingType_d;
      oss2 << " " << JOBPROP_STITCHING_COUNT          << "=" << iStitchingCount_d;
      oss2 << " " << JOBPROP_STITCHING_ANGLE          << "=" << iStitchingAngle_d;

      JobProperties::standarizeJPOrder (oss, oss2.str ());

      return new std::string (oss.str ());
   }

   return 0;
}

BinaryData * DeviceStitching::
getData ()
{
   return pbdData_d;
}

PSZCRO DeviceStitching::
getDeviceID ()
{
   return 0;
}

int DeviceStitching::
getStitchingPosition ()
{
   return iStitchingPosition_d;
}

std::string * DeviceStitching::
getStitchingReferenceEdge ()
{
   if (pszStitchingReferenceEdge_d)
   {
      return new std::string (pszStitchingReferenceEdge_d);
   }

   return 0;
}

std::string * DeviceStitching::
getStitchingType ()
{
   if (pszStitchingType_d)
   {
      return new std::string (pszStitchingType_d);
   }

   return 0;
}

int DeviceStitching::
getStitchingCount ()
{
   return iStitchingCount_d;
}

int DeviceStitching::
getStitchingAngle ()
{
   return iStitchingAngle_d;
}

int DeviceStitching::
referenceEdgeIndex (PSZCRO pszReferenceEdge)
{
   if (  !pszReferenceEdge
      || !*pszReferenceEdge
      )
   {
      return -1;
   }

   static PSZCRO aDirections[] = {
      "Bottom",
      "Left",
      "Right",
      "Top"
   };
   int iLow  = 0;
   int iMid  = (int)dimof (aDirections) / 2;
   int iHigh = (int)dimof (aDirections) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszReferenceEdge, aDirections[iMid]);

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

int DeviceStitching::
typeIndex (PSZCRO pszType)
{
   if (  !pszType
      || !*pszType
      )
   {
      return -1;
   }

   static PSZCRO aDirections[] = {
      "Corner",
      "Saddle",
      "Side"
   };
   int iLow  = 0;
   int iMid  = (int)dimof (aDirections) / 2;
   int iHigh = (int)dimof (aDirections) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszType, aDirections[iMid]);

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

void DeviceStitching::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceStitching::
toString (std::ostringstream& oss)
{
   oss << "{DeviceStitching: "
       << "iStitchingPosition_d = " << iStitchingPosition_d
       << " pszStitchingReferenceEdge_d = " << SAFE_PRINT_PSZ (pszStitchingReferenceEdge_d)
       << " pszStitchingType_d = " << SAFE_PRINT_PSZ (pszStitchingType_d)
       << " iStitchingCount_d = " << iStitchingCount_d
       << " iStitchingAngle_d = " << iStitchingAngle_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceStitching& const_self)
{
   DeviceStitching&   self = const_cast<DeviceStitching&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceStitching::
getComponents (PSZCRO  pszJobProperties,
               int    *piStitchingPosition,
               PSZRO  *pszStitchingReferenceEdge,
               int    *pindexStitchingReferenceEdge,
               PSZRO  *pszStitchingType,
               int    *pindexStitchingType,
               int    *piStitchingCount,
               int    *piStitchingAngle)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_STITCHING_POSITION))
      {
         int iPosition = 0;

         iPosition = atoi (pszValue);

         if (piStitchingPosition)
         {
            *piStitchingPosition = iPosition;
         }

         // @TBD
         fRet = true;
      }
      else if (0 == strcmp (pszKey, JOBPROP_STITCHING_REFERENCE_EDGE))
      {
         int iLow  = 0;
         int iMid  = (int)dimof (apszStitchingReferenceEdge) / 2;
         int iHigh = (int)dimof (apszStitchingReferenceEdge) - 1;
         int iResult;

         while (iLow <= iHigh)
         {
            iResult = strcmp (pszValue, apszStitchingReferenceEdge[iMid]);

            if (0 == iResult)
            {
               fRet = true;

               if (pindexStitchingReferenceEdge)
               {
                  *pindexStitchingReferenceEdge = iMid;
               }
               if (pszStitchingReferenceEdge)
               {
                  *pszStitchingReferenceEdge = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*pszStitchingReferenceEdge)
                  {
                     strcpy ((char *)*pszStitchingReferenceEdge, pszValue);
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
      else if (0 == strcmp (pszKey, JOBPROP_STITCHING_TYPE))
      {
         int iLow  = 0;
         int iMid  = (int)dimof (apszStitchingType) / 2;
         int iHigh = (int)dimof (apszStitchingType) - 1;
         int iResult;

         while (iLow <= iHigh)
         {
            iResult = strcmp (pszValue, apszStitchingType[iMid]);

            if (0 == iResult)
            {
               fRet = true;

               if (pindexStitchingType)
               {
                  *pindexStitchingType = iMid;
               }
               if (pszStitchingType)
               {
                  *pszStitchingType = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*pszStitchingType)
                  {
                     strcpy ((char *)*pszStitchingType, pszValue);
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
      else if (0 == strcmp (pszKey, JOBPROP_STITCHING_COUNT))
      {
         int iCount = 0;

         iCount = atoi (pszValue);

         if (piStitchingCount)
         {
            *piStitchingCount = iCount;
         }

         if (iCount)
         {
            fRet = true;
         }
      }
      else if (0 == strcmp (pszKey, JOBPROP_STITCHING_ANGLE))
      {
         int iAngle = 0;

         iAngle = atoi (pszValue);

         if (piStitchingAngle)
         {
            *piStitchingAngle = iAngle;
         }

         // @TBD
         fRet = true;
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}

DefaultStitching::
DefaultStitching (Device     *pDevice,
                  PSZRO       pszJobProperties)
   : DeviceStitching (pDevice,
                      pszJobProperties,
                      0)
{
}

DeviceStitching * DefaultStitching::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int iStitchingPosition          = -1;
   int indexStitchingReferenceEdge = -1;
   int indexStitchingType          = -1;
   int iStitchingCount             = -1;
   int iStitchingAngle             = -1;

   if (getComponents (pszJobProperties,
                      &iStitchingPosition,
                      0,
                      &indexStitchingReferenceEdge,
                      0,
                      &indexStitchingType,
                      &iStitchingCount,
                      &iStitchingAngle))
   {
      if (  DEFAULT_STITCHING_POSITION             == iStitchingPosition
         && DEFAULT_STITCHING_INDEX_REFERENCE_EDGE == indexStitchingReferenceEdge
         && DEFAULT_STITCHING_INDEX_TYPE           == indexStitchingType
         && DEFAULT_STITCHING_COUNT                == iStitchingCount
         && DEFAULT_STITCHING_ANGLE                == iStitchingAngle
         )
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultStitching (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceStitching * DefaultStitching::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultStitching::
isSupported (PSZCRO pszJobProperties)
{
   int iStitchingPosition          = -1;
   int indexStitchingReferenceEdge = -1;
   int indexStitchingType          = -1;
   int iStitchingCount             = -1;
   int iStitchingAngle             = -1;

   if (getComponents (pszJobProperties,
                      &iStitchingPosition,
                      0,
                      &indexStitchingReferenceEdge,
                      0,
                      &indexStitchingType,
                      &iStitchingCount,
                      &iStitchingAngle))
   {
      return    DEFAULT_STITCHING_POSITION             == iStitchingPosition
             && DEFAULT_STITCHING_INDEX_REFERENCE_EDGE == indexStitchingReferenceEdge
             && DEFAULT_STITCHING_INDEX_TYPE           == indexStitchingType
             && DEFAULT_STITCHING_COUNT                == iStitchingCount
             && DEFAULT_STITCHING_ANGLE                == iStitchingAngle;
   }

   return false;
}

Enumeration * DefaultStitching::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultStitchingValueEnumerator : public Enumeration
   {
   public:
      DefaultStitchingValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultStitchingValueEnumerator ()
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

   return new DefaultStitchingValueEnumerator ();
}

void DefaultStitching::
writeDefaultJP (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss2 << JOBPROP_STITCHING_POSITION
        << "="
        << DEFAULT_STITCHING_POSITION
        << " "
        << JOBPROP_STITCHING_REFERENCE_EDGE
        << "="
        << apszStitchingReferenceEdge[DEFAULT_STITCHING_INDEX_REFERENCE_EDGE]
        << " "
        << JOBPROP_STITCHING_TYPE
        << "="
        << apszStitchingType[DEFAULT_STITCHING_INDEX_TYPE]
        << " "
        << JOBPROP_STITCHING_COUNT
        << "="
        << DEFAULT_STITCHING_COUNT
        << " "
        << JOBPROP_STITCHING_ANGLE
        << "="
        << DEFAULT_STITCHING_ANGLE;

   JobProperties::standarizeJPOrder (oss, oss2.str ());
}

#ifndef RETAIL

void DefaultStitching::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DefaultStitching::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{DefaultStitching: "
       << DeviceStitching::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DefaultStitching& const_self)
{
   DefaultStitching&  self = const_cast<DefaultStitching&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
